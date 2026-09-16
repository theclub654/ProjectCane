#include "ruby.h"
#include "boss.h"
#include "game.h"
#include "missile.h"
#include "rythm.h"
#include "save.h"
#include "shd.h"
#include "sm.h"
#include "stepzap.h"
#include "rwm.h"
#include "xform.h"
#include <cmath>
#include <cstring>

namespace
{
constexpr OID s_aoidRubyAseg[4] = {(OID)0x4df, (OID)0x4e0, (OID)0x4e1, (OID)0x4e2};
SNIP s_asnipPostRubyLoad[5] = {
    {1, (OID)0x4eb, offsetof(RUBY, psmRuby)},
    {0, (OID)0x4e7, offsetof(RUBY, aprwmAttack[0])},
    {0, (OID)0x4e8, offsetof(RUBY, aprwmAttack[1])},
    {0, (OID)0x4e9, offsetof(RUBY, aprwmAttack[2])},
    {0, (OID)0x4ea, offsetof(RUBY, aprwmAttack[3])},
};
constexpr struct { OID oid; float time; } s_aeventRuby[9] = {
    {(OID)0x35, 50.0f}, {(OID)0x36, 50.0f}, {(OID)0x38, 80.0f},
    {(OID)0x3b, 80.0f}, {(OID)0x60, 70.0f}, {(OID)0x50, 220.0f},
    {(OID)0x52, 80.0f}, {(OID)0x53, 80.0f}, {(OID)0x54, 80.0f},
};
RUBYMUSICSTATE s_rubymusicstate = RUBYMUSICSTATE_Idle;
int s_iRubyMusicSectionPending = 0;
}

RUBY* NewRuby()
{ 
    return new RUBY{}; 
}

void InitRuby(RUBY* pruby)
{
    InitStepGuard(pruby);
    pruby->iMeasure = 0;
    pruby->cQuarterBeat = -1;
    pruby->iQuarterBeat = -1;

    pruby->framesPerBeat = 60;
    pruby->beatsPerMeasure = 8;
    pruby->dtBeatTarget = 1.0f;
    pruby->dtBeat = pruby->dtBeatTarget;
    pruby->measureDuration = 8.0f;
    pruby->uBeat = 0.0f;
    pruby->tBeatStart = g_clock.t;
    pruby->tBeatNext = g_clock.t + pruby->dtBeat;

    pruby->aRythmEvent.clear();
    pruby->aRythmEvent.reserve(1024 / sizeof(RUBYRYTHMEVENT));

    pruby->psw->fRubyMusicActive = 1;
    KillMusic();

    s_rubymusicstate = RUBYMUSICSTATE_RestartRhythm;
    s_iRubyMusicSectionPending = 0;
}

void LoadRubyFromBrx(RUBY* pruby, CBinaryInputStream* pbis)
{
    LoadStepGuardFromBrx(pruby, pbis);

    // Retail resolves these immediately after LoadStepguardFromBrx.  The
    // lookup also strips/snips each special ASEG, so doing it in PostRubyLoad
    // is too late: PostStepguardLoad has already consumed the hierarchy.
    for (int iAseg = 0; iAseg < 4; ++iAseg)
        pruby->apasegAttack[iAseg] = PasegFindStepguard(pruby, s_aoidRubyAseg[iAseg]);
}

void CloneRuby(RUBY* pruby, RUBY* prubyBase) 
{ 
    CloneStepguard(pruby, prubyBase); 
}

void PostRubyLoad(RUBY* pruby)
{
    PostStepguardLoad(pruby);

    SetSwGameplayHud(pruby->psw, pruby);
    SnipAloObjects(pruby, 5, s_asnipPostRubyLoad);
    pruby->psmaRuby = PsmaApplySm(pruby->psmRuby, nullptr, OID_Nil, 0);
    SubscribeSmaStruct(pruby->psmaRuby, pruby);
    for (int iEvent = 0; iEvent < 9; ++iEvent) {
        pruby->aevent[iEvent].oid = s_aeventRuby[iEvent].oid;
        pruby->aevent[iEvent].time = s_aeventRuby[iEvent].time;
        pruby->aevent[iEvent].plo = PloFindSwObject(pruby->psw, 1, s_aeventRuby[iEvent].oid, pruby);
    }
}

void OnRubyRemove(RUBY* pruby) 
{
    OnPoRemove(pruby); 
}

void OnRubyAdd(RUBY* pruby) 
{
    OnPoAdd(pruby); 
}

int FCanRubyAttack(RUBY* pruby)
{
    if (!pruby->psmaRuby) return 0;
    int oidState = OID_Nil;
    GetSmaCur(pruby->psmaRuby, (OID*)&oidState);
    switch (oidState) 
    {
        case 1261: case 1264: case 1267: case 1270:
        return pruby->pasegAttackOverride != nullptr;
        case 1262: case 1265: case 1268: case 1271:
        if (pruby->attackLatch == 0) pruby->attackLatch = 1;
        return pruby->attackLatch == 2;
        default:
        return 0;
    }
}

int FDetectRuby(RUBY* pruby) 
{
    return 1;
}

void DrawRubyHaloFire(RPL* prpl)
{
    if (!prpl || !prpl->palo)
        return;

    RUBY* pruby = static_cast<RUBY*>(prpl->palo);

    static SPKD spkdFire = []
    {
        SPKD spkd{};
        spkd.spkk = SPKK_Mirror;
        spkd.uRepeat = 10.0f;
        spkd.svu = 0.25f;

        spkd.aspkr[0].clq = {0.0f, 0.0f, 0.0f};
        spkd.aspkr[0].dz = 100.0f;
        spkd.aspkr[0].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);

        spkd.aspkr[1].clq = {0.0f, 0.5f, 0.0f};
        spkd.aspkr[1].dz = 50.0f;
        spkd.aspkr[1].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);

        spkd.aspkr[2].clq = {80.0f, 1.0f, 0.0f};
        spkd.aspkr[2].v = 1.0f;
        spkd.aspkr[2].dz = -100.0f;
        spkd.aspkr[2].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);
        return spkd;
    }();

    static constexpr OID s_aoidHalo[7] = {
        (OID)0x35, (OID)0x36, (OID)0x37, (OID)0x3a,
        (OID)0x50, (OID)0x58, (OID)0x170
    };
    static constexpr float s_asRadius[7] = {30.0f, 30.0f, 30.0f, 30.0f, 60.0f, 30.0f, 30.0f};

    SPKS aspks[7]{};
    int cspks = 0;
    for (int i = 0; i < 7; ++i) {
        ALO* palo = static_cast<ALO*>(PloFindSwObject(pruby->psw, 1, s_aoidHalo[i], pruby));
        if (!palo)
            continue;
        aspks[cspks++] = {s_aoidHalo[i], s_asRadius[i], palo};
    }

    SHD* pshdFire = PshdFindShader((OID)349);
    spkdFire.apshd[0] = pshdFire;
    spkdFire.apshd[1] = pshdFire;
    DrawHaloSpks(&pruby->xf.posWorld, cspks, aspks, &spkdFire, prpl);
}

void RenderRubyAll(RUBY* pruby, CM* pcm, RO* pro)
{
    static RUBY* s_prubyLast = nullptr;
    static int s_sgsLast = -999;
    static int s_hiddenLast = -1;
    static int s_inWorldLast = -1;
    static ALO* s_paloChildLast = nullptr;

    const int inWorld = FIsLoInWorld(pruby);
    ALO* paloChild = pruby->dlChild.paloFirst;
    if (pruby != s_prubyLast || static_cast<int>(pruby->sgs) != s_sgsLast ||
        pruby->fHidden != s_hiddenLast || inWorld != s_inWorldLast ||
        paloChild != s_paloChildLast) {

        int iChild = 0;
        for (ALO* child = paloChild; child != nullptr && iChild < 32;
             child = child->dleChild.paloNext, ++iChild) {
        }
        s_prubyLast = pruby;
        s_sgsLast = static_cast<int>(pruby->sgs);
        s_hiddenLast = pruby->fHidden;
        s_inWorldLast = inWorld;
        s_paloChildLast = paloChild;
    }

    RO ro{};

    if (pruby->sgs == SGS_Stun) 
    {
        DupAloRo(pruby, pro, &ro);
        ro.uAlphaCelBorder = 0.0f;
        pro = &ro;
    }

    RenderAloAll(pruby, pcm, pro);
}

void RenderRubySelf(RUBY* pruby, CM* pcm, RO* pro)
{
    RenderStepguardSelf(pruby, pcm, pro);
    if (pruby->sgs != SGS_Stun || !pcm)
        return;

    RPL rpl{};
    rpl.PFNDRAWRPL = DrawRubyHaloFire;
    rpl.rp = RP_Translucent;
    rpl.palo = pruby;
    rpl.ro.model = glm::mat4(1.0f);
    rpl.ro.uAlpha = 1.0f;
    rpl.ro.darken = 1.0f;
    rpl.ro.warpType = WARP_NONE;
    rpl.z = glm::dot(pruby->xf.posWorld - pcm->pos, pcm->mat[0]);

    g_translucentAddPrpl[g_translucentAddCount] = rpl;
    g_translucentAddPrpl[g_translucentAddCount].PFNDRAWRPL = DrawRubyHaloFire;
    ++g_translucentAddCount;
}

void OnRubyExitingSgs(RUBY* pruby, SGS sgsNext)
{
    OnStepguardExitingSgs(pruby, sgsNext);

    if (pruby->sgs == SGS_Attack) 
    { 
        pruby->attackLatch = 0; 
        return; 
    }
    if (pruby->sgs != SGS_Stun) 
        return;

    pruby->attackLatch = 0;
    int oidState = OID_Nil;
    GetSmaCur(pruby->psmaRuby, (OID*)&oidState);

    if (oidState == 1262 || oidState == 1265 || oidState == 1268)
        SetSmaGoal(pruby->psmaRuby, static_cast<OID>(oidState + 1));
}

void DamageRubyBoss(RUBY* pruby)
{
    DecrementBossHealth(&g_boss);
    pruby->cStun = 2;
    g_boss.uHealthDisplay = (int)g_boss.uHealthTarget;
}

void OnRubyEnteringSgs(RUBY* pruby, SGS sgsPrev, ASEG* pasegOverride)
{
    if (!pruby)
        return;


    if (pruby->sgs == SGS_Attack && pruby->psmaRuby) {
        int oidState = OID_Nil;
        GetSmaCur(pruby->psmaRuby, (OID*)&oidState);
        if (oidState == 1261 || oidState == 1264 || oidState == 1267 || oidState == 1270) {
            pasegOverride = pruby->pasegAttackOverride;
            pruby->pasegAttackOverride = nullptr;
        }
    }
    if (pruby->sgs == SGS_Dying) {
        DecrementBossHealth(&g_boss);
        SetWorldComplete(GAMEWORLD_Voodoo);
        AutosaveCurrentGame(&g_saveData);
    }
    else if (pruby->sgs == SGS_Stun) {
        DecrementBossHealth(&g_boss);
    }

    OnStepguardEnteringSgs(pruby, sgsPrev, pasegOverride);


    switch (pruby->sgs) {
    case SGS_PatrolIdle:
    case SGS_PursueIdle:
        if (pruby->psmaRuby && pruby->pasegaSgs && pruby->pasegaSgs->paseg) {
            int oidState = OID_Nil;
            GetSmaCur(pruby->psmaRuby, (OID*)&oidState);

            const int cBeats = oidState >= 1260 && oidState < 1263 ? 2 : 4;
            AddRubyRythmEvent(pruby, pruby->pasegaSgs, cBeats);
            pruby->pasegaSgs->tLocal =
                pruby->pasegaSgs->paseg->tMax * (pruby->uBeat / static_cast<float>(cBeats));

        }
        break;

    case SGS_Dying:
    case SGS_Stun:
        pruby->fIncapacitated = 0;
        break;

    case SGS_Attack:
        if (pruby->psmaRuby && pruby->pasegaSgs && pruby->pasegaSgs->paseg) {
            int oidState = OID_Nil;
            GetSmaCur(pruby->psmaRuby, (OID*)&oidState);

            float dtAttack = 0.0f;
            if (oidState == 1262) {
                dtAttack = pruby->dtBeat * 2.0f;
            }
            else if (oidState == 1265 || oidState == 1268 || oidState == 1271) {
                dtAttack = pruby->dtBeat * 4.0f;
            }

            if (dtAttack > 0.0001f) {
                pruby->pasegaSgs->svtLocal = pruby->pasegaSgs->paseg->tMax / dtAttack;
                pruby->pasegaSgs->tLocal = 1.0f / 12.0f;
            }
        }
        break;

    default:
        break;
    }
}

void AddRubyRythmMatch(RUBY* pruby, const RYTHMCOMMAND* prythmCommand)
{
    const int iWeapon = prythmCommand->iWeapon;

    pruby->iAttackWeapon = iWeapon;
    pruby->pasegAttackOverride = pruby->apasegAttack[iWeapon];
    pruby->tAttackEvent = g_clock.t;

    RYTHMMATCH match;
    match.button = iWeapon;
    match.time = static_cast<float>(prythmCommand->iMeasure) * 4.0f + prythmCommand->tBeat + static_cast<float>(pruby->beatsPerMeasure);

    AddRythmMatch(pruby->prythm, &match);
}

void HandleRubyMessage(RUBY* pruby, MSGID msgid, void* pv)
{
    if (!pruby) {
        return;
    }

    if (msgid == MSGID_label && pv) {
        const ASEGLABELMSG* pmsg = static_cast<const ASEGLABELMSG*>(pv);

        // Label 421 is the release point in Ruby's attack animation. Retail
        // launches the weapon selected by the rhythm pattern at this point.
        if (pmsg->pasega == pruby->pasegaSgs && pmsg->label == 421 &&
            pruby->prythm && pruby->iAttackWeapon >= 0 && pruby->iAttackWeapon < 4) {
            RWM* prwm = pruby->aprwmAttack[pruby->iAttackWeapon];
            XFM* pxfmButton = pruby->prythm->pxfmButton[pruby->iAttackWeapon];

            if (prwm && pxfmButton) {
                ReloadRwm(prwm);
                SO* psoAmmo = prwm->psoAmmo;

                if (psoAmmo) {
                    const float dtPredict =
                        (pruby->measureDuration + pruby->tAttackEvent - g_clock.t) +
                        pruby->prythm->dtInputWindow * pruby->dtBeat * 0.5f;

                    if (dtPredict > 0.0001f) {
                        glm::vec3 posRythm;
                        glm::mat3 matRythm;
                        PredictAloTransform(pxfmButton->paloParent, nullptr, dtPredict, &posRythm, &matRythm, nullptr, nullptr);

                        // Retail reads XFM::posLocal at raw +0x40 and transforms it
                        // through the predicted transform of the XFM's ALO parent.
                        prwm->rwti.pos = posRythm + matRythm * pxfmButton->posLocal;

                        glm::vec3 posFire;
                        glm::mat3 matFire;
                        GetRwfiPosMat(&prwm->rwfi, &posFire, &matFire, nullptr);

                        const float svLaunch = glm::length(prwm->rwti.pos - posFire) / dtPredict;
                        if (FIsBasicDerivedFrom(psoAmmo, CID_MISSILE)) {
                            MISSILE* pmissile = static_cast<MISSILE*>(psoAmmo);
                            pmissile->svMissileLaunch = svLaunch;

                            if (FIsBasicDerivedFrom(psoAmmo, CID_ACCMISS)) {
                                static_cast<ACCMISS*>(psoAmmo)->svMissileMax = svLaunch;
                            }
                        }

                        prwm->rwfi.tMax = dtPredict * 1.25f;
                        FFireRwm(prwm, true);
                        return;
                    }
                }
            }
        }
    }
    else if (msgid == MSGID_asega_retracted) {
        RemoveRubyRythmEvent(pruby, static_cast<ASEGA*>(pv));
    }
    else if (msgid == MSGID_sma_transition && pv == pruby->psmaRuby) {
        int oidState = OID_Nil;
        GetSmaCur(pruby->psmaRuby, (OID*)&oidState);

        int iMusicSection = -1;
        switch (oidState) {
        case 1260: iMusicSection = 0; break;
        case 1261: iMusicSection = 1; break;
        case 1262: iMusicSection = 2; break;
        case 1263: iMusicSection = 3; break;
        case 1264: iMusicSection = 4; break;
        case 1265:
        case 1266: iMusicSection = 5; break;
        case 1267: iMusicSection = 6; break;
        case 1268:
        case 1269: iMusicSection = 7; break;
        case 1270: iMusicSection = 8; break;
        case 1271: iMusicSection = 9; break;
        case 1272: iMusicSection = 10; break;
        default: break;
        }

        if (iMusicSection >= 0 && s_amRegisters[0] != iMusicSection) {
            s_rubymusicstate = RUBYMUSICSTATE_WaitingForMusic;
            s_iRubyMusicSectionPending = iMusicSection;
            SetAMRegister(0, static_cast<byte>(iMusicSection));
        }
    }
    HandleStepguardMessage(pruby, msgid, pv);
}

void SyncRubyRythmEvents(RUBY* pruby)
{
    const float dtUntilNextBeat = pruby->tBeatNext - g_clock.t;

    for (RUBYRYTHMEVENT& event : pruby->aRythmEvent)
    {
        ASEGA* pasega = event.pasega;
        const int cBeats = event.cBeats;

        if (!pasega || !pasega->paseg || cBeats <= 0) {
            continue;
        }

        const float tAnimationMax = pasega->paseg->tMax;

        float tTarget = (tAnimationMax / static_cast<float>(cBeats)) *
            static_cast<float>((pruby->cQuarterBeat + 1) % cBeats);

        // Select the next occurrence when this point has already been reached.
        if (tTarget <= pasega->tLocal)
            tTarget += tAnimationMax;

        pasega->svtLocal = (tTarget - pasega->tLocal) / dtUntilNextBeat;

    }
}

void UpdateRuby(RUBY* pruby, float dt)
{
    UpdateStepguard(pruby, dt);

    if (s_rubymusicstate == RUBYMUSICSTATE_RestartRhythm) {
        if (pruby->dtBeat != pruby->dtBeatTarget) {
            pruby->dtBeat = pruby->dtBeatTarget;
            pruby->measureDuration =
                pruby->dtBeat * static_cast<float>(pruby->beatsPerMeasure);
        }

        pruby->tBeatStart = g_clock.t;
        pruby->tBeatNext = g_clock.t + pruby->dtBeat;

        pruby->iMeasure = 0;
        pruby->cQuarterBeat = 0;
        pruby->iQuarterBeat = 0;

        if (pruby->prythmSequencePending) {
            RYTHMSEQUENCE* prythmSequence = pruby->prythmSequencePending;

            pruby->prythmSequencePending = nullptr;
            pruby->prythmSequenceActive = prythmSequence;
            ActivateRythmSequence(prythmSequence);
        }

        s_rubymusicstate = RUBYMUSICSTATE_Idle;
    }

    if (g_clock.t >= pruby->tBeatNext) {
        pruby->tBeatStart = pruby->tBeatNext;

        ++pruby->cQuarterBeat;
        ++pruby->iQuarterBeat;

        if (pruby->iQuarterBeat > 3) {
            pruby->iQuarterBeat = 0;
            ++pruby->iMeasure;

            if (pruby->dtBeat != pruby->dtBeatTarget) {
                pruby->dtBeat = pruby->dtBeatTarget;
                pruby->measureDuration =
                    pruby->dtBeat *
                    static_cast<float>(pruby->beatsPerMeasure);
            }
        }

        if (pruby->attackLatch == 1 && pruby->psmaRuby) {
            int oidState = OID_Nil;
            GetSmaCur(pruby->psmaRuby, (OID*)&oidState);

            if (oidState == 1262 ||
                oidState == 1265 ||
                oidState == 1268 ||
                oidState == 1271) {
                pruby->attackLatch = 2;
            }
        }

        pruby->tBeatNext += pruby->dtBeat;
    }

    pruby->uBeat = static_cast<float>(pruby->iQuarterBeat) + (g_clock.t - pruby->tBeatStart) / pruby->dtBeat;

    if (pruby->prythmSequenceActive)
        UpdateRythmCommands(pruby->prythmSequenceActive);

    if (s_rubymusicstate == RUBYMUSICSTATE_WaitingForMusic) {
        RefreshMidiRegister(1);
        if (s_amRegisters[1] == s_iRubyMusicSectionPending) {
            s_rubymusicstate = RUBYMUSICSTATE_RestartRhythm;
        }
    }

    PostSwCallback(pruby->psw, reinterpret_cast<PFNMQ>(SyncRubyRythmEvents), pruby, MSGID_callback, nullptr);
}

void UpdateRubyBlot(RUBY* pruby)
{
    int oidState = OID_Nil;
    if (pruby->psmaRuby) GetSmaCur(pruby->psmaRuby, (OID*)&oidState);
    const bool fCombatBeat = oidState == 1262 || oidState == 1265 || oidState == 1268 || oidState == 1271;
    if (fCombatBeat && FCanShowGameplayBlot()) ShowBlot(&g_boss);
    else HideBlot(&g_boss);
}

void UpdateRubyGoal(RUBY* pruby, int fEnter)
{
    if (pruby->sgs > SGS_Dying && pruby->sgs < SGS_Attack) {
        glm::vec3 posGoal = pruby->xf.posWorld;
        if (pruby->pxfmGoal)
            GetXfmPos(pruby->pxfmGoal, &posGoal);
        SetStepguardGoal(pruby, &posGoal);
        return;
    }
    UpdateStepguardGoal(pruby, fEnter);
}

void UpdateRubySgs(RUBY* pruby)
{
    int oidState = OID_Nil;
    if (pruby->psmaRuby) GetSmaCur(pruby->psmaRuby, (OID*)&oidState);

    const bool fCanUseCustomHeading =
        pruby->sgs >= SGS_PursueIdle && pruby->sgs <= SGS_Attack;
    glm::vec3 vecDirection{};

    switch (oidState) 
    {
        case 1260: 
        case 1263: 
        case 1266:
        case 1269:
        if (!fCanUseCustomHeading || !pruby->pxfmGoal) {
            UpdateStepguardSgs(pruby);
            return;
        }
        {
            glm::mat3 matGoal;
            GetXfmMat(pruby->pxfmGoal, &matGoal);
            vecDirection = matGoal[0];
        }
        break;

        case 1261: 
        case 1264: 
        case 1267: 
        case 1270:
        if (PO* ppoPlayer = PpoCur()) vecDirection = ppoPlayer->xf.posWorld - pruby->xf.posWorld;
        else { UpdateStepguardSgs(pruby); return; }
        break;

        case 1262: 
        case 1265: 
        case 1268: 
        case 1271:
        if (!fCanUseCustomHeading) {
            UpdateStepguardSgs(pruby);
            return;
        }
        if (PO* ppoPlayer = PpoCur()) vecDirection = pruby->xf.posWorld - ppoPlayer->xf.posWorld;
        else { UpdateStepguardSgs(pruby); return; }
            break;

        default:
        UpdateStepguardSgs(pruby);
        return;
    }

    SetStepguardTargetHeading(pruby, atan2f(vecDirection.y, vecDirection.x), 0.0f);
}

void SetRubyPendingRythm(RUBY* pruby, RYTHMSEQUENCE* prythmSequence)
{
    pruby->prythmSequencePending = prythmSequence;

    // The commandless sequence is the level-start bootstrap. Its activation
    // splice enables the initial pillar interaction, but it is not a combat
    // sequence and must not remain installed as Ruby's active sequence.
    if (prythmSequence && prythmSequence->aCommand.empty()) {
        pruby->prythmSequencePending = nullptr;
        ActivateRythmSequence(prythmSequence);
    }
}

void ClearRubyRythmSequence(RUBY* pruby)
{
    DeactivateRythmSequence(pruby->prythmSequenceActive);
    pruby->prythmSequenceActive = nullptr;
}

void SetRubyFramesPerBeat(RUBY* pruby, int framesPerBeat)
{
    pruby->framesPerBeat = framesPerBeat;
    pruby->dtBeatTarget = 60.0 / (float)framesPerBeat;
}

void SetRubyBeatsPerMeasure(RUBY* pruby, int beatsPerMeasure)
{
    pruby->beatsPerMeasure = beatsPerMeasure;
    pruby->measureDuration = pruby->dtBeat * (float)beatsPerMeasure;
}

void AddRubyRythmEvent(RUBY* pruby, ASEGA* pasega, int cBeats)
{
    if (!pruby || !pasega || cBeats <= 0)
        return;

    constexpr size_t cRythmEventMax = 1024 / sizeof(RUBYRYTHMEVENT);
    if (pruby->aRythmEvent.size() >= cRythmEventMax)
        return;

    pruby->aRythmEvent.push_back({pasega, cBeats});
    SubscribeAsegaObject(pasega, pruby);
}

void RemoveRubyRythmEvent(RUBY* pruby, ASEGA* pasega)
{
    if (!pruby)
        return;
    for (auto it = pruby->aRythmEvent.begin(); it != pruby->aRythmEvent.end(); ++it) {
        if (it->pasega != pasega)
            continue;
        pruby->aRythmEvent.erase(it);
        return;
    }
}

int FCheckRubyXpBase(RUBY* pruby, XP* pxp, int ixpd) 
{ 
    return 1; 
}

int GetRubySize() 
{ 
    return sizeof(RUBY); 
}

void DeleteRuby(RUBY* pruby)
{
    delete pruby;
}

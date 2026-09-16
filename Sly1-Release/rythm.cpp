#include "rythm.h"
#include "ruby.h"
#include "game.h"
#include "jt.h"
#include "sm.h"
#include "sw.h"
#include "xform.h"
#include <cmath>
#include <intrin.h>

namespace {
constexpr float kInputWindowEasy = 0.5f;
constexpr float kInputWindowHard = 0.8f;
constexpr float kDifficultyCurveConstant = 0.0f;
constexpr float kDifficultyCurveLinear = 1.0f;
constexpr float kDifficultyCurveQuadratic = 0.0f;
constexpr float kThrowBack = 700.0f;
constexpr float kThrowUp = 650.0f;
SNIP s_asnipLoadRythm[] = {{0, (OID)0x4c9, offsetof(RYTHM, psm)}};
SNIP s_asnipPostRythmLoad[] = {
    {4, (OID)0x4d0, offsetof(RYTHM, pxfmButton[0])},
    {4, (OID)0x4d1, offsetof(RYTHM, pxfmButton[1])},
    {4, (OID)0x4d2, offsetof(RYTHM, pxfmButton[2])},
    {4, (OID)0x4d3, offsetof(RYTHM, pxfmButton[3])},
};

}

RYTHM* NewRythm() 
{ 
    return new RYTHM{}; 
}

void InitRythm(RYTHM* prythm)
{
    InitPo(prythm);
    const float uSuck = g_plsCur->uSuck;
    const float uDifficulty = glm::clamp(
        kDifficultyCurveConstant + uSuck *
        (kDifficultyCurveLinear + uSuck * kDifficultyCurveQuadratic),
        0.0f, 1.0f);
    prythm->dtInputWindow =
        (1.0f - uDifficulty) * kInputWindowEasy + uDifficulty * kInputWindowHard;
}

void LoadRythmFromBrx(RYTHM* prythm, CBinaryInputStream* pbis)
{
    LoadSoFromBrx(prythm, pbis);
    SnipAloObjects(prythm, 1, s_asnipLoadRythm);
}

void CloneRythm(RYTHM* prythm, RYTHM* prythmBase)
{
    ClonePo(prythm, prythmBase);

    prythm->psm = prythmBase->psm;
    prythm->psma = prythmBase->psma;
    for (int iButton = 0; iButton < 4; ++iButton)
        prythm->pxfmButton[iButton] = prythmBase->pxfmButton[iButton];
    prythm->pjsgFinish = prythmBase->pjsgFinish;
    prythm->iMatch = prythmBase->iMatch;
    prythm->cMatch = prythmBase->cMatch;
    for (int iMatch = 0; iMatch < 8; ++iMatch)
        prythm->aMatch[iMatch] = prythmBase->aMatch[iMatch];
    prythm->tInput = prythmBase->tInput;
    prythm->buttonInput = prythmBase->buttonInput;
    prythm->pxfmStart = prythmBase->pxfmStart;
    prythm->fInputEnabled = prythmBase->fInputEnabled;
    prythm->dtInputWindow = prythmBase->dtInputWindow;
    prythm->fInvulnerable = prythmBase->fInvulnerable;
    prythm->pruby = prythmBase->pruby;
}

void PostRythmCallback(RYTHM* prythm)
{
    if (!g_pjt) return;
    prythm->pzi = g_pjt->pzi;
    prythm->paloUseCharm = g_pjt->paloUseCharm;
}

void PostRythmLoad(RYTHM* prythm)
{
    PostAloLoad(prythm);
    SnipAloObjects(prythm, 4, s_asnipPostRythmLoad);
    prythm->psma = PsmaApplySm(prythm->psm, prythm, (OID)1226, 1);
    PostSwCallback(prythm->psw, reinterpret_cast<PFNMQ>(PostRythmCallback), prythm, MSGID_callback, nullptr);
}

void OnRythmActive(RYTHM* prythm, int fActive, PO* ppoOther)
{
    OnPoActive(prythm, fActive, ppoOther);
    SetSmaGoal(prythm->psma, fActive ? (OID)1227 : (OID)1226);
    if (!fActive) { if (g_pjt) g_pjt->dtCharmFlash = 2.0f; return; }

    // Each rhythm phase installs a different world-owned attachment marker.
    // The first phase uses the turtle marker (1567), while the second uses the
    // fire-pole marker (1691).  Resolve again on every activation and select
    // the marker nearest Sly, matching the phase he just entered.
    if (g_pjt) {
        static constexpr OID s_aoidStartMarker[] = {(OID)1567, (OID)1691};
        XFM* pxfmBest = nullptr;
        float sBestSq = 0.0f;

        for (OID oidStart : s_aoidStartMarker) {
            LO* ploStart = PloFindSwObject(prythm->psw, 4, oidStart, nullptr);
            if (!ploStart || !FIsBasicDerivedFrom(ploStart, CID_XFM))
                continue;

            XFM* pxfmStart = static_cast<XFM*>(ploStart);
            glm::vec3 posStart;
            GetXfmPos(pxfmStart, &posStart);
            const glm::vec3 dpos = posStart - g_pjt->xf.posWorld;
            const float sSq = glm::dot(dpos, dpos);
            if (!pxfmBest || sSq < sBestSq) {
                pxfmBest = pxfmStart;
                sBestSq = sSq;
            }
        }

        if (pxfmBest)
            prythm->pxfmStart = pxfmBest;
    }

    prythm->pruby = reinterpret_cast<RUBY*>(PloFindSwObjectByClass(prythm->psw, 5, CID_RUBY, nullptr));
    if (prythm->pruby) prythm->pruby->prythm = prythm;
}

void ApplyRythmThrow(RYTHM* prythm, JT* pjt)
{
    glm::vec3 velocity = prythm->xf.matWorld[0] * -kThrowBack + prythm->xf.matWorld[2] * kThrowUp;
    pjt->pvtso->pfnSetSoVelocityVec(pjt, &velocity);
    pjt->radTarget = RadNormalize(atan2f(velocity.y, velocity.x) + 3.1415927f);
    FixStepAngularVelocity(pjt);
    SetJtJts(pjt, 4, 43);
}

void UpdateRythm(RYTHM* prythm, JOY* pjoy, float dt)
{
    if (prythm->pxfmStart && g_pjt) {
        glm::vec3 posStart;
        glm::mat3 matStart;
        GetXfmPos(prythm->pxfmStart, &posStart);
        GetXfmMat(prythm->pxfmStart, &matStart);
        g_pjt->pvtso->pfnTranslateSoToPos(g_pjt, &posStart);
        g_pjt->pvtso->pfnRotateSoToMat(g_pjt, matStart);
        prythm->pvtso->pfnTranslateSoToPos(prythm, &posStart);
        prythm->pvtso->pfnRotateSoToMat(prythm, matStart);
    }

    (void)dt;

    if (!prythm->pruby || !pjoy)
        return;

    const float tRythm =
        static_cast<float>(prythm->pruby->iMeasure) * 4.0f + prythm->pruby->uBeat;

    if (prythm->fInputEnabled) {
        int iButton = -1;

        // Retail button order: Square, Triangle, Circle, Cross.
        if (pjoy->IsPressed(BTN_SQUARE)) {
            iButton = 0;
            pjoy->SetHandled(BTN_SQUARE);
        }
        else if (pjoy->IsPressed(BTN_TRIANGLE)) {
            iButton = 1;
            pjoy->SetHandled(BTN_TRIANGLE);
        }
        else if (pjoy->IsPressed(BTN_CIRCLE)) {
            iButton = 2;
            pjoy->SetHandled(BTN_CIRCLE);
        }
        else if (pjoy->IsPressed(BTN_CROSS)) {
            iButton = 3;
            pjoy->SetHandled(BTN_CROSS);
        }

        if (iButton >= 0) {
            bool fAccepted = false;
            if (prythm->iMatch < prythm->cMatch) {
                const RYTHMMATCH& rythmMatch = prythm->aMatch[prythm->iMatch];
                fAccepted = rythmMatch.button == iButton &&
                    std::abs(rythmMatch.time - tRythm) < prythm->dtInputWindow * 0.5f;
            }

            if (fAccepted) {
                prythm->buttonInput = iButton;
                prythm->tInput = tRythm;
            }
            else {
                StartSound((SFXID)123, nullptr, nullptr, nullptr,
                    3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            }
        }
    }

    if (prythm->iMatch >= prythm->cMatch)
        return;

    const RYTHMMATCH& rythmMatch = prythm->aMatch[prythm->iMatch];
    const float dtInputHalf = prythm->dtInputWindow * 0.5f;
    const bool fAccepted = prythm->buttonInput == rythmMatch.button &&
        std::abs(rythmMatch.time - prythm->tInput) < dtInputHalf;

    if (!fAccepted && tRythm - rythmMatch.time <= dtInputHalf)
        return;

    if (fAccepted) {
        static constexpr int s_aoidRythmState[4] = {0x4cc, 0x4ce, 0x4cd, 0x4cf};
        static constexpr int s_asfxidRythmSuccess[4] = {574, 576, 575, 577};

        if (prythm->psma && prythm->buttonInput >= 0 && prythm->buttonInput < 4) {
            SetSmaGoal(prythm->psma, (OID)s_aoidRythmState[prythm->buttonInput]);
            StartSound((SFXID)s_asfxidRythmSuccess[prythm->buttonInput], nullptr, nullptr, nullptr, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, nullptr, nullptr);
        }
    }
    else {
        StartSound((SFXID)81, nullptr, nullptr, nullptr, 30000.0f, 20000.0f, 1.5f, 0.3f, 0.0f, nullptr, nullptr);

        if (!FCharmAvailable()) {
            const int fInvulnerable = prythm->pvtrythm->pfnFIsRythmInvulnerable(prythm);
            if (!fInvulnerable && g_pjt) {
                SwitchToPo(g_pjt);
                ZPR zpr{};
                InitZpr(&zpr, (ZPK)0, prythm);
                zpr.pfnzap = reinterpret_cast<PFNZAP>(ApplyRythmThrow);
                g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zpr);
            }

            if (!fInvulnerable) {
                ClearRubyRythmSequence(prythm->pruby);
                prythm->pruby->tTaunt = g_clock.t;
                SetAMRegister(2, 1);
            }
        }
        else {
            const int fInvulnerable = prythm->pvtrythm->pfnFIsRythmInvulnerable(prythm);
            if (!fInvulnerable) {
                prythm->pvtrythm->pfnUsePoCharm(prythm);
                prythm->fInvulnerable = 1;

                if (g_pjt) {
                    g_pjt->fFlash = 1;
                    g_pjt->tFlashPulse = g_clock.t;
                    g_pjt->tCharm = g_clock.t;
                    g_pjt->dtCharmFlash = 2.0f;

                    RYTHMSEQUENCE* sequence = prythm->pruby->prythmSequenceActive;
                    if (sequence) {
                        for (int iCommand = sequence->iCommand;
                            iCommand < static_cast<int>(sequence->aCommand.size());
                            ++iCommand) {
                            const RYTHMCOMMAND& command = sequence->aCommand[iCommand];
                            if (command.cmd != RYTHMCOMMAND_DisableInput)
                                continue;

                            const float tDisable =
                                static_cast<float>(command.iMeasure + sequence->iMeasureBase) * 4.0f +
                                command.tBeat;
                            const float tCurrent =
                                static_cast<float>(prythm->pruby->iMeasure) * 4.0f +
                                prythm->pruby->uBeat;
                            g_pjt->dtCharmFlash = (tDisable - tCurrent) * prythm->pruby->dtBeat;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (rythmMatch.button == 3 && g_pjt) {
        SwitchToPo(g_pjt);
        if (prythm->pjsgFinish) {
            ApplyJsg(prythm->pjsgFinish, g_pjt);
            prythm->pjsgFinish = nullptr;
        }
    }

    ++prythm->iMatch;
    if (prythm->iMatch == prythm->cMatch) {
        prythm->iMatch = 0;
        prythm->cMatch = 0;
    }
}

bool FIsRythmInvulnerable(RYTHM* prythm) 
{ 
    return (g_grfcht & 1U) || FSwHandsOff(prythm->psw) || prythm->fInvulnerable; 
}

int FTakeRythmDamage(RYTHM* prythm, ZPR* pzpr) 
{ 
    return 0; 
}

void AddRythmMatch(RYTHM* prythm, const RYTHMMATCH* pmatch) 
{
    const int iMatch = prythm->cMatch++;
    prythm->aMatch[iMatch] = *pmatch;
}

void PredictRythmPosition(RYTHM* prythm, float dt, glm::vec3* ppos, glm::vec3* pv)
{
    XFM* pxfmStart = prythm->pxfmStart;
    if (!pxfmStart || !pxfmStart->paloParent) {
        PredictAloPosition(prythm, dt, ppos, pv);
        return;
    }

    glm::vec3 pos;
    glm::vec3 v;
    glm::vec3 w;
    glm::mat3 mat;
    // The marker belongs to the moving turtle path, not to RYTHM.  Predict
    // that owner's world transform and then apply the marker's local offset.
    // Walking from RYTHM toward this unrelated object can never reach it and
    // eventually dereferences a null parent in PredictAloTransform.
    PredictAloTransform(pxfmStart->paloParent, nullptr, dt, &pos, &mat, &v, &w);
    glm::vec3 dpos = mat * pxfmStart->posLocal;
    if (ppos) *ppos = pos + dpos;
    if (pv) *pv = v + glm::cross(w, dpos);
}

void PredictRythmRotation(RYTHM* prythm, float dt, glm::mat3* pmat, glm::vec3* pw)
{
    XFM* pxfmStart = prythm->pxfmStart;
    if (!pxfmStart || !pxfmStart->paloParent) {
        PredictAloRotation(prythm, dt, pmat, pw);
        return;
    }

    glm::mat3 mat;
    glm::vec3 w;
    PredictAloTransform(pxfmStart->paloParent, nullptr, dt, nullptr, &mat, nullptr, &w);
    if (pmat) *pmat = mat * pxfmStart->matLocal;
    if (pw) *pw = w;
}

int GetRythmSize() 
{ 
    return sizeof(RYTHM); 
}

void DeleteRythm(RYTHM* prythm) 
{ 
    delete prythm; 
}

RYTHMSEQUENCE* NewRythmSequence()
{
    return new RYTHMSEQUENCE{};
}

void InitRythmSequence(RYTHMSEQUENCE* prythmSequence)
{
    InitLo(prythmSequence);
    prythmSequence->aCommand.clear();
    prythmSequence->aCommand.reserve(192);
    prythmSequence->iCommand = 0;
    prythmSequence->pruby = nullptr;
    prythmSequence->iMeasureBase = 0;
}

void CloneRythmSequence(RYTHMSEQUENCE* prythmSequence, RYTHMSEQUENCE* prythmSequenceBase)
{
    CloneLo(prythmSequence, prythmSequenceBase);
    prythmSequence->aCommand = prythmSequenceBase->aCommand;
    prythmSequence->iCommand = prythmSequenceBase->iCommand;
    prythmSequence->pruby = prythmSequenceBase->pruby;
    prythmSequence->iMeasureBase = prythmSequenceBase->iMeasureBase;

}

void PostRythmSequenceLoad(RYTHMSEQUENCE* prythmSequence)
{
    PostLoLoad(prythmSequence);
    prythmSequence->pruby = static_cast<RUBY*>(PloFindSwObjectByClass(prythmSequence->psw, 5, CID_RUBY, nullptr));
}

void ActivateRythmSequence(RYTHMSEQUENCE* prythmSequence)
{
    if (!prythmSequence)
        return;

    prythmSequence->iCommand = 0;
    prythmSequence->iMeasureBase = prythmSequence->pruby ? prythmSequence->pruby->iMeasure : 0;
    HandleLoSpliceEvent(prythmSequence, 26, 0, nullptr);
}

void DeactivateRythmSequence(RYTHMSEQUENCE* prythmSequence)
{
    if (!prythmSequence)
        return;

    HandleLoSpliceEvent(prythmSequence, 28, 0, nullptr);

    const RYTHMCOMMAND disableEffect = {RYTHMCOMMAND_DisableEffect, 0, 0.0f, 0};
    DispatchRythmCommand(prythmSequence, &disableEffect);

    if (prythmSequence->pruby && prythmSequence->pruby->prythm) {
        const RYTHMCOMMAND disableInput = {
            RYTHMCOMMAND_DisableInput, 0, 0.0f, 0};
        DispatchRythmCommand(prythmSequence, &disableInput);
    }
}

void UpdateRythmCommands(RYTHMSEQUENCE* prythmSequence)
{
    if (!prythmSequence || !prythmSequence->pruby)
        return;

    RUBY* pruby = prythmSequence->pruby;
    while (prythmSequence->iCommand >= 0 && prythmSequence->iCommand < static_cast<int>(prythmSequence->aCommand.size())) 
    {
        const RYTHMCOMMAND& command = prythmSequence->aCommand[prythmSequence->iCommand];
        const int iMeasure = command.iMeasure + prythmSequence->iMeasureBase;

        if (pruby->iMeasure < iMeasure || (pruby->iMeasure == iMeasure && pruby->uBeat < command.tBeat)) {
            return;
        }

        DispatchRythmCommand(prythmSequence, &command);
        ++prythmSequence->iCommand;
    }
}

void DispatchRythmCommand(RYTHMSEQUENCE* prythmSequence, const RYTHMCOMMAND* prythmCommand)
{
    if (!prythmSequence || !prythmCommand || !prythmSequence->pruby)
        return;

    RUBY* pruby = prythmSequence->pruby;
    RYTHM* prythm = pruby->prythm;

    switch (prythmCommand->cmd) {
    case RYTHMCOMMAND_AddMatch:
        AddRubyRythmMatch(pruby, prythmCommand);
        break;

    case RYTHMCOMMAND_SpliceEvent:
    {
        void* apvArgs[] = {const_cast<int*>(&prythmCommand->iWeapon)};
        HandleLoSpliceEvent(prythmSequence, 27, 1, apvArgs);
        break;
    }

    case RYTHMCOMMAND_EnableInput:
        if (prythm) {
            prythm->fInputEnabled = 1;
            g_jticon.pvtblot->pfnSetBlotBlots(&g_jticon, BLOTS_Visible);
            g_joy.StartJoySelection();
            StartSound((SFXID)0, nullptr, nullptr, nullptr, 80000.0f, 70000.0f, 1.5f, 0.5f, 0.0f, nullptr, nullptr);
        }
        break;

    case RYTHMCOMMAND_DisableInput:
        if (prythm) {
            prythm->fInputEnabled = 0;
            prythm->fInvulnerable = 0;
            g_jticon.pvtblot->pfnSetBlotBlots(&g_jticon, BLOTS_Hidden);
        }
        break;

    case RYTHMCOMMAND_EnableEffect:
        pruby->fRythmEffect = 1;
        g_rubyicon.pvtblot->pfnSetBlotBlots(&g_rubyicon, BLOTS_Visible);
        StartSound((SFXID)0, nullptr, nullptr, nullptr, 80000.0f, 70000.0f, 1.5f, -0.25f, 0.0f, nullptr, nullptr);
        break;

    case RYTHMCOMMAND_DisableEffect:
        pruby->fRythmEffect = 0;
        g_rubyicon.pvtblot->pfnSetBlotBlots(&g_rubyicon, BLOTS_Hidden);
        break;
    }
}

RYTHMCOMMAND* AddRythmSequenceCommand(RYTHMSEQUENCE* prythmSequence, RYTHMCOMMANDKIND commandKind, int iMeasure, float tBeat)
{
    if (!prythmSequence || prythmSequence->aCommand.size() >= 192)
        return nullptr;

    prythmSequence->aCommand.push_back({commandKind, iMeasure, tBeat, 0});
    return &prythmSequence->aCommand.back();
}

void AddRythmSequenceMatch(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat, int iWeapon)
{
    if (RYTHMCOMMAND* prythmCommand = AddRythmSequenceCommand(
        prythmSequence, RYTHMCOMMAND_AddMatch, iMeasure, tBeat)) {
        prythmCommand->iWeapon = iWeapon;
    }
}

void AddRythmSequenceSpliceEvent(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat, int iWeapon)
{
    if (RYTHMCOMMAND* prythmCommand = AddRythmSequenceCommand(prythmSequence, RYTHMCOMMAND_SpliceEvent, iMeasure, tBeat)) {
        prythmCommand->iWeapon = iWeapon;
    }
}

void AddRythmSequenceEnableInput(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat)
{
    AddRythmSequenceCommand(prythmSequence, RYTHMCOMMAND_EnableInput, iMeasure, tBeat);
}

void AddRythmSequenceDisableInput(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat)
{
    AddRythmSequenceCommand(prythmSequence, RYTHMCOMMAND_DisableInput, iMeasure, tBeat);
}

void AddRythmSequenceEnableEffect(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat)
{
    AddRythmSequenceCommand(prythmSequence, RYTHMCOMMAND_EnableEffect, iMeasure, tBeat);
}

void AddRythmSequenceDisableEffect(RYTHMSEQUENCE* prythmSequence, int iMeasure, float tBeat)
{
    AddRythmSequenceCommand(prythmSequence, RYTHMCOMMAND_DisableEffect, iMeasure, tBeat);
}

int GetRythmSequenceSize()
{
    return sizeof(RYTHMSEQUENCE);
}

void DeleteRythmSequence(RYTHMSEQUENCE* prythmSequence)
{
    delete prythmSequence;
}


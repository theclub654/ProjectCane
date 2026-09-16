#include "mb.h"
#include <intrin.h>
#include "steprun.h"
#include "sm.h"
#include "jt.h"
#include "acp.h"
#include "rwm.h"
#include "acpbl.h"
#include "expl.h"
#include "boss.h"
#include "asega.h"
#include "actseg.h"
#include "stepzap.h"
#include "bone.h"
#include "pnt.h"
#include "xform.h"
#include "acs.h"

MBG* NewMbg()
{
	return new MBG{};
}

void InitMbg(MBG *pmbg)
{
	InitStepGuard(pmbg);
    pmbg->oidSma = OID_Nil;
}

int GetMbgSize()
{
	return sizeof(MBG);
}

void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis)
{
	LoadStepGuardFromBrx(pmbg, pbis);
    SnipAloObjects(pmbg, 2, s_asnipLoadMbg);
}

void CloneMbg(MBG* pmbg, MBG* pmbgBase)
{
    CloneStepguard(pmbg, pmbgBase);
    pmbg->fAbandonExternal = pmbgBase->fAbandonExternal;
    for (int i = 0; i < 6; ++i)
        pmbg->ablrun[i] = pmbgBase->ablrun[i];
}

void PostMbgLoad(MBG* pmbg)
{
    PostStepguardLoad(pmbg);
    SetSwGameplayHud(pmbg->psw, pmbg);
    SnipAloObjects(pmbg, 3, s_asnipPostMbgLoad);
    PostStepLoadRun(pmbg, pmbg->ablrun, &pmbg->pasegblRun);

    pmbg->mpsgssga[12].paseg = (ASEG*)pmbg->pasegblRun;
    pmbg->psmaMbg = PsmaApplySm(pmbg->psm, nullptr, (OID)1080, 0);

    for (int i = 0; i < 3; i++)
        pmbg->mpmbakpaseg[i] = PasegFindStepguard(pmbg, s_mpmbakoidAseg[i]);

    pmbg->clBone = 6;
    pmbg->ambglbone.assign(std::begin(s_ambgLboneMbg), std::end(s_ambgLboneMbg));
    pmbg->clBone = (int)pmbg->ambglbone.size();

    for (MBGLBONE& mbgLbone : pmbg->ambglbone)
        mbgLbone.plbone = (LBONE*)PloFindSwObject(pmbg->psw, 1, mbgLbone.oidLbone, pmbg);
}

void DrawMbgHaloFire(RPL* prpl)
{
    if (prpl == nullptr || prpl->palo == nullptr)
        return;

    MBG* pmbg = static_cast<MBG*>(prpl->palo);

    static SPKD spkdFire = []
    {
        SPKD spkd{};
        spkd.spkk = SPKK_Mirror;
        spkd.uRepeat = 5.0f;
        spkd.svu = 0.5f;

        spkd.aspkr[0].clq = { 0.0f, 0.0f, 0.0f };
        spkd.aspkr[0].v = 0.0f;
        spkd.aspkr[0].dz = 100.0f;
        spkd.aspkr[0].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);

        spkd.aspkr[1].clq = { 0.0f, 0.5f, 0.0f };
        spkd.aspkr[1].v = 0.0f;
        spkd.aspkr[1].dz = 50.0f;
        spkd.aspkr[1].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);

        spkd.aspkr[2].clq = { 80.0f, 1.0f, 0.0f };
        spkd.aspkr[2].v = 1.0f;
        spkd.aspkr[2].dz = -100.0f;
        spkd.aspkr[2].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);
        return spkd;
    }();

    SPKS aspks[6]{};
    int cspks = 0;

    const int clBone = std::min(pmbg->clBone, static_cast<int>(pmbg->ambglbone.size()));
    for (int i = 0; i < clBone && cspks < 6; ++i)
    {
        const MBGLBONE& mbgLbone = pmbg->ambglbone[i];
        if (mbgLbone.plbone == nullptr)
            continue;

        aspks[cspks].oid = mbgLbone.oidLbone;
        aspks[cspks].sRadius = mbgLbone.g;
        aspks[cspks].palo = mbgLbone.plbone;
        ++cspks;
    }

    SHD* pshdFireRim = PshdFindShader((OID)349);
    spkdFire.apshd[0] = pshdFireRim;
    spkdFire.apshd[1] = pshdFireRim;

    DrawHaloSpks(&pmbg->xf.posWorld, cspks, aspks, &spkdFire, prpl);
}

void RenderMbgSelf(MBG* pmbg, CM* pcm, RO* pro)
{
    RenderStepguardSelf(pmbg, pcm, pro);

    if (pmbg->fRenderBurnEffect != 0)
    {
        RPL rpl{};
        rpl.PFNDRAWRPL = DrawMbgHaloFire;
        rpl.rp = RP_Translucent;
        rpl.palo = pmbg;
        rpl.ro.model = glm::mat4(1.0f);
        rpl.ro.uAlpha = 1.0f;
        rpl.ro.darken = 1.0f;
        rpl.ro.warpType = WARP_NONE;

        const glm::vec3 dpos = pmbg->xf.posWorld - pcm->pos;
        rpl.z = glm::dot(dpos, pcm->mat[0]);

        g_translucentAddPrpl[g_translucentAddCount] = rpl;
        g_translucentAddPrpl[g_translucentAddCount].PFNDRAWRPL = DrawMbgHaloFire;
        g_translucentAddCount++;
    }
}

SO* PsoHitTestCylinderObjects(GRFHTL grfhtl, glm::vec3* ppos0, glm::vec3* ppos1, float sRadius, std::vector<SO*>& apso, LSG* plsgBest)
{
    SO* psoBest = nullptr;
    float uBest = FLT_MAX;

    for (SO* pso : apso) {
        LSG alsg[2];

        int clsg = ClsgClipCylinderToObject(pso, ppos0, ppos1, sRadius, 2, alsg);

        if (clsg <= 0)
            continue;

        int ilsg = 0;

        if (alsg[0].au[0] == 0.0f) {
            if (!(grfhtl & 1))
                continue;

            ilsg = 1;
        }

        if (ilsg >= clsg)
            continue;

        float uHit = alsg[ilsg].au[0];

        if (uHit < uBest) {
            uBest = uHit;
            psoBest = pso;

            if (plsgBest != nullptr)
                *plsgBest = alsg[ilsg];
        }
    }

    return psoBest;
}

int FFilterMbgAttackObjects(MBG* pmbg, SO* psoOther)
{
    for (int i = 0; i < 5; i++) {
        if (FIsBasicDerivedFrom(psoOther->paloRoot, s_acidReject[i]))
            return 0;
    }

    if (psoOther->paloRoot->pvtbasic->cid == CID_SO)
        return 0;

    if (FIsBasicDerivedFrom(psoOther->paloRoot, CID_BARRIER)) {
        if (!psoOther->paloRoot->pvtmbg->pfnFIgnoreStepguardIntersection((STEPGUARD*)psoOther->paloRoot, pmbg))
            return 0;
    }

    if (FMatchesLoName(psoOther->paloRoot, (OID)1097))
        return 0;

    if (FMatchesLoName(psoOther->paloRoot, (OID)1098))
        return 0;

    return 1;
}

int FObstructedMbg(MBG* pmbg, glm::vec3* ppos0, glm::vec3* ppos1)
{
    LSG lsg;
    std::vector <SO*> apso;
    IntersectSwBoundingBox(pmbg->psw, nullptr, ppos0, ppos1, (PFNFILTER)FFilterMbgAttackObjects, pmbg, apso);

    SO* psoHit = PsoHitTestCylinderObjects(0, ppos0, ppos1, S_MbgVisCylinder, apso, &lsg);

    return psoHit != nullptr;
}

int FCanMbgAttack(MBG* pmbg) 
{
    if (g_clock.t < pmbg->tAttackNext) {
        return false;
    }

    ALO* paloTarget = (ALO*)pmbg->pvtstepguard->pfnPsoEnemyStepguard(pmbg);
    if (!paloTarget) {
        return false;
    }

    if (paloTarget->pvtpo->grfcid & 0x8) {
        if (paloTarget->pvtpo->pfnJthsCurrentPo((PO*)paloTarget)) {
            return false;
        }
    }

    OID goal[4]{};
    GetSmaCur(pmbg->psmaMbg, goal);

    if (goal[0] == 1086) {
        return true;
    }

    if (goal[0] != 1082 && goal[0] != 1084) {
        return false;
    }

    glm::vec3* pposMbg = &pmbg->xf.posWorld;

    if (FObstructedMbg(pmbg, pposMbg, &paloTarget->xf.posWorld)) {
        glm::vec3 posEnemyPredict{};
        PredictAloTransform(paloTarget, nullptr, DT_PredictMbg, &posEnemyPredict, nullptr, nullptr, nullptr);

        if (FObstructedMbg(pmbg, pposMbg, &posEnemyPredict)) {
            return false;
        }
    }

    return true;
}

int FDetectMbg(MBG* pmbg)
{
    if (FSwHandsOff(pmbg->psw))
        return 0;

    if (pmbg->fAbandonExternal)
        return 0;

    if (pmbg->sgs == SGS_Stun)
        return 0;

    if (pmbg->sgs == SGS_Dying)
        return 0;

    STEP* pstepEnemy = (STEP*)pmbg->pvtstepguard->pfnPsoEnemyStepguard(pmbg);

    if (pstepEnemy == nullptr)
        return 0;

    OID oidGoal = OID_Nil;
    GetSmaCur(pmbg->psmaMbg, &oidGoal);

    return oidGoal == (OID)1082 || oidGoal == (OID)1084 || oidGoal == (OID)1086;
}

void OnMbgEnteringSgs(MBG* pmbg, SGS sgsPrev, ASEG* pasegOverride)
{
    OnStepguardEnteringSgs(pmbg, sgsPrev, pasegOverride);

    if (pmbg->sgs == SGS_Dying)
    {
        SetWorldComplete(GAMEWORLD_Muggshot);
        AutosaveCurrentGame(&g_saveData);
    }
    else if (pmbg->sgs == SGS_Stun)
    {
        pmbg->fIncapacitated = 0;
        pmbg->cStun++;
    }
}

SGS SgsNextMbgAi(MBG* pmbg)
{
    if (pmbg->sgs == SGS_ReturnToPatrol) 
    {
        OID oidCurrent = OID_Nil;
        GetSmaCur(pmbg->psmaMbg, &oidCurrent);

        if (oidCurrent == (OID)1080 && SggsGetStepguard(pmbg) == SGGS_Patrol)
            return SGS_Patrol;
    }

    return SgsNextStepguardAI(pmbg);
}

void UpdateMbgAi(MBG* pmbg)
{
    DecrementBossHealth(&g_boss);
    int iPhase = pmbg->iAPhase;

    SetStepguardSgs(pmbg, s_mbgAiSgs[iPhase], pmbg->mpmbakpaseg[iPhase]);

    pmbg->oidSma = (OID)s_mbgAiSmaOid[iPhase];
    pmbg->iAPhase++;
}

void UpdateMbg(MBG* pmbg, float dt)
{
    UpdateStepguard(pmbg, dt);
}

void UpdateMbgBossCounter(MBG* pmbg)
{
    OID oidState;

    GetSmaCur(pmbg->psmaMbg, &oidState);

    bool fBossCounterState = false;

    switch (oidState)
    {
        case static_cast<OID>(1082):
        case static_cast<OID>(1084):
        case static_cast<OID>(1086):
        fBossCounterState = true;
        break;
    }

    if (fBossCounterState && FCanShowGameplayBlot())
        ShowBlot(&g_boss);
    else
        HideBlot(&g_boss);
}

void UpdateMbgGoal(MBG* pmbg, int enter) 
{
    if (pmbg->sgs != SGS_ReturnToPatrol)
    {
        UpdateStepguardGoal(pmbg, enter);
        return;
    }

    if (pmbg->pxfm)
    {
        glm::vec3 goal{};
        GetXfmPos(pmbg->pxfm, &goal);
        SetStepguardGoal(pmbg, &goal);
        return;
    }

    int smaCurrent[4]{};
    GetSmaCur(pmbg->psmaMbg, (OID*)smaCurrent);

    if (smaCurrent[0] > 1079 && smaCurrent[0] < 1082)
        SetStepguardGoal(pmbg, &pmbg->xf.posWorld);
    else 
        SetStepguardGoal(pmbg, &g_pjt->xf.posWorld);
}

void UpdateMbgSgs(MBG* pmbg)
{
    UpdateStepguardSgs(pmbg);

    if (pmbg->sgs == SGS_Pursue)
    {
        if (pmbg->pasegaSgs == nullptr)
            return;

        if (g_pjt != nullptr && g_pjt->fStun != 0)
            return;

        ALO* paloTarget = pmbg->pvtmbg->pfnPsoEnemyStepguard(pmbg);

        float runBlend = 1.0f;

        if (paloTarget != nullptr)
        {
            const glm::vec3 dposTarget = pmbg->xf.posWorld - paloTarget->xf.posWorld;
            const float distance = glm::length(dposTarget);

            const float nearSpeed = glm::clamp(kMbgPursueNearSpeedBase + distance * (kMbgPursueNearSpeedLinear + distance * kMbgPursueNearSpeedQuad), 0.0f, 1.0f);
            const float farSpeed = glm::clamp(kMbgPursueFarSpeedBase + distance * (kMbgPursueFarSpeedLinear + distance * kMbgPursueFarSpeedQuad), 0.0f, 1.0f);

            float attackBlend = 0.0f;
            GetMbgAttackBlend(pmbg, &attackBlend);

            runBlend = glm::mix(nearSpeed, farSpeed, attackBlend);
        }

        const float radCurrent = std::atan2(pmbg->xf.matWorld[0].y, pmbg->xf.matWorld[0].x);
        const float radDelta = RadNormalize(radCurrent - pmbg->radTarget);
        const float radDeltaAbs = std::abs(radDelta);
        const float turnBlend = glm::clamp(kMbgTurnBlendBase + radDeltaAbs * (kMbgTurnBlendLinear + radDeltaAbs * kMbgTurnBlendQuad), 0.0f, 1.0f);

        SetStepRunTarget(static_cast<STEP*>(pmbg), pmbg->radTarget, runBlend * turnBlend, pmbg->ablrun, pmbg->pasegblRun);

        int imrsgLeft = -1;
        int imrsgRight = -1;

        // PostStepLoadRun creates these two marker groups explicitly from
        // mpfootsmrsgLeft and mpfootsmrsgRight. Read them directly instead of
        // recovering them through a blended object channel: Muggshot walks on
        // his hands, and the right-hand channel can legitimately differ from
        // the regular STEP foot-channel layout.
        if (!pmbg->pasegblRun->amrsgc.empty())
        {
            const MRSGC& mrsgcLeft = pmbg->pasegblRun->amrsgc[0];

            if (mrsgcLeft.cmrsg > 0 && !mrsgcLeft.amrsg.empty())
                imrsgLeft = ImrsgLookUp(pmbg->pasegaSgs->tLocal, pmbg->pasegblRun->tMax, mrsgcLeft.cmrsg, mrsgcLeft.amrsg.data(), nullptr);
        }

        if (pmbg->pasegblRun->amrsgc.size() > 1)
        {
            const MRSGC& mrsgcRight = pmbg->pasegblRun->amrsgc[1];

            if (mrsgcRight.cmrsg > 0 && !mrsgcRight.amrsg.empty())
                imrsgRight = ImrsgLookUp(pmbg->pasegaSgs->tLocal, pmbg->pasegblRun->tMax, mrsgcRight.cmrsg, mrsgcRight.amrsg.data(), nullptr);
        }

        if (imrsgLeft == 0 && !pmbg->fLeftFootAirborne)
        {
            JoltCm(g_pcm);
            StartSound(static_cast<SFXID>(460), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        }

        if (imrsgRight == 0)
        {
            if (!pmbg->fRightFootAirborne)
            {
                JoltCm(g_pcm);
                StartSound(static_cast<SFXID>(460), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            }

            pmbg->fRightFootAirborne = true;
        }
        else
        {
            pmbg->fRightFootAirborne = false;
        }

        pmbg->fLeftFootAirborne = imrsgLeft == 0;
        return;
    }

    if (pmbg->sgs == SGS_Attack && !FStepguardLookLocked(pmbg))
    {
        float attackBlend = 0.0f;
        GetMbgAttackBlend(pmbg, &attackBlend);

        const float dtAttackMax = glm::mix(0.0f, DT_PredictMbg, attackBlend);

        ALO* paloTarget = pmbg->pvtmbg->pfnPsoEnemyStepguard(pmbg);

        if (paloTarget == nullptr)
            return;

        const glm::vec3 dposTarget = paloTarget->xf.posWorld - pmbg->xf.posWorld;
        const float distance = glm::length(dposTarget);
        const float dtDistance = glm::clamp(distance * (1.0f / 6000.0f), 0.0f, 1.0f);
        const float dtPredict = glm::min(dtDistance, dtAttackMax);

        PredictAloTransform(paloTarget, nullptr, dtPredict, &pmbg->posPredictedTarget, nullptr, nullptr, nullptr);

        const glm::vec3 dposPredicted = pmbg->posPredictedTarget - pmbg->xf.posWorld;
        const float radTarget = std::atan2(dposPredicted.y, dposPredicted.x);

        SetStepguardTargetHeading(static_cast<STEPGUARD*>(pmbg), radTarget, 0);
    }
}

void OnMbgExitingSgs(MBG* pmbg, SGS sgsNext)
{
    OnStepguardExitingSgs(pmbg, sgsNext);
    if ((pmbg->sgs == SGS_Stun) && (pmbg->oidSma != -1)) 
    {
        SetSmaGoal(pmbg->psmaMbg, (OID)pmbg->oidSma);
        pmbg->oidSma = OID_Nil;
    }
}

void HandleMbgMessage(MBG* pmbg, MSGID msgid, void* pv) 
{
    if (msgid != MSGID_label || pv == nullptr) {
        HandleStepguardMessage(pmbg, msgid, pv);
        return;
    }

    const ASEGLABELMSG* pmsg = static_cast<const ASEGLABELMSG*>(pv);
    ASEGA* pasega = pmsg->pasega;
    int label = pmsg->label;

    if (pasega != pmbg->pasegaSgs) {
        HandleStepguardMessage(pmbg, msgid, pv);
        return;
    }

    switch (label) 
    {
        case 1092:
        pmbg->fRenderBurnEffect = true;
        HandleStepguardMessage(pmbg, msgid, pv);
        return;

        case 1093:
        pmbg->fRenderBurnEffect = false;
        HandleStepguardMessage(pmbg, msgid, pv);
        return;

        case 1090:
        case 1091:
        break;

        default:
        HandleStepguardMessage(pmbg, msgid, pv);
        return;
    }

    RWM* prwm = label == 1090 ? pmbg->prwmLeftFire : pmbg->prwmRightFire;

    OID goal[4]{};
    GetSmaCur(pmbg->psmaMbg, goal);

    if (goal[0] == 1082 || goal[0] == 1084) {
        prwm->rwti.pos = pmbg->posPredictedTarget;
        prwm->rwti.pos.z = pmbg->xf.posWorld.z + kMbgFireFlatZOffset;
    }
    else if (goal[0] == 1086) {
        prwm->rwti.pos = pmbg->posPredictedTarget;
        prwm->rwti.pos.z += kMbgFirePredictZOffset;
    }

    FFireRwm(prwm, true);
}

void GetMbgAttackBlend(MBG* pmbg, float* pgBlend)
{
    int goal[4]{};
    GetSmaCur(pmbg->psmaMbg, (OID*)goal);

    float uSuck = g_plsCur->uSuck;

    if (goal[0] == 1082 || goal[0] == 1084) 
    {
        *pgBlend = glm::clamp(uSuck * 1.3f - 0.3f, 0.0f, 1.0f);
        return;
    }

    *pgBlend = uSuck * uSuck;
}

void DeleteMbg(MBG* pmbg)
{
	delete pmbg;
}

BHG* NewBhg()
{
	return new BHG{};
}

void InitBhg(BHG *pbhg)
{
	InitStepGuard(pbhg);
}

int GetBhgSize()
{
	return sizeof(BHG);
}

void CloneBhg(BHG* pbhg, BHG* pbhgBase)
{
    CloneStepguard(pbhg, pbhgBase);

    // Clone scalar members
    pbhg->fDetect = pbhgBase->fDetect;
}

void PostBhgLoad(BHG* pbhg)
{
    PostStepguardLoad(pbhg);
    SetSwGameplayHud(pbhg->psw, pbhg);
}

int FDetectBhg(BHG* bhg)
{
    if (FSwHandsOff(bhg->psw))
        return 0;

    return bhg->fDetect;
}

void UpdateBhgGoal(BHG* pbhg, int fEnter)
{
    UpdateStepguardGoal(pbhg, fEnter);

    if (pbhg->sgs == SGS_Stun) {
        STEP* pstepEnemy = (STEP*)pbhg->pvtstepguard->pfnPsoEnemyStepguard(pbhg);

        if (pstepEnemy != nullptr)
            SetStepguardGoal(pbhg, &pstepEnemy->xf.posWorld);
    }
}

void UpdateBhgSgs(BHG* pbhg)
{
    UpdateStepguardSgs(pbhg);

    if (pbhg->sgs == SGS_Stun)
        LookStepguardAtGoal(pbhg);
}

void OnBhgEnteringSgs(BHG* pbhg, SGS sgsPrev, ASEG* pasegOverride)
{
    OnStepguardEnteringSgs(pbhg, sgsPrev, pasegOverride);
    if (pbhg->sgs == SGS_Dying) 
    {
        SetWorldComplete(GAMEWORLD_Snow);
        AutosaveCurrentGame(&g_saveData);
    }
}

void UpdateBhg(BHG* pbhg, float dt)
{
    UpdateStepguard(pbhg, dt);

    if (pbhg->pasegaCur && pbhg->pasegaCur->tLocal == 0.0f) 
    {
        RetractAsega(pbhg->pasegaCur);
        pbhg->pasegaCur = nullptr;
    }
}

void UpdateBhgBlot()
{
    if (FCanShowGameplayBlot())
        g_boss.pvtblot->pfnShowBlot(&g_boss);
    else
        g_boss.pvtblot->pfnHideBlot(&g_boss);
}

int FAbsorbBhgWkr(BHG* pbhg, WKR* pwkr) 
{
    if (pbhg->pasegHit && (pwkr->grfic & 32U)) 
    {
        pbhg->cAbsorbedHits++;
        DecrementBossHealth(&g_boss);

        if (pbhg->pexpl != nullptr)
        {
            EXPLSO explso{};

            explso.grfexplso = 6;
            explso.posOrigin = pwkr->pos;

            const float speed = glm::length(pwkr->v);

            if (speed < 0.0001f)
                explso.vec = g_normalX;
            else
                explso.vec = pwkr->v / speed;

            pbhg->pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pbhg->pexpl, &explso);
        }

        ASEG* pasegHit = pbhg->pasegHit;

        if ((pwkr->grftak & 2U) != 0) {
            if (g_pjt && g_pjt->jtcs != JTCS_SweepFore && g_pjt->jtcs != JTCS_StandingSweepFore) {
                pasegHit = pbhg->pasegSweepCounterHit;
            }
        }
        else if ((pwkr->grftak & 4U) != 0) {
            pasegHit = pbhg->pasegSmashHit;
        }

        ApplyAsegCur(pasegHit, pbhg, 0.0f, 1.0f, (GRFAPL)1, &pbhg->pasegaCur);
        pwkr->grfic = pwkr->grfic & 4294967263;
    }

    return FAbsorbStepguardWkr(pbhg, pwkr);
}

void SetBhgHitAnimations(BHG* pbhg, ASEG* pasegHit, ASEG* pasegSweepCounterHit, ASEG* pasegSmashHit)
{
    pbhg->pasegSmashHit = pasegSmashHit;
    pbhg->pasegHit = pasegHit;
    pbhg->pasegSweepCounterHit = pasegSweepCounterHit;
}

void DeleteBhg(BHG* phg)
{
	delete phg;
}

SCENTMAP* NewScentmap()
{
	return new SCENTMAP{};
}

void InitScentmap(SCENTMAP* pscentmap)
{
	InitAlo(pscentmap);
}

int GetScentmapSize()
{
	return sizeof(SCENTMAP);
}

void CloneScentmap(SCENTMAP* pscentmap, SCENTMAP* pscentmapBase)
{
    CloneAlo(pscentmap, pscentmapBase);

    pscentmap->cxMax = pscentmapBase->cxMax;
    pscentmap->cyMax = pscentmapBase->cyMax;
    pscentmap->cscp = pscentmapBase->cscp;
    pscentmap->ascp = pscentmapBase->ascp;
    pscentmap->mpixiypscpFirst = pscentmapBase->mpixiypscpFirst;
    pscentmap->posMin = pscentmapBase->posMin;
    pscentmap->posMax = pscentmapBase->posMax;
}

void PostScentmapLoad(SCENTMAP* pscentmap)
{
    PostAloLoad(pscentmap);
}

void UpdateScentmap(SCENTMAP* pscentmap, float dt)
{
    UpdateAlo(pscentmap, dt);
}

void DeleteScentmap(SCENTMAP* pscentmap)
{
	delete pscentmap;
}

SNIP s_asnipLoadMbg[2] = 
{
    0, (OID)0x82, offsetof(MBG, ablrun[0].paseg),
    0, (OID)0x83, offsetof(MBG, ablrun[1].paseg)
};

SNIP s_asnipPostMbgLoad[3] =
{
    1, (OID)0x437, offsetof(MBG, psm),
    0, (OID)0x440, offsetof(MBG, prwmLeftFire),
    0, (OID)0x441, offsetof(MBG, prwmRightFire)
};

OID s_mpmbakoidAseg[3] =
{
    (OID)0x446,
    (OID)0x447,
    (OID)0x448,
};

CID  s_acidReject[5] = 
{
    CID_MS, 
    CID_PO, 
    CID_MISSILE, 
    CID_VOLBTN, 
    CID_DECOY
};

float S_MbgVisCylinder = 150.0;

MBGLBONE s_ambgLboneMbg[6] = 
{
    { (OID)0x35,  50.0f, nullptr },
    { (OID)0x36,  50.0f, nullptr },
    { (OID)0x38,  80.0f, nullptr },
    { (OID)0x3b,  80.0f, nullptr },
    { (OID)0x60,  70.0f, nullptr },
    { (OID)0x50, 220.0f, nullptr },
};

float kMbgPursueNearSpeedBase = 0.0;
float kMbgPursueNearSpeedLinear = 0.00125;
float kMbgPursueNearSpeedQuad = 0.0;
float kMbgPursueFarSpeedBase = 0.0;
float kMbgPursueFarSpeedLinear = 0.00039999999;
float kMbgPursueFarSpeedQuad = 0.0;
float kMbgTurnBlendQuad = 0.0;
float kMbgTurnBlendLinear = -0.2546479;
float kMbgTurnBlendBase = 1.3;
int s_mbgAiSmaOid[3] = {0x43B, 0x43D, 0x43F};
SGS s_mbgAiSgs[3] = {SGS_Stun, SGS_Stun, SGS_Dying};
float kMbgFireFlatZOffset = 20.0;
float kMbgFirePredictZOffset = 20.0;
float DT_PredictMbg = 0.25;

#include "jt.h"
#include "pzo.h"
#include "rail.h"
#include "pipe.h"
#include "target.h"
#include "sm.h"
#include "actadj.h"
#include "wm.h"
#include "dialog.h"
#include "binoc.h"
#include "decoy.h"
#include "hnd.h"
#include "match.h"
#include "acp.h"
#include "spire.h"
#include "actval.h"
#include "steppower.h"
#include "stepguard.h"
#include "steppipe.h"
#include "stephang.h"
#include "stepcane.h"
#include "stepside.h"
#include "stephide.h"
#include "steprun.h"
#include "jsg.h"
#include "jump.h"
#include "tail.h"
#include "asegbl.h"
#include "bbmark.h"
#include "actadd.h"
#include "actseg.h"
#include "water.h"
#include "call.h"

void StartupJtIcon(JTICON* pjticon)
{
    pjticon->pvtjticon = &g_vtjticon;
}

void PostJtIconLoad(JTICON* pjticon)
{
    PostBlotLoad(pjticon);
    pjticon->pfont = PfontFromFont(2);

    const char icon = pjticon == &g_jticon ? 'L' : 'K';
    std::snprintf(pjticon->achzDraw, sizeof(pjticon->achzDraw), "%c", icon);

    pjticon->pfont->PushScaling(1.3f, 1.3f);
    pjticon->dx = pjticon->pfont->DxFromPchz(pjticon->achzDraw);
    pjticon->dy = static_cast<float>(pjticon->pfont->m_dyUnscaled) * pjticon->pfont->m_ryScale;
    pjticon->pfont->PopScaling();
}

void DrawJtIcon(JTICON* pjticon)
{
    constexpr float overshoot = 1.3f;
    constexpr float settleTime = 0.1f;
    const float acceleration = (1.0f - overshoot) / (settleTime * settleTime);
    const float peakTime = std::sqrt(-overshoot / acceleration);

    float iconScale = 1.0f;
    const float elapsed = g_clock.tReal - pjticon->tBlots;
    if (elapsed <= peakTime + settleTime)
    {
        const float t = elapsed - peakTime;
        iconScale = std::max(0.0f, overshoot + t * t * acceleration);
    }

    iconScale *= pjticon->rFontScale;
    if (iconScale <= 0.0f)
        return;

    pjticon->pfont->PushScaling(iconScale, iconScale);

    CTextBox tbx;
    tbx.SetPos(pjticon->x, pjticon->y);
    tbx.SetSize(pjticon->pfont->DxFromPchz(pjticon->achzDraw),
        static_cast<float>(pjticon->pfont->m_dyUnscaled) * pjticon->pfont->m_ryScale);
    tbx.SetTextColor(&pjticon->rgba);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);
    pjticon->pfont->DrawPchz(pjticon->achzDraw, &tbx);

    pjticon->pfont->PopScaling();
}

JT* NewJt()
{
	return new JT{};
}

void InitJt(JT* pjt)
{
	InitStep(pjt);

    pjt->jts = -1;
    pjt->jtcs = -1;
    pjt->jtbs = -1;
    pjt->jtjk = -1;
    
    pjt->sffRun = s_sffRun;
    pjt->sffSidestep = s_sffSidestep;
    pjt->sffStand = s_sffStand;
    pjt->sffFlatten = s_sffFlatten;
    pjt->sffDuck = s_sffDuck;

    pjt->fNoFreeze = 1;
    pjt->cpaloFindSwObjects &= ~1U;

    pjt->sRadiusHook = 15.0f;
    pjt->dvGravity = glm::vec3(0.0f, 0.0f, -1600.0f);
    pjt->posEdgeTarget = s_posEdgeTargetDefault;

    pjt->dtCharmFlash = 2.0f;
    pjt->fRenderCaneTrail = 1;
    pjt->jtpdk = -1;
    pjt->uAlpha = 1.0f;
    pjt->tStun = FLT_MAX;
    pjt->uAlphaCelBorder = 1.0f;
}

void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis)
{
    LoadSoFromBrx(pjt, pbis);
    SnipAloObjects(pjt, 151, s_asnipLoad);
    InferExpl(&pjt->pexplHook, pjt);

    pjt->tSweepPending = -1.0;
    pjt->tJumpPending = -1.0;
    g_pjt = pjt;
}

// Function bodies
void* GetJtFRenderCaneTrail(JT* pjt)
{
    return &pjt->fRenderCaneTrail;
}

void SetJtFRenderCaneTrail(JT* pjt, int fRenderCaneTrail)
{
    pjt->fRenderCaneTrail = fRenderCaneTrail;
}

void* GetJtJtpdk(JT* pjt)
{
    return &pjt->jtpdk;
}

void* GetJtFCancelPeek(JT* pjt)
{
    return &pjt->fCancelPeek;
}

void SetJtFCancelPeek(JT* pjt, int fCancelPeek)
{
    pjt->fCancelPeek = fCancelPeek;
}

void* GetJtFDisableStepCustomXps(JT* pjt)
{
    return &pjt->fDisableStepCustomXps;
}

void SetJtFDisableStepCustomXps(JT* pjt, int fDisableStepCustomXps)
{
    pjt->fDisableStepCustomXps = fDisableStepCustomXps;
}

void CloneJt(JT* pjt, JT* pjtBase)
{
    CloneStep(pjt, pjtBase);

    // Clone arrays like ablrunRun, ablssSidestep, ablssFlatten, etc.
    for (int i = 0; i < 6; ++i)
    {
        pjt->ablrunRun[i] = pjtBase->ablrunRun[i];
        pjt->ablrunBasket[i] = pjtBase->ablrunBasket[i];
        pjt->ablrunRail[i] = pjtBase->ablrunRail[i];
    }

    for (size_t i = 0; i < std::size(pjt->ablssSidestep); ++i)
    {
        pjt->ablssSidestep[i] = pjtBase->ablssSidestep[i];
        pjt->ablssFlatten[i] = pjtBase->ablssFlatten[i];
    }

    for (size_t i = 0; i < std::size(pjt->ablssDuck); ++i)
    {
        pjt->ablssDuck[i] = pjtBase->ablssDuck[i];
    }

    for (int i = 0; i < 2; ++i)
    {
        pjt->ablcl[i] = pjtBase->ablcl[i];
    }

    for (int i = 0; i < 57; ++i)
    {
        pjt->mpjtbspaseg[i] = pjtBase->mpjtbspaseg[i];
    }

    for (int i = 0; i < 15; ++i)
    {
        pjt->mpjtspso[i] = pjtBase->mpjtspso[i];
        pjt->mpjthkpso[i] = pjtBase->mpjthkpso[i];
    }

    // Clone simple variables
    pjt->fStandTight = pjtBase->fStandTight;
    pjt->xmgLeftBall = pjtBase->xmgLeftBall;
    pjt->xmgRightBall = pjtBase->xmgRightBall;
    pjt->fRenderCaneTrail = pjtBase->fRenderCaneTrail;
    pjt->tSweepPending = pjtBase->tSweepPending;
    pjt->cposHook = pjtBase->cposHook;

    // Clone arrays of glm::vec3
    for (int i = 0; i < 3; ++i)
    {
        pjt->aposHook[i] = pjtBase->aposHook[i];
    }

    pjt->sqtrmCane = pjtBase->sqtrmCane;

    // Clone pointers (assuming these need shallow copying unless deeper copying is necessary)
    pjt->ptargetCur = pjtBase->ptargetCur;
    pjt->phndCur = pjtBase->phndCur;
    pjt->ppipeCur = pjtBase->ppipeCur;
    pjt->phndUnhook = pjtBase->phndUnhook;
    pjt->ppipeUnhook = pjtBase->ppipeUnhook;
    pjt->pwater = pjtBase->pwater;
    pjt->paloShadow = pjtBase->paloShadow;

    // Clone other variables like float and int
    pjt->sPipeCur = pjtBase->sPipeCur;
    pjt->sPipeUnhook = pjtBase->sPipeUnhook;
    pjt->fPipeReverse = pjtBase->fPipeReverse;
    pjt->tJts = pjtBase->tJts;
    pjt->tJtbs = pjtBase->tJtbs;
    pjt->tJtcs = pjtBase->tJtcs;

    // Clone nested structs like ASEGA, SMA, etc.
    pjt->pasegaCur = pjtBase->pasegaCur;
    pjt->psmaCur = pjtBase->psmaCur;
    pjt->jtjk = pjtBase->jtjk;
    pjt->jsglk = pjtBase->jsglk;

    // Clone additional variables
    pjt->grfjtsc = pjtBase->grfjtsc;
    pjt->jtsNext = pjtBase->jtsNext;
    pjt->jtbsNext = pjtBase->jtbsNext;
    pjt->jtcsNext = pjtBase->jtcsNext;
    pjt->jtjkNext = pjtBase->jtjkNext;
    pjt->fShortenJump = pjtBase->fShortenJump;
    pjt->svxyAirSteer = pjtBase->svxyAirSteer;
    pjt->tJumpPending = pjtBase->tJumpPending;
    pjt->vTargetCoast = pjtBase->vTargetCoast;
    pjt->tHangDeflectLast = pjtBase->tHangDeflectLast;
    pjt->jtbsHangLatch = pjtBase->jtbsHangLatch;
    pjt->fHangReverse = pjtBase->fHangReverse;
    pjt->pxaHang = pjtBase->pxaHang;
    pjt->afxHang[0] = pjtBase->afxHang[0];
    pjt->afxHang[1] = pjtBase->afxHang[1];
    pjt->uSwingForward = pjtBase->uSwingForward;
    pjt->uSwingActive = pjtBase->uSwingActive;
    pjt->paloSkinStandard = pjtBase->paloSkinStandard;
    pjt->paloSkinElectric = pjtBase->paloSkinElectric;
    pjt->paloSkinFire = pjtBase->paloSkinFire;
    pjt->jthk = pjtBase->jthk;

    // Clone additional pointers and objects
    pjt->phshape = pjtBase->phshape;
    pjt->sParamHshape = pjtBase->sParamHshape;
    pjt->phpnt = pjtBase->phpnt;
    pjt->phbsk = pjtBase->phbsk;
    pjt->pjmtCur = pjtBase->pjmtCur;
    pjt->pspireCur = pjtBase->pspireCur;
    pjt->prailCur = pjtBase->prailCur;
    pjt->uRailCur = pjtBase->uRailCur;
    pjt->sRailCur = pjtBase->sRailCur;
    pjt->fRailReverse = pjtBase->fRailReverse;
    pjt->pvaultCur = pjtBase->pvaultCur;
    pjt->paloTool = pjtBase->paloTool;
    pjt->paloHook = pjtBase->paloHook;
    pjt->paloKey = pjtBase->paloKey;
    pjt->zpk = pjtBase->zpk;
    pjt->posPipeHandLocal = pjtBase->posPipeHandLocal;
    pjt->posPipeFootLocal = pjtBase->posPipeFootLocal;
    pjt->dposPipeXp = pjtBase->dposPipeXp;
    pjt->sPipeHand = pjtBase->sPipeHand;
    pjt->dsPipeHand = pjtBase->dsPipeHand;
    pjt->sRadiusHook = pjtBase->sRadiusHook;
    pjt->tContact = pjtBase->tContact;
    pjt->pexplHook = pjtBase->pexplHook;
    pjt->swToolSpinTarget = pjtBase->swToolSpinTarget;
    pjt->swToolSpin = pjtBase->swToolSpin;
    pjt->radToolSpin = pjtBase->radToolSpin;
    pjt->pwater = pjtBase->pwater;
    pjt->paloShadow = pjtBase->paloShadow;
    pjt->sffRun = pjtBase->sffRun;
    pjt->sffSidestep = pjtBase->sffSidestep;
    pjt->sffStand = pjtBase->sffStand;
    pjt->sffFlatten = pjtBase->sffFlatten;
    pjt->sffDuck = pjtBase->sffDuck;

    // Clone complex structs like TAIL, EXPL, etc.
    pjt->ptailTail = pjtBase->ptailTail;
    pjt->ptailMask = pjtBase->ptailMask;
    pjt->fCanBoost = pjtBase->fCanBoost;
}

void PostJtLoad(JT* pjt)
{
    PostStepLoad(pjt);
    SnipAloObjects(pjt, 27, s_asnipJtPostLoad);

    PostStepLoadRun(pjt, pjt->ablrunRun, &pjt->pasegblRun);
    PostJtLoadPipe(pjt);
    PostJtLoadSidestep(pjt, pjt->ablssFlatten, &pjt->pasegblFlatten);
    PostJtLoadSidestep(pjt, pjt->ablssDuck, &pjt->pasegblDuck);
    PostStepLoadRun(pjt, pjt->ablrunBasket, &pjt->pasegblBasket);
    PostJtLoadSwing(pjt, pjt->ablSwing, &pjt->pasegblSwing);
    PostStepLoadRun(pjt, pjt->ablrunRail, &pjt->pasegblRail);

    // The original game shallow-copies the complete JT template before
    // ClonePo/CloneStep run, so this PZI pointer is already present there.
    // Our field-by-field clone has no SNIP entry for it; resolve the matching
    // child explicitly so charm prizes can orbit and enter Sly's bag.
    pjt->pzi.paloBag = PaloFindPoBag(pjt);

    if (pjt->pzi.paloBag == nullptr)
        pjt->pzi.paloBag = static_cast<ALO*>(
            PloFindSwObject(pjt->psw, 0x104, OID_step_bag, pjt));

    if (pjt->pzi.paloBag == nullptr)
        pjt->pzi.paloBag = pjt->pzi.mpccharmpaloCharm[0];

    if (pjt->pzi.paloBag == nullptr)
        pjt->pzi.paloBag = pjt;

    pjt->pzi.paloCollect = pjt;
    pjt->pasegCollectKey = pjt->mpjtbspaseg[28];

    pjt->pactaddGut = PactaddNew(pjt->psw, pjt->paloGut).get();
    pjt->pactaddGut->ackPos = ACK_SmoothNoLock;
    pjt->pactaddGut->ackRot = ACK_SmoothNoLock;
    pjt->pactaddGut->nPriority = 3;
    pjt->pactaddGut->cpactAdd = 2;
    InsertAloAct(pjt->paloGut, pjt->pactaddGut);

    CreateAloActadj(pjt->paloGut, 5, &pjt->pactadjGut);
    CreateAloActadj(pjt->paloLeftHand, 5, &pjt->pactadjLeftHand);
    CreateAloActadj(pjt->paloRightHand, 5, &pjt->pactadjRightHand);
    CreateAloActadj(pjt->paloLeftFoot, 7, &pjt->pactadjLeftFoot);
    CreateAloActadj(pjt->paloRightFoot, 7, &pjt->pactadjRightFoot);

    pjt->pactvalGut = PactvalNew(pjt->psw, pjt->paloGut).get();
    pjt->pactvalGut->nPriority = 1;
    InsertAloAct(pjt->paloGut, pjt->pactvalGut);

    pjt->sqtrmCane.palo = pjt->paloTool;
    pjt->sqtrmCane.pshd = PshdFindShader((OID)346);
    pjt->sqtrmCane.rAlpha = 0.5f;

    pjt->sqtrmCane.clqSvToAlpha = s_clqSvToAlpha;

    pjt->sqtrmCane.cposLocal = 2;
    pjt->sqtrmCane.clqDtToAlpha = s_clqDtToAlpha;

    ConvertAloPos(nullptr, pjt->paloTool, &pjt->paloHook->xf.posWorld, &pjt->sqtrmCane.aposLocal[0]);

    float hookLength = glm::length(pjt->sqtrmCane.aposLocal[0]);

    if (hookLength > 0.0001f)
        pjt->sqtrmCane.aposLocal[0] *= (hookLength + pjt->sRadiusHook) / hookLength;

    EnsureAsegBlendDynamic(pjt, sizeof(pjt->ablSweepAttackFore[0]), static_cast<int>(std::size(pjt->ablSweepAttackFore)), pjt->ablSweepAttackFore, 0, nullptr, nullptr, &pjt->pasegblSweepAttackFore);
    EnsureAsegBlendDynamic(pjt, sizeof(pjt->ablSweepAttackBack[0]), static_cast<int>(std::size(pjt->ablSweepAttackBack)), pjt->ablSweepAttackBack, 0, nullptr, nullptr, &pjt->pasegblSweepAttackBack);
    EnsureAsegBlendDynamic(pjt, sizeof(pjt->ablStandingSweepAttackFore[0]), static_cast<int>(std::size(pjt->ablStandingSweepAttackFore)), pjt->ablStandingSweepAttackFore, 0, nullptr, nullptr, &pjt->pasegblStandingSweepAttackFore);
    EnsureAsegBlendDynamic(pjt, sizeof(pjt->ablStandingSweepAttackBack[0]), static_cast<int>(std::size(pjt->ablStandingSweepAttackBack)), pjt->ablStandingSweepAttackBack, 0, nullptr, nullptr, &pjt->pasegblStandingSweepAttackBack);

    if (pjt->paloShadow != nullptr)
    {
        if (pjt->paloShadow->pvtalo->pfnAddLo != nullptr)
            pjt->paloShadow->pvtalo->pfnAddLo(pjt->paloShadow);

        SetDyshShadow((DYSH*)pjt->paloShadow, pjt->pshadow.get());
    }

    pjt->ptnHideDynamic = static_cast<TN*>(PloNew(CID_TN, pjt->psw, nullptr, (OID)790, -1));

    ResetJtDynamicTunnel(pjt);
    PostJtLoadSidestep(pjt, pjt->ablssSidestep, &pjt->pasegblSidestep);
    PostJtLoadZap(pjt);

    if (pjt->pvtstep->pfnCloneStepPhys != nullptr)
        pjt->pvtstep->pfnCloneStepPhys(pjt, pjt->mpjtspso[0], 1);

    SetJtJts(pjt, 0, -1);

    pjt->zBaseTarget = pjt->xf.posWorld.z - 75.0f;

    SetStepGravityRatio(pjt, 1.3);
    SetSoMass(pjt, 238689.0);

    SO* apsoEffects[] =
    {
        pjt->psoSweepEffect,
        pjt->psoRushEffect,
        pjt->psoSmashEffect,
        pjt->psoDiveEffect,
        pjt->psoBallEffect
    };

    for (SO* psoEffect : apsoEffects)
    {
        if (psoEffect != nullptr)
            psoEffect->fNoXpsSelf = 1;
    }

    if (pjt->psoBallEffect != nullptr)
    {
        if (pjt->psoBallEffect->palox == nullptr)
            pjt->psoBallEffect->palox = std::make_shared <ALOX>();

        pjt->psoBallEffect->palox->grfalox |= 8;
        pjt->psoBallEffect->palox->scj.paloSchRot = nullptr;
    }

    if (pjt->mpjtspso[8] == nullptr)
        pjt->mpjtspso[8] = pjt->mpjtspso[2];

    pjt->pactvalJump = PactvalNew(pjt->psw, pjt).get();
    InsertAloAct(pjt, pjt->pactvalJump);

    pjt->pactvalHangHand = PactvalNew(pjt->psw, pjt->paloRightHand).get();
    pjt->pactvalHangHand->nPriority = 5;
    InsertAloAct(pjt->paloRightHand, pjt->pactvalHangHand);

    pjt->pactvalHangTool = PactvalNew(pjt->psw, pjt->paloTool).get();
    pjt->pactvalHangTool->nPriority = 5;
    InsertAloAct(pjt->paloTool, pjt->pactvalHangTool);

    pjt->viss = 0;
    pjt->mrds = 0;

    EnsureAsegBlendDynamic(pjt, sizeof(pjt->ablRailBalance[0]), static_cast<int>(std::size(pjt->ablRailBalance)), pjt->ablRailBalance, 0, nullptr, nullptr, &pjt->pasegblRailBalance);

    pjt->ablRailBalance[1].u = 1.0f;

    if (pjt->pasegblRailBalance != nullptr)
        ReblendAsegbl(pjt->pasegblRailBalance, sizeof(pjt->ablRailBalance[0]), static_cast<int>(std::size(pjt->ablRailBalance)), pjt->ablRailBalance);

    if (pjt->ptailTail != nullptr)
        pjt->ptailTail->cIterate = 2;
}

void PresetJtAccelBase(JT* pjt)
{
    SMPA smpaSpin = s_smpaSpin;

    if (pjt->jts == 1)
    {
        const float speed = glm::length(pjt->xf.v);

        float uSpin = s_clqSvToUSpin.g0 + speed * (s_clqSvToUSpin.g1 + speed * s_clqSvToUSpin.g2);

        uSpin = std::clamp(uSpin, 0.0f, 1.0f);

        float uSteer = s_clqUDeflectToUSpin.g0 + pjt->uJoyDeflect * (s_clqUDeflectToUSpin.g1 + pjt->uJoyDeflect * s_clqUDeflectToUSpin.g2);

        uSteer = std::clamp(uSteer, 0.0f, 1.0f);

        smpaSpin.svFast = glm::mix(s_smpaSpin.svFast, s_smpaSpinRun.svFast, uSpin) * uSteer;
        smpaSpin.svSlow = glm::mix(s_smpaSpin.svSlow, s_smpaSpinRun.svSlow, uSpin) * uSteer;
        smpaSpin.dtFast = glm::mix(s_smpaSpin.dtFast, s_smpaSpinRun.dtFast, uSpin);
        smpaSpin.sdvMax = glm::mix(s_smpaSpin.sdvMax, s_smpaSpinRun.sdvMax, uSpin);
    }

    glm::vec3 normalTilt{};
    const float tiltTarget = RadBetweenVectors(&pjt->xf.mat[2], &pjt->matTarget[2], &normalTilt);

    glm::mat3 dmat{};
    LoadRotateMatrixRad(tiltTarget, &normalTilt, &dmat);

    glm::mat3 mat = dmat * pjt->xf.mat;

    glm::vec3 normalSpin{};
    float spinTarget = RadBetweenVectors(&mat[0], &pjt->matTarget[0], &normalSpin);

    if (glm::dot(normalSpin, pjt->matTarget[2]) < 0.0f)
        spinTarget = -spinTarget;

    normalSpin = pjt->matTarget[2];

    glm::vec3 wTilt{};
    glm::vec3 wSpin{};

    const float tiltCurrent = SProjectVector(&pjt->xf.w, &normalTilt, &wTilt, &wSpin);
    const float spinCurrent = glm::dot(wSpin, normalSpin);

    float tiltNext = 0.0f;
    float spinNext = 0.0f;

    GSmoothA(0.0f, tiltCurrent, tiltTarget, g_clock.dt, &s_smpaTilt, &tiltNext);
    GSmoothA(0.0f, spinCurrent, spinTarget, g_clock.dt, &smpaSpin, &spinNext);

    const glm::vec3 angularVelocityTarget = normalTilt * tiltNext + normalSpin * spinNext;

    glm::vec3 angularAcceleration = (angularVelocityTarget - pjt->xf.w) / g_clock.dt;

    AddSoAngularAcceleration(pjt, &angularAcceleration);

    const glm::vec3* posBase = pjt->fBaseXp ? &pjt->posBaseXp : pjt->pposBase;
    const glm::vec3 dpos = pjt->xf.posWorld - *posBase;

    glm::vec3 acceleration = glm::cross(pjt->xf.dw, dpos);
    AddSoAcceleration(pjt, &acceleration);

    acceleration = glm::cross(pjt->xf.w, glm::cross(pjt->xf.w, dpos));
    AddSoAcceleration(pjt, &acceleration);
}

void PresetJtAccel(JT* pjt, float dt)
{
    PresetSoAccel(pjt, dt);

    switch (static_cast<int>(pjt->jts))
    {
        case 2:
        PresetJtAccelJump(pjt);
        break;

        case 3:
        PresetJtAccelHang(pjt);
        break;

        case 4:
        PresetJtAccelZap(pjt);
        break;

        case 6:
        PresetJtAccelHide(pjt);
        break;

        case 7:
        PresetJtAccelPipe(pjt);
        break;

        case 9:
        {
            if (g_clock.t - pjt->tJts <= 0.25f)
            {
                PresetJtAccelBase(pjt);
                break;
            }

            glm::vec3 acceleration = -pjt->dvGravity;
            AddSoAcceleration(pjt, &acceleration);
            AccelSoTowardPosSpring(pjt, nullptr, nullptr, &g_vecZero, &s_clqJtState9Spring, dt);
            AccelSoTowardMatSmooth(pjt, g_clock.dt, &pjt->matTarget, &pjt->smpSpin);

            break;
        }

        case 11:
        break;

        case 13:
        {
            const int jtbs = static_cast<int>(pjt->jtbs);

            if (jtbs == 54 || jtbs == 55)
            {
                glm::vec3 angularVelocity{};

                SetVectorCylind(&angularVelocity, pjt->radTarget + 1.5707964f, (pjt->uJoyDeflect * SV_JtBallMax) / pjt->sRadiusSelf, 0.0f);

                CLQ damping{};
                damping.g0 = 0.0f;
                damping.g1 = 2.5f * pjt->uJoyDeflect;
                damping.g2 = 0.0f;
                damping.gUnused = 0.0f;

                AccelSoTowardMatSpring(pjt, nullptr, nullptr, &angularVelocity, &damping, dt);
            }
            else if (jtbs == 56)
            {
                glm::mat3 matTarget{};
                LoadRotateMatrixRad(pjt->radTarget, &g_normalZ, &matTarget);
                AccelSoTowardMatSmooth(pjt, g_clock.dt, &matTarget, &s_smpRecover);
            }

            break;
        }

        default:
        PresetJtAccelBase(pjt);
        break;
    }
}

void AdjustJtNewXp(JT* pjt, XP* pxp, int ixpd)
{
    if (pjt->jts == 13)
    {
        // Ball
        pxp->mu = MU_JtBall;
        pxp->elas = ELAS_JtBall;
    }
    else if (pjt->jts == 7)
    {
        // Pipe
        pxp->mu = 0.0f;
        pxp->elas = 0.0f;
    }
    else if (!pjt->pvtstep->pfnFCheckStepXpBase(pjt, pxp, ixpd))
    {
        if (pjt->jts == 4)
        {
            // Zap
            pxp->mu = MU_JtZap;

            if (pjt->jtbs == 40 || pjt->jtbs == 49)
                pxp->elas = 0.0f;
            else
                pxp->elas = ELAS_JtZap;
        }
        else
        {
            pxp->mu = 0.0f;
            pxp->elas = 0.0f;
        }
    }
    else
    {
        // Normal ground contact
        pxp->mu = MU_JtBase;
        pxp->elas = 0.0f;

        // Slippery rail while hiding
        if (pjt->jts == 6 && pjt->jthk == 4 && pjt->prailCur->fSlippery != 0)
            pxp->mu = 0.0f;
    }

    pxp->mu /= pjt->rGravity;
}

void AdjustJtDz(JT* pjt, GRFADJ grfadj, DZ* pdz, int ixpd, float dt)
{
    if (pjt->jts != 4)
    {
        AdjustStepDzBase(pjt, grfadj, pdz, ixpd);
        return;
    }

    if ((grfadj & 0x10U) == 0)
        return;

    float sfMax;

    if ((grfadj & 0x01U) != 0)
        sfMax = -pjt->m * pjt->dvGravity.z * g_clock.dt * R_JtLimitPivotFrictionMax;
    else
        sfMax = -pjt->m * pjt->dvGravity.z * DT_JtLimitImpactFrictionMax;

    if (sfMax < pdz->sfMax)
        pdz->sfMax = sfMax;
}

void HandleJtGrfjtsc(JT* pjt)
{
    if (pjt->grfjtsc == 0)
        return;

    if ((pjt->grfjtsc & 0x08U) != 0)
    {
        pjt->grfjtsc &= ~0x08U;
        pjt->jtjk = pjt->jtjkNext;
    }

    if ((pjt->grfjtsc & 0x01U) != 0)
        SetJtJts(pjt, pjt->jtsNext, pjt->jtbsNext);

    if ((pjt->grfjtsc & 0x02U) != 0)
        SetJtJtcs(pjt, pjt->jtcsNext);

    if ((pjt->grfjtsc & 0x10U) != 0)
        FinishJtZap(pjt);
}

void UpdateJtInternalXps(JT* pjt)
{
    FreeSwXpList(pjt->psw, pjt->pxpInternal);
    pjt->pxpInternal = nullptr;

    if (pjt->jts == 7)
    {
        UpdateJtInternalXpsPipe(pjt);
        return;
    }

    if (pjt->jts == 6)
    {
        UpdateJtInternalXpsHide(pjt);
        return;
    }

    if (pjt->jts != 8)
        return;

    glm::vec3 posEdge{};
    glm::mat3 mat{};

    ConvertAloPos(pjt->psoEdgeCur, nullptr, &pjt->posEdgeLocal, &posEdge);

    posEdge += pjt->dposEdge;

    mat[0] = -pjt->normalEdgeCur;
    GetNormalVectors(&mat[0], &mat[1], &g_normalZ, &g_normalZ);

    posEdge -= mat * pjt->posEdgeTarget;

    AddJtHookXps(pjt, 7, pjt->psoEdgeCur, &posEdge, nullptr, &pjt->xf.posWorld);
}

int FCheckJtXpBase(JT* pjt, XP* pxp, int ixpd)
{
    if (pjt->jts != 13)
        return FCheckStepXpBase(pjt, pxp, ixpd);

    if (pxp->normal.z * g_mpixpdr[ixpd] < 0.7f)
        return 0;

    if (pxp->xpk <= XPK_EdgeToEdge || pxp->xpk >= XPK_SphereToSphere)
        return 0;

    return ixpd == 0;
}

void AdjustJtXpVelocity(JT* pjt, XP* pxp, int ixpd)
{
    const int ixpdOther = 1 - ixpd;
    SO* psoOther = pxp->axpd[ixpdOther].psoLeaf;
    const bool fButton1524 =
        psoOther != nullptr && static_cast<int>(psoOther->oid) == 1524;

    if (pjt->jts == 4)
    {
        if (pxp->xpk == XPK_PointToSurface && ixpd == 0 && pxp->pv0 == pjt->pposBumper)
        {
            glm::vec3 velocity = pjt->matTarget * pjt->vTarget;
            velocity -= pxp->normal * glm::dot(velocity, pxp->normal);
            pxp->axpd[0].v += velocity;
        }

        return;
    }

    if (pjt->jts == 13)
        return;

    if (pjt->pvtstep->pfnFCheckStepXpBase(pjt, pxp, ixpd))
    {
		// A button can discard and rebuild its contact XP in the same solver
		// timestamp when its animation starts.  Do not attach JT to the rebuilt
		// base when the two contact points are already separating; doing so
		// replaces the launch velocity with the platform's downward velocity.
		if (fButton1524)
		{
			const glm::vec3 relativeVelocity =
				pxp->axpd[ixpd].v - pxp->axpd[ixpdOther].v;
			const float separatingSpeed =
				glm::dot(relativeVelocity, pxp->normal) * g_mpixpdr[ixpd];

			if (separatingSpeed > 0.0f)
				return;
		}

        AdjustStepXpVelocityBase(pjt, pxp, ixpd);
        return;
    }

    if (pxp->axpd[ixpd].psoLeaf == pjt)
    {
        glm::vec3 contactOffset = pxp->axpd[ixpd].pos - pjt->xf.posWorld;
        glm::vec3 rotationalVelocity = glm::cross(pjt->xf.w, contactOffset);
        pxp->axpd[ixpd].v -= rotationalVelocity;
    }
}

void HandleJtMessage(JT* pjt, MSGID msgid, void* pv)
{
    HandlePoMessage(pjt, msgid, pv);

    switch (msgid)
    {
        case MSGID_removed:
        {
            void* objectCurrent = nullptr;

            if (pjt->jts == 6)
            {
                if (pjt->jthk == 4)
                    objectCurrent = pjt->prailCur;
                else if (pjt->jthk == 5)
                    objectCurrent = pjt->pspireCur;
                else
                    return;
            }
            else if (pjt->jts == 3)
                objectCurrent = pjt->phndCur;
            else if (pjt->jts == 7)
                objectCurrent = pjt->ppipeCur;
            else
                return;

            if (objectCurrent == pv)
                UnhookJt(pjt);

            break;
        }

        case MSGID_asega_limit:
        {
            if (pv == pjt->pasegaJtcs)
            {
                pjt->grfjtsc |= 2;
                pjt->jtcsNext = (pjt->jtcs == (JTCS)10) ? (JTCS)2 : (JTCS)-1;
                break;
            }

            if (pv != pjt->pasegaCur)
                return;

            switch (pjt->jts)
            {
                case 2:
                if (pjt->jtbs != 7 && pjt->jtbs != 8)
                    return;

                pjt->jtsNext = (JTS)2;
                pjt->jtbsNext = (JTBS)2;
                pjt->grfjtsc |= 1;
                break;

                case 3:
                case 5:
                case 6:
                case 8:
                case 11:
                case 13:
                break;

                case 4:
                pjt->grfjtsc |= 16;
                break;

                case 7:
                if (pjt->jtbs != 27 && pjt->jtbs != 23)
                    return;

                pjt->jtsNext = (JTS)7;
                pjt->jtbsNext = (JTBS)24;
                pjt->grfjtsc |= 1;
                break;

                case 9:
                case 10:
                case 14:
                case 15:
                case 16:
                pjt->jtsNext = (JTS)0;
                pjt->jtbsNext = (JTBS)-1;
                pjt->grfjtsc |= 1;
                break;

                case 12:
                if (pjt->jtbs == 33)
                {
                    pjt->jtsNext = (JTS)12;
                    pjt->jtbsNext = (JTBS)34;
                    pjt->grfjtsc |= 1;
                }
                else if (pjt->jtbs == 35)
                {
                    pjt->jtsNext = (JTS)0;
                    pjt->jtbsNext = (JTBS)-1;
                    pjt->grfjtsc |= 1;
                }
                else
                    return;

                break;

                default:
                return;
            }

            break;
        }

        case MSGID_asega_wrapped:
        {
            if (pv == pjt->pasegaCur && pjt->jts > 4 && pjt->jts < 7)
                pjt->sslfCur = SSLF_Nil;

            break;
        }

        case MSGID_water_entered:
        {
            WATER_EVENT_MESSAGE* pmsg = reinterpret_cast<WATER_EVENT_MESSAGE*>(pv);

            if (pmsg->pjt != pjt)
                return;

            pjt->pwater = pmsg->pwater;

            for (int i = 0; i < 8; ++i)
                pjt->jtWaterContact[i].tContact = 0.0f;

            break;
        }

        case MSGID_water_left:
        {
            WATER_EVENT_MESSAGE* pmsg = reinterpret_cast<WATER_EVENT_MESSAGE*>(pv);

            if (pmsg->pjt == pjt && pmsg->pwater == pjt->pwater)
                pjt->pwater = nullptr;

            break;
        }

        case MSGID_label:
        {
            const ASEGLABELMSG* pmsg = static_cast<const ASEGLABELMSG*>(pv);
            ASEGA* pasega = pmsg->pasega;

            if (pasega != pjt->pasegaCur && pasega != pjt->pasegaJtcs)
            {
                if (pjt->psmaCur == nullptr || pasega != pjt->psmaCur->pasegaCur)
                    return;
            }

            switch (pmsg->label)
            {
                case 312:
                {
                    CHN* pchn = pasega->paseg->pchnStrip;

                    if (pchn == nullptr || pchn->pacp == nullptr)
                        break;

                    glm::vec3 vLocal(0.0f);
                    glm::vec3 vWorld(0.0f);

                    pchn->pacp->pvtacp->pfnEvaluateAcp(pchn->pacp.get(), pjt, pasega->tLocal, pasega->svtLocal, 0, nullptr, &vLocal);
                    vWorld.x = pjt->matTarget[0].x * vLocal.x + pjt->matTarget[1].x * vLocal.y + pjt->matTarget[2].x * vLocal.z;
                    vWorld.y = pjt->matTarget[0].y * vLocal.x + pjt->matTarget[1].y * vLocal.y + pjt->matTarget[2].y * vLocal.z;
                    vWorld.z = pjt->matTarget[0].z * vLocal.x + pjt->matTarget[1].z * vLocal.y + pjt->matTarget[2].z * vLocal.z;
                    pjt->pvtalo->pfnSetAloVelocityVec(pjt, &vWorld);

                    if (pasega == pjt->pasegaCur)
                        pjt->vTarget = -vWorld;

                    return;
                }

                case 314:
                pjt->fLockBase = 1;
                break;

                case 315:
                pjt->fLockBase = 0;
                pjt->dposSmoothCpdefi = pjt->posBasePrev - pjt->xf.pos;
                pjt->dposSmoothCpdefi.z = 0.0f;
                break;

                case 316:
                pjt->swToolSpinTarget = (pjt->jtbs == 8) ? SW_JtToolSpinDive : SW_JtToolSpinRush;
                break;

                case 317:
                pjt->swToolSpinTarget = 0.0f;
                break;

                case 318:
                {
                    const float tTurn = TFindAsegLabel(pjt->pasegaCur->paseg, (OID)320);
                    const float dtAvailable = tTurn - pmsg->tLocal;
                    const int keyCount = std::min(pjt->plockgCur->cplock, g_pwsCur->ckey);

                    pjt->cmatchCollect = 0;

                    for (int i = 0; i < keyCount; ++i)
                    {
                        LOCK* plock = pjt->plockgCur->aplock[i];
                        MATCH* pmatch = reinterpret_cast<MATCH*>(PripNewRipg(RIPT_Match, nullptr));

                        if (pmatch == nullptr)
                            continue;

                        pjt->pmatch[pjt->cmatchCollect++] = pmatch;

                        pmatch->pvtrip->pfnInitRip(pmatch, &pjt->paloCollectTarget[0]->xf.posWorld, R_JtKeyScale, nullptr);
                        SubscribeRipObject(pmatch, pjt);

                        const glm::vec3 dpos = plock->paloKey->xf.posWorld - pjt->paloCollectTarget[0]->xf.posWorld;
                        const float dtTravel = std::min(glm::length(dpos), 300.0f) * 0.00055555557f;

                        pmatch->dtLifetime = std::min(dtAvailable, dtTravel);
                        pmatch->paloRender = plock->paloKey;
                        pmatch->paloLaunch = pjt->paloCollectTarget[0];
                        pmatch->emitdv.dv = g_psw->dvGravity;
                    }

                    return;
                }

                case 320:
                {
                    LOCKG* plockg = pjt->plockgCur;

                    for (int i = 0; i < plockg->cplock; ++i)
                    {
                        SMA* psma = plockg->aplock[i]->psma;

                        if (i < g_pwsCur->ckey)
                            SetSmaGoal(psma, (g_pwsCur->ckey < plockg->cplock) ? (OID)560 : (OID)559);
                        else
                            SetSmaGoal(psma, (OID)561);
                    }

                    if (g_pwsCur->ckey < plockg->cplock)
                        SetSmaGoal(plockg->psma, (OID)560);
                    else
                    {
                        SetSmaGoal(plockg->psma, (OID)559);
                        g_pwsCur->fws |= plockg->grfws;
                    }

                    EA* peaGrab = PeaFindAsegLabel(pjt->pasegaCur->paseg, (OID)319);

                    if (peaGrab == nullptr)
                        break;

                    const float dtAvailable = peaGrab->tLocal - pjt->pasegaCur->tLocal;
                    const int keyCount = std::min(plockg->cplock, g_pwsCur->ckey);

                    pjt->cmatchCollect = 0;

                    for (int i = 0; i < keyCount; ++i)
                    {
                        LOCK* plock = plockg->aplock[i];
                        MATCH* pmatch = reinterpret_cast<MATCH*>(PripNewRipg(RIPT_Match, nullptr));

                        if (pmatch == nullptr)
                            continue;

                        pjt->pmatch[pjt->cmatchCollect++] = pmatch;

                        pmatch->pvtrip->pfnInitRip(pmatch, &plock->paloKey->xf.posWorld, 1.0f / R_JtKeyScale, nullptr);
                        SubscribeRipObject(pmatch, pjt);

                        const glm::vec3 dpos = plock->paloKey->xf.posWorld - pjt->paloCollectTarget[0]->xf.posWorld;
                        const float dtTravel = std::min(glm::length(dpos), 300.0f) * 0.0016666667f;

                        pmatch->dtLifetime = std::min(dtAvailable, dtTravel);
                        pmatch->paloRender = pjt->paloCollectTarget[0];
                        pmatch->paloLaunch = plock->paloKey;
                        pmatch->emitdv.dv  = g_psw->dvGravity;
                        pmatch->tCreated   = g_clock.t + (dtAvailable - pmatch->dtLifetime);
                    }

                    return;
                }

                case 323:
                pjt->fCaneInterruptible = 1;
                break;

                case 324:
                pjt->grfjtsc |= 16;
                break;

                case 350:
                pjt->fCharmEnabled = 1;
                break;

                case 351:
                pjt->fCharmEnabled = 0;
                break;

                default:
                break;
            }

            break;
        }

        default:
        break;
    }
}

int FTurnJtToTarget(JT* pjt)
{
    glm::vec3 targetPosition{};

    if (pjt->ptargetCur != nullptr)
        GetXfmPos(pjt->ptargetCur, &targetPosition);
    else
    {
        if (pjt->ppipeCur == nullptr)
            return 0;

        CRV* pcrv = pjt->ppipeCur->pcrv.get();

        if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
            pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeCur, &targetPosition, nullptr);

        ConvertAloPos(pjt->ppipeCur->paloParent, nullptr, &targetPosition, &targetPosition);
    }

    glm::vec3 direction = targetPosition - pjt->xf.posWorld;
    pjt->radTarget = std::atan2(direction.y, direction.x);

    return 1;
}

void UpdateJtBalance(JT* pjt)
{
    ACK ackGoal = ACK_Nil;

    if (pjt->jts == 6 && pjt->jthk == 4 && pjt->jtbs == 16 && static_cast<unsigned int>(pjt->jtcs - 4) < 2)
    {
        ackGoal = ACK_Velocity;

        if (pjt->prailCur->fSlippery == 0)
        {
            const glm::vec3 dposFeet = pjt->pactadjLeftFoot->dposGoal - pjt->pactadjRightFoot->dposGoal;
            const float radFeet = std::atan2(dposFeet.z, DZ_JtRailFootSpan);

            glm::mat3 matFeet(1.0f);
            LoadRotateMatrixRad(U_JtRailGutRoll * radFeet, &g_normalX, &matFeet);

            const glm::vec3 dposGut = s_posJtRailGut - s_posJtRailGutPivot;

            glm::vec3 dposGutRotated;
            dposGutRotated.x = matFeet[0].x * dposGut.x + matFeet[1].x * dposGut.y + matFeet[2].x * dposGut.z;
            dposGutRotated.y = matFeet[0].y * dposGut.x + matFeet[1].y * dposGut.y + matFeet[2].y * dposGut.z;
            dposGutRotated.z = matFeet[0].z * dposGut.x + matFeet[1].z * dposGut.y + matFeet[2].z * dposGut.z;

            pjt->pactvalGut->posGoal = s_posJtRailGutPivot + dposGutRotated;
            LoadRotateMatrixEuler(&s_radJtRailGut, &pjt->pactvalGut->matGoal);
        }
        else
        {
            pjt->pactvalGut->posGoal = s_posJtSlipperyRailGut;
            LoadRotateMatrixEuler(&s_radJtSlipperyRailGut, &pjt->pactvalGut->matGoal);
        }
    }

    if (pjt->pactvalGut->ackPos != ackGoal)
    {
        pjt->pactvalGut->ackPos = ackGoal;
        pjt->pactvalGut->ackRot = ackGoal;
        pjt->paloGut->pvtalo->pfnRecacheAloActList(pjt->paloGut);
    }

    if (pjt->pactadjGut == nullptr || pjt->pactadjGut->ackPos == ACK_Nil)
        return;

    ALO* paloFrom = nullptr;
    glm::vec3* ppos = nullptr;

    glm::vec3 pos(0.0f);
    glm::vec3 vMovement(0.0f);
    glm::vec3 vCurrent(0.0f);
    glm::vec3 vPredicted(0.0f);
    glm::vec3 dvAdjust(0.0f);

    if (pjt->jts == 0)
    {
        if (pjt->fBaseXp != 0)
        {
            paloFrom = pjt->psoBaseXp;
            ppos = &pjt->posBaseXp;
        }
    }
    else if (pjt->jts == 6 && pjt->jtbs == 16)
    {
        if (pjt->jthk == 4)
        {
            RAIL* prail = pjt->prailCur;
            CRV* pcrv = prail->pcrv.get();

            paloFrom = prail->paloParent;

            glm::vec3 normalRailLocal(0.0f);

            if (pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
                pcrv->pvtcrv->pfnEvaluateCrvFromU(pcrv, pjt->uRailCur, &pos, &normalRailLocal);

            ConvertAloPos(paloFrom, nullptr, &pos, &pos);
            ppos = &pos;

            if (prail->fSlippery != 0)
            {
                const float normalLength = glm::length(normalRailLocal);

                if (normalLength >= 0.0001f)
                    normalRailLocal /= normalLength;
                else
                    normalRailLocal = g_normalZ;

                const float svRail = glm::dot(pjt->xf.v, normalRailLocal);

                vCurrent = normalRailLocal * svRail;

                float sPredict = pjt->sRailCur + svRail * DT_JtBalancePredict;
                const float sMax = pcrv->pvtcrv->pfnSMaxCrv(pcrv);

                if (pcrv->fClosed == 0)
                    sPredict = std::clamp(sPredict, pcrv->mpicvs[0], sMax);
                else
                    sPredict = GModPositive(sPredict, sMax);

                glm::vec3 normalPredictLocal(0.0f);

                if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
                    pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, sPredict, nullptr, &normalPredictLocal);

                glm::vec3 normalPredictWorld(0.0f);
                ConvertAloVec(paloFrom, nullptr, &normalPredictLocal, &normalPredictWorld);

                const float normalPredictLength = glm::length(normalPredictWorld);

                if (normalPredictLength >= 0.0001f)
                    normalPredictWorld /= normalPredictLength;
                else
                    normalPredictWorld = g_normalZ;

                vPredicted = normalPredictWorld * svRail;
                dvAdjust = vPredicted - vCurrent;

                if (pjt->pasegblRailBalance != nullptr)
                {
                    glm::vec3 normalSide = glm::cross(g_normalZ, normalPredictWorld);
                    const float normalSideLength = glm::length(normalSide);

                    if (normalSideLength >= 0.0001f)
                        normalSide /= normalSideLength;
                    else
                        normalSide = g_normalY;

                    float uTarget = SProjectVector(&dvAdjust, &normalSide, nullptr, &dvAdjust) / SDV_JtBalanceMax;
                    uTarget = std::clamp(uTarget, -1.0f, 1.0f);

                    if (pjt->fRailReverse != 0)
                        uTarget = -uTarget;

                    uTarget *= 2.0f - std::abs(uTarget);

                    pjt->uRailBalance = GSmoothA(pjt->uRailBalance, pjt->duRailBalance, uTarget, g_clock.dt, &s_smpRailBalance, &pjt->duRailBalance);

                    const float uPositive = std::max(pjt->uRailBalance, 0.0f);
                    const float uNegative = std::max(-pjt->uRailBalance, 0.0f);

                    pjt->ablRailBalance[0].u = uPositive;
                    pjt->ablRailBalance[1].u = 1.0f - uPositive - uNegative;
                    pjt->ablRailBalance[2].u = uNegative;

                    ReblendAsegbl(pjt->pasegblRailBalance, sizeof(BL), 3, pjt->ablRailBalance);
                }
            }
        }
        else if (pjt->jthk == 5)
        {
            paloFrom = pjt->pspireCur->paloParent;
            GetPntPos(pjt->pspireCur, &pos);
            ppos = &pos;
        }
    }

    if (paloFrom == nullptr)
        return;

    CalculateAloMovement(paloFrom, nullptr, *ppos, &vMovement, nullptr, nullptr, nullptr);
    ConvertAloPos(nullptr, paloFrom, ppos, &vCurrent);

    vPredicted = glm::vec3(0.0f);
    PredictAloTransformAdjust(paloFrom, nullptr, DT_JtBalancePredict, &vCurrent, nullptr, &vPredicted, nullptr);

    const glm::vec3 dvWorld = (dvAdjust + vPredicted - vMovement) / DT_JtBalancePredict;
    const float sdv = glm::length(dvWorld);

    glm::vec3 dposGoal(0.0f);

    if (sdv > 0.0001f)
    {
        const float uSdv = std::min(sdv / SDV_JtBalanceMax, 1.0f);

        float uLean = s_clqUSdvToULean.g0 + uSdv * (s_clqUSdvToULean.g1 + uSdv * s_clqUSdvToULean.g2);
        uLean = std::clamp(uLean, 0.0f, 1.0f);

        glm::vec3 dvLocal;
        dvLocal.x = glm::dot(pjt->matTarget[0], dvWorld);
        dvLocal.y = glm::dot(pjt->matTarget[1], dvWorld);
        dvLocal.z = glm::dot(pjt->matTarget[2], dvWorld);

        dposGoal.x = dvLocal.x * XY_JtBalanceMax * uLean / sdv;
        dposGoal.y = dvLocal.y * XY_JtBalanceMax * uLean / sdv;
        dposGoal.z = Z_JtBalanceMax * uLean;
    }

    pjt->pactadjGut->dposGoal = dposGoal;

    const float zLeftFoot = pjt->pactadjLeftFoot->dposGoal.z;
    const float zRightFoot = pjt->pactadjRightFoot->dposGoal.z;

    const float zLeftHeel = s_clqGutHeelAdjust.g0 + zLeftFoot * (s_clqGutHeelAdjust.g1 + zLeftFoot * s_clqGutHeelAdjust.g2);
    const float zRightHeel = s_clqGutHeelAdjust.g0 + zRightFoot * (s_clqGutHeelAdjust.g1 + zRightFoot * s_clqGutHeelAdjust.g2);

    if (dposGoal.z < zLeftHeel)
        pjt->pactadjGut->dposGoal.z = std::min(zRightHeel, dposGoal.z);
    else
        pjt->pactadjGut->dposGoal.z = std::min(zRightHeel, zLeftHeel);

    if (pjt->jtcs == -1)
    {
        const glm::vec3 dposHand = dposGoal * U_JtBalanceHands;

        pjt->pactadjLeftHand->dposGoal = dposHand;
        pjt->pactadjRightHand->dposGoal = dposHand;
    }
    else
    {
        pjt->pactadjLeftHand->dposGoal = glm::vec3(0.0f);
        pjt->pactadjRightHand->dposGoal = pjt->pactadjLeftHand->dposGoal;
    }
}

void RebuildJtXmg(JT* pjt, ALO* paloExtrem, float sRadius, ALO* paloIkh, ACTADJ* pactadj, XMG* pxmg)
{
    ASEGA* pasega = nullptr;

    if (pjt->jts == 0)
    {
        if (pjt->jtcs < 10 && pjt->jtcs > 5)
            pasega = pjt->pasegaJtcs;
        else
            pasega = pjt->psmaCur->pasegaCur;
    }
    else if (pjt->psmaCur == nullptr)
        pasega = pjt->pasegaCur;
    else
        pasega = pjt->psmaCur->pasegaCur;

    if (pasega->paseg == pxmg->paseg)
        return;

    pxmg->paseg = pasega->paseg;

    float tLocal = 0.0f;
    EA* pea = PeaFindAsegLabel(pasega->paseg, (OID)322);

    if (pea != nullptr)
        tLocal = pea->tLocal;

    ALOX* palox = paloIkh->palox.get();
    ALO* paloShoulder = palox->ikh.paloShoulder;
    ALO* paloElbow = palox->ikh.paloElbow;

    glm::vec3 posExtrem{};
    glm::vec3 posShoulder{};
    glm::vec3 posIkh{};
    glm::mat3 matIkh{};

    PredictAsegEffect(pasega->paseg, tLocal, 0.0f, paloExtrem, pjt, 1, &posExtrem, nullptr, nullptr, nullptr);

    posExtrem.z -= sRadius;

    PredictAsegEffect(pasega->paseg, tLocal, 0.0f, paloShoulder, pjt, 1, &posShoulder, nullptr, nullptr, nullptr);
    PredictAsegEffect(pasega->paseg, tLocal, 0.0f, paloIkh, pjt, 1, &posIkh, &matIkh, nullptr, nullptr);

    posIkh += matIkh * palox->ikh.posIkh;
    pxmg->posSphere = posShoulder + (posExtrem - posIkh);

    pxmg->sSphere = glm::length(palox->ikh.posWrist) + glm::length(paloElbow->xf.pos) + 10.0f;

    glm::vec3 positions[2];
    positions[0] = posExtrem + g_normalZ * DZ_StepMatchGroundUp;
    positions[1] = posExtrem + g_normalZ * DZ_StepMatchGroundDown;

    LSG lsg{};
    const int intersectionCount = ClsgClipEdgeToSphere(&pxmg->posSphere, pxmg->sSphere, &positions[0], &positions[1], 1, &lsg);

    pxmg->fValid = intersectionCount > 0;
    pxmg->posDown = lsg.apos[1];
    pxmg->posExtrem = posExtrem;
    pxmg->posUp = lsg.apos[0];
}

int FMatchJtXmg(JT* pjt, XMG* pxmg, ACTADJ* pactadj)
{
    if (pxmg->fValid == 0)
        return 1;

    glm::vec3 positions[2]{};

    ConvertAloPos(pjt, nullptr, &pxmg->posUp, &positions[0]);
    ConvertAloPos(pjt, nullptr, &pxmg->posDown, &positions[1]);

    std::vector<SO*> objects;
    IntersectSwBoundingBox(pjt->psw, static_cast<SO*>(pjt->paloRoot), &positions[0], &positions[1], nullptr, nullptr, objects);

    LSG bestIntersection{};
    SO* hitObject = PsoHitTestLineObjects(0, &positions[0], &positions[1], objects, &bestIntersection);

    if (hitObject != nullptr)
    {
        glm::vec3 hitPosition = bestIntersection.apos[0] - g_normalZ;
        glm::vec3 hitPositionLocal{};

        ConvertAloPos(nullptr, pjt, &hitPosition, &hitPositionLocal);
        pactadj->dposGoal = hitPositionLocal - pxmg->posExtrem;

        return 1;
    }

    pactadj->dposGoal = g_vecZero;

    if (pjt->fBaseXp != 0 && pxmg->posDown.z < -85.0f)
        return 0;

    return 1;
}

void UpdateJtStand(JT* pjt)
{
    if (!FTurnJtToTarget(pjt) && pjt->fBaseXp != 0)
    {
        glm::vec3 angularVelocity{};

        CalculateAloMovement(pjt->psoBaseXp, nullptr, pjt->posBaseXp, nullptr, &angularVelocity, nullptr, nullptr);
        angularVelocity.z = GLimitAbs(angularVelocity.z, 10.0f);
        pjt->radTarget += angularVelocity.z * g_clock.dt;
    }

    if (pjt->pactadjLeftFoot == nullptr)
        return;

    RebuildJtXmg(pjt, pjt->paloLeftBall,  5.0f, pjt->paloLeftHeel,  pjt->pactadjLeftFoot,  &pjt->xmgLeftBall);
    RebuildJtXmg(pjt, pjt->paloRightBall, 5.0f, pjt->paloRightHeel, pjt->pactadjRightFoot, &pjt->xmgRightBall);

    bool leftMatched = FMatchJtXmg(pjt, &pjt->xmgLeftBall, pjt->pactadjLeftFoot) != 0;
    bool rightMatched = leftMatched && FMatchJtXmg(pjt, &pjt->xmgRightBall, pjt->pactadjRightFoot) != 0;

    if (rightMatched || pjt->fStandTight != 0 || pjt->psmIdle == nullptr)
        return;

    if (IsmsFindSmOptional(pjt->psmIdle, (OID)328) >= 0)
        SeekSma(pjt->psmaCur, (OID)328);

    pjt->fStandTight = 1;
}

void ThrowJt(JT* pjt, glm::vec3* pposTarget, float angVert, float uSteer)
{
    glm::vec3 posLaunch = pjt->xf.posWorld;
    posLaunch.z -= 75.0f;

	glm::vec3 muzzleVelocity{};
	FCalculateMuzzleVelocity(posLaunch, *pposTarget, angVert, muzzleVelocity, pjt);
	pjt->pvtalo->pfnSetAloVelocityVec(pjt, &muzzleVelocity);

    if (pjt->jts == 4)
        return;

    SetJtJts(pjt, 2, 6);
    pjt->svxyAirSteer *= uSteer;

    if (uSteer == 0.0f)
        pjt->fCanBoost = 0;
}

void UnhookJt(JT* pjt)
{
    if (pjt->jts == 6)
    {
        if (pjt->jthk == 4 || pjt->jthk == 5)
            SetJtJts(pjt, 2, 2);

        return;
    }

    if (pjt->jts != 3 && pjt->jts != 7)
        return;

    pjt->jtjk = (JTJK)1;
    SetJtJts(pjt, 2, 2);
}

void CalculateJtToolSpin(JT* pjt, float u, glm::mat3* pmat)
{
    const glm::vec3& axis = pjt->jtbs == 8 ? g_normalZ : g_normalX;

    glm::mat3 spinMatrix{};
    float angle = pjt->radToolSpin - g_clock.dt * pjt->swToolSpin * u;

    LoadRotateMatrixRad(angle, &axis, &spinMatrix);
    *pmat = pjt->xf.mat * spinMatrix * pjt->paloRightHand->xf.mat;
}

void RenderJtAll(JT* pjt, CM* pcm, RO* pro)
{
    if (pjt->fHidden != 0)
        return;

    float alphaTarget = 1.0f;
    float celBorderTarget = 1.0f;

    if (pjt->jts == 6 && pjt->jthk == 3 && FJtSafeFromStepguards(pjt) != 0)
    {
        float elapsed = g_clock.t - pjt->tJts;
        float blend = elapsed >= 0.5f ? 1.0f : g_clock.dt / (0.5f - elapsed);

        pjt->uAlpha = blend * U_JtAlphaNonchalant + (1.0f - blend) * pjt->uAlpha;
        celBorderTarget = U_JtAlphaCelBorderNonchalant;
    }
    else
        pjt->uAlpha = GSmooth(pjt->uAlpha, 1.0f, g_clock.dt, &s_smpAlpha, nullptr);

    pjt->uAlphaCelBorder = GSmooth(pjt->uAlphaCelBorder, celBorderTarget, g_clock.dt, &s_smpAlpha, nullptr);

    RO adjustedRo{};

    if (pjt->uAlpha < 1.0f || pjt->uAlphaCelBorder < 1.0f)
    {
        DupAloRo(pjt, pro, &adjustedRo);
        adjustedRo.uAlphaCelBorder = pjt->uAlphaCelBorder;
        adjustedRo.uAlpha *= pjt->uAlpha;
        pro = &adjustedRo;
    }

    pjt->sRadiusRenderAll = 500.0f;

    if (pjt->radToolSpin == 0.0f && pjt->swToolSpin == 0.0f)
    {
        int preserveTrail = pjt->jts == 3 || pjt->jts == 4 || pjt->jts == 7;
        UpdateSqtrm(&pjt->sqtrmCane, nullptr, nullptr, 0.0f, g_clock.dt);
    }
    else
    {
        glm::mat3 toolSpin{};
        CalculateJtToolSpin(pjt, 0.0f, &toolSpin);

        glm::vec3 toolPosition = pjt->paloRightHand->xf.posWorld + toolSpin * pjt->paloTool->xf.pos;
        glm::mat3 toolMatrix = toolSpin * pjt->paloTool->xf.mat;

        UpdateSqtrm(&pjt->sqtrmCane, &toolPosition, &toolMatrix, pjt->swToolSpin, g_clock.dt);
        pjt->paloRightHand->fHidden = 1;
    }

    // Keep Sly visible while the enter/exit ASEGs play.  The binocular mask
    // replaces him only during the active peek substate.
    bool hideForPeek = pjt->jts == JTS_Peek && pjt->jtbs == JTBS_Peek_Peek;
    bool worldMapBlocksRender = g_wmc.pwmCurrent != nullptr && static_cast<unsigned int>(g_wmc.pwmCurrent->wms + ~WMS_Appearing) <= 1;

    if (!hideForPeek && !worldMapBlocksRender)
    {
        RenderAloAll(pjt, pcm, pro);
        pjt->paloRightHand->fHidden = 0;
    }
}

void UpdateJtXfWorld(JT* pjt)
{
    UpdateStepXfWorld(pjt);

    if (pjt->jts == 6 && pjt->jthk == 7)
    {
        pjt->xmgLeftBall.fRetractValid = 0;
        pjt->xmgRightBall.fRetractValid = 0;
        return;
    }

    if (pjt->paloLeftFoot == nullptr || pjt->paloRightFoot == nullptr)
    {
        pjt->xmgLeftBall.fRetractValid = 0;
        pjt->xmgRightBall.fRetractValid = 0;
        return;
    }

    RetractStepExtremity(pjt, pjt->paloLeftFoot,  6.0f, pjt->paloLeftFoot, pjt->paloLeftHeel, &pjt->xmgLeftBall);
    RetractStepExtremity(pjt, pjt->paloRightFoot, 6.0f, pjt->paloRightFoot, pjt->paloRightHeel, &pjt->xmgRightBall);
}

void RenderJtSelf(JT* pjt, CM* pcm, RO* pro)
{
    if (pjt->jts != 6 || pjt->jthk != 7)
    {
        ApplyStepExtremityRetraction(pjt, pjt->paloLeftFoot, pjt->paloLeftFoot, &pjt->xmgLeftBall);
        ApplyStepExtremityRetraction(pjt, pjt->paloRightFoot, pjt->paloRightFoot, &pjt->xmgRightBall);
    }

    if ((pjt->radToolSpin != 0.0f || pjt->swToolSpin != 0.0f) && pjt->paloRightHand != nullptr)
    {
        ALO* paloRightHand = pjt->paloRightHand;

        /*
         * The normal right-hand hierarchy is hidden while the tool-spin
         * copies are rendered. Temporarily make it visible.
         */
        paloRightHand->fHidden = false;

        RO roHand = {};
        DupAloRo(pjt, pro, &roHand);
        roHand.uAlpha *= 0.25f;

        for (int i = 0; i < 4; ++i)
        {
            float uSpin = static_cast<float>(i) * 0.25f;

            glm::mat3 matSpin;
            CalculateJtToolSpin(pjt, uSpin, &matSpin);
            LoadMatrixFromPosRot(&paloRightHand->xf.posWorld, &matSpin, &roHand.model);
            paloRightHand->pvtalo->pfnRenderAloAll(paloRightHand, pcm, &roHand);
        }

        paloRightHand->fHidden = true;
    }

    if (pjt->jts == 3)
        UpdateJtHangIkGoals(pjt);

    RenderStepSelf(pjt, pcm, pro);

    if (pjt->fRenderCaneTrail)
        RenderSqtrm(&pjt->sqtrmCane, pcm);

    RenderJtSelfZap(pjt, pcm, pro);
}

void UpdateJtTool(JT* pjt)
{
    constexpr float TwoPi = 6.2831855f;

    if (pjt->swToolSpinTarget != 0.0f)
    {
        pjt->swToolSpin = pjt->swToolSpinTarget;
        pjt->radToolSpin = std::fmod(pjt->radToolSpin + pjt->swToolSpin * g_clock.dt, TwoPi);
        return;
    }

    if (pjt->radToolSpin == 0.0f)
        return;

    pjt->radToolSpin = GSmooth(pjt->radToolSpin, TwoPi, g_clock.dt, &s_smpRad, &pjt->swToolSpin);

    if (pjt->radToolSpin == TwoPi)
        pjt->radToolSpin = 0.0f;
}

void AdjustJtXps(JT* pjt)
{
    XP* pxpFree = nullptr;

    AdjustSoZapXps(pjt, &pjt->pxpZap);

    // Remove invalid basket contacts and transfer accepted decoy contacts.
    for (OX* pox = pjt->poxa->pox; pox != nullptr; pox = pox->poxNext)
    {
        XP** ppxp = &pox->pxp;

        while (*ppxp != nullptr)
        {
            XP* pxp = *ppxp;
            SO* psoRoot0 = pxp->axpd[0].psoRoot;

            const int iJt = (psoRoot0 != pjt) ? 1 : 0;
            const int iOther = 1 - iJt;

            // Contacts involving Sly's basket are ignored unless the other
            // object is a dart.
            if (pxp->axpd[iJt].psoLeaf == pjt->phbsk)
            {
                BASIC* pbasicOther = pxp->axpd[iOther].psoRoot;

                if (pbasicOther != nullptr && FIsBasicDerivedFrom(pbasicOther, CID_DART))
                {
                    ppxp = &pxp->pxpNext;
                    continue;
                }

                *ppxp = pxp->pxpNext;
                pxp->pxpNext = pxpFree;
                pxpFree = pxp;
                continue;
            }

            // Transfer selected contacts to the active decoy's private list.
            if (pxp->axpd[iOther].psoRoot == pjt->pdecoy)
            {        
                const bool fTransfer = pjt->pvtjt->pfnFCheckJtXpBase(pjt, pxp, iOther);

                if (fTransfer)
                {
                    *ppxp = pxp->pxpNext;
                    pxp->pxpNext = pjt->pdecoy->pxp;
                    pjt->pdecoy->pxp = pxp;
                    continue;
                }
            }

            ppxp = &pxp->pxpNext;
        }

        pox->poxOther->pxp = pox->pxp;
    }

    AdjustStepXps(pjt);

    // While Sly is in the edge state, use the edge object as his base and
    // discard ordinary contacts against that object's root.
    if (pjt->jts == 8)
    {
        pjt->fBaseXp = 1;
        pjt->tBaseXp = g_clock.t;
        pjt->psoBaseXp = pjt->psoEdgeCur;
        pjt->normalBaseXp = g_normalZ;

        ConvertAloPos(pjt->psoEdgeCur, nullptr, &pjt->posEdgeLocal, &pjt->posBaseXp);

        for (OX* pox = pjt->poxa->pox; pox != nullptr; pox = pox->poxNext)
        {
            XP** ppxp = &pox->pxp;

            while (*ppxp != nullptr)
            {
                XP* pxp = *ppxp;

                // Equality constraints remain attached.
                if (pxp->xpk != XPK_Equality)
                {
                    SO* psoRoot0 = pxp->axpd[0].psoRoot;
                    const int iOther = (psoRoot0 == pjt) ? 1 : 0;

                    if (pxp->axpd[iOther].psoRoot == pjt->psoEdgeCur->paloRoot)
                    {
                        *ppxp = pxp->pxpNext;
                        pxp->pxpNext = pxpFree;
                        pxpFree = pxp;
                        continue;
                    }
                }

                ppxp = &pxp->pxpNext;
            }

            pox->poxOther->pxp = pox->pxp;
        }
    }

    FreeSwXpList(pjt->psw, pxpFree);
}

void UpdateJtPosWorldPrev(JT* pjt)
{
    UpdateSoPosWorldPrev(pjt);
    pjt->cposHook = 0;
}

void ProjectJtTransform(JT* pjt)
{
    pjt->cposHook = 0;
    pjt->aposHook[1] = pjt->paloHook->xf.posWorld;

    ProjectSoTransform(pjt, g_clock.dt, 0);

    if (pjt->jtcs != 0)
    {
        pjt->cposHook = 0;
        return;
    }

    pjt->aposHook[0] = pjt->xf.posWorld;
    pjt->aposHook[2] = pjt->paloHook->xf.posWorld;
    pjt->cposHook = 3;

    pjt->pvtjt->pfnUpdateJtBounds(pjt);
}

void UpdateJtBounds(JT* pjt)
{
    UpdateSoBounds(pjt);

    for (int i = 0; i < pjt->cposHook; ++i)
        ExtendSoBounds(pjt, &pjt->aposHook[i], pjt->sRadiusHook);
}

SO* PsoGetJtEffect(JT* pjt, int* pigrfic)
{
    if (pjt->psoSweepEffect != nullptr &&
        FIsLoInWorld(pjt->psoSweepEffect))
    {
        *pigrfic = 0;
        return pjt->psoSweepEffect;
    }

    if (pjt->psoRushEffect != nullptr &&
        FIsLoInWorld(pjt->psoRushEffect))
    {
        *pigrfic = 1;
        return pjt->psoRushEffect;
    }

    if (pjt->psoSmashEffect != nullptr &&
        FIsLoInWorld(pjt->psoSmashEffect))
    {
        *pigrfic = 2;
        return pjt->psoSmashEffect;
    }

    if (pjt->psoDiveEffect != nullptr &&
        FIsLoInWorld(pjt->psoDiveEffect))
    {
        *pigrfic = 2;
        return pjt->psoDiveEffect;
    }

    if (pjt->psoBallEffect != nullptr &&
        FIsLoInWorld(pjt->psoBallEffect))
    {
        *pigrfic = 1;
        return pjt->psoBallEffect;
    }

    return nullptr;
}

void AddJtCustomXps(JT* pjt, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst)
{
    const int jts = pjt->jts;

    // Replace this name with the actual JT field at offset 0x2BD8.
    const bool disableStepCustomXps = pjt->fDisableStepCustomXps != 0;

    if (jts == 4)
    {
        if (!disableStepCustomXps)
        {
            const int jtbs = pjt->jtbs;

            if (jtbs == 36 || jtbs == 42 || jtbs == 43)
                AddStepCustomXpsBase(pjt, psoOther, pbspPruned, ppxpFirst);
        }
    }
    else if (jts != 3 && jts != 7 && jts != 13)
    {
        if (!disableStepCustomXps)
            AddStepCustomXpsBase(pjt, psoOther, pbspPruned, ppxpFirst);
    }

    ClipStepRetractionToObject(pjt, psoOther, pbspPruned, &pjt->xmgLeftBall);
    ClipStepRetractionToObject(pjt, psoOther, pbspPruned, &pjt->xmgRightBall);

    int igrfic = 0;
    SO* psoEffect = PsoGetJtEffect(pjt, &igrfic);

    if (psoEffect == nullptr)
        return;

    for (SO* pso = psoOther; pso != nullptr; pso = static_cast<SO*>(pso->paloParent))
    {
        if (pso->ficg.agrfic[igrfic] == 0)
            continue;

        if (psoOther->fSphere)
        {
            MarkSoContactsSphereBsp(psoOther, &psoOther->xf.posWorld, psoOther->sRadiusSelf, psoEffect, psoEffect->bspc.cbsp, psoEffect->bspc.absp.data(), nullptr, &pjt->pxpEffect);
            return;
        }

        if (abspPruned == nullptr)
            return;

        MarkSoContactsBspBsp(psoEffect, psoOther, cbspPruned, abspPruned, pbspPruned, &pjt->pxpEffect);
        return;
    }
}

CT CtTorqueJt(JT* pjt)
{
    if ((pjt->jts != 3) && (pjt->jts != 13))
        return CT_Locked;

    return CT_Free;
}

void GetJtCpdefi(JT* pjt, float dt, CPDEFI* pcpdefi)
{
    const float zBasePrev = pjt->posBasePrev.z;

    GetPoCpdefi(pjt, dt, pcpdefi);

    JTS jts = (JTS)pjt->jts;

    if (pjt->fLockBase == 0)
    {
        if (jts == 7)
        {
            PIPE* ppipe = pjt->ppipeCur;
            CRV* pcrv = ppipe->pcrv.get();

            glm::vec3 posPipeLocal(0.0f);
            glm::vec3 tangentPipeLocal(0.0f);
            glm::vec3 posPipeWorld(0.0f);
            glm::vec3 tangentPipeWorld(0.0f);

            if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
                pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand, &posPipeLocal, &tangentPipeLocal);

            ConvertAloPos(ppipe->paloParent, nullptr, &posPipeLocal, &posPipeWorld);
            ConvertAloVec(ppipe->paloParent, nullptr, &tangentPipeLocal, &tangentPipeWorld);

            pcpdefi->posBase = posPipeWorld;

            const float tangentLength = glm::length(tangentPipeWorld);

            if (tangentLength >= 0.0001f)
                tangentPipeWorld /= tangentLength;
            else
                tangentPipeWorld = glm::vec3(0.0f);

            pcpdefi->vBase = tangentPipeWorld * pjt->dsPipeHand;
            pjt->zBaseTarget = pcpdefi->posBase.z - 125.0f;
        }
        else
        {
            if (jts == 3)
            {
                glm::vec3 posHandle(0.0f);

                GetXfmPos(pjt->phndCur, &posHandle);
                pjt->zBaseTarget = posHandle.z + DZ_JtBaseTargetHang;
            }
            else if (pjt->fBaseXp != 0)
                pjt->zBaseTarget = pjt->posBaseXp.z;
            else if (g_pcm->fCut != 0)
                pjt->zBaseTarget = pjt->xf.posWorld.z - 75.0f;

            const float zTargetMin = pjt->xf.pos.z + DZ_JtBaseTargetMin;
            const float zTargetMax = pjt->xf.pos.z + DZ_JtBaseTargetMax;

            pjt->zBaseTarget = std::clamp(pjt->zBaseTarget, zTargetMin, zTargetMax);

            if (g_pcm->fCut == 0)
            {
                const float zBase = GSmooth(zBasePrev, pjt->zBaseTarget + 125.0f, g_clock.dt, &s_smpZ, nullptr);
                const float zBaseMin = pjt->xf.pos.z + DZ_JtBaseMin;
                const float zBaseMax = pjt->xf.pos.z + DZ_JtBaseMax;

                pcpdefi->posBase.z = std::clamp(zBase, zBaseMin, zBaseMax);
            }
            else
                pcpdefi->posBase.z = pjt->zBaseTarget + 125.0f;
        }

        if (pjt->jts == 6 && pjt->jthk == 0)
            GetJtCpdefiFlatten(pjt, pcpdefi);

        glm::vec3 dposTarget = pjt->posBasePrev - pcpdefi->posBase;

        pjt->dposSmoothCpdefi = PosSmooth(pjt->dposSmoothCpdefi, dposTarget, dt, &s_smpSmoothCpdefi, nullptr);
        pcpdefi->posBase += pjt->dposSmoothCpdefi;

        jts = (JTS)pjt->jts;
    }

    // Enable the first CPD flag for airborne or freely moving states.
    if (jts < 6)
    {
        if (jts >= 3)
            pcpdefi->grfcpd |= 1;
        else if (jts == 2 && pjt->fCanBoost != 0)
            pcpdefi->grfcpd |= 1;
    }
    else if (jts == 6)
    {
        switch (pjt->jthk)
        {
            case 0:
            case 1:
            pcpdefi->cfk = CFK_Side;
            break;

            case 2:
            case 4:
            case 5:
            break;

            default:
            pcpdefi->grfcpd |= 1;
            break;
        }
    }

    /*
     * Standing still does not update tMoveLast:
     *   JTS_Stand
     *   JTS_Hide + JTBS_Hide_Stand
     *   JTS_Pipe + JTBS_Pipe_Stay
     */
    bool fMoving = true;

    if (jts == 6)
        fMoving = pjt->jtbs != 16;
    else if (jts < 7)
    {
        if (jts == 0)
            fMoving = false;
    }
    else if (jts == 7)
        fMoving = pjt->jtbs != 24;

    if (fMoving)
        pjt->tMoveLast = g_clock.t;

    pcpdefi->tMoveLast = pjt->tMoveLast;

    // Disable automatic camera adjustment in selected states.
    switch (pjt->jts)
    {
        case 4:
        {
            const int jtbs = pjt->jtbs;

            if (jtbs == 41 || (jtbs >= 45 && jtbs <= 49))
                pcpdefi->sAdjust = 0.0f;

            break;
        }

        case 0:
        {
            const float swz = std::abs(pjt->xf.w.z);

            float uAdjust =
                s_clqSwzToUAdjust.g0 +
                swz * (s_clqSwzToUAdjust.g1 +
                    swz * s_clqSwzToUAdjust.g2);

            uAdjust = std::clamp(uAdjust, 0.0f, 1.0f);
            pcpdefi->sAdjust *= uAdjust;
            break;
        }

        case 7:
        case 13:
        pcpdefi->sAdjust = 0.0f;
        break;

        default:
        break;
    }

    pjt->posBasePrev = pcpdefi->posBase;
}

void GetJtCpdefiFlatten(JT* pjt, CPDEFI* pcpdefi)
{
    pcpdefi->radHome = atan2f(pjt->xf.matWorld[0][1], pjt->xf.matWorld[0][0]);

    if (pjt->ptnCur != pjt->ptnHideDynamic)
        return;

    pcpdefi->sAdjust = 0.0f;

    HSHAPE* phshape = pjt->phshape;
    const int hfdk = pjt->hfdk;

    if (hfdk < 3)
    {
        if (hfdk == 0)
        {
            pjt->posFocus = pjt->xf.posWorld;
            pjt->posFocus.z += 50.0f;
        }
        else if (hfdk == 1 || hfdk == 2)
        {
            const float focusOffset = pjt->uFlattenClamp * pjt->uFlattenClamp * 125.0f;

            if (phshape == nullptr)
            {
                const glm::vec3 lateral = pjt->xf.matWorld[1] * focusOffset;

                if (hfdk == 1)
                    pjt->posFocus = pjt->xf.posWorld + lateral;
                else
                    pjt->posFocus = pjt->xf.posWorld - lateral;

                pjt->posFocus.z += 50.0f;
            }
            else
            {
                CRV* pcrv = phshape->pcrv.get();
                float sFocus;

                if (hfdk == 1)
                {
                    sFocus = pjt->sParamHshape + focusOffset;

                    if (!pcrv->fClosed)
                    {
                        float sMaximum = pcrv->mpicvs[pcrv->ccv - 1];

                        if ((phshape->grfhp & 2U) != 0)
                            sMaximum += 100.0f;

                        sFocus = std::min(sFocus, sMaximum);
                    }
                }
                else
                {
                    sFocus = pjt->sParamHshape - focusOffset;

                    if (!pcrv->fClosed)
                    {
                        float sMinimum = pcrv->mpicvs[0];

                        if ((phshape->grfhp & 1U) != 0)
                            sMinimum -= 100.0f;

                        sFocus = std::max(sFocus, sMinimum);
                    }
                }

                glm::vec3 posFocusLocal{};

                if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
                    pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, sFocus, &posFocusLocal, 0);

                ConvertAloPos(phshape->paloParent, nullptr, &posFocusLocal, &pjt->posFocus);
                pjt->posFocus.z += 125.0f;
            }
        }
    }

    if (phshape == nullptr || phshape->pcrv->fClosed)
        return;

    CRV* pcrv = phshape->pcrv.get();
    const float sMaximum = pcrv->pvtcrv->pfnSMaxCrv(pcrv);
    const float sCurrent = pjt->sParamHshape;
    const float fadeDistance = std::min((sMaximum - 60.0f) * 0.5f, 300.0f);
    const float fadeEnd = (sMaximum - 30.0f) - fadeDistance;

    float cameraOffset = 0.0f;

    if (sCurrent < fadeDistance + 30.0f)
    {
        const float u = std::clamp(((fadeDistance + 30.0f) - sCurrent) / fadeDistance, 0.0f, 1.0f);
        cameraOffset = -u * u * 150.0f;
    }
    else if (sCurrent > fadeEnd)
    {
        const float u = std::clamp((sCurrent - fadeEnd) / fadeDistance, 0.0f, 1.0f);
        cameraOffset = u * u * 150.0f;
    }

    CLQ& cameraCurve = pjt->ptnHideDynamic->tnfn.aclqCam[0];
    cameraCurve.g0 = GSmooth(cameraCurve.g0, cameraOffset, g_clock.dt, &s_smpPeek, nullptr);
}

void OnJtActive(JT* pjt, int fActive)
{
    OnPoActive(pjt, fActive, nullptr);

    if (fActive == 0)
        SetFsp(FSP_Nil);
    else
        SetFsp((FSP)g_pgsCur->nPowerupLast);
}

int FCheckJtBumperPath(JT* pjt, const glm::vec3* pvec)
{
    glm::vec3 posStart = *pjt->pposBumper;
    glm::vec3 posEnd = posStart + *pvec;

    std::vector<SO*> objects;
    IntersectSwBoundingBox(pjt->psw, pjt, &posStart, &posEnd, nullptr, nullptr, objects);
    
    for (SO* pso : objects)
    {
        LSG alsg[8]{};
        const int clsg = ClsgClipEdgeToObject(pso, &posStart, &posEnd, 8, alsg);

        for (int ilsg = 0; ilsg < clsg; ++ilsg)
        {
            const LSG& lsg = alsg[ilsg];

            if (lsg.au[0] != 0.0f && lsg.anormal[0].z > 0.7f)
                return 1;
        }
    }

    return 0;
}

void UpdateJtActive(JT* pjt, JOY* pjoy, float dt)
{
    if (FDrivenAlo(pjt))
    {
        const bool fFaceButtonPressed =
            pjoy->IsPressed(BTN_TRIANGLE) ||
            pjoy->IsPressed(BTN_CIRCLE) ||
            pjoy->IsPressed(BTN_CROSS) ||
            pjoy->IsPressed(BTN_SQUARE);

        if (pjoy->uDeflect <= 0.0f && !fFaceButtonPressed)
            return;

        RetractAloDrive(pjt);
    }

    JOY joyOverride = {};
    bool fUseJoyOverride = false;

    if (g_cdialogTriggered > 0 || pjt->plockg != nullptr || pjt->psoDisplacePending != nullptr || pjt->psoDisplacePendingSecondary != nullptr)
    {
        switch (pjt->jts)
        {
            case 2:
            if (pjt->jtbs == 4 || (pjt->jtbs > 9 && pjt->jtbs < 16))
                SetJtJts(pjt, 2, 2);
            break;

            case 8:
            if (g_clock.t - pjt->tJts > 0.2f)
            {
                pjt->jtjk = (JTJK)4;
                SetJtJts(pjt, 2, 0);
            }
            break;

            case 12:
            if (pjt->pdialogPeekPending == nullptr)
                SetJtJts(pjt, 12, 35);
            break;
        }

        pjoy = &joyOverride;
        fUseJoyOverride = true;
    }

    if (pjt->jts == 4)
        ReadStepJoystick(pjt, pjoy);
    else if (pjt->pjsgCur != nullptr && FIsJsgActive(pjt->pjsgCur))
    {
        JSG* pjsg = pjt->pjsgCur;

        if (g_joy.IsPressed(BTN_START) && g_wipe.wipes == 0 && pjsg->ijsgeCur >= 0 && pjsg->ijsgeCur < pjsg->cjsge)
        {
            for (int ijsge = pjsg->ijsgeCur; ijsge < pjsg->cjsge; ++ijsge)
            {
                JSGE* pjsge = &pjsg->ajsge[ijsge];

                if (pjsge->jsgek != 1)
                    continue;

                LO* ploWarp = PloFindSwObject(pjsg->psw, static_cast<GRFFSO>(260), static_cast<OID>(pjsge->oid), pjsg->ploContext);

                if (ploWarp == nullptr || !FIsBasicDerivedFrom(ploWarp, CID_WARP))
                    continue;

                WipeToWorldWarp(g_transition.m_plevelCurrent, static_cast<OID>(ploWarp->oid), WIPEK_Keyhole);
                g_joy.SetHandled(BTN_START);
                break;
            }
        }

        joyOverride = {};
        ReadJsgJoystick(pjt->pjsgCur, &joyOverride);
        pjoy = &joyOverride;
    }
    else
    {
        if (pjt->pjsgCur != nullptr)
            RetractJsg(pjt->pjsgCur);

        ReadStepJoystick(pjt, pjoy);
    }

    float radFacing = std::atan2(pjt->xf.mat[0][1], pjt->xf.mat[0][0]);
    RadNormalize(pjt->radJoy - radFacing);

    bool fJump = false;
    bool fFall = false;
    bool fBoost = false;

    if (pjoy->IsPressed(BTN_CROSS))
    {
        switch (pjt->jts)
        {
            case 2:
            if ((pjt->jtbs < 7 || pjt->jtbs > 8) && pjt->fCanBoost)
            {
                glm::vec3 posBumperTest = -g_normalZ * pjt->dzBase;
                posBumperTest += pjt->xf.v * DT_JtBoostPredict;
                posBumperTest += pjt->dvGravity * DT2_JtBoostPredict;
                fBoost = !FCheckJtBumperPath(pjt, &posBumperTest);
            }
            break;

            case 3:
            case 7:
            case 8:
            fJump = true;
            break;

            case 4:
            break;

            case 6:
            // Basket, spire, and rail hides all accept Cross. Basket uses
            // its dedicated jump-out substate in the transition below.
            fJump = true;
            break;

            default:
            {
                glm::vec3 posBumperTest = -g_normalZ * (pjt->dzBase + SV_JtHideSearch);

                if (g_clock.t - pjt->tBaseXp < 0.1f || FCheckJtBumperPath(pjt, &posBumperTest))
                    fJump = true;

                break;
            }
        }

        if (!fJump)
            pjt->tJumpPending = g_clock.t;
    }
    else if (g_clock.t - pjt->tJumpPending < 0.1f && g_clock.t - pjt->tBaseXp < 0.1f)
        fJump = true;

    switch (pjt->jts)
    {
        case 0:
        case 1:
        case 5:
        case 6:
        case 14:
        case 15:
        case 16:
        if (g_clock.t - pjt->tBaseXp > 0.1f)
        {
            float dzSearch = pjt->jts == 6 && pjt->jthk == 3 ? SV_JtBasketSearch : SV_JtHideSearch;
            glm::vec3 posBumperTest = -g_normalZ * (pjt->dzBase + dzSearch);
            fFall = !FCheckJtBumperPath(pjt, &posBumperTest);
        }
        break;
    }

    bool fNoJoy = pjt->uJoyDeflect <= 0.0f;
    bool fRun = !fNoJoy;
    bool fPeek = false;
    const bool fHideHeld = pjoy->IsHeld(BTN_CIRCLE);
    const bool fHidePressed = pjoy->IsPressed(BTN_CIRCLE);
    const bool fRushPressed = pjoy->IsPressed(BTN_TRIANGLE);
    const bool fSmashPressed = pjoy->IsPressed(BTN_SQUARE);

    bool fCanRush = g_fsp != FSP_Dive;

    UpdateJtActivePowerUp(pjt, pjoy);

    // Release dialog/peek input handling. L1/R1 either finishes the current
    // skippable dialog, starts an available instruction, or requests a peek
    // transition. Camera policies suppress these inputs while active.
    if (FActiveCplcy(&g_pcm->cpman) == 0)
    {
        JOY* pjoyDialog = (g_grfjoyt & 4U) != 0 ? &g_joy : &g_joyZero;
        const bool fDialogButtonPressed =
            pjoyDialog->IsPressed(BTN_L1) ||
            pjoyDialog->IsPressed(BTN_R1);
        const bool fPeekButtonPressed =
            pjoy->IsPressed(BTN_L1) ||
            pjoy->IsPressed(BTN_R1);

        int fCancelPeek = pjt->fCancelPeek;

        if (g_binoc.pdialogPlaying != nullptr &&
            g_binoc.pdialogPlaying->dialogs == DIALOGS_Playing &&
            g_binoc.pdialogPlaying->fNoSkip == 0 &&
            fDialogButtonPressed &&
            *g_binoc.pdialogPlaying->pfPlayed != 0)
        {
            fPeek = pjt->jts == JTS_Peek;

            if (fPeek)
                pjt->pdialogPeekPending = nullptr;

            FinishDialogEvents(g_binoc.pdialogPlaying);

            if (pjoyDialog->IsPressed(BTN_L1))
                pjoyDialog->SetHandled(BTN_L1);
            if (pjoyDialog->IsPressed(BTN_R1))
                pjoyDialog->SetHandled(BTN_R1);
        }
        else if (fCancelPeek != 0 || fPeekButtonPressed)
        {
            if (pjt->jts == JTS_Peek)
            {
                if (g_binoc.pdialogPlaying == nullptr)
                {
                    pjt->pdialogPeekPending = nullptr;
                    fPeek = true;
                }
            }
            else if (g_binoc.pdialogPlaying == nullptr)
            {
                const bool fTriggerInstruction =
                    pjoy->IsPressed(BTN_L1) &&
                    g_call.pdialogTriggered != nullptr &&
                    (fCancelPeek == 0 ||
                     g_call.pdialogTriggered->dialogk == DIALOGK_Instruct);

                if (fTriggerInstruction)
                    SetDialogDialogs(g_call.pdialogTriggered, DIALOGS_Triggered);
                else if (fCancelPeek == 0)
                {
                    pjt->pdialogPeekPending = nullptr;
                    fPeek = true;
                }
            }
        }
    }

    // Restore the central retail state-transition loop.  The reconstructed
    // function previously computed these requests but never consumed them,
    // leaving Sly stuck in his current state even with valid joystick input.
    for (;;)
    {
        const JTS jtsPrevious = (JTS)pjt->jts;
        const JTBS jtbsPrevious = (JTBS)pjt->jtbs;
        JTS jtsNext = (JTS)pjt->jts;
        JTBS jtbsNext = (JTBS)pjt->jtbs;

        switch (pjt->jts)
        {
            case JTS_Stand:
                if (fJump)
                {
                    pjt->jtjk = JTJK_Standing;
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Init;
                }
                else if (fFall)
                {
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Fall;
                }
                else if (fRushPressed && g_fsp == FSP_Dive &&
                         (GetAvailableVaultFlags() & 0x0002U) != 0)
                {
                    jtsNext = JTS_Rush;
                    jtbsNext = JTBS_Rush_Attack;
                }
                else
                {
                    const JTBS jtbsHide = fHideHeld
                        ? static_cast<JTBS>(JtbsChooseJtHide(pjt, nullptr, &pjt->jthk))
                        : JTBS_Nil;

                    if (jtbsHide != JTBS_Nil)
                    {
                        jtsNext = JTS_Hide;
                        jtbsNext = jtbsHide;
                    }
                    else if (fPeek)
                    {
                        jtsNext = JTS_Peek;
                        jtbsNext = JTBS_Peek_Enter;
                    }
                    else if (fRun)
                    {
                        jtsNext = JTS_Run;
                        jtbsNext = JTBS_Nil;
                    }
                }
                break;

            case JTS_Run:
                if (fJump)
                {
                    pjt->jtjk = JTJK_Running;
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Init;
                }
                else if (fFall)
                {
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Fall;
                }
                else if (fRushPressed && g_fsp == FSP_Dive &&
                         (GetAvailableVaultFlags() & 0x0002U) != 0)
                {
                    jtsNext = JTS_Rush;
                    jtbsNext = JTBS_Rush_Attack;
                }
                else
                {
                    const JTBS jtbsHide = fHideHeld
                        ? static_cast<JTBS>(JtbsChooseJtHide(pjt, nullptr, &pjt->jthk))
                        : JTBS_Nil;

                    if (jtbsHide != JTBS_Nil)
                    {
                        jtsNext = JTS_Hide;
                        jtbsNext = jtbsHide;
                    }
                    else if (fPeek)
                    {
                        jtsNext = JTS_Peek;
                        jtbsNext = JTBS_Peek_Enter;
                    }
                    else if (!fRun)
                    {
                        jtsNext = JTS_Stand;
                        jtbsNext = JTBS_Nil;
                    }
                }
                break;

            case JTS_Jump:
            {
                bool fCanJumpAction = false;

                switch (pjt->jtbs)
                {
                    case JTBS_Jump_Dive:
                    case JTBS_Jump_Spire:
                    case JTBS_Jump_Rail:
                    case JTBS_Jump_Hshape:
                    case JTBS_Jump_Hpnt:
                    case JTBS_Hide_Stand:    // Handle target (numeric state 14)
                    case JTBS_Hide_Sidestep: // Pipe target (numeric state 15)
                    break;

                    case JTBS_Jump_Out:
                    fCanJumpAction = g_clock.t - pjt->tJtbs > 0.05f;
                    break;

                    default:
                    fCanJumpAction = true;
                    break;
                }

                // Retail gives Circle landing priority over the aerial smash.
                if (fHidePressed && fCanJumpAction)
                {
                    pjoy->SetHandled(BTN_CIRCLE);
                    SetJtJtcs(pjt, JTCS_Nil);
                    jtbsNext = static_cast<JTBS>(JtbsChooseJtLanding(pjt, nullptr));
                }
                else if (fRushPressed && !fCanRush && fCanJumpAction &&
                         (GetAvailableVaultFlags() & 0x0010U) != 0)
                {
                    // With the Dive power-up selected, Triangle changes the
                    // current aerial action into the dedicated dive attack.
                    pjoy->SetHandled(BTN_TRIANGLE);
                    SetJtJtcs(pjt, JTCS_Nil);
                    jtbsNext = JTBS_Jump_Dive;
                }
                else if (fSmashPressed && fCanJumpAction &&
                         (pjt->jtbs != JTBS_Jump_Thrown || pjt->svxyAirSteer != 0.0f))
                {
                    pjoy->SetHandled(BTN_SQUARE);
                    SetJtJtcs(pjt, JTCS_Nil);
                    jtbsNext = JTBS_Jump_Smash;
                }
                else if (fBoost)
                    jtbsNext = JTBS_Jump_Boost;
                break;
            }

            case JTS_Hang:
                if (fJump)
                {
                    pjt->jtjk = JTJK_Swinging;
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Init;
                }
                break;

            case JTS_Pipe:
                if (fJump)
                {
                    pjt->jtjk = JTJK_Swinging;
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Init;
                }
                break;

            case JTS_Edge:
                if (fJump)
                {
                    pjt->jtjk = JTJK_Edge;
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Init;
                }
                break;

            case JTS_Hide:
                // Retail exits ordinary flatten/duck/nonchalant/reach hides
                // as soon as Circle is released. Basket, rail, and spire
                // hides have their own exit rules.
                if (fJump)
                {
                    pjt->jtjk = pjt->jthk == JTHK_Rail
                        ? JTJK_Swinging
                        : JTJK_Standing;
                    jtsNext = JTS_Jump;
                    jtbsNext = pjt->jthk == JTHK_Basket
                        ? JTBS_Jump_Out
                        : JTBS_Jump_Init;
                }
                else if (!fHideHeld &&
                    pjt->jthk != JTHK_Basket &&
                    pjt->jthk != JTHK_Rail &&
                    pjt->jthk != JTHK_Spire &&
                    (g_grfusr & 1U) == 0)
                {
                    jtsNext = static_cast<JTS>(JtsResetJt(pjt));
                    jtbsNext = JTBS_Nil;
                }
                break;

            case JTS_Sidestep:
                if (fJump)
                {
                    pjt->jtjk = JTJK_Standing;
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Init;
                }
                else if (fFall)
                {
                    jtsNext = JTS_Jump;
                    jtbsNext = JTBS_Jump_Fall;
                }
                else
                {
                    const JTBS jtbsHide = fHideHeld
                        ? static_cast<JTBS>(JtbsChooseJtHide(pjt, nullptr, &pjt->jthk))
                        : JTBS_Nil;

                    if (jtbsHide != JTBS_Nil)
                    {
                        jtsNext = JTS_Hide;
                        jtbsNext = jtbsHide;
                    }
                    else if (fPeek)
                    {
                        jtsNext = JTS_Peek;
                        jtbsNext = JTBS_Peek_Enter;
                    }
                    else if (!fRun)
                    {
                        jtsNext = JTS_Stand;
                        jtbsNext = JTBS_Nil;
                    }
                }
                break;

            case JTS_Peek:
            {
                // Retail treats a closed Binocucom overlay as the normal end
                // of the peek.  The overlay/dialog code owns BINOCS_None;
                // JT must consume that state and play Peek_Exit so its camera
                // policy and character constraints are released as well.
                const bool fDialogCalling =
                    g_binoc.binocs == BINOCS_Dialog &&
                    g_binoc.pdialogPlaying != nullptr &&
                    g_binoc.pdialogPlaying->dialogs == DIALOGS_Calling;

                const bool fOverlayFinished =
                    pjt->jtbs == JTBS_Peek_Peek &&
                    (g_binoc.binocs == BINOCS_None || fDialogCalling);

                if ((fPeek || fOverlayFinished) &&
                    pjt->jtbs == JTBS_Peek_Peek)
                {
                    jtbsNext = JTBS_Peek_Exit;

                    // An explicit player cancellation closes the active
                    // blot here. Normal dialog completion already popped it
                    // through the Binocucom/dialog teardown path.
                    if (fPeek)
                        PopUiActiveBlot(&g_ui);
                }
                else if (pjt->jtbs == JTBS_Peek_Enter &&
                         pjt->pasegPeekEnter == nullptr &&
                         g_clock.t - pjt->tJtbs >= 0.5f)
                {
                    jtbsNext = JTBS_Peek_Peek;
                }

                if (pjt->jtbs == JTBS_Peek_Exit &&
                    pjt->pasegPeekExit == nullptr &&
                    g_clock.t - pjt->tJtbs >= 0.25f)
                {
                    jtsNext = static_cast<JTS>(JtsResetJt(pjt));
                    jtbsNext = JTBS_Nil;
                }

                break;
            }

            default:
                break;
        }

        if (jtsNext == jtsPrevious && jtbsNext == jtbsPrevious)
            break;

        SetJtJts(pjt, jtsNext, jtbsNext);

        // A transition consumes one-frame requests; evaluate the newly
        // entered state once more only for stable follow-up transitions.
        fJump = false;
        fFall = false;
        fBoost = false;
        fPeek = false;
    }

    /*
     * The central state-transition loop remains structurally identical to
     * the decompilation. These are the DAT substitutions inside it:
     */

    if (pjt->jts == 4 && pjt->fCharmEnabled)
    {
        if (g_clock.t - pjt->tCharmPending < DT_JtCharmBeforeEnabled)
            FinishJtZap(pjt);
    }

    if (pjt->jts == 13)
    {
        if (!pjoy->IsHeld(BTN_TRIANGLE))
        {
            if (g_clock.t - pjt->tJts > DT_JtMinBall)
                SetJtJts(pjt, 13, 56);
            else
                pjt->radTarget = pjt->radJoy;
        }
        else
            pjt->radTarget = pjt->radJoy;
    }

    UpdateJtActiveCane(pjt, pjoy);

    switch (pjt->jts)
    {
        case 1:
        SetStepRunTarget(pjt, pjt->radJoy, pjt->uJoyDeflect, pjt->ablrunRun, pjt->pasegblRun);

        // Keep the target basis synchronized with the heading selected by
        // this input frame.  Waiting for the later UpdateJt pass allowed an
        // animation/constraint update to consume a stale sideways basis.
        UpdateStepMatTarget(pjt);

        break;

        case 2:
        UpdateJtActiveJump(pjt);
        break;

        case 3:
        UpdateJtActiveHang(pjt, pjoy);
        break;

        case 4:
        case 10:
        pjt->vTarget = glm::vec3(0.0f);

        if (pjt->pasegaCur != nullptr && pjt->pasegaCur->paseg->pchnStrip != nullptr)
        {
            ACP* pacp = pjt->pasegaCur->paseg->pchnStrip->pacp.get();

            if (pacp != nullptr)
            {
                glm::vec3 vTarget;
                pacp->pvtacp->pfnEvaluateAcp(pacp, pjt, pjt->pasegaCur->tLocal, pjt->pasegaCur->svtLocal, 0, nullptr, &vTarget);
                pjt->vTarget = -vTarget;
            }
        }
        break;

        case 5:
        SetJtSidestepTarget(pjt, pjt->ablssSidestep, pjt->pasegblSidestep, pjt->radJoy, pjt->uJoyDeflect);
        break;

        case 6:
        UpdateJtActiveHide(pjt, pjoy);
        break;

        case 7:
        UpdateJtActivePipe(pjt, pjoy);
        break;
    }

    UpdateJtEffect(pjt);
}

void UpdateJt(JT* pjt, float dt)
{
    UpdateStep(pjt, dt);

    // JT overrides PO's update function and therefore does not pass through
    // UpdatePo. Keep Sly's persistent backpack charm synchronized here.
    UpdatePoCharmVisibility(pjt);

    HandleJtGrfjtsc(pjt);

    if (FDrivenAlo(pjt))
        UpdateJtDrive(pjt);

    if (pjt->fFlash && pjt->jts != 4 && g_clock.t - pjt->tCharm > pjt->dtCharmFlash)
        pjt->fFlash = false;

    switch (pjt->jts)
    {
        case 0:
        UpdateJtStand(pjt);
        break;

        case 1:
        {
            AdjustStepRun(pjt, pjt->ablrunRun, pjt->pasegblRun, pjt->pasegaCur);

            ASEGA* pasega = pjt->pasegaCur;

            if (pasega != nullptr && pasega->paseg != nullptr)
            {
                float uFootfall = GModPositive((pasega->tLocal + DT_StepFootfallOffset * pasega->svtLocal) / pasega->paseg->tMax, 1.0f);
                UpdateStepFootfall(pjt, uFootfall, &pjt->sffRun);
            }

            break;
        }

        case 2:
        UpdateJtJump(pjt);
        break;

        case 4:
        UpdateJtZap(pjt);
        break;

        case 5:
        {
            AdjustJtSidestep(pjt, pjt->ablssSidestep, pjt->pasegblSidestep);

            ASEGA* pasega = pjt->pasegaCur;

            if (pasega != nullptr && pasega->paseg != nullptr)
            {
                float uFootfall = GModPositive((pasega->tLocal + DT_StepFootfallOffset * pasega->svtLocal) / pasega->paseg->tMax, 1.0f);
                UpdateStepFootfall(pjt, uFootfall, &pjt->sffSidestep);
            }

            break;
        }

        case 6:
        UpdateJtHide(pjt);
        break;

        case 8:
        {
            pjt->dposEdge = PosSmooth(pjt->dposEdge, glm::vec3(0.0f), dt, &s_smpEdge, nullptr);

            glm::vec3 posEdgeWorld;
            ConvertAloPos(pjt->psoEdgeCur, nullptr, &pjt->posEdgeLocal, &posEdgeWorld);

            if (!FCheckJtEdgeGrab(pjt, pjt->psoEdgeCur, &posEdgeWorld, pjt->xpkEdge, pjt->pvEdge, &pjt->normalEdgeCur))
            {
                SetJtJts(pjt, 2, 2);
                break;
            }

            pjt->radTarget = std::atan2(-pjt->normalEdgeCur.y, -pjt->normalEdgeCur.x);
            break;
        }

        case 9:
        {
            glm::vec3 posTarget = pjt->plockgCur != nullptr ? pjt->plockgCur->xf.posWorld : g_pcm->pos;
            glm::vec3 dposTarget = posTarget - pjt->xf.posWorld;
            pjt->radTarget = std::atan2(dposTarget.y, dposTarget.x);
            break;
        }

        case 12:
        // The look-around policy can remain queued during Peek_Enter. Its pan
        // is not valid until Peek_Peek begins, so preserve Sly's entry facing.
        if (pjt->jtbs == JTBS_Peek_Peek)
            pjt->radTarget = g_pcm->cplook.radPan;
        break;

        case 13:
        {
            if (pjt->jtbs == 56)
            {
                bool fUpright = pjt->xf.mat[2][2] > Z_JtBallRecoverMin;
                bool fStoppedRotating = glm::length(pjt->xf.w) < SW_JtBallRecoverMax;

                if (fUpright && fStoppedRotating)
                    SetJtJts(pjt, 2, 2);
            }
            else if (pjt->jtbs == 54 && pjt->fBaseXp != 0)
                SetJtJts(pjt, 13, 55);

            break;
        }
    }

    UpdateJtCane(pjt);
    UpdateStepMatTarget(pjt);
    CheckJtUnhook(pjt);
    UpdateJtWater(pjt);
    ResolveAlo(pjt);

    /*
     * While the cane is reaching, test its hook polyline against the
     * current pipe or handle target.
     */
    if (pjt->jtcs == 0)
    {
        if (pjt->phndCur == nullptr)
        {
            PIPE* ppipe = pjt->ppipeCur;

            if (ppipe != nullptr && pjt->cposHook > 1)
            {
                int cposHook = pjt->cposHook;

                std::vector<glm::vec3> aposHookWorld(cposHook);
                std::vector<glm::vec3> aposPipeWorld(cposHook);

                for (int i = 0; i < cposHook; ++i)
                {
                    ConvertAloPos(nullptr, ppipe->paloParent, &pjt->aposHook[i], &aposHookWorld[i]);

                    CRV* pcrv = ppipe->pcrv.get();

                    if (pcrv != nullptr && pcrv->pvtcrv->pfnFindCrvClosestPointFromS != nullptr)
                        pcrv->pvtcrv->pfnFindCrvClosestPointFromS(pcrv, &aposHookWorld[i], pjt->sPipeCur, nullptr, &aposPipeWorld[i], nullptr, nullptr, nullptr);
                }

                for (int i = 1; i < cposHook; ++i)
                {
                    float au[2];
                    float distance;
                    glm::vec3 normal;

                    FindClosestPointBetweenLineSegments(&aposHookWorld[i - 1], &aposHookWorld[i], &aposPipeWorld[i - 1], &aposPipeWorld[i], au, &distance, &normal);

                    if (distance < pjt->sRadiusHook)
                    {
                        SetJtJts(pjt, 7, 23);
                        break;
                    }
                }
            }
        }
        else if (pjt->ptargetCur != nullptr && pjt->cposHook > 1)
        {
            glm::vec3 posHandle;
            GetXfmPos(pjt->phndCur, &posHandle);

            for (int i = 1; i < pjt->cposHook; ++i)
            {
                float u;
                float distance;

                FindClosestPointOnLineSegment(&posHandle, &pjt->aposHook[i - 1], &pjt->aposHook[i], &u, &distance);

                if (distance < pjt->ptargetCur->sRadiusTarget + pjt->sRadiusHook)
                {
                    SetJtJts(pjt, 3, 51);
                    break;
                }
            }
        }
    }

    /*
     * Process the zap contacts accumulated in pxpZap.
     */
    XP* pxp = pjt->pxpZap;

    while (pxp != nullptr)
    {
        int ixpdOther = pxp->axpd[0].psoRoot == pjt ? 1 : 0;
        SO* pso = pxp->axpd[ixpdOther].psoLeaf;

        while (pso != nullptr)
        {
            int zok = static_cast<int>(pso->zok);

            if (zok != 0)
            {
                if (zok == 1 || zok == 4)
                {
                    ZPR zpr;
                    InitZpr(&zpr, (ZPK)0, pso);

                    SO* psoZap = pso;

                    while (psoZap != nullptr && !psoZap->pvtso->pfnFInflictSoZap(psoZap, pxp, &zpr))
                        psoZap = static_cast<SO*>(psoZap->paloParent);

                    FTakeJtDamage(pjt, &zpr);
                }

                break;
            }

            pso = static_cast<SO*>(pso->paloParent);
        }

        pxp = pxp->pxpNext;
    }

    if (pjt->pxpZap != nullptr)
    {
        FreeSwXpList(pjt->psw, pjt->pxpZap);
        pjt->pxpZap = nullptr;
    }

    UpdateJtTool(pjt);
    UpdateJtBalance(pjt);
    UpdateJtJumpTargetHistory(pjt);
    UpdateJtPendingInteractions(pjt);
}

void UpdateJtDrive(JT* pjt)
{
    if (pjt->jts == 3)
        SetJtJts(pjt, static_cast<JTS>(2), static_cast<JTBS>(2));
    else if (pjt->jts == 13)
        SetJtJts(pjt, static_cast<JTS>(13), static_cast<JTBS>(56));

    const int jts = pjt->jts;

    SetJtJtcs(pjt, static_cast<JTCS>(-1));

    switch (jts)
    {
        case 2:
        case 3:
        case 13:
        case 14:
        case 15:
        case 16:
        return;

        case 4:
        case 12:
        pjt->vTarget = glm::vec3(0.0f);
        return;
    
        default:
        break;
    }

    float radTarget = 0.0f;
    float svTarget = 0.0f;

    CalculateAloDrive(pjt, nullptr, nullptr, g_clock.dt, pjt->radTarget, &radTarget, nullptr, &svTarget);

    pjt->radTarget = radTarget;
    // The retail _pextlw/_pcpyld sequence packs -svTarget into the first
    // vector component.  Sly's authored forward direction is local -X.
    pjt->vTarget = glm::vec3(-svTarget, 0.0f, 0.0f);

    SetJtJts(pjt, static_cast<JTS>(svTarget > 0.0f), static_cast<JTBS>(-1));
}

void ChooseJtPhys(JT* pjt, SO* pso)
{
    if (pjt->jts == 13)
        return;

    if (pjt->jts == 4)
    {
        int jtbs = pjt->jtbs;

        if (jtbs != -1)
        {
            pso = pjt->mpjtbspso[jtbs + 4];

            while (pso == nullptr)
            {
                jtbs = s_mpjtbsjtbsNext[jtbs - 36];

                if (jtbs == -1)
                    break;

                pso = pjt->mpjtbspso[jtbs + 4];
            }
        }
    }
    else if (pjt->jts == 2 && pjt->jtbs >= 12 && pjt->jtbs < 14)
        pso = pjt->mpjtbspso[25];
    else if (pjt->jts == 6)
    {
        if ((pjt->jthk == 0 && pjt->jtbs == 21) || pjt->jthk == 7)
            pso = pjt->mpjtbspso[25];
        else
            pso = pjt->mpjthkpso[pjt->jthk + 2];
    }
    else
    {
        pso = pjt->mpjtspso[pjt->jts];
    }

    if (pso == nullptr)
        pso = pjt->mpjtspso[0];

    SetStepPhys(static_cast<STEP*>(pjt), pso, 0);
}
void EnableJtActadj(JT* pjt, int grfjta)
{
    if (pjt->pactadjGut == nullptr)
        return;

    const ACK ackUpperBody = (grfjta & 1U) != 0 ? ACK_SmoothNoLock : ACK_Nil;
    const ACK ackLowerBody = (grfjta & 2U) != 0 ? ACK_SmoothNoLock : ACK_Nil;

    pjt->pactadjGut->ackPos = ackUpperBody;
    pjt->pactadjLeftHand->ackPos = ackUpperBody;
    pjt->pactadjRightHand->ackPos = ackUpperBody;
    pjt->pactadjLeftFoot->ackPos = ackLowerBody;
    pjt->pactadjRightFoot->ackPos = ackLowerBody;

    pjt->paloGut->pvtalo->pfnRecacheAloActList(pjt->paloGut);
    pjt->paloLeftHand->pvtalo->pfnRecacheAloActList(pjt->paloLeftHand);
    pjt->paloRightHand->pvtalo->pfnRecacheAloActList(pjt->paloRightHand);
    pjt->paloLeftFoot->pvtalo->pfnRecacheAloActList(pjt->paloLeftFoot);
    pjt->paloRightFoot->pvtalo->pfnRecacheAloActList(pjt->paloRightFoot);
}

void SetJtJts(JT* pjt, int jts, int jtbs)
{
    if (pjt->jts == jts && pjt->jtbs == jtbs) return;

    float tLocal = 0.0f, svtLocal = 1.0f;

    int grfAseg = 1, grfjta = 0, oidInitialState = -1, fKeepAseg = 0, sfxid = -1;

    ASEGBL* pasegbl = nullptr; SM* psm = nullptr;

    pjt->grfjtsc &= ~1u;
    if (pjt->jts != jts)
    {
        switch (pjt->jts)
        {
            case 0:

            if (pjt->jtcs == 7)
                SetJtJtcs(pjt, 5);
            else if (pjt->jtcs == 6)
                SetJtJtcs(pjt, 4);
            else if (pjt->jtcs == 8 || pjt->jtcs == 9)
                SetJtJtcs(pjt, -1);
            break;

            case 1:
            break;

            case 2:
            {
                pjt->ploJumpSource = nullptr;

                if (jts != 3 && jts != 7)
                    StartSound((SFXID)73, nullptr, pjt, nullptr, 1000.0f, 500.0f, U_JtSoundPrimary, 0.0f, 0.0f, nullptr, nullptr);

                pjt->jtjk = -1;

                if (pjt->jtcs == 0 && jts != 3 && jts != 7)
                    SetJtJtcs(pjt, -1);

                if (pjt->pwater == nullptr)
                {
                    EXPL* pexplLanding = nullptr;

                    if (((jts == 0) || (jts == 1) || (jts == 5) || (jts == 9) || (jts == 10) || (jts == 12) || (jts == 6 && pjt->jthk != 2 && pjt->jthk != 4 && pjt->jthk != 5)))
                        pexplLanding = pjt->pexplJumpLand;
                    else if (jts == 6 && (pjt->jthk == 4 || pjt->jthk == 5))
                        pexplLanding = pjt->pexplHideJumpLand;

                    if (pexplLanding != nullptr)
                        ExplodeExpl(pexplLanding);
                }

                if ((GetAvailableVaultFlags() & 0x1000) == 0)
                    pjt->fSlowDownClock = 0;

                break;
            }

            case 3:
            if (jts == 2)
                pjt->ploJumpSource = pjt->phndCur;

            HandleLoSpliceEvent(pjt->phndCur, 3, 0, nullptr);
            pjt->phndCur->pvtlo->pfnUnsubscribeLoObject(pjt->phndCur, pjt);
            pjt->phndUnhook = pjt->phndCur;
            SetJtJtcs(pjt, -1);

            pjt->pactvalHangHand->ackPos = ACK_Nil;
            pjt->pactvalHangHand->ackRot = ACK_Nil;
            pjt->paloRightHand->pvtalo->pfnRecacheAloActList(pjt->paloRightHand);
            pjt->pactvalHangTool->ackRot = ACK_Nil;
            pjt->paloTool->pvtalo->pfnRecacheAloActList(pjt->paloTool);
            LimitStepHands(pjt, 1);

            if (pjt->pxaHang != nullptr)
            {
                RemoveSoXa(pjt->pxaHang->psoTarget, pjt->pxaHang.get());
                FreeSwXaList(pjt->psw, pjt->pxaHang);
                pjt->pxaHang = nullptr;
            }

            ClearSqtrm(&pjt->sqtrmCane);
            break;

            case 4:
            pjt->zpk = -1;

            if (pjt->ptailTail != nullptr)
                pjt->ptailTail->dvGravity = glm::vec3(0.0f);

            if (pjt->ptailMask != nullptr)
                pjt->ptailMask->dvGravity = glm::vec3(0.0f);

            pjt->pzi.fCollectEnabled = 1;
            ClearSqtrm(&pjt->sqtrmCane);
            pjt->tCharm = g_clock.t;
            break;

            case 5:
            pjt->sslfCur = SSLF_Nil;
            break;

            case 6:
            {
                // The retail jump table covers hide kinds 2 through 7.  These actions
                // are expanded explicitly here rather than retaining the indirect
                // decompiler jump.
                switch (pjt->jthk)
                {
                    case 2:
                    if (pjt->pasegaCur != nullptr)
                    {
                        RetractAsega(pjt->pasegaCur);
                        pjt->pasegaCur = nullptr;
                    }
                    else if (pjt->psmaCur != nullptr)
                    {
                        RetractSma(pjt->psmaCur);
                        pjt->psmaCur = nullptr;
                    }

					StartSound((SFXID)77, nullptr, pjt, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                    // Original release switch target 0x00175554. The call at
                    // 0x001755c8 receives raw state 0 in its delay slot.
                    SetHbskHbsks(pjt->phbsk, (HBSKS)0);
                    break;

                    case 4:
                    HandleLoSpliceEvent(pjt->prailCur, 3, 0, nullptr);
                    pjt->prailCur->pvtlo->pfnUnsubscribeLoObject(pjt->prailCur, pjt);
                    UpdateJtHookOx(pjt, pjt->prailCur, 0);
                    break;

                    case 5:
                    HandleLoSpliceEvent(pjt->pspireCur, 3, 0, nullptr);
                    pjt->pspireCur->pvtlo->pfnUnsubscribeLoObject(pjt->pspireCur, pjt);
                    UpdateJtHookOx(pjt, pjt->pspireCur, 0);
                    break;

                    case 6:
                    if (pjt->jtcs == 0 && jts != 7 && jts != 3)
                        SetJtJtcs(pjt, -1);
                    break;

                    case 7:
                    pjt->radTarget = RadNormalize(std::atan2(pjt->pvaultCur->xf.matWorld[0][1], pjt->pvaultCur->xf.matWorld[0][0]) + 1.0f);
                    pjt->pvaultCur = nullptr;
                    break;
                }

                // The original basket branch retains phbsk. JTBS_Jump_In/Out
                // owns the pointer and clears it when that transition ends.
                if (pjt->jthk != 2)
                    pjt->phbsk = nullptr;

                if (jts == 2)
                {
                    if (pjt->phshape != nullptr)
                        pjt->ploJumpSource = pjt->phshape;
                    else if (pjt->phpnt != nullptr)
                        pjt->ploJumpSource = pjt->phpnt;
                    else if (pjt->pspireCur != nullptr)
                        pjt->ploJumpSource = pjt->pspireCur;
                    else
                        pjt->ploJumpSource = pjt->prailCur;
                }

                pjt->jthk = -1;
                pjt->sslfCur = SSLF_Nil;
                pjt->phshape = nullptr;
                pjt->phpnt = nullptr;
                pjt->pspireCur = nullptr;
                pjt->prailCur = nullptr;

                if (pjt->ptnCur != nullptr)
                {
                    SetTnTnsOverride(pjt->ptnCur, TNS_Out);
                    pjt->ptnCur = nullptr;
                }

                pjt->jtfock = JTFOCK_Relax;

                if (pjt->pexcHide != nullptr)
                {
                    UnsetExcitement(pjt->pexcHide);
                    pjt->pexcHide = nullptr;
                }

                if (pjt->pemitterShadow != nullptr) pjt->pemitterShadow->pvtlo->pfnRemoveLo(pjt->pemitterShadow);
                break;
            }

            case 7:
            if (jts == 2)
                pjt->ploJumpSource = pjt->ppipeCur;

            HandleLoSpliceEvent(pjt->ppipeCur, 3, 0, nullptr);
            pjt->ppipeCur->pvtlo->pfnUnsubscribeLoObject(pjt->ppipeCur, pjt);
            UpdateJtHookOx(pjt, pjt->ppipeCur, 0);
            pjt->ppipeUnhook = pjt->ppipeCur;
            pjt->sPipeUnhook = pjt->sPipeCur;
            SetJtJtcs(pjt, jts == 2 && pjt->pasegblPipeExit != nullptr ? 11 : -1);
            pjt->dposSmoothCpdefi = pjt->posBasePrev - pjt->xf.pos;
            pjt->dposSmoothCpdefi.z = 0.0f;
            ClearSqtrm(&pjt->sqtrmCane);
            break;

            case 8:
            UpdateJtHookOx(pjt, pjt->psoEdgeCur, 0);
            pjt->psoEdgeCur = nullptr;
            break;

            case 9:
            pjt->plockgCur = nullptr;
            pjt->pvtpo->pfnSetPoPck(pjt, PCK_Key);
            break;

            case 10:
            pjt->ptargetCur = nullptr;
            pjt->phndCur = nullptr;
            if (pjt->psoRushEffect != nullptr) pjt->psoRushEffect->pvtlo->pfnRemoveLo(pjt->psoRushEffect);
            break;

            case 11:
            SetSoConstraints(pjt, CT_Free, nullptr, CT_Locked, nullptr);
            SetSoNoXpsAll(pjt, 0);
            break;

            case 12:
            RevokeCmPolicy(g_pcm, 15, CPP_LookAround, &g_pcm->cplook,
                g_pcm->acpr[0].psoFocus, nullptr);

            if (g_binoc.fActive != 0)
                PopUiActiveBlot(&g_ui);
            break;

            case 13:
            pjt->fSphere = 0;
            pjt->plvo = pjt->plvoSave;

            if (pjt->mpjtspso[13] != nullptr)
                SetStepPhys(pjt, pjt->mpjtspso[13], 1);
            break;
        }

        switch (jts)
        {
            case 0:
            oidInitialState = 326;
            grfjta = 3;
            psm = pjt->psmIdle;
            pjt->fStandTight = 0;

            if (pjt->jts == 2)
            {
                int landingKind = pjt->jsglk;

                if (landingKind == -1)
                {
                    float dtBigLanding = pjt->jtbs == 3 ? 1.25f : 0.95f;
                    landingKind = dtBigLanding < g_clock.t - pjt->tJts;
                }

                if (landingKind == 1 && IsmsFindSmOptional(psm, (OID)330))
                    oidInitialState = 330;
                else if (IsmsFindSmOptional(psm, (OID)329))
                    oidInitialState = 329;

            }

            pjt->vTarget = glm::vec3(0.0f);
            break;

            case 1:
            pasegbl = pjt->pasegblRun;
            break;

            case 2:
            if (pjt->jtjk == -1)
                pjt->jtjk = 0;

            pjt->jsglk = -1;
            pjt->fCanBoost = 0;
            pjt->tCancelJump = G_MAX_FLOAT;
            break;

            case 3:
            {
                pjt->jtbsHangLatch = -1;
                pjt->uSwingForward = 1.0f;
                pjt->uSwingActive = 1.0f;
                pjt->tHangDeflectLast = g_clock.t;
                pasegbl = pjt->pasegblSwing;
                tLocal = 0.5f;

                LimitStepHands(pjt, 0);
                pjt->pactvalHangHand->ackPos = ACK_Velocity;
                pjt->pactvalHangHand->ackRot = ACK_Velocity;
                pjt->paloRightHand->pvtalo->pfnRecacheAloActList(pjt->paloRightHand);
                pjt->pactvalHangTool->ackRot = ACK_Velocity;
                pjt->paloTool->pvtalo->pfnRecacheAloActList(pjt->paloTool);

                glm::vec3 posHangHand;
                glm::mat3 matHangHand;
                glm::mat3 matHangTool;
                UpdateJtIkHang(pjt, &posHangHand, &matHangHand, &matHangTool);
                pjt->dposHangHand = pjt->paloRightHand->xf.pos - posHangHand;
                CalculateDmat(&matHangHand, &pjt->paloRightHand->xf.mat, &pjt->dmatHangHand);
                CalculateDmat(&matHangTool, &pjt->paloTool->xf.mat, &pjt->dmatHangTool);

                HandleLoSpliceEvent(pjt->phndCur, 2, 0, nullptr);
                pjt->phndCur->pvtlo->pfnSubscribeLoObject(pjt->phndCur, pjt);

                float radForward;
                GetJtHangHeading(pjt, &radForward);
                float radBackward = RadNormalize(radForward + glm::pi<float>());
                int hndk = pjt->phndCur != nullptr ? pjt->phndCur->hndk : 1;

                if (hndk == 1 || (hndk == 0 && std::abs(RadNormalize(radForward - pjt->radJoy)) < glm::half_pi<float>()))
                {
                    pjt->fHangReverse = 0;
                    pjt->radTarget = radForward;
                }
                else if (hndk == 2 || hndk == 0)
                {
                    pjt->fHangReverse = 1;
                    pjt->radTarget = radBackward;
                }

                sfxid = pjt->phndCur->sfxidGrab;
                SetJtJtcs(pjt, 1);

                for (SO* palo = (SO*)pjt->phndCur->paloParent; palo != nullptr; palo = (SO*)palo->paloParent)
                {
                    if ((palo->pvtalo->grfcid & 2) == 0)
                        continue;

                    bool fDrivenPosition = palo->constrForce.ct  != CT_Locked && palo->pactPos != nullptr && palo->pactPos->ackPos == ACK_Spring;
                    bool fDrivenRotation = palo->constrTorque.ct != CT_Locked && palo->pactRot != nullptr && palo->pactRot->ackRot == ACK_Spring;

                    if (!fDrivenPosition && !fDrivenRotation)
                        continue;

                    pjt->pxaHang = PxaAllocSw(pjt->psw);
                    pjt->pxaHang->psoSource = pjt;
                    pjt->pxaHang->psoTarget = static_cast<SO*>(palo);
                    AddSoXa(static_cast<SO*>(palo), pjt->pxaHang.get());
                    break;
                }

                pjt->vTarget = glm::vec3(0.0f);
                CalculateJtHangAccel(pjt);
                break;
            }

            case 4:
            if (pjt->ptailTail != nullptr)
                pjt->ptailTail->dvGravity = s_dvTailInWater;

            if (pjt->ptailMask != nullptr)
                pjt->ptailMask->dvGravity = s_dvTailInWater;

            pjt->vTarget = glm::vec3(0.0f);
            SetJtJtcs(pjt, -1);
            pjt->pzi.fCollectEnabled = 0;
            pjt->tCharm = G_MAX_FLOAT;
            break;

            case 5:
            pasegbl = pjt->pasegblSidestep;
            break;

            case 6:
            {
                bool fSetHideExcitement = false;

                switch (pjt->jthk)
                {
                    case 0:
                    case 1:
                    fSetHideExcitement = true;

                    if (pjt->jts != 2 || pjt->jtbs < 12 || pjt->jtbs > 13)
                        StartSound((SFXID)148, nullptr, pjt, &pjt->xf.posWorld, 3000.0f, 300.0f, U_JtSoundPrimary, 0.0f, 0.0f, nullptr, nullptr);
                    break;

                    case 2:
                    if (pjt->pasegaCur != nullptr)
                    {
                        RetractAsega(pjt->pasegaCur);
                        pjt->pasegaCur = nullptr;
                    }
                    else if (pjt->psmaCur != nullptr)
                    {
                        RetractSma(pjt->psmaCur);
                        pjt->psmaCur = nullptr;
                    }

                    fSetHideExcitement = true;
                    StartSound((SFXID)77, nullptr, pjt, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                    SetHbskHbsks(pjt->phbsk, (HBSKS)1);
                    break;
                    
                    case 3:
                    fSetHideExcitement = pjt->pemitterShadow != nullptr;

                    if (pjt->pemitterShadow != nullptr)
                        if (pjt->pemitterShadow != nullptr) pjt->pemitterShadow->pvtlo->pfnAddLo(pjt->pemitterShadow);

                    if (FJtSafeFromStepguards(pjt))
                        StartSound((SFXID)148, nullptr, pjt, &pjt->xf.posWorld, 3000.0f, 300.0f, U_JtSoundPrimary, 0.0f, 0.0f, nullptr, nullptr);
                    else
                        StartSound((SFXID)126, nullptr, pjt, &pjt->xf.posWorld, 3000.0f, 300.0f, U_JtSoundSecondary, 0.0f, 0.0f, nullptr, nullptr);
                    break;

                    case 4:
                    {
                
                        grfjta = 1;
                        HandleLoSpliceEvent(pjt->prailCur, 2, 0, nullptr);
                        pjt->prailCur->pvtlo->pfnSubscribeLoObject(pjt->prailCur, pjt);
                        UpdateJtHookOx(pjt, pjt->prailCur, 1);
                        pjt->jtMoveDir = -1;

                        glm::vec3 tangentLocal(0.0f);
                        pjt->prailCur->pcrv->pvtcrv->pfnEvaluateCrvFromU(pjt->prailCur->pcrv.get(), pjt->uRailCur, nullptr, &tangentLocal);

                        glm::vec3 tangentWorld;
                        ConvertAloVec(pjt->prailCur->paloParent, nullptr, &tangentLocal, &tangentWorld);
                        tangentWorld.z = 0.0f;

                        if (glm::length2(tangentWorld) > 0.00000001f)
                            tangentWorld = glm::normalize(tangentWorld);
                        else
                            tangentWorld = g_normalX;

                        pjt->fRailReverse = glm::dot(tangentWorld, pjt->xf.matWorld[0]) < DOT_JtRailReverse;
                        break;
                    }

                    case 5:
                    grfjta = 1;
                    HandleLoSpliceEvent(pjt->pspireCur, 2, 0, nullptr);
                    pjt->pspireCur->pvtlo->pfnSubscribeLoObject(pjt->pspireCur, pjt);
                    UpdateJtHookOx(pjt, pjt->pspireCur, 1);
                    break;

                    case 7:
                    {
                        fSetHideExcitement = true;
                        glm::vec3 zero(0.0f);
                        pjt->pvtso->pfnSetSoVelocityVec(pjt, &zero);
                        pjt->pvtso->pfnSetSoAngularVelocityVec(pjt, &zero);
                        pjt->vTarget = zero;
                        SwitchToPo(pjt->pvaultCur);
                        break;
                    }
                }

                pjt->hfdk = -1;

                if (fSetHideExcitement)
                    pjt->pexcHide = PexcSetExcitement(-20);
                break;
            }

            case 7:
            {
                HandleLoSpliceEvent(pjt->ppipeCur, 2, 0, nullptr);
                pjt->ppipeCur->pvtlo->pfnSubscribeLoObject(pjt->ppipeCur, pjt);
                UpdateJtHookOx(pjt, pjt->ppipeCur, 1);
                pjt->jtMoveDir = -1;

                CRV* pcrv = pjt->ppipeCur->pcrv.get();

                if (pjt->ppipeCur->unkInt0 == 0)
                    pjt->fPipeReverse = 0;
                else if (pjt->ppipeCur->unkInt0 == 1)
                    pjt->fPipeReverse = 1;
                else
                {
                    glm::vec3 posForwardLocal;
                    glm::vec3 posBackwardLocal;
                    pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand + DS_JtPipeProbe, &posForwardLocal, nullptr);
                    pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand - DS_JtPipeProbe, &posBackwardLocal, nullptr);
                    ConvertAloPos(pjt->ppipeCur->paloParent, nullptr, &posForwardLocal, &posForwardLocal);
                    ConvertAloPos(pjt->ppipeCur->paloParent, nullptr, &posBackwardLocal, &posBackwardLocal);

                    glm::vec3 direction = posForwardLocal - posBackwardLocal;
                    direction = glm::length2(direction) > 0.00000001f ? glm::normalize(direction) : g_normalZ;
                    pjt->fPipeReverse = direction.z < -DZ_JtPipeReverse;
                }

                pjt->sPipeHand = pjt->sPipeCur;

                if (pcrv->fClosed == 0)
                {
                    float sMin = pcrv->mpicvs[0] + (pjt->fPipeReverse ? DS_JtPipeLimitHand : DS_JtPipeLimitFoot);
                    float sMax = pcrv->mpicvs[pcrv->ccv - 1] - (pjt->fPipeReverse ? DS_JtPipeLimitFoot : DS_JtPipeLimitHand);
                    pjt->sPipeHand = glm::clamp(pjt->sPipeHand, sMin, sMax);
                }

                glm::vec3 posPipeHand;
                pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand, &posPipeHand, nullptr);
                ConvertAloPos(pjt->ppipeCur->paloParent, nullptr, &posPipeHand, &posPipeHand);

                glm::vec3 posHand;
                ConvertAloPos(pjt, nullptr, &pjt->posPipeHandLocal, &posHand);
                pjt->dposPipeXp = posPipeHand - posHand;
                pjt->dposSmoothCpdefi = pjt->posBasePrev - posPipeHand;
                sfxid = pjt->ppipeCur->sfxidGrab;
                break;
            }

            case 8:
            {
                SetJtJtcs(pjt, -1);
                sfxid = 89;
                UpdateJtHookOx(pjt, pjt->psoEdgeCur, 1);

                glm::vec3 posEdge;
                ConvertAloPos(pjt->psoEdgeCur, nullptr, &pjt->posEdgeLocal, &posEdge);

                glm::mat3 matEdge(1.0f);
                matEdge[0] = -pjt->normalEdgeCur;
                GetNormalVectors(&matEdge[0], &matEdge[1], &g_normalZ, &g_normalZ);
                glm::vec3 posTarget = posEdge - matEdge * pjt->posEdgeTarget;
                pjt->dposEdge = pjt->xf.posWorld - posTarget;
                pjt->vTarget = glm::vec3(0.0f);
                pjt->dposPipeXp = glm::vec3(0.0f);
                pasegbl = (ASEGBL*)pjt->pasegEdge;
                break;
            }

            case 9:
            {
                glm::vec3 posTarget = pjt->plockgCur != nullptr ? pjt->plockgCur->xf.posWorld : g_pcm->pos;
                glm::vec3 dpos = posTarget - pjt->xf.posWorld;
                pjt->radTarget = std::atan2(dpos.y, dpos.x);
                FixStepAngularVelocity(pjt);
                pjt->vTarget = glm::vec3(0.0f);
                SetJtJtcs(pjt, -1);
                break;
            }

            case 10:
            SetJtJtcs(pjt, -1);
            pjt->cpaloIgnoreEffect = 0;
            ChooseJtRushTarget(pjt);
            break;

            case 11:
            SetJtJtcs(pjt, -1);
            SetSoConstraints(pjt, CT_Locked, nullptr, CT_Locked, nullptr);
            SetSoNoXpsAll(pjt, 1);
            pjt->fBaseXp = 0;
            pjt->vTarget = glm::vec3(0.0f);
            break;

            case 12:
            pjt->vTarget = glm::vec3(0.0f);
            break;

            case 13:
            pjt->plvoSave = pjt->plvo;
            pjt->fSphere = 1;
            pjt->sRadiusSelf = S_JtBallRadius;
            pjt->plvo = nullptr;

            if (pjt->mpjtspso[13] != nullptr)
                SetStepPhys(pjt, nullptr, 0);

            pjt->vTarget = glm::vec3(0.0f);
            SetJtJtcs(pjt, -1);
            break;

            case 14:
            if (FIsLoInWorld(pjt->pdecoy))
                DamageJtDecoy(pjt->pdecoy, pjt, 1);

            if (pjt->pdecoy != nullptr) pjt->pdecoy->pvtlo->pfnAddLo(pjt->pdecoy);
            pjt->pdecoy->pvtlo->pfnSetLoParent(pjt->pdecoy, pjt);
            if (pjt->pdecoy != nullptr) pjt->pdecoy->pvtlo->pfnRemoveLo(pjt->pdecoy);
            pasegbl = (ASEGBL*)pjt->pasegDecoy;
            pjt->vTarget = glm::vec3(0.0f);
            break;

            case 15:
            pasegbl = (ASEGBL*)pjt->pasegStun;
            pjt->vTarget = glm::vec3(0.0f);
            break;

            case 16:
            pjt->vTarget = glm::vec3(0.0f);
            break;
        }

        CheckJtUnhook(pjt);
    }
    if (pjt->jtbs != jtbs)
    {
        switch (pjt->jtbs)
        {
            case 4:
            if (!(jts == 6 && pjt->jthk == 2))
            {
                HBSK* phbsk = pjt->phbsk;
                pjt->phbsk = nullptr;
                if (phbsk != nullptr)
                    RecalcSwOxfFilterForObject(phbsk->psw, phbsk);
            }

            pjt->pactvalJump->ackPos = ACK_Nil;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            pjt->vTarget = glm::vec3(0.0f);
            break;

            case 5:
            {
                HBSK* phbsk = pjt->phbsk;
                pjt->phbsk = nullptr;
                if (phbsk != nullptr)
                    RecalcSwOxfFilterForObject(phbsk->psw, phbsk);
                break;
            }

            case 7:
            case 8:
            if (pjt->jtcs == -1)
            {
                pjt->ptargetCur = nullptr;
                pjt->phndCur = nullptr;
            }

            if (pjt->psoSmashEffect != nullptr) pjt->psoSmashEffect->pvtlo->pfnRemoveLo(pjt->psoSmashEffect);
            if (pjt->psoDiveEffect != nullptr)  pjt->psoDiveEffect->pvtlo->pfnRemoveLo(pjt->psoDiveEffect);
            break;

            case 9:
            if (pjt->pjmtCur != nullptr)
            {
                if (pjt->pjmtCur->psoSetIgnoreJt != nullptr)
                    pjt->pjmtCur->psoSetIgnoreJt->zok = ZOK_Inherit;

                pjt->pjmtCur = nullptr;
            }

            pjt->pactvalJump->ackPos = ACK_Nil;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            pjt->vTarget = glm::vec3(0.0f);
            break;

            case 10:
            if (!(jts == 6 && pjt->jthk == 5))
                pjt->pspireCur = nullptr;

            pjt->pactvalJump->ackPos = ACK_Nil;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            break;

            case 11:
            if (!(jts == 6 && pjt->jthk == 4))
                pjt->prailCur = nullptr;

            pjt->pactvalJump->ackPos = ACK_Nil;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            break;

            case 12:
            if (!(jts == 6 && pjt->phshape != nullptr && pjt->jthk == pjt->phshape->jthk))
                pjt->phshape = nullptr;

            pjt->pactvalJump->ackPos = ACK_Nil;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            break;

            case 13:
            if (!(jts == 6 && pjt->phpnt != nullptr && pjt->jthk == pjt->phpnt->jthk))
                pjt->phpnt = nullptr;

            pjt->pactvalJump->ackPos = ACK_Nil;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            break;

            case 14:
            case 15:
            pjt->pactvalJump->ackPos = ACK_Nil;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            break;

            case 17:
            pjt->sslfCur = SSLF_Nil;
            break;

            case 23:
            if (jts == 7)
                SetJtJtcs(pjt, pjt->pasegblPipeEnter != nullptr ? 10 : 2);
            break;

            case 28:
            if (pjt->ptnCur != nullptr)
            {
                SetTnTnsOverride(pjt->ptnCur, TNS_Out);
                pjt->ptnCur = nullptr;
            }
            break;

            case 34:
            SetMvgkRvol(1.0f, 5, 0);
            SetMvgkRvol(1.0f, 5, 1);
            break;

            case 39:
            case 47:
            if (pjt->ptailTail != nullptr)
                pjt->ptailTail->dvGravity = glm::vec3(0.0f);
            break;

            case 44:
            SetSoNoXpsAll(pjt, 0);
            pjt->jtJumpTarget = {};
            pjt->pactvalJump->ackPos = ACK_Nil;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            break;

            case 55:
            if (pjt->psoBallEffect != nullptr) pjt->psoBallEffect->pvtlo->pfnRemoveLo(pjt->psoBallEffect);
            break;
        }

        switch (jtbs)
        {
            case 0:
            {
                pjt->fShortenJump = 0;
                pjt->fCanBoost = 1;
                pjt->jtjj = 0;
                pjt->mpjtjjt[0] = g_clock.t;
                pjt->mpjtjjt[1] = g_clock.t;
                pjt->mpjtjjt[2] = g_clock.t;
                pjt->fBaseXp = 0;
                pjt->svxyAirSteer = SVXY_JtAirSteer * std::sqrt(pjt->rGravity);

                StartSound((SFXID)79, nullptr, pjt, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

                glm::vec3 velocity;
                GetJtJumpVelocity(pjt, &velocity);

                switch (pjt->jtjk)
                {
                    case 0:
                    pasegbl = (ASEGBL*)pjt->pasegJumpStanding;
                    break;
                    case 1:
                    pasegbl = (ASEGBL*)pjt->pasegJumpSwinging;
                    break;
                    case 2:
                    pasegbl = (ASEGBL*)pjt->pasegJumpRunning;
                    break;
                    case 3:
                    pasegbl = (ASEGBL*)pjt->pasegJumpSwinging;
                    pjt->svxyAirSteer *= R_JtFromWaterJumpAirSteer;
                    break;
                    case 4:
                    pasegbl = (ASEGBL*)pjt->pasegJumpSwinging;
                    pjt->svxyAirSteer *= R_JtEdgeJumpAirSteer;
                    break;
                }

                if (pasegbl == nullptr)
                    pasegbl = (ASEGBL*)pjt->pasegJumpStanding;

                if (pjt->jtjk == 4 && pjt->uJoyDeflect == 0.0f)
                {
                    SetVectorCylind(&velocity, pjt->radTarget, 100.0f, velocity.z);
                    pjt->jtjj = 3;
                }
                else
                    SetVectorCylind(&velocity, pjt->radJoy, pjt->svxyAirSteer* pjt->uJoyDeflect, velocity.z);

                pjt->pvtso->pfnSetSoVelocityVec(pjt, &velocity);
                break;
            }

            case 1:
            fKeepAseg = 1;
            break;

            case 2:
            if (pjt->jtbs == 1 || pjt->jtbs == 5)
                fKeepAseg = 1;
            else
            {
                switch (pjt->jtjk)
                {
                    case 0:
                    pasegbl = (ASEGBL*)pjt->pasegJumpStanding;
                    break;
                    case 1:
                    case 3:
                    case 4:
                    pasegbl = (ASEGBL*)pjt->pasegJumpSwinging;
                    break;
                    case 2:
                    pasegbl = (ASEGBL*)pjt->pasegJumpRunning;
                    break;
                }

                tLocal = TFindAsegLabel(pasegbl, (OID)308);
            }
            break;

            case 3:
            {
                pjt->fCanBoost = 0;
                pasegbl = (ASEGBL*)pjt->pasegJumpBoost;
                glm::vec3 velocity;
                GetJtJumpBoostVelocity(pjt, &velocity);
                pjt->pvtso->pfnSetSoVelocityVec(pjt, &velocity);
                break;
            }

            case 4:
            pjt->pactvalJump->ackPos = ACK_Spline;
            pjt->pvtalo->pfnRecacheAloActList(pjt);

            // Retail preserves the currently playing jump animation when this
            // transition begins from JTS_Jump. Otherwise it falls back to the
            // ordinary standing-jump animation. The basket itself is supplied
            // through the raw release OID 96 override below.
            if (pjt->jts == 2)
                fKeepAseg = 1;
            else
                pasegbl = (ASEGBL*)pjt->pasegJumpStanding;

            RecalcSwOxfFilterForObject(pjt->phbsk->psw, pjt->phbsk);
            pjt->vTarget = glm::vec3(0.0f);
            break;

            case 5:
            {
                pasegbl = (ASEGBL*)pjt->pasegBasketJumpOut;
                const glm::vec3 upLocal = pjt->xf.mat[2];
                const float speedJumpOut = std::sqrt(pjt->rGravity) * 800.0f;
                glm::vec3 velocity = pjt->xf.v + upLocal * speedJumpOut;

                pjt->pvtso->pfnSetSoVelocityVec(pjt, &velocity);
                pjt->fCanBoost = 1;
                break;
            }

            case 6:
            pjt->fShortenJump = 1;
            pjt->jtjj = 2;
            pjt->fCanBoost = 1;
            pjt->vTargetCoast = pjt->xf.v;
            pjt->vTargetCoast.z = 0.0f;
            pjt->vTarget = glm::vec3(0.0f);
            pjt->fBaseXp = 0;
            pjt->svxyAirSteer = SVXY_JtAirSteer * std::sqrt(pjt->rGravity);
            pasegbl = (ASEGBL*)pjt->pasegJumpStanding;
            break;

            case 7:
            pjt->cpaloIgnoreEffect = 0;
            pjt->fShortenJump = 1;
            pjt->vTarget = glm::vec3(0.0f);
            sfxid = 10;
            pasegbl = (ASEGBL*)pjt->pasegSmashAttack;
            ChooseJtSmashTarget(pjt);
            pjt->tContact = g_clock.t + DT_JtSmashContact;
            break;

            case 8:
            pjt->vTarget = glm::vec3(0.0f);
            pjt->fShortenJump = 1;
            pjt->cpaloIgnoreEffect = 0;
            pasegbl = (ASEGBL*)pjt->pasegDiveAttack;
            break;

            case 9:
            pjt->pactvalJump->ackPos = ACK_Spline;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            fKeepAseg = pjt->jts == 2;
            pjt->vTarget = glm::vec3(0.0f);
            pasegbl = (ASEGBL*)pjt->pasegJumpTarget;

            if (pjt->pjmtCur != nullptr && pjt->pjmtCur->psoSetIgnoreJt != nullptr)
                pjt->pjmtCur->psoSetIgnoreJt->zok = ZOK_Ignore;
            break;

            case 10:
            case 11:
            {
                glm::vec3 velocity;
                GetJtJumpToTargetVelocity(pjt, &velocity);
                pjt->pvtso->pfnSetSoVelocityVec(pjt, &velocity);
                pjt->pactvalJump->ackPos = ACK_Spline;
                pjt->pvtalo->pfnRecacheAloActList(pjt);
                pjt->fCanBoost = 0;
                pjt->vTarget = glm::vec3(0.0f);
                pasegbl = (ASEGBL*)pjt->pasegJumpTarget;
                StartSound((SFXID)80, nullptr, pjt, nullptr, 3000.0f, 300.0f, U_JtImpactSound, U_JtImpactSoundSecondary, 0.0f, nullptr, nullptr);
                break;
            }

            case 12:
            case 13:
            case 14:
            case 15:
            fKeepAseg = 1;
            pjt->pactvalJump->ackPos = ACK_Spline;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            pjt->vTarget = glm::vec3(0.0f);
            pjt->fCanBoost = 0;

            if (jtbs == 12 || jtbs == 13)
                StartSound((SFXID)148, nullptr, pjt, &pjt->xf.posWorld, 3000.0f, 300.0f, U_JtSoundPrimary, 0.0f, 0.0f, nullptr, nullptr);
            break;

            case 16:
            case 21:

            switch (pjt->jthk)
            {
                case 0:
                pasegbl = (ASEGBL*)pjt->pasegFlattenStand;
                break;
                case 1:
                case 3:
                pasegbl = (ASEGBL*)pjt->pasegDuckStand;
                break;
                case 2:
                pasegbl = (ASEGBL*)pjt->pasegBasketStand;
                break;
                case 4:
                if (pjt->prailCur->fSlippery && pjt->pasegblRailBalance != nullptr)
                {
                    pjt->uRailBalance = 0.0f;
                    pjt->duRailBalance = 0.0f;
                    pasegbl = pjt->pasegblRailBalance;
                }
                else
                {
                    psm = pjt->psmIdle;
                    oidInitialState = 333;
                }
                grfjta = 3;
                break;
                case 5:
                psm = pjt->psmIdle;
                oidInitialState = 332;
                break;
                case 6:
                pasegbl = (ASEGBL*)pjt->pasegReachStand;
                break;
                case 7:
                pasegbl = nullptr;
                break;
            }
            break;

            case 17:
            if (pjt->jthk == 1)
            {
                svtLocal = 1.75f;
                pasegbl = pjt->pasegblDuck;
            }
            else if (pjt->jthk == 0)
            {
                svtLocal = 2.0f;
                pasegbl = pjt->pasegblFlatten;
                if (pjt->ptnCur == nullptr)
                {
                    if (pjt->phshape != nullptr && pjt->phshape->fTunnel)
                    {
                        pjt->ptnCur = pjt->phshape->ptnHide;
                        if (pjt->ptnCur == nullptr)
                        {
                            pjt->ptnCur = pjt->ptnHideDynamic;
                            ResetJtDynamicTunnel(pjt);
                            pjt->ptnCur->pvtlo->pfnSetLoParent(pjt->ptnCur, pjt->phshape->paloParent);
                            pjt->ptnCur->pcrv = pjt->phshape->pcrv;
                        }
                    }
                    else if (pjt->phpnt != nullptr && pjt->phpnt->fTunnel)
                    {
                        pjt->ptnCur = pjt->phpnt->ptnHide;
                        if (pjt->ptnCur == nullptr)
                        {
                            pjt->ptnCur = pjt->ptnHideDynamic;
                            ResetJtDynamicTunnel(pjt);
                            pjt->ptnCur->pvtlo->pfnSetLoParent(pjt->ptnCur, pjt->phpnt->paloParent);
                            pjt->ptnCur->fFakeCylinder = 1;
                            pjt->ptnCur->posFake = pjt->phpnt->posLocal;
                            pjt->ptnCur->sRadFake = pjt->phpnt->sFlattenRadius;
                        }
                    }
                    if (pjt->ptnCur == pjt->ptnHideDynamic)
                    {
                        glm::vec3 zero(0.0f); glm::mat3 identity(1.0f);
                        pjt->ptnCur->pvtalo->pfnTranslateAloToPos(pjt->ptnCur, &zero);
                        pjt->ptnCur->pvtalo->pfnRotateAloToMat(pjt->ptnCur, &identity);
                        pjt->ptnCur->pvtalo->pfnSetAloVelocityVec(pjt->ptnCur, &zero);
                        pjt->ptnCur->pvtalo->pfnSetAloAngularVelocityVec(pjt->ptnCur, &zero);
                    }
                    if (pjt->ptnCur != nullptr)
                    {
                        pjt->ptnCur->ppo = pjt;
                        SetTnTnsOverride(pjt->ptnCur, TNS_In);
                    }
                }
            }
            else if (pjt->jthk == 3)
                pasegbl = pjt->pasegblSidestep;
            break;

            case 18:
            if (pjt->jthk == 0)
                pasegbl = (ASEGBL*)pjt->pasegFlattenPeekLeft;
            else if (pjt->jthk == 1)
                pasegbl = (ASEGBL*)pjt->pasegDuckPeekLeft;
            break;

            case 19:
            if (pjt->jthk == 0)
                pasegbl = (ASEGBL*)pjt->pasegFlattenPeekRight;
            else if (pjt->jthk == 1)
                pasegbl = (ASEGBL*)pjt->pasegDuckPeekRight;
            break;

            case 20:
            if (pjt->jthk == 2)
                pasegbl = pjt->pasegblBasket;
            else if (pjt->jthk == 3)
                pasegbl = pjt->pasegblRun;
            else if (pjt->jthk == 4)
                pasegbl = pjt->pasegblRail;
            break;

            case 23:
            pasegbl = (ASEGBL*)pjt->pasegPipeInit;
            pjt->uPipeClimbSmooth = 0.0f;

            if (pasegbl == nullptr)
            {
                pasegbl = (ASEGBL*)pjt->pasegPipeSpin;
                svtLocal = 0.5f;
                tLocal = pasegbl->tMax * 0.5f;
            }
            break;

            case 24:
            pasegbl = (ASEGBL*)pjt->pasegPipeStay;
            break;

            case 25:
            pasegbl = pjt->pasegblClimb;
            break;

            case 26:
            svtLocal = -1.0f;
            pasegbl = pjt->pasegblClimb;
            break;

            case 27:
            pasegbl = (ASEGBL*)pjt->pasegPipeSpin;
            pjt->fPipeReverse = !pjt->fPipeReverse;
            break;

            case 28:
            case 29:
            case 30:
            pasegbl = (ASEGBL*)pjt->mpjtbspaseg[jtbs];

            if (jtbs == 28 && pjt->ptnKeyCelebration != nullptr)
            {
                pjt->ptnCur = pjt->ptnKeyCelebration;
                SetTnTnsOverride(pjt->ptnCur, TNS_In);
            }

            if (jtbs == 30)
                pjt->pvtpo->pfnSetPoPck(pjt, (PCK)3);

            CancelSwDialogPlaying(pjt->psw);
            break;

            case 31:
            pasegbl = (ASEGBL*)pjt->pasegRushAttack;
            sfxid = 14;
            break;

            case 32:
            pasegbl = (ASEGBL*)pjt->pasegRushBounce;
            break;

            case 33:
            if (pjt->pasegPeekEnter != nullptr)
                pasegbl = (ASEGBL*)pjt->pasegPeekEnter;
            // Retail keeps the outgoing camera policy's focus when the peek
            // policy is inserted. Scripted level-entry dialogs use that focus
            // to preserve their authored view of the arena.
            SetCmPolicy(g_pcm, CPP_LookAround, &g_pcm->cplook,
                g_pcm->acpr[0].psoFocus, nullptr);
            break;

            case 34:
            SetMvgkRvol(0.6f, 5, 0);
            SetMvgkRvol(0.6f, 5, 1);
            StartCplookSound(&g_pcm->cplook);
            SetCmCut(g_pcm, 0);
            g_binoc.pdialogPlaying = pjt->pdialogPeekPending;
            pjt->pdialogPeekPending = nullptr;
            PushUiActiveBlot(&g_ui, &g_binoc);
            break;

            case 35:
            if (pjt->pasegPeekExit != nullptr)
                pasegbl = (ASEGBL*)pjt->pasegPeekExit;
            RevokeCmPolicy(g_pcm, 15, CPP_LookAround, &g_pcm->cplook,
                g_pcm->acpr[0].psoFocus, nullptr);
            SetCmCut(g_pcm, 0);
            break;

            case 37:
            case 38:
            grfAseg = 0;
            pjt->radTarget = std::atan2(pjt->xf.matWorld[0][1], pjt->xf.matWorld[0][0]);
            pjt->radJoy = pjt->radTarget;
            pasegbl = (ASEGBL*)pjt->mpjtbspaseg[jtbs];

            if (F_JtZapFreeze)
                svtLocal = 0.0f;
            break;

            case 36:
            case 42:
            case 43:
            pasegbl = (ASEGBL*)pjt->mpjtbspaseg[jtbs];
            break;

            case 39:
            case 45:
            case 46:
            case 47:
            if (jtbs == 45)
            {
                float velocityAlongTarget = glm::dot(pjt->xf.v, pjt->matTarget[0]);

                if (velocityAlongTarget > -250.0f)
                    jtbs = 46;
                else
                {
                    float speed = glm::length(pjt->xf.v);
                    float turn = glm::clamp(CLQ_JtWaterDeathTurn0 + speed * (CLQ_JtWaterDeathTurn1 + speed * CLQ_JtWaterDeathTurn2), 0.0f, 1.0f);
                    float radVelocityBack = std::atan2(pjt->xf.v.y, pjt->xf.v.x) + glm::pi<float>();
                    float drad = RadNormalize(radVelocityBack - pjt->radTarget);
                    pjt->radTarget = RadNormalize(pjt->radTarget + drad * turn);
                    FixStepAngularVelocity(pjt);
                }
            }

            pasegbl = (ASEGBL*)pjt->mpjtbspaseg[jtbs];

            if ((jtbs == 39 || jtbs == 47) && pjt->ptailTail != nullptr)
                pjt->ptailTail->dvGravity = s_dvTailInWater;

            if (jtbs == 47)
            {
                glm::vec3 dposCamera = g_pcm->pos - pjt->xf.posWorld;
                pjt->radTarget = std::atan2(dposCamera.y, dposCamera.x);
            }
            break;

            case 40:
            case 49:
            {
                glm::vec3 zero(0.0f);
                pjt->pvtso->pfnSetSoAngularVelocityVec(pjt, &zero);
                pjt->radTarget = std::atan2(pjt->xf.matWorld[0][1], pjt->xf.matWorld[0][0]);
                pjt->radJoy = pjt->radTarget;
                pasegbl = (ASEGBL*)pjt->mpjtbspaseg[jtbs];
                break;
            }

            case 41:
            case 48:
            {
                glm::vec3 dposCamera = g_pcm->pos - pjt->xf.posWorld;
                pjt->radTarget = std::atan2(dposCamera.y, dposCamera.x);
                pasegbl = (ASEGBL*)pjt->mpjtbspaseg[jtbs];
                break;
            }

            case 44:
            pjt->fCanBoost = 1;
            pjt->fBaseXp = 0;
            pasegbl = pjt->czqe == 0 ? (ASEGBL*)pjt->pasegJumpStanding : (ASEGBL*)pjt->mpjtbspaseg[43];
            pjt->vTarget = glm::vec3(0.0f);

            if (pjt->jtJumpTarget.plo != nullptr && FIsBasicDerivedFrom(pjt->jtJumpTarget.plo, CID_PIPE))
            {
                pjt->radTarget = std::atan2(pjt->xf.v.y, pjt->xf.v.x);
                FixStepAngularVelocity(pjt);
            }

            pjt->pactvalJump->ackPos = ACK_Spline;
            pjt->pvtalo->pfnRecacheAloActList(pjt);
            SetSoNoXpsAll(pjt, 1);
            pjt->fBaseXp = 0;
            break;

            case 50:
            case 51:
            case 52:
            pasegbl = pjt->pasegblSwing;

            if (pjt->pasegaCur != nullptr && pjt->pasegaCur->paseg == pasegbl)
                fKeepAseg = 1;
            else
            {
                svtLocal = 0.0f;
                tLocal = pasegbl->tMax * 0.5f;
            }
            break;

            case 53:
            psm = pjt->psmIdle;
            oidInitialState = 331;
            break;

            case 54:
            case 56:
            pasegbl = (ASEGBL*)pjt->pasegBall;
            break;

            case 55:
            {
                glm::vec3 velocity = pjt->xf.v;
                float speed = glm::length(velocity);

                if (speed >= SV_JtBallUseHeading)
                {
                    if (speed < SV_JtBallImpulse)
                        velocity *= SV_JtBallImpulse / speed;
                }
                else
                    SetVectorCylind(&velocity, pjt->radTarget, SV_JtBallImpulse, 0.0f);

                pjt->pvtso->pfnSetSoVelocityVec(pjt, &velocity);

                glm::vec3 angularVelocity(0.0f);
                float horizontalSpeed = glm::length(glm::vec2(velocity));

                if (horizontalSpeed >= 0.0001f)
                    angularVelocity = glm::vec3(-velocity.y, velocity.x, 0.0f) / pjt->sRadiusSelf;

                pjt->pvtso->pfnSetSoAngularVelocityVec(pjt, &angularVelocity);
                pasegbl = (ASEGBL*)pjt->pasegBall;

                if (pjt->psoBallEffect != nullptr && (GetAvailableVaultFlags() & 0x400) != 0)
                    pjt->psoBallEffect->pvtlo->pfnAddLo(pjt->psoBallEffect);
                break;
            }

            case 57:
            pasegbl = (ASEGBL*)pjt->pasegMinePlace;
            break;

            case 58:
            pasegbl = (ASEGBL*)pjt->pasegMineActivate;
            break;

            default:
            break;
        }
    }
    if (pjt->jts != jts) { pjt->jts = jts; pjt->tJts = g_clock.t; }

    if (pjt->jtbs != jtbs) { pjt->jtbs = jtbs; pjt->tJtbs = g_clock.t; }

    ChooseJtPhys(pjt, nullptr);
    EnableJtActadj(pjt, grfjta);
    if (sfxid != -1) StartSound((SFXID)sfxid, nullptr, nullptr, &pjt->xf.posWorld, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
    if (fKeepAseg) return;
    if (pjt->pasegaCur != nullptr) { RetractAsega(pjt->pasegaCur); pjt->pasegaCur = nullptr; }
    else if (pjt->psmaCur != nullptr) { RetractSma(pjt->psmaCur); pjt->psmaCur = nullptr; }
    if (pasegbl == nullptr)
    {
        if (psm != nullptr) pjt->psmaCur = PsmaApplySm(psm, pjt, (OID)oidInitialState, grfAseg);
        return;
    }
    OVR overrideHbsk = {}; int coverride = 0;
    // Original release SetJtJts loads raw OID 0x61 into the override record at
    // 0x0017775c. Proto and release OID tables are different, so keep it raw.
    if (pjt->phbsk != nullptr) { overrideHbsk.oid = (OID)97; overrideHbsk.palo = pjt->phbsk; coverride = 1; }

    ApplyAsegOvr(pasegbl, pjt, coverride, coverride ? &overrideHbsk : nullptr, tLocal, svtLocal, grfAseg, &pjt->pasegaCur);
}

void SetJtPuppet(JT* pjt, EA* pea)
{
    if (pea == nullptr)
        SetJtJts(pjt, static_cast<JTS>(0), static_cast<JTBS>(-1));
    else
        SetJtJts(pjt, static_cast<JTS>(11), static_cast<JTBS>(-1));
}

void SetJtDisableStepCustomXps(JT* pjt, int fDisable)
{
    pjt->fDisableStepCustomXps = fDisable;
    pjt->grfpvaXpValid = 0;
}

ALO* PaloAbsorbWkr(WKR* pwkr, int cpaloIgnore, ALO** apaloIgnore)
{
    ALO* palo = reinterpret_cast<ALO*>(pwkr->ploTarget);

    if ((palo->pvtlo->grfcid & 1U) == 0)
        palo = palo->paloParent;

    while (palo != nullptr)
    {
        for (int i = 0; i < cpaloIgnore; ++i)
        {
            if (palo == apaloIgnore[i])
                return nullptr;
        }

        switch (pwkr->grftak)
        {
            case 2:
            pwkr->grfic = palo->ficg.grficSweep;
            break;

            case 4:
            pwkr->grfic = palo->ficg.grficSmash;
            break;

            case 8:
            pwkr->grfic = palo->ficg.grficRush;
            break;

            case 16:
            pwkr->grfic = palo->ficg.grficBomb;
            break;

            case 32:
            pwkr->grfic = palo->ficg.grficShock;
            break;

            default:
            pwkr->grfic = 0;
            break;
        }

        // DECOY supplies the SO absorb slot through VTDECOY.  Calling that
        // class-specific table through the shorter VTALO view can resolve the
        // slot as null, so dispatch the decoy override explicitly.
        const bool fAbsorbed =
            palo->pvtlo->cid == CID_DECOY
                ? FAbsorbDecoyWkr(reinterpret_cast<DECOY*>(palo), pwkr)
                : palo->pvtalo->pfnFAbsorbAloWkr(palo, pwkr) != 0;

        if (fAbsorbed)
            return palo;

        palo = palo->paloParent;
    }

    return nullptr;
}

int NCmpWkr(WKR* pwkr1, WKR* pwkr2)
{
    return pwkr1->gSort > pwkr2->gSort ? 1 : -1;
}

void UpdateJtEffect(JT* pjt)
{
    if (pjt->jts == 4)
    {
        FreeSwXpList(pjt->psw, pjt->pxpEffect);
        pjt->pxpEffect = nullptr;
        return;
    }

    SO* psoEffect = nullptr;
    uint32_t grftak = 0;

    if (pjt->jtbs == 7)
    {
        psoEffect = pjt->psoSmashEffect;
        grftak = 4;
    }
    else if (pjt->jtcs == 4 || pjt->jtcs == 5 || pjt->jtcs == 6 || pjt->jtcs == 7)
    {
        psoEffect = pjt->psoSweepEffect;
        grftak = 2;
    }
    else if (pjt->jts == 10)
    {
        psoEffect = pjt->psoRushEffect;
        grftak = 8;
    }
    else if (pjt->jtbs == 8)
    {
        psoEffect = pjt->psoDiveEffect;
        grftak = 4;
    }
    else if (pjt->jtbs == 55 && (GetAvailableVaultFlags() & 1024U) != 0)
    {
        psoEffect = pjt->psoBallEffect;
        grftak = 8;
    }
    else
    {
        FreeSwXpList(pjt->psw, pjt->pxpEffect);
        pjt->pxpEffect = nullptr;
        return;
    }

    std::vector<WKR> workers;
    workers.reserve(64);

    if (FIsLoInWorld(psoEffect))
    {
        TARGET* ptarget = g_dlTarget.ptargetFirst;

        while (ptarget != nullptr && workers.size() < 64)
        {
            TARGET* ptargetNext = reinterpret_cast<TARGET*>(ptarget->dleTarget.pvNext);

            if (ptarget->fHitTest && (ptarget->grftak & grftak) != 0)
            {
                glm::vec3 posTarget{};
                SBI sbi{};

                GetXfmPos(reinterpret_cast<XFM*>(ptarget), &posTarget);

                if (CsbiIntersectSphereBsp(&posTarget, ptarget->sRadiusTarget, psoEffect->bspc.cbspFull, psoEffect->bspc.absp.data(), nullptr, &psoEffect->geomWorld, 1, &sbi) > 0)
                {
                    WKR worker{};

                    worker.ploSource = pjt;
                    worker.ploTarget = ptarget;
                    worker.sftMax = pjt->m * 1000.0f;
                    worker.grftak = grftak;
                    worker.pos = posTarget;

                    CalculateAloMovement(ptarget->paloParent, nullptr, worker.pos, &worker.v, nullptr, nullptr, nullptr);

                    worker.gSort = glm::dot(psoEffect->xf.matWorld[0], worker.pos);
                    workers.push_back(worker);
                }
            }

            ptarget = ptargetNext;
        }
    }

    for (XP* pxp = pjt->pxpEffect; pxp != nullptr && workers.size() < 64; pxp = pxp->pxpNext)
    {
        const int ixpdEffect = pjt != static_cast<JT*>(pxp->axpd[0].psoRoot);

        if (pxp->axpd[ixpdEffect].psoLeaf != psoEffect)
            continue;

        const int ixpdOther = 1 - ixpdEffect;

        WKR worker{};

        worker.ploSource = pjt;
        worker.ploTarget = pxp->axpd[ixpdOther].psoLeaf;
        worker.sftMax = pjt->m * 1000.0f;
        worker.grftak = grftak;
        worker.pos = pxp->axpd[ixpdOther].pos;

        CalculateAloMovement(pxp->axpd[ixpdEffect].psoLeaf, nullptr, worker.pos, &worker.v, nullptr, nullptr, nullptr);

        worker.gSort = glm::dot(psoEffect->xf.matWorld[0], worker.pos);
        workers.push_back(worker);
    }

    std::sort(workers.begin(), workers.end(), [](const WKR& lhs, const WKR& rhs)
    {
        return lhs.gSort < rhs.gSort;
    });

    for (WKR& worker : workers)
    {
        ALO* paloAbsorbed = PaloAbsorbWkr(&worker, pjt->cpaloIgnoreEffect, pjt->apaloIgnoreEffect);

        if (paloAbsorbed == nullptr)
            continue;

        if ((worker.grfic & 2U) != 0)
        {
            if ((grftak & 2U) != 0)
            {
                SetJtJts(pjt, static_cast<JTS>(0), static_cast<JTBS>(-1));

                if (pjt->jtcs == 4 || pjt->jtcs == 6)
                    SetJtJtcs(pjt, static_cast<JTCS>(8));
                else
                    SetJtJtcs(pjt, static_cast<JTCS>(9));
            }
            else if ((grftak & 4U) != 0)
            {
                SetJtJts(pjt, static_cast<JTS>(0), static_cast<JTBS>(-1));
                SetJtJtcs(pjt, static_cast<JTCS>(8));
            }
        }

        if ((worker.grfic & 1U) != 0)
        {
            if (pjt->jtbs == 55)
                SetJtJts(pjt, static_cast<JTS>(13), static_cast<JTBS>(56));
            else
                psoEffect->pvtlo->pfnRemoveLo(psoEffect);

            FreeSwXpList(pjt->psw, pjt->pxpEffect);
            pjt->pxpEffect = nullptr;
            return;
        }

        if (pjt->cpaloIgnoreEffect < 32)
        {
            pjt->apaloIgnoreEffect[pjt->cpaloIgnoreEffect] = paloAbsorbed;
            ++pjt->cpaloIgnoreEffect;
        }
    }

    FreeSwXpList(pjt->psw, pjt->pxpEffect);
    pjt->pxpEffect = nullptr;
}

bool FIsJtSoundBase(JT* pjt)
{
    if (pjt->jts == 12 && pjt->jtbs == 34)
        return false;

    return FActiveCplcy(&g_pcm->cpman) == 0;
}

void CollectJtPrize(JT* pjt, PCK pck, ALO* paloOther)
{
    CollectPoPrize(pjt, pck, paloOther);

    if (pck == PCK_Key) 
        SetJtJts(pjt, 9, 28);
    else if (pck == PCK_Gold)
        SetJtJts(pjt, 9, 29);
}

void GetJtDiapi(JT* pjt, DIALOG* pdialog, DIAPI* pdiapi)
{
    const int fInstruct = pdialog->dialogk == DIALOGK_Instruct;

    switch (pjt->jts)
    {
        case 0:
        case 1:
        case 5:
        pdiapi->fPlayable = 1;
        pdiapi->fCallable = 1;
        break;

        case 2:
        case 8:
        case 10:
        case 13:
        case 14:
        case 15:
        case 16:
        pdiapi->fCallable = 1;
        pdiapi->fPlayable = fInstruct;
        break;

        case 3:
        case 6:
        case 7:
        pdiapi->fPlayable = fInstruct;
        pdiapi->fCallable = fInstruct;
        break;

        case 4:
        case 9:
        pdiapi->fCallable = 0;
        pdiapi->fPlayable = fInstruct;
        break;

        case 11:
        pdiapi->fCallable = 0;
        pdiapi->fPlayable = 1;
        break;

        case 12:
        pdiapi->fCallable = 0;
        pdiapi->fPlayable = pjt->pdialogPeekPending == pdialog;
        break;

        default:
        break;
    }

    if (g_ui.uis != 3)
        pdiapi->fCallable = 0;
}

void PlayJtDialog(JT* pjt, DIALOG* pdialog)
{
    if (static_cast<uint32_t>(pdialog->dialogk + DIALOGK_Nil) < 2U)
    {
        g_binoc.pdialogPlaying = pdialog;
        PushUiActiveBlot(&g_ui, static_cast<BLOT*>(&g_binoc));
        return;
    }

    if (pjt->jts == 12)
        return;

    pjt->pdialogPeekPending = pdialog;
    pjt->jtbsNext = static_cast<JTBS>(33);
    pjt->jtsNext = static_cast<JTS>(12);
    pjt->grfjtsc |= 1U;
}

void TranslateJtToPos(JT* pjt, glm::vec3* pvector)
{
    TranslateSoToPos(pjt, pvector);
    ClearSqtrm(&pjt->sqtrmCane);
    pjt->cjumpTargetHistory = 0;
}

void OnJtAdd(JT* pjt)
{
    ClearSqtrm(&pjt->sqtrmCane);
    OnPoAdd(pjt);
}

void UpdateJtPendingInteractions(JT* pjt)
{
    if (pjt->jts < 0 || (pjt->jts >= 2 && pjt->jts != 5))
        return;

    SO* psoPending = pjt->psoDisplacePending;

    if (psoPending != nullptr)
        pjt->psoDisplacePending = nullptr;
    else
    {
        psoPending = pjt->psoDisplacePendingSecondary;

        if (psoPending != nullptr)
            pjt->psoDisplacePendingSecondary = nullptr;
        else
        {
            LOCKG* plockg = pjt->plockg;

            if (plockg == nullptr)
                return;

            pjt->plockg = nullptr;

            int oidCur;
            int oidGoal;
            GetSmaCur(plockg->psma, (OID*)&oidCur);
            GetSmaGoal(plockg->psma, (OID*)&oidGoal);

            if (oidCur != 558 || (oidGoal != 558 && oidGoal != -1))
                return;

            if (g_pwsCur->ckey > 0)
            {
                pjt->plockgCur = plockg;
                SetJtJts(pjt, 9, 30);
                return;
            }

            for (int i = 0; i < plockg->cplock; ++i)
                SetSmaGoal(plockg->aplock[i]->psma, (OID)561);

            const glm::vec3 dposLock = plockg->xf.posWorld - pjt->xf.posWorld;
            pjt->radTarget = std::atan2(dposLock.y, dposLock.x);
            return;
        }
    }

    if (psoPending->dleRoot.pvNext == nullptr)
        psoPending->pvtso->pfnDisplaceSo(psoPending, 1);
}

void TriggerJtLockg(JT* pjt, LOCKG* plockg)
{
    if (pjt->jts == 9)
        return;

    int oidCur;
    int oidGoal;

    GetSmaCur(plockg->psma,  (OID*)&oidCur);
    GetSmaGoal(plockg->psma, (OID*)&oidGoal);

    if (oidCur == 558 && (oidGoal == 558 || oidGoal == -1))
        pjt->plockg = plockg;
}

void TriggerJtDisplace(JT* pjt, SO* pso)
{
    if (pjt->jts != 9 && pso->dleRoot.pvNext == nullptr)
        pjt->psoDisplacePending = pso;
}

void TriggerJtDisplaceSecondary(JT* pjt, SO* pso)
{
    if (pjt->jts != 9 && pso->dleRoot.pvNext == nullptr)
        pjt->psoDisplacePendingSecondary = pso;
}

void CalculateJtJumpTrajectory(JT* pjt, float dt, JTJUMPTARGET* ptarget, glm::vec3* pvLaunch, glm::vec3* pposGoal, glm::vec3* pvGoal)
{
    static const glm::vec3 s_dposJtJumpTargetDefault(0.0f, 0.0f, 75.0f);
    static const glm::vec3 s_dposJtJumpTargetHnd(0.0f, 0.0f, -225.0f);
    static constexpr float S_JtJumpTargetPipe = -75.0f;

    LO* ploTarget = ptarget->plo;
    ALO* paloParent = nullptr;

    glm::vec3 posTarget;
    glm::vec3 dposTarget = s_dposJtJumpTargetDefault;

    if (ploTarget == nullptr)
    {
        posTarget = ptarget->pos;
    }
    else
    {
        paloParent = ploTarget->paloParent;

        if (FIsBasicDerivedFrom(ploTarget, CID_HND))
        {
            HND* phnd = reinterpret_cast<HND*>(ploTarget);
            posTarget = phnd->posLocal;
            dposTarget = s_dposJtJumpTargetHnd;
        }
        else if (FIsBasicDerivedFrom(ploTarget, CID_PIPE))
        {
            PIPE* ppipe = reinterpret_cast<PIPE*>(ploTarget);
            CRV* pcrv = ppipe->pcrv.get();

            if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
                pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, ptarget->s, &posTarget, nullptr);

            glm::mat3 matPipeGoal;
            GetJtPipeGoalMat(pjt, ppipe, ptarget->s, &matPipeGoal);
            dposTarget = matPipeGoal[0] * S_JtJumpTargetPipe;
        }
        else if (FIsBasicDerivedFrom(ploTarget, CID_HSHAPE))
        {
            HSHAPE* phshape = reinterpret_cast<HSHAPE*>(ploTarget);
            GetHshapeHidePos(phshape, ptarget->s, &posTarget, nullptr);
            ConvertAloPos(nullptr, paloParent, &posTarget, &posTarget);
        }
        else if (FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_HPNT))
        {
            HPNT* phpnt = reinterpret_cast<HPNT*>(ploTarget);
            GetHpntClosestHidePos(phpnt, ptarget->s, &posTarget, nullptr);
            ConvertAloPos(nullptr, paloParent, &posTarget, &posTarget);
        }
        else if (FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_SHAPE))
        {
            SHAPE* pshape = reinterpret_cast<SHAPE*>(ploTarget);
            CRV* pcrv = pshape->pcrv.get();

            if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
                pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, ptarget->s, &posTarget, nullptr);
        }
        else if (FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_PNT))
        {
            PNT* ppnt = reinterpret_cast<PNT*>(ploTarget);
            posTarget = ppnt->posLocal;
        }
        else
            posTarget = glm::vec3(0.0f);
    }

    glm::vec3 vTarget(0.0f);
    PredictAloTransformAdjust(paloParent, nullptr, dt, &posTarget, nullptr, &vTarget, nullptr);

    const glm::vec3 posGoal = posTarget + dposTarget;
    const glm::vec3 vLaunch = (posGoal - pjt->xf.posWorld) / dt - pjt->dvGravity * (dt * 0.5f);
    const glm::vec3 vGoal = vLaunch + pjt->dvGravity * dt;

    if (pvLaunch != nullptr)
        *pvLaunch = vLaunch;

    if (pposGoal != nullptr)
        *pposGoal = posGoal;

    if (pvGoal != nullptr)
        *pvGoal = vGoal;
}

void StartJtZapJumpToTarget(JT* pjt, float dtJump, const JTJUMPTARGET* ptarget)
{
    JTJUMPTARGET target{};

    if (ptarget == nullptr)
    {
        std::vector<LO*> jumpObjects;
        FindBestJtJumpTarget(pjt, jumpObjects, &target, &dtJump);        
        ptarget = &target;
    }

    glm::vec3 vLaunch;
    glm::vec3 posGoal;
    glm::vec3 vGoal;

    CalculateJtJumpTrajectory(pjt, dtJump, const_cast<JTJUMPTARGET*>(ptarget), &vLaunch, &posGoal, &vGoal);

    pjt->pactvalJump->tMatch = g_clock.t + dtJump;
    pjt->pactvalJump->posGoal = posGoal;
    pjt->pactvalJump->vGoal = vGoal;

    pjt->pvtalo->pfnSetAloVelocityVec(pjt, &vLaunch);

    pjt->jtJumpTarget = *ptarget;

    SetJtJts(pjt, 4, 44);
}

void FindBestJtJumpDuration(JT* pjt, JTJUMPTARGET* ptarget, std::vector<SO*>* papso, float* pdtBest)
{
    glm::vec3 posStart = pjt->xf.posWorld;
    posStart.z -= 75.0f;

    float scoreBest = std::numeric_limits<float>::max();
    float dtBest = s_adtJtJumpCandidate[0];

    for (float dtJump : s_adtJtJumpCandidate)
    {
        glm::vec3 vLaunch;
        glm::vec3 posGoal;
        glm::vec3 vGoal;

        CalculateJtJumpTrajectory(pjt, dtJump, ptarget, &vLaunch, &posGoal, &vGoal);
        posGoal.z -= 75.0f;

        float scoreFactor = 1.0f;
        float t = 0.0f;
        glm::vec3 posPrev = posStart;

        while (t < dtJump)
        {
            float tNext = std::min(t + 0.25f, dtJump);
            glm::vec3 posNext;
            LSG alsg[8];

            EvaluateBezierPos(dtJump, tNext, 1.0f, &posStart, &vLaunch, &posGoal, &vGoal, nullptr, nullptr, &posNext);

            int clsg = ClsgClipEdgeToObjects(&posPrev, &posNext, papso, 8, alsg);

            for (int i = 0; i < clsg; ++i)
                scoreFactor += glm::distance(alsg[i].apos[0], alsg[i].apos[1]);

            posPrev = posNext;
            t = tNext;
        }

        float score = scoreFactor * glm::length(vLaunch);

        if (score < scoreBest)
        {
            scoreBest = score;
            dtBest = dtJump;
        }
    }

    *pdtBest = dtBest;
}

float ScoreJtJumpTarget(JT* pjt, JTJUMPTARGET* ptarget, std::vector<STEPGUARD*>& stepguards, JTJUMPTARGET* ptargetAdjusted, float* pdtJump)
{
    constexpr float dtTest = 0.7f;
    constexpr float normalZMin = 0.7f;
    constexpr float scoreBadLanding = 2000.0f;
    constexpr float scoreUnsupportedLanding = 1500.0f;

    glm::vec3 posGoal;
    CalculateJtJumpTrajectory(pjt, dtTest, ptarget, nullptr, &posGoal, nullptr);

    glm::vec3 posMin = glm::min(pjt->xf.posWorld, posGoal) - s_dposJtJumpSearchPadding;
    glm::vec3 posMax = glm::max(pjt->xf.posWorld, posGoal) + s_dposJtJumpSearchPadding;
    posMax.z += 200.0f;

    std::vector<SO*> collisionObjects;
    IntersectSwBoundingBox(pjt->psw, pjt, &posMin, &posMax, nullptr, nullptr, collisionObjects);

    JTJUMPTARGET target = *ptarget;
    float score = 0.0f;

    if (target.plo == nullptr)
    {
        glm::vec3 posRayStart = posGoal;
        glm::vec3 posRayEnd = posGoal;
        posRayStart.z += 125.0f;
        posRayEnd.z -= 175.0f;

        LSG lsgFloor = {};
        SO* psoFloor = PsoHitTestLineObjects(0, &posRayStart, &posRayEnd, collisionObjects, &lsgFloor);

        bool fValidFloor = psoFloor != nullptr && lsgFloor.au[0] != 0.0f && lsgFloor.anormal[0].z >= normalZMin;

        if (fValidFloor)
            target.pos = lsgFloor.apos[0];
        else
        {
            target.pos = posGoal;
            score = scoreBadLanding;
        }

        posRayStart = target.pos;
        posRayEnd = target.pos;
        posRayStart.z += 180.0f;
        posRayEnd.z += 5.0f;

        LSG lsgClearance = {};

        if (PsoHitTestLineObjects(0, &posRayStart, &posRayEnd, collisionObjects, &lsgClearance) != nullptr)
            score = std::max(score, scoreBadLanding);

        for (glm::vec3& dposProbe : s_adposJtLandingProbe)
        {
            glm::vec3 posProbe = target.pos + dposProbe;
            posRayStart = posProbe;
            posRayEnd = posProbe;
            posRayStart.z += 50.0f;
            posRayEnd.z -= 30.0f;

            LSG lsgSupport = {};

            if (PsoHitTestLineObjects(0, &posRayStart, &posRayEnd, collisionObjects, &lsgSupport) == nullptr)
            {
                score = std::max(score, scoreUnsupportedLanding);
                break;
            }
        }
    }

    for (STEPGUARD* pstepguard : stepguards)
    {
        if (pstepguard == nullptr)
            continue;

        glm::vec3 posAvoid = target.pos;

        if (pstepguard->geomLocal.apos.size() != 0 && pstepguard->iposBase >= 0 && pstepguard->iposBase < pstepguard->geomLocal.cpos)
            posAvoid.z -= pstepguard->dzBase + pstepguard->geomLocal.apos[pstepguard->iposBase].z;

        float distance = glm::distance(pstepguard->xf.posWorld, posAvoid) - pstepguard->sRadiusSelf;
        float avoidScore = s_clqJtJumpAvoidance.g0 + distance * (s_clqJtJumpAvoidance.g1 + distance * s_clqJtJumpAvoidance.g2);
        avoidScore = std::clamp(avoidScore, s_lmJtJumpAvoidance.gMin, s_lmJtJumpAvoidance.gMax);
        score = std::max(score, avoidScore);
    }

    FindBestJtJumpDuration(pjt, &target, &collisionObjects, pdtJump);

    *ptargetAdjusted = target;
    return score;
}

int CompareJtJumpTargetsByDistance(const void* pvA, const void* pvB)
{
    const JTJUMPTARGET* ptargetA = static_cast<const JTJUMPTARGET*>(pvA);
    const JTJUMPTARGET* ptargetB = static_cast<const JTJUMPTARGET*>(pvB);

    glm::vec3 dposA = ptargetA->pos - g_pjt->xf.posWorld;
    glm::vec3 dposB = ptargetB->pos - g_pjt->xf.posWorld;

    float distanceSqA = glm::dot(dposA, dposA);
    float distanceSqB = glm::dot(dposB, dposB);

    if (distanceSqA < distanceSqB)
        return -1;

    if (distanceSqA > distanceSqB)
        return 1;

    return 0;
}

void FindBestJtJumpTarget(JT* pjt, std::vector<LO*>& jumpObjects, JTJUMPTARGET* ptargetBest, float* pdtBest)
{
    std::vector <LO*> foundObjects(32);
    int cstepguard = CploFindSwObjectsByClass(pjt->psw, CID_PO, CID_STEPGUARD, nullptr, static_cast<int>(foundObjects.size()), foundObjects.data());

    std::vector <STEPGUARD*> stepguards;
    stepguards.reserve(cstepguard);

    for (int i = 0; i < cstepguard; ++i)
        stepguards.push_back(static_cast<STEPGUARD*>(foundObjects[i]));

    std::vector<JTJUMPTARGET> targets;
    targets.reserve(jumpObjects.size());

    for (LO* plo : jumpObjects)
    {
        if (plo == nullptr || !FIsLoInWorld(plo))
            continue;

        JTJUMPTARGET target = {};
        target.plo = plo;

        if (FIsBasicDerivedFrom(plo, CID_SHAPE))
        {
            SHAPE* pshape = static_cast<SHAPE*>(plo);
            CRV* pcrv = pshape->pcrv.get();

            if (pcrv != nullptr && pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
            {
                glm::vec3 posLocal;
                ConvertAloPos(nullptr, plo->paloParent, &pjt->xf.posWorld, &posLocal);
                pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &posLocal, nullptr, nullptr, nullptr, nullptr, &target.s);
            }
        }

        CalculateJtJumpTrajectory(pjt, 0.7f, &target, nullptr, &target.pos, nullptr);
        targets.push_back(target);
    }

    std::sort(targets.begin(), targets.end(), [pjt](const JTJUMPTARGET& a, const JTJUMPTARGET& b)
    {
        glm::vec3 dposA = a.pos - pjt->xf.posWorld;
        glm::vec3 dposB = b.pos - pjt->xf.posWorld;
        return glm::dot(dposA, dposA) < glm::dot(dposB, dposB);
    });

    float scoreBest = std::numeric_limits<float>::max();
    bool fFoundTarget = false;

    for (JTJUMPTARGET& target : targets)
    {
        JTJUMPTARGET targetAdjusted = {};
        float dtJump = 0.0f;
        float score = ScoreJtJumpTarget(pjt, &target, stepguards, &targetAdjusted, &dtJump);

        if (score >= scoreBest)
            continue;

        scoreBest = score;
        *ptargetBest = targetAdjusted;
        *pdtBest = dtJump;
        fFoundTarget = true;

        if (score == 0.0f)
            return;
    }

    if (scoreBest > 1000.0f)
    {
        for (int i = 0; i < pjt->cjumpTargetHistory; ++i)
        {
            JTJUMPTARGET* ptarget = &pjt->ajtJumpTargetHistory[i];

            if (ptarget->plo != nullptr && !FIsLoInWorld(ptarget->plo))
                continue;

            JTJUMPTARGET targetAdjusted = {};
            float dtJump = 0.0f;
            float score = ScoreJtJumpTarget(pjt, ptarget, stepguards, &targetAdjusted, &dtJump);

            if (score >= scoreBest)
                continue;

            scoreBest = score;
            *ptargetBest = targetAdjusted;
            *pdtBest = dtJump;
            fFoundTarget = true;

            if (score == 0.0f)
                return;
        }
    }

    if (!fFoundTarget)
    {
        *ptargetBest = {};
        ptargetBest->pos = pjt->xf.posWorld;
        ptargetBest->pos.z -= 75.0f;
        *pdtBest = 0.7f;
    }

}

void UpdateJtJumpTargetHistory(JT* pjt)
{
    constexpr int cHistoryMax = 32;
    constexpr float sDuplicateMax = 50.0f;

    LO* ploTarget = nullptr;
    float sTarget = 0.0f;
    glm::vec3 posTarget(0.0f);

    auto FGetBaseTarget = [&]() -> bool
    {
        if (!pjt->fBaseXp)
            return false;

        if (pjt->psoBaseXp != nullptr && (pjt->psoBaseXp->fCenterXp) != 0)
            return false;

        posTarget = pjt->posBaseXp;
        return true;
    };

    switch (pjt->jts)
    {
        case 0:
        case 1:
        case 5:
        case 9:
        case 10:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        if (!FGetBaseTarget())
            return;
        break;

        case 2:
        case 4:
        case 8:
        case 11:
        return;

        case 3:
        if (pjt->phndCur == nullptr || pjt->phndCur->unkFloat1 != 0.0f)
            return;

        ploTarget = pjt->phndCur;
        break;

        case 6:
        switch (pjt->jthk)
        {
            case 0:
            if (pjt->phshape != nullptr)
            {
                if (pjt->phshape->dzMax != 0.0f)
                    return;

                ploTarget = pjt->phshape;
                sTarget = pjt->sParamHshape;
            }
            else
            {
                if (pjt->phpnt == nullptr || pjt->phpnt->unkInt != 0)
                    return;

                glm::vec3 posPoint;
                GetPntPos(pjt->phpnt, &posPoint);

                glm::vec3 dpos = pjt->xf.posWorld - posPoint;
                ploTarget = pjt->phpnt;
                sTarget = std::atan2(dpos.y, dpos.x) * pjt->phpnt->sFlattenRadius;
            }
            break;

            case 4:
            if (pjt->prailCur == nullptr || pjt->prailCur->unkInt != 0)
                return;

            ploTarget = pjt->prailCur;
            sTarget = pjt->sRailCur;
            break;

            case 5:
            if (pjt->pspireCur == nullptr || pjt->pspireCur->unkInt != 0)
                return;

            ploTarget = pjt->pspireCur;
            break;

            case 7:
            return;

            default:
            if (!FGetBaseTarget())
                return;
            break;
        }
        break;

        case 7:
        if (pjt->ppipeCur == nullptr || pjt->ppipeCur->unkInt1 != 0)
            return;

        ploTarget = pjt->ppipeCur;
        sTarget = pjt->sPipeHand;
        break;

        default:
        return;
    }

    int iMatchingTarget = -1;

    for (int i = 1; i < pjt->cjumpTargetHistory; ++i)
    {
        JTJUMPTARGET& target = pjt->ajtJumpTargetHistory[i];

        if (target.plo != ploTarget)
            continue;

        if (glm::distance(target.pos, posTarget) >= sDuplicateMax)
            continue;

        if (std::abs(target.s - sTarget) >= sDuplicateMax)
            continue;

        iMatchingTarget = i;
        break;
    }

    if (iMatchingTarget < 0)
    {
        if (pjt->cjumpTargetHistory < cHistoryMax)
            ++pjt->cjumpTargetHistory;

        for (int i = pjt->cjumpTargetHistory - 1; i > 0; --i)
            pjt->ajtJumpTargetHistory[i] = pjt->ajtJumpTargetHistory[i - 1];
    }
    else if (iMatchingTarget > 1)
    {
        JTJUMPTARGET matchingTarget = pjt->ajtJumpTargetHistory[iMatchingTarget];

        for (int i = iMatchingTarget; i > 1; --i)
            pjt->ajtJumpTargetHistory[i] = pjt->ajtJumpTargetHistory[i - 1];

        pjt->ajtJumpTargetHistory[1] = matchingTarget;
    }

    JTJUMPTARGET& newestTarget = pjt->ajtJumpTargetHistory[0];
    newestTarget = {};
    newestTarget.plo = ploTarget;
    newestTarget.s = sTarget;
    newestTarget.uTarget = g_clock.t;
    newestTarget.vTarget = 0.0f;
    newestTarget.pos = posTarget;
}

int GetJtSize()
{
	return sizeof(JT);
}

void DeleteJt(JT* pjt)
{
	delete pjt;
}

JT* g_pjt = nullptr;

SNIP s_asnipPostLoadJt[27] =
{
    { 0x04, (OID)0x060, offsetof(JT, paloGut) },
    { 0x02, (OID)0x161, offsetof(JT, pzi.mpccharmpaloCharm[1]) },
    { 0x02, (OID)0x162, offsetof(JT, pzi.mpccharmpaloCharm[2]) },
    { 0x02, (OID)0x163, offsetof(JT, pzi.mpccharmpaloCharm[3]) },
    { 0x04, (OID)0x05B, offsetof(JT, paloSkinStandard) },
    { 0x02, (OID)0x062, offsetof(JT, paloSkinElectric) },
    { 0x02, (OID)0x063, offsetof(JT, paloSkinFire) },
    { 0x04, (OID)0x16F, offsetof(JT, paloTool) },
    { 0x04, (OID)0x170, offsetof(JT, paloHook) },
    { 0x02, (OID)0x173, offsetof(JT, paloKey) },
    { 0x04, (OID)0x160, offsetof(JT, pzi.mpccharmpaloCharm[0]) },
    { 0x20, (OID)0x164, offsetof(JT, paloCollectTarget[0]) },
    { 0x20, (OID)0x165, offsetof(JT, paloCollectTarget[1]) },
    { 0x20, (OID)0x166, offsetof(JT, paloCollectTarget[2]) },
    { 0x22, (OID)0x169, offsetof(JT, ptnKeyCelebration) },
    { 0x22, (OID)0x07E, offsetof(JT, psoSweepEffect) },
    { 0x02, (OID)0x07F, offsetof(JT, psoSmashEffect) },
    { 0x02, (OID)0x080, offsetof(JT, psoRushEffect) },
    { 0x02, (OID)0x1B9, offsetof(JT, psoDiveEffect) },
    { 0x22, (OID)0x081, offsetof(JT, psoBallEffect) },
    { 0x06, (OID)0x050, offsetof(JT, plboneMineOrigin) },
    { 0x26, (OID)0x18E, offsetof(JT, apaloMineHat[0]) },
    { 0x26, (OID)0x18F, offsetof(JT, apaloMineHat[1]) },
    { 0x26, (OID)0x190, offsetof(JT, apaloMineHat[2]) },
    { 0x26, (OID)0x191, offsetof(JT, apaloMineHat[3]) },
    { 0x22, (OID)0x192, offsetof(JT, pmine) },
    { 0x26, (OID)0x151, offsetof(JT, pdecoy) },
};

glm::vec3 s_posEdgeTargetDefault = {37.0, 0.0, 25.0};
SFF s_sffDuck = {0.0, 0.5, 0.5, 0.5};
SFF s_sffFlatten = {0.0, 0.5, 0.5, 0.5};
SFF s_sffStand = {0.0, 0.25, 0.44999999, 0.44999999};
SFF s_sffSidestep = {0.0, 0.25, 0.44999999, 0.44999999};
SFF s_sffRun = {0.0, 0.5, 0.5, 0.5};
glm::vec3 s_posThrowJt = {0.0, 200.0,0.0};
char g_worldWarpContext[] = "world_warp";

VTJTICON g_vtjticon;
JTICON g_rubyicon;
JTICON g_jticon;

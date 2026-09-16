#include "dartgun.h"
#include "dart.h"
#include "jt.h"
#include "actla.h"
#include "sm.h"
#include "rat.h"
#include "actla.h"
#include "hide.h"

DARTGUN* NewDartgun()
{
	return new DARTGUN{};
}

void InitDartgun(DARTGUN* pdartgun)
{
	InitBreak(pdartgun);
    pdartgun->oidDart = OID_Nil;
}

void SetDartgunOidDart(DARTGUN* pdartgun, OID oidDart)
{
    pdartgun->oidDart = oidDart;
}

void* GetDartgunOidDart(DARTGUN* pdartgun)
{
    return &pdartgun->oidDart;
}

void SetDartgunCDarts(DARTGUN* pdartgun, int cDarts)
{
    pdartgun->cDarts = cDarts;
}

void* GetDartgunCDarts(DARTGUN* pdartgun)
{
    return &pdartgun->cDarts;
}

void SetDartgunDtFireInterval(DARTGUN* pdartgun, float dtFireInterval)
{
    pdartgun->dtFireInterval = dtFireInterval;
}

void* GetDartgunDtFireInterval(DARTGUN* pdartgun)
{
    return &pdartgun->dtFireInterval;
}

void SetDartgunDzTarget(DARTGUN* pdartgun, float dzTarget)
{
    pdartgun->dzTarget = dzTarget;
}

void* GetDartgunDzTarget(DARTGUN* pdartgun)
{
    return &pdartgun->dzTarget;
}

void HandleDartgunMessage(DARTGUN* pdartgun, MSGID msgid, void* pv)
{
    HandleAloMessage(pdartgun, msgid, pv);

    if (msgid != MSGID_sma_transition || pv != pdartgun->psma)
        return;

    OID oidCurrent;
    GetSmaGoal((SMA*)pv, &oidCurrent);

    if (oidCurrent != static_cast<OID>(695))
        return;

    if (!pdartgun->fBroken) {
        pdartgun->paloLookAt->pactla->fPaused = 1;
        pdartgun->paloLookAt->pactla->fUseTarget = 0;

        ALO* paloLookAt = pdartgun->paloLookAt;
        paloLookAt->pvtalo->pfnTranslateAloToPos(paloLookAt, &paloLookAt->posOrig);
        paloLookAt->pvtalo->pfnRotateAloToMat(paloLookAt, &paloLookAt->matOrig);

        ALO* paloLid = pdartgun->paloLid;
        paloLid->pvtalo->pfnRotateAloToMat(paloLid, &paloLid->matOrig);
    }
    else 
    {
        RetractSma(pdartgun->psma);
        pdartgun->psma = nullptr;
    }
}

int GetDartgunSize()
{
	return sizeof(DARTGUN);
}

void CloneDartgun(DARTGUN* pdartgun, DARTGUN* pdartgunBase)
{
    CloneBreak(pdartgun, pdartgunBase);

    pdartgun->oidDart = pdartgunBase->oidDart;
    pdartgun->cDarts = pdartgunBase->cDarts;
    pdartgun->pdartReadyToFire = pdartgunBase->pdartReadyToFire;
    pdartgun->pdartFading = pdartgunBase->pdartFading;
    pdartgun->dtFireInterval = pdartgunBase->dtFireInterval;
    pdartgun->dzTarget = pdartgunBase->dzTarget;
    pdartgun->tLastFire = pdartgunBase->tLastFire;
    pdartgun->paloTarget = pdartgunBase->paloTarget;
    pdartgun->pratTarget = pdartgunBase->pratTarget;
    pdartgun->posOrigDart = pdartgunBase->posOrigDart;
    pdartgun->matOrigDart = pdartgunBase->matOrigDart;
    pdartgun->paloOrigDartParent = pdartgunBase->paloOrigDartParent;
    pdartgun->paloLookAt = pdartgunBase->paloLookAt;
    pdartgun->paloLid = pdartgunBase->paloLid;
    pdartgun->psm = pdartgunBase->psm;
    pdartgun->psma = pdartgunBase->psma;

    pdartgun->coidRatTarget = pdartgunBase->coidRatTarget;
    std::memcpy(pdartgun->aoidRatTarget, pdartgunBase->aoidRatTarget, sizeof(pdartgun->aoidRatTarget));

    pdartgun->cpratTarget = pdartgunBase->cpratTarget;
    std::memcpy(pdartgun->apratTarget, pdartgunBase->apratTarget, sizeof(pdartgun->apratTarget));

    pdartgun->cpratInTargetArea = pdartgunBase->cpratInTargetArea;
    std::memcpy(pdartgun->apratInTargetArea, pdartgunBase->apratInTargetArea, sizeof(pdartgun->apratInTargetArea));

    pdartgun->fJtInTargetArea = pdartgunBase->fJtInTargetArea;
}

void BindDartgun(DARTGUN* pdartgun)
{
    BindAlo(pdartgun);

    for (int i = 0; i < pdartgun->coidRatTarget; i++)
    {
        OID oidRat = pdartgun->aoidRatTarget[i];

        RAT* prat = (RAT*)PloFindSwNearest(pdartgun->psw, oidRat, pdartgun);

        if (prat == nullptr)
            continue;

        if (!FIsBasicDerivedFrom(prat, CID_RAT))
            continue;

        int targetIndex = pdartgun->cpratTarget;
        pdartgun->apratTarget[targetIndex] = prat;
        pdartgun->cpratTarget = targetIndex + 1;
    }
}

void SetDartgunGutTarget(DARTGUN* pdartgun)
{
    if (g_pjt != nullptr)
        pdartgun->paloTarget = g_pjt->paloGut;
}

void PostDartgunLoad(DARTGUN* pdartgun)
{
    PostBrkLoad(pdartgun);
    SnipAloObjects(pdartgun, 3, s_asnipDartgun);

    pdartgun->paloLookAt->pactla->fPaused = 1;

    DART* pdart = reinterpret_cast<DART*>(PloFindSwChild(pdartgun->psw, (OID)pdartgun->oidDart, pdartgun));

    pdartgun->posOrigDart = pdart->posOrig;
    pdartgun->matOrigDart = pdart->matOrig;
    pdartgun->paloOrigDartParent = pdart->paloParent;

    for (int idart = 1; idart < pdartgun->cDarts; ++idart) 
    {
        LO* ploClone = PloCloneLo(pdart, pdart->psw, nullptr);

        if (ploClone->pvtlo->pfnBindLo != nullptr)
            ploClone->pvtlo->pfnBindLo(ploClone);

        ploClone->pvtlo->pfnPostLoLoad(ploClone);
        ploClone->pvtlo->pfnRemoveLo(ploClone);
    }

    pdart->pvtlo->pfnSetLoParent(pdart, nullptr);
    pdart->pvtlo->pfnRemoveLo(pdart); 

    PostSwCallback(pdartgun->psw, (PFNMQ)SetDartgunGutTarget, pdartgun, MSGID_callback, nullptr);

    pdartgun->psma = PsmaApplySm(pdartgun->psm, pdartgun, (OID)695, 1);
}

void UpdateDartgun(DARTGUN* pdartgun, float dt)
{
    constexpr OID OID_DartgunIdle = static_cast<OID>(695);
    constexpr OID OID_DartgunTrack = static_cast<OID>(696);
    constexpr OID OID_DartgunTrackRats = static_cast<OID>(697);
    constexpr OID OID_DartgunFire = static_cast<OID>(698);
    constexpr OID OID_DartgunFireRats = static_cast<OID>(699);

    UpdateBrk(pdartgun, dt);

    if (pdartgun->fBroken || pdartgun->paloTarget == nullptr || pdartgun->psma == nullptr)
        return;

    OID oidStateCur;
    OID oidStateGoal;

    GetSmaCur(pdartgun->psma, &oidStateCur);
    GetSmaGoal(pdartgun->psma, &oidStateGoal);

    if (oidStateGoal == OID_Nil)
        oidStateGoal = oidStateCur;

    if (oidStateCur != oidStateGoal)
        return;

    switch (oidStateCur) {
    case OID_DartgunIdle:
        if (pdartgun->fJtInTargetArea) {
            oidStateGoal = OID_DartgunTrack;
        }
        else if (pdartgun->cpratInTargetArea > 0) {
            oidStateGoal = OID_DartgunTrackRats;
        }
        break;

    case OID_DartgunTrack:
        if (pdartgun->fJtInTargetArea) {
            TrackDartgun(pdartgun, &oidStateGoal);
        }
        else if (pdartgun->cpratInTargetArea > 0) {
            oidStateGoal = OID_DartgunTrackRats;
        }
        else {
            oidStateGoal = OID_DartgunIdle;
        }
        break;

    case OID_DartgunTrackRats:
        if (pdartgun->fJtInTargetArea) {
            oidStateGoal = OID_DartgunTrack;
        }
        else if (pdartgun->cpratInTargetArea > 0) {
            TrackDartgun(pdartgun, &oidStateGoal);
        }
        else {
            oidStateGoal = OID_DartgunIdle;
        }
        break;

    case OID_DartgunFire:
        FireDartgun(pdartgun);
        oidStateGoal = OID_DartgunTrack;
        break;

    case OID_DartgunFireRats:
        FireDartgun(pdartgun);
        oidStateGoal = OID_DartgunTrackRats;
        break;
    }

    if (oidStateGoal != oidStateCur)
        SetDartgunGoalState(pdartgun, oidStateGoal);
}

int FIgnoreDartgunIntersection(DARTGUN* pdartgun, DART* psoOther)
{
    if (FIsBasicDerivedFrom(psoOther, CID_DART)) {
        DART* pdart = (DART*)psoOther;

        if (pdart->pdartgunFiredFrom == pdartgun) {
            return 1;
        }
    }

    return FIgnoreSoIntersection((SO*)pdartgun, psoOther);
}

void BreakDartgun(DARTGUN* pdartgun)
{
    DART* pdartReady = pdartgun->pdartReadyToFire;

    if (pdartReady != nullptr) {
        pdartReady->pvtlo->pfnSetLoParent(pdartReady, nullptr);
        pdartReady->pvtlo->pfnRemoveLo(pdartReady);
    }

    if (pdartgun->psma != nullptr) {
        constexpr OID OID_DartgunIdle = static_cast<OID>(695);

        SeekSma(pdartgun->psma, OID_DartgunIdle);

        OID oidStateGoal;
        GetSmaGoal(pdartgun->psma, &oidStateGoal);

        if (oidStateGoal == OID_DartgunIdle) {
            RetractSma(pdartgun->psma);
            pdartgun->psma = nullptr;
        }
    }

    ACTLA* pactlaLookAt = pdartgun->paloLookAt->pactla.get();
    pactlaLookAt->pvtact->pfnRetractAct(pactlaLookAt, 7);

    BreakBrk(pdartgun);
}

void SetDartgunGoalState(DARTGUN* pdartgun, OID oidStateGoal)
{
    constexpr OID OID_DartgunIdle = static_cast<OID>(695);
    constexpr OID OID_DartgunTrack = static_cast<OID>(696);
    constexpr OID OID_DartgunTrackRats = static_cast<OID>(697);
    constexpr OID OID_DartgunFire = static_cast<OID>(698);
    constexpr OID OID_DartgunFireRats = static_cast<OID>(699);

    OID oidStateCurrent;
    GetSmaGoal(pdartgun->psma, &oidStateCurrent);

    if (oidStateCurrent == OID_Nil)
        GetSmaCur(pdartgun->psma, &oidStateCurrent);

    if (oidStateGoal == oidStateCurrent)
        return;

    if (oidStateCurrent == OID_DartgunIdle)
        pdartgun->paloLookAt->pactla->fPaused = 0;

    if (oidStateGoal == OID_DartgunTrack || oidStateGoal == OID_DartgunTrackRats) {
        pdartgun->paloLookAt->pactla->fUseTarget = oidStateGoal == OID_DartgunTrackRats;

        if (oidStateCurrent != OID_DartgunFire && oidStateCurrent != OID_DartgunFireRats)
            pdartgun->tLastFire = 0.0f;
    }

    SetSmaGoal(pdartgun->psma, oidStateGoal);
}

void TrackDartgun(DARTGUN* pdartgun, OID* poidStateGoal)
{
    constexpr OID OID_DartgunTrack = static_cast<OID>(696);
    constexpr OID OID_DartgunFire = static_cast<OID>(698);
    constexpr OID OID_DartgunFireRats = static_cast<OID>(699);

    if (*poidStateGoal == OID_DartgunTrack) {
        pdartgun->paloLookAt->pactla->fUseTarget = 0;

        if (pdartgun->dtFireInterval < g_clock.t - pdartgun->tLastFire && FPrepareDartgunToFire(pdartgun))
            *poidStateGoal = OID_DartgunFire;

        return;
    }

    RAT* pratTarget = pdartgun->pratTarget;

    if (pratTarget == nullptr || !FIsLoInWorld(pratTarget) || pratTarget->rats == RATS_Dead || pratTarget->rats == RATS_Hidden) {
        pratTarget = PratGetDartgunRatTarget(pdartgun);
        pdartgun->pratTarget = pratTarget;
    }

    if (pratTarget == nullptr)
        return;

    SetActlaTarget(pdartgun->paloLookAt->pactla.get(), pratTarget, &g_vecZero);

    if (pdartgun->dtFireInterval < g_clock.t - pdartgun->tLastFire && FPrepareDartgunToFire(pdartgun))
        *poidStateGoal = OID_DartgunFireRats;
}

int FPrepareDartgunToFire(DARTGUN* pdartgun)
{
    if (pdartgun->pdartReadyToFire != nullptr)
        return 1;

    if (pdartgun->pdartFading != nullptr && pdartgun->pdartFading->darts != DARTS_Fading)
        pdartgun->pdartFading = nullptr;

    if (FIsDlEmpty(&pdartgun->psw->dlDartFree) || pdartgun->pdartFading != nullptr) {
        AgeDartgunStuckDart(pdartgun);
        return 0;
    }

    DART* pdart = pdartgun->psw->dlDartFree.pdartFirst;

    pdart->pdartgunFiredFrom = pdartgun;

    pdart->pvtlo->pfnAddLo(pdart);
    pdart->pvtlo->pfnSetLoParent(pdart, pdartgun->paloOrigDartParent);

    SetSoConstraints(pdart, CT_Locked, nullptr, CT_Locked, nullptr);

    pdart->pvtalo->pfnTranslateAloToPos(pdart, &pdartgun->posOrigDart);
    pdart->pvtalo->pfnRotateAloToMat(pdart, &pdartgun->matOrigDart);
    pdart->pvtalo->pfnSetAloVelocityVec(pdart, &g_vecZero);
    pdart->pvtalo->pfnSetAloAngularVelocityVec(pdart, &g_vecZero);

    SetDartDarts(pdart, DARTS_ReadyToFire);

    pdartgun->pdartReadyToFire = pdart;
    pdartgun->tLastFire = g_clock.t;

    return 1;
}

void FireDartgun(DARTGUN* pdartgun)
{
    ALO* paloTarget;
    float dzTarget;

    if (pdartgun->pratTarget != nullptr && pdartgun->paloLookAt->pactla->fUseTarget != 0) {
        RAT* pratTarget = pdartgun->pratTarget;

        if (!FIsLoInWorld(pratTarget) || pratTarget->rats == RATS_Dead || pratTarget->rats == RATS_Hidden) {
            pdartgun->pratTarget = nullptr;
            return;
        }

        paloTarget = reinterpret_cast<ALO*>(pratTarget);
        dzTarget = 15.0f;
    }
    else {
        paloTarget = pdartgun->paloTarget;
        dzTarget = pdartgun->dzTarget;
    }

    DART* pdart = pdartgun->pdartReadyToFire;

    pdartgun->pratTarget = nullptr;
    pdartgun->pdartReadyToFire = nullptr;

    pdart->pvtlo->pfnSetLoParent(pdart, nullptr);
    SetSoConstraints(pdart, CT_Free, nullptr, CT_Free, nullptr);

    if (pdart->pactRestore != nullptr)
        pdart->pactRestore->pvtact->pfnRetractAct(pdart->pactRestore.get(), 7);

    glm::vec3 posTarget = paloTarget->xf.posWorld;
    posTarget.z += dzTarget;

    glm::vec3 dposTarget = posTarget - pdart->xf.posWorld;
    glm::mat3 mat;

    LoadLookAtMatrix(&dposTarget, &mat);
    pdart->pvtalo->pfnRotateAloToMat(pdart, &mat);

    pdart->dtLaunchToTarget = glm::length(dposTarget) / pdart->svDart;

    glm::vec3 vTarget;
    ConvertAloVec(paloTarget->paloRoot, nullptr, &paloTarget->paloRoot->xf.v, &vTarget);

    glm::vec3 posTargetPredicted = posTarget + vTarget * pdart->dtLaunchToTarget;
    dposTarget = posTargetPredicted - pdart->xf.posWorld;

    pdart->dtLaunchToTarget = glm::length(dposTarget) / pdart->svDart;

    glm::vec3 vDart = glm::normalize(dposTarget) * pdart->svDart;
    pdart->pvtalo->pfnSetAloVelocityVec(pdart, &vDart);

    SetDartDarts(pdart, DARTS_Airborne);

    pdart->paloTarget = paloTarget;
    pdart->dzTarget = dzTarget;

    StandardSmokeCloud(&pdart->xf.posWorld, pdart->sRadiusFireCloud);

    void* pvArgs = &pdart;
    HandleLoSpliceEvent(pdartgun, 13, 1, &pvArgs);
}

void AgeDartgunStuckDart(DARTGUN* pdartgun)
{
    if (pdartgun->pdartFading != nullptr)
        return;

    DART* pdartOldest = nullptr;
    float tOldest = FLT_MAX;

    for (HBSK* phbsk = g_dlHbsk.phbskFirst; phbsk != nullptr; phbsk = phbsk->dleHbsk.phbskNext) {
        DART* pdart = PdartOldestHbskStuck(phbsk);

        if (pdart != nullptr && pdart->tDarts < tOldest) {
            pdartOldest = pdart;
            tOldest = pdart->tDarts;
        }
    }

    if (pdartOldest != nullptr) {
        SetDartDarts(pdartOldest, DARTS_Fading);
        pdartgun->pdartFading = pdartOldest;
    }
}

RAT* PratGetDartgunRatTarget(DARTGUN* pdartgun)
{
    for (int i = 0; i < pdartgun->cpratInTargetArea; ++i) {
        RAT* prat = pdartgun->apratInTargetArea[i];

        if (prat == nullptr)
            continue;

        if (!FIsLoInWorld(prat))
            continue;

        if (prat->rats == RATS_Dead || prat->rats == RATS_Hidden)
            continue;

        return prat;
    }

    return nullptr;
}

void AddDartgunRatTarget(DARTGUN* pdartgun, OID oid)
{
    if (pdartgun->coidRatTarget >= 8)
        return;

    pdartgun->aoidRatTarget[pdartgun->coidRatTarget++] = oid;
}

void StartDartgunTargetAreaChange(DARTGUN* pdartgun)
{
    pdartgun->fJtInTargetArea = 0;
    pdartgun->cpratInTargetArea = 0;
}

void AddDartgunTargetAreaTarget(DARTGUN* pdartgun, ALO* paloTarget)
{
    if (FIsBasicDerivedFrom(paloTarget, CID_JT)) {
        pdartgun->fJtInTargetArea = 1;
        return;
    }

    if (!FIsBasicDerivedFrom(paloTarget, CID_RAT))
        return;

    RAT* pratTarget = reinterpret_cast<RAT*>(paloTarget);
    bool fValidTarget = false;

    for (int i = 0; i < pdartgun->cpratTarget; ++i) {
        if (pdartgun->apratTarget[i] == pratTarget) {
            fValidTarget = true;
            break;
        }
    }

    if (!fValidTarget || pdartgun->cpratInTargetArea >= 8)
        return;

    pdartgun->apratInTargetArea[pdartgun->cpratInTargetArea++] = pratTarget;
}

void DeleteDartgun(DARTGUN* pdartgun)
{
	delete pdartgun;
}

SNIP s_asnipDartgun[3] = {
    { 0x04, (OID)0x2B4, offsetof(DARTGUN, paloLookAt) },
    { 0x04, (OID)0x2B5, offsetof(DARTGUN, paloLid) },
    { 0x00, (OID)0x2B6, offsetof(DARTGUN, psm) },
};
#include "smartguard.h"
#include "chkpnt.h"
#include "flash.h"
#include "actla.h"

SMARTGUARD* NewSmartguard()
{
	return new SMARTGUARD{};
}

void InitSmartGuard(SMARTGUARD* psmartguard)
{
	InitStepGuard(psmartguard);

    psmartguard->sDetectNear = 300.0f;
    psmartguard->zDetectNear = 300.0f;

    psmartguard->sSneak = 2000.0f;
    psmartguard->zSneak = 300.0f;

    psmartguard->oidSneakVol = OID_Nil;
    psmartguard->pvolSneak = nullptr;

    for (int i = 16; i >= 0; i--)
        psmartguard->mpsgssgft[i].oidTarget = OID_Nil;

    psmartguard->sgf.oidFlash = OID_Nil;
}

void* GetSmartguardSgf(SMARTGUARD* psmartguard)
{
    return &psmartguard->sgf;
}

void SetSmartguardSgf(SMARTGUARD* psmartguard, const SGF& sgf)
{
    psmartguard->sgf = sgf;
}

void* GetSmartguardSgsFlash(SMARTGUARD* psmartguard)
{
    return &psmartguard->sgf.pflash;
}

void* GetSmartguardSDetectNear(SMARTGUARD* psmartguard)
{
    return &psmartguard->sDetectNear;
}

void SetSmartguardSDetectNear(SMARTGUARD* psmartguard, float sDetectNear)
{
    psmartguard->sDetectNear = sDetectNear;
}

void* GetSmartguardZDetectNear(SMARTGUARD* psmartguard)
{
    return &psmartguard->zDetectNear;
}

void SetSmartguardZDetectNear(SMARTGUARD* psmartguard, float zDetectNear)
{
    psmartguard->zDetectNear = zDetectNear;
}

void* GetSmartguardSSneak(SMARTGUARD* psmartguard)
{
    return &psmartguard->sSneak;
}

void SetSmartguardSSneak(SMARTGUARD* psmartguard, float sSneak)
{
    psmartguard->sSneak = sSneak;
}

void* GetSmartguardZSneak(SMARTGUARD* psmartguard)
{
    return &psmartguard->zSneak;
}

void SetSmartguardZSneak(SMARTGUARD* psmartguard, float zSneak)
{
    psmartguard->zSneak = zSneak;
}

void* GetSmartguardOidSneakVol(SMARTGUARD* psmartguard)
{
    return &psmartguard->oidSneakVol;
}

void SetSmartguardOidSneakVol(SMARTGUARD* psmartguard, OID oidSneakVol)
{
    psmartguard->oidSneakVol = oidSneakVol;
}

void* GetSmartguardUnk0(SMARTGUARD* psmartguard)
{
    return &psmartguard->unk0;
}

void SetSmartguardUnk0(SMARTGUARD* psmartguard, int Unk0)
{
    psmartguard->unk0 = Unk0;
}

void* GetSmartguardfNoDetect(SMARTGUARD* psmartguard)
{
    return &psmartguard->fNoDetect;
}

void SetSmartguardfNoDetect(SMARTGUARD* psmartguard, int fNoDetect)
{
    psmartguard->fNoDetect = fNoDetect;
}

void* GetSmartguardgrfDetection(SMARTGUARD* psmartguard)
{
    return &psmartguard->grfsgsc;
}

void SetSmartguardgrfDetection(SMARTGUARD* psmartguard, int grfDetection)
{
    psmartguard->grfsgsc = grfDetection;
}

int GetSmartguardSize()
{
	return sizeof(SMARTGUARD);
}

void CloneSmartguard(SMARTGUARD* psmartguard, SMARTGUARD* psmartguardBase)
{
    CloneStepguard(psmartguard, psmartguardBase);

    psmartguard->sgf = psmartguardBase->sgf;  // Copy SGF structure (assuming it's trivially copyable)
    psmartguard->sDetectNear = psmartguardBase->sDetectNear;  // Copy the float value
    psmartguard->zDetectNear = psmartguardBase->zDetectNear;  // Copy the float value
    psmartguard->sSneak = psmartguardBase->sSneak;  // Copy the float value
    psmartguard->zSneak = psmartguardBase->zSneak;  // Copy the float value
    psmartguard->oidSneakVol = psmartguardBase->oidSneakVol;  // Copy the OID value

    // Shallow copy of pointer to VOL and ALO (Assuming these are pointer types that need shallow copy)
    psmartguard->pvolSneak = psmartguardBase->pvolSneak;  // Shallow copy of pointer to VOL
    psmartguard->paloFlashTarget = psmartguardBase->paloFlashTarget;  // Shallow copy of pointer to ALO

    // Copying array of SGFT structures
    for (int i = 0; i < 17; i++) {
        psmartguard->mpsgssgft[i] = psmartguardBase->mpsgssgft[i];  // Shallow copy of array elements
    }

    // Shallow copy of EXC pointer (Assuming it's a pointer type)
    psmartguard->pexcSneak = psmartguardBase->pexcSneak;  // Shallow copy of pointer to EXC

    // Copying the integer for the search idle state
    psmartguard->cidleSearch = psmartguardBase->cidleSearch;

    psmartguard->cbarrier = psmartguardBase->cbarrier;

    for (int i = 0; i < 4; i++)
        psmartguard->smartguardBarrier[i] = psmartguardBase->smartguardBarrier[i];
}

void AddSmartguardBarrier(SMARTGUARD* psmartguard, OID oidBarrier)
{
    int cbarrier = psmartguard->cbarrier;

    if (cbarrier < 4) 
    {
        psmartguard->smartguardBarrier[cbarrier].oidBarrier = oidBarrier;
        psmartguard->cbarrier = cbarrier + 1;
    }
}

void PostSmartguardLoad(SMARTGUARD* psmartguard)
{
    PostStepguardLoad(psmartguard);
	PostSmartguardLoadFlashlight(psmartguard);

	if (psmartguard->oidSneakVol != OID_Nil)
		psmartguard->pvolSneak = (VOL*)PloFindSwNearest(psmartguard->psw, (OID)psmartguard->oidSneakVol, psmartguard);

    for (int i = 0; i < psmartguard->cbarrier; i++)
        psmartguard->smartguardBarrier[i].pbarrier = (BARRIER*)PloFindSwObject(psmartguard->psw, 260, psmartguard->smartguardBarrier[i].oidBarrier, (LO*)psmartguard);
}

void PostSmartguardLoadFlashlight(SMARTGUARD* psmartguard)
{
    for (int i = 0; i < 17; i++)
    {
        SGFT* psgft = &psmartguard->mpsgssgft[i];

        if (psgft->oidTarget == OID_Nil)
            psgft->paloTarget = nullptr;
        else
            psgft->paloTarget = (ALO*)PloFindSwNearest(psmartguard->psw, psgft->oidTarget, (LO*)psmartguard);
    }

    FLASH* pflash = nullptr;

    int cplo = CploFindSwObjectsByClass(psmartguard->psw, 513, CID_FLASH, psmartguard, 0, nullptr);

    std::vector <LO*> aplo(cplo);

    cplo = CploFindSwObjectsByClass(psmartguard->psw, 513, CID_FLASH, psmartguard, cplo, aplo.data());

    for (int i = 0; i < cplo; i++)
    {
        LO* plo = aplo[i];
        const int fMatch = FMatchesLoName(plo, psmartguard->sgf.oidFlash);

        if (fMatch == 0)
            plo->pvtlo->pfnRemoveLo(plo);
        else
            pflash = (FLASH*)plo;
    }

    psmartguard->sgf.pflash = pflash;

    if (psmartguard->sgf.pflash != nullptr)
    {
        psmartguard->pactlaSmartguard = PactlaNew(psmartguard->psw, psmartguard->sgf.pflash, &g_vtactla);
        psmartguard->pactlaSmartguard->ackRot = ACK_Velocity;
        psmartguard->pactlaSmartguard->fUseTarget = 1;
        psmartguard->pactlaSmartguard->lmTilt.gMin = 0.0f;
        psmartguard->pactlaSmartguard->lmTilt.gMax = 0.0f;
        InsertAloAct(psmartguard->sgf.pflash, psmartguard->pactlaSmartguard.get());
        psmartguard->ovrSgs.palo = psmartguard->sgf.pflash;
    }
}

int FFilterSmartguardDetect(SMARTGUARD* psmartguard, SO* pso)
{
    if (pso->fNoXpsSelf)
        return 0;

    // The visibility segment starts inside the smartguard hierarchy.  Do not
    // allow the guard (or one of its child collision objects) to occlude its
    // own sight ray.
    if (pso == static_cast<SO*>(psmartguard) ||
        pso->paloRoot == static_cast<ALO*>(psmartguard) ||
        FFindLoParent(pso, psmartguard))
    {
        return 0;
    }

    // The release filter excludes every PO-derived root.  Until the port's
    // superclass metadata is repaired safely, explicitly exclude this
    // guard's enemy so the target itself cannot obstruct its visibility ray.
    SO* psoEnemy = psmartguard->pvtstepguard->pfnPsoEnemyStepguard(psmartguard);

    if (psoEnemy != nullptr &&
        (pso == psoEnemy || pso->paloRoot == static_cast<ALO*>(psoEnemy)))
    {
        return 0;
    }

    if (FIsBasicDerivedFrom(pso->paloRoot, CID_PO))
        return 0;

    if (FIsBasicDerivedFrom(pso->paloRoot, CID_DECOY))
        return 0;

    for (int i = 0; i < psmartguard->cbarrier; i++)
    {
        if (FFindLoParent(pso, (ALO*)psmartguard->smartguardBarrier[i].pbarrier))
            return 0;
    }

    return 1;
}

void UseSmartguardFlashlightTarget(SMARTGUARD* psmartguard, SGS sgs, OID oidTarget)
{
    FValidSgs(sgs);
    psmartguard->mpsgssgft[sgs].oidTarget = oidTarget;
}

void HandleSmartguardMessage(SMARTGUARD* psmartguard, MSGID msgid, void* pv)
{
    HandleStepguardMessage(psmartguard, msgid, pv);

    if ((msgid == MSGID_asega_wrapped) && (pv == psmartguard->pasegaSgs))
        psmartguard->cidleSearch = psmartguard->cidleSearch + 1;
}

int FDetectSmartguard(SMARTGUARD* psmartguard)
{
    if (FSwHandsOff(psmartguard->psw))
        return 0;

    if (psmartguard->fNoDetect)
        return 0;

    if (psmartguard->unk0)
        return 1;

    STEP* pstepEnemy = (STEP*)psmartguard->pvtstepguard->pfnPsoEnemyStepguard(psmartguard);

    if (pstepEnemy == nullptr)
        return 0;

    if (psmartguard->sgf.pflash == nullptr)
        return 0;

    if (psmartguard->sgs == SGS_Dying || psmartguard->sgs == SGS_Stun)
        return 0;

    glm::vec3 posEnemy = pstepEnemy->xf.posWorld;

    if (FIsBasicDerivedFrom((BASIC*)pstepEnemy, CID_STEP)) {
        if (pstepEnemy->fBaseXp)
            posEnemy = pstepEnemy->posBaseXp;
        else
            posEnemy.z -= 70.0f;
    }

    glm::vec3 dpos = posEnemy - psmartguard->xf.posWorld;
    float sEnemy = glm::length(dpos);
    float zEnemy = fabsf(dpos.z);

    int fSneak = 0;

    if (psmartguard->pvolSneak != nullptr)
        fSneak = FCheckVolPoint(psmartguard->pvolSneak, &posEnemy);
    else
        fSneak = sEnemy < psmartguard->sSneak && zEnemy < psmartguard->zSneak;

    if (fSneak) {
        if (psmartguard->pexcSneak == nullptr)
            psmartguard->pexcSneak = PexcSetExcitement(-20);
    }
    else {
        UnsetExcitementHyst(psmartguard->pexcSneak);
        psmartguard->pexcSneak = nullptr;
    }

    const int fFlashWithin = FPosFlashWithin(psmartguard->sgf.pflash, &posEnemy);

    if (!fFlashWithin) {
        if (sEnemy > psmartguard->sDetectNear)
            return 0;

        if (zEnemy > psmartguard->zDetectNear)
            return 0;
    }

    SGGS sggs = SggsGetStepguard(psmartguard);

    if (!(psmartguard->grfDetection & 0x1) &&
        sggs != SGGS_Attack &&
        FCheckStepguardEnemyHidden(psmartguard)) {
        return 0;
    }

    if (psmartguard->grfDetection & 0x2)
        return 1;

    const glm::vec3* aposTestLocal = s_aposTestPatrol;

    if (sggs == SGGS_Attack)
        aposTestLocal = s_aposTestAttack;

    ALO* paloLookFrom = psmartguard->paloHead != nullptr
        ? psmartguard->paloHead
        : psmartguard;

    glm::vec3 posLookFrom = paloLookFrom->xf.posWorld;

    glm::vec3 posMin = posLookFrom;
    glm::vec3 posMax = posLookFrom;
    glm::vec3 aposTestWorld[4];

    for (int i = 0; i < 4; i++) {
        aposTestWorld[i] = aposTestLocal[i];

        ConvertAloPos((ALO*)pstepEnemy, nullptr, &aposTestWorld[i], &aposTestWorld[i]);

        posMin = glm::min(posMin, aposTestWorld[i]);
        posMax = glm::max(posMax, aposTestWorld[i]);
    }

    std::vector<SO*> apso;

    IntersectSwBoundingBox(psmartguard->psw, nullptr, &posMin, &posMax, (PFNFILTER)FFilterSmartguardDetect, psmartguard, apso);

    for (int i = 0; i < 4; i++) {
        SO* psoHit = PsoHitTestLineObjects(0, &posLookFrom, &aposTestWorld[i], apso, nullptr);

        if (psoHit == nullptr)
            return 1;
    }

    return 0;
}

void UpdateSmartguard(SMARTGUARD* psmartguard, float dt)
{
    UpdateStepguard(psmartguard, dt);
    UpdateSmartguardFlashlight(psmartguard);
}

void UpdateSmartguardFlashlight(SMARTGUARD* psmartguard)
{
    FLASH* pflash = psmartguard->sgf.pflash;

    if (pflash == nullptr)
        return;

    ACTLA* pactla = psmartguard->pactlaSmartguard.get();
    SGGS sggs = SggsGetStepguard((STEPGUARD*)psmartguard);

    float scale = 1.3f;
    ALO* paloTarget = nullptr;
    glm::vec3 posTargetLocal = s_posFlashSelf;
    bool fUpdateTarget = false;

    if (sggs == SGGS_Attack) {
        fUpdateTarget = true;

        if (psmartguard->paloFlashTarget != nullptr) {
            paloTarget = psmartguard->paloFlashTarget;
            posTargetLocal = s_posFlashTarget;
        }
        else if (!FStepguardLookLocked((STEPGUARD*)psmartguard)) {
            paloTarget = psmartguard->pvtstepguard->pfnPsoEnemyStepguard(psmartguard);

            if (paloTarget != nullptr) {
                posTargetLocal = s_posFlashTarget;
            }
            else {
                paloTarget = psmartguard;
                posTargetLocal = s_posFlashSelf;
            }
        }
        else {
            paloTarget = psmartguard;
            posTargetLocal = s_posFlashSelf;
        }
    }
    else if (sggs >= SGGS_Patrol && sggs < SGGS_Dead) {
        fUpdateTarget = true;

        if (psmartguard->paloFlashTarget != nullptr) {
            paloTarget = psmartguard->paloFlashTarget;
            posTargetLocal = s_posFlashTarget;
        }
        else {
            paloTarget = psmartguard;
            posTargetLocal = s_posFlashSelf;
        }

        if (sggs == SGGS_Patrol)
            scale = 1.0f;
    }
    else if (sggs == SGGS_Dead) {
        fUpdateTarget = true;
        paloTarget = psmartguard;
        posTargetLocal = s_posFlashSelf;
        scale = 0.1f;
    }

    if (fUpdateTarget) {
        pactla->paloTarget = paloTarget;
        pactla->posTargetLocal = posTargetLocal;
    }

    pflash->gScaleTarget = scale;

}

void OnSmartguardEnteringSgs(SMARTGUARD* psmartguard, SGS sgsPrev, ASEG* pasegOverride)
{
    OnStepguardEnteringSgs(psmartguard, sgsPrev, pasegOverride);

    if (psmartguard->sgs == SGS_SearchIdle)
        psmartguard->cidleSearch = 0;
    else if (psmartguard->sgs == SGS_Dying)
    {
        FLASH* pflash = psmartguard->sgf.pflash;

        if (pflash != nullptr)
            pflash->pvtlo->pfnRemoveLo(pflash);

        UnsetExcitement(psmartguard->pexcSneak);
        psmartguard->pexcSneak = nullptr;
    }

    psmartguard->paloFlashTarget = psmartguard->mpsgssgft[psmartguard->sgs].paloTarget;
}

int FCanSmartguardAttack(SMARTGUARD* psmartguard)
{
    const int fCanBase = FCanStepguardAttack(psmartguard);

    if (fCanBase == 0)
        return 0;

    const int fDetect = psmartguard->pvtsmartguard->pfnFDetectSmartguard(psmartguard);

    return fDetect;
}

int SgasGetSmartguard(SMARTGUARD* psmartguard)
{
    if (psmartguard->sgs == SGS_Dying) {
        return 1;
    }

    if (psmartguard->sgs == SGS_SearchIdle && psmartguard->cidleSearch > 0) {
        if (g_clock.t - psmartguard->tSgs <= 5.0f) {
            return 1;
        }

        return 2;
    }

    return 0;
}

void FreezeSmartguard(SMARTGUARD* psmartguard, int fFreeze)
{
    FreezeSo(psmartguard, fFreeze);

    if (fFreeze != 0) 
    {
        UnsetExcitement(psmartguard->pexcSneak);
        psmartguard->pexcSneak = nullptr;
    }
}

void DeleteSmartGuard(SMARTGUARD* psmartguard)
{
	delete psmartguard;
}

glm::vec3 s_posFlashSelf = { 1000.0f, 0.0f, 0.0f };
glm::vec3 s_posFlashTarget = {0.0f, 0.0f, 0.0f};

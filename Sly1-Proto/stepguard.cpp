#include "stepguard.h"

STEPGUARD* NewStepguard()
{
	return new STEPGUARD{};
}

void InitStepGuard(STEPGUARD* pstepguard)
{
	InitStep(pstepguard);

    // Store original ABSP flags
    //uint64_t abspFlags = *reinterpret_cast<uint64_t*>(&pstepguard->bspcCamera.absp);

    // Clear fields
    pstepguard->oidPathzone = OID_Nil;
    //*reinterpret_cast<uint64_t*>(&pstepguard->bspcCamera.absp) = (abspFlags & 0xffffff0fffffffff) | 0x2000000000;

    // Attack range settings
    pstepguard->radAttackMax = 0.1745329f;
    pstepguard->sgs = SGS_Nil;

    pstepguard->lmAttackS.gMax = 300.0f;
    pstepguard->lmAttackS.gMin = 0.0f;

    pstepguard->lmAttackT.gMax = 0.5f;
    pstepguard->lmAttackT.gMin = 0.2f;

    // Clear ASEG slots
    for (int i = 0; i < 0x11; ++i) {
        pstepguard->mpsgssga[i].oidAseg = OID_Nil;
    }

    // Clear physics object slots
    for (int i = 0; i < 0x11; ++i) {
        pstepguard->mpsgssgp[i].oidPhys = OID_Nil;
    }

    // Initialize stepguard state
    //pstepguard->ichkDead = IchkAllocChkmgr(&g_chkmgr);
    pstepguard->ccoin = -1;
    pstepguard->fFirstFrame = 1;
    pstepguard->oidEnemyExternal = OID_Nil;
    pstepguard->clqSToUPursue.g0 = 1.0f;

    // Update ABSP flags again (redundant but retained for parity)
    //abspFlags = *reinterpret_cast<uint64_t*>(&pstepguard->bspcCamera.absp);
    //*reinterpret_cast<uint64_t*>(&pstepguard->bspcCamera.absp) = (abspFlags & 0xffffff0fffffffff) | 0x2000000000;

    // Set PO as not playable
    //pstepguard->pvtpo->pfnSetPoPlayable(pstepguard, 0);

    // Freefall limits
    pstepguard->lmRDtFreefall.gMin = 1.0f;
    pstepguard->lmRDtFreefall.gMax = 2.5f;

    // Physics and timing state
    pstepguard->rGravity = 1.5f;
    pstepguard->tLostPlayer = std::numeric_limits<float>::max();
    pstepguard->uIdleStart = -1.0f;
    pstepguard->ficg.grficBomb = ' ';
    pstepguard->pstepguardBase = pstepguard;
    pstepguard->tTaunt = std::numeric_limits<float>::max();
    pstepguard->fAutoFadeDying = 1;
    pstepguard->uFling = 1.0f;
}

void OnStepguardAdd(STEPGUARD* pstepguard)
{
	OnPoAdd(pstepguard);
}

void OnStepguardRemove(STEPGUARD* pstepguard)
{
	OnPoRemove(pstepguard);
}

void CloneStepguard(STEPGUARD* pstepguard, STEPGUARD* pstepguardBase)
{
	SGG *psgg = pstepguard->psgg;
	int ichkDead = pstepguard->ichkDead;

	ClonePo(pstepguard, pstepguardBase);

    pstepguard->sgs = pstepguardBase->sgs;
    pstepguard->tSgs = pstepguardBase->tSgs;
    pstepguard->grfsgsc = pstepguardBase->grfsgsc;
    pstepguard->sgsNext = pstepguardBase->sgsNext;
    pstepguard->paloHead = pstepguardBase->paloHead;
    pstepguard->lmAttackS = pstepguardBase->lmAttackS;
    pstepguard->lmAttackT = pstepguardBase->lmAttackT;
    pstepguard->tAttackNext = pstepguardBase->tAttackNext;
    pstepguard->radAttackMax = pstepguardBase->radAttackMax;
    pstepguard->pasegPatrol = pstepguardBase->pasegPatrol;
    pstepguard->pasegaPatrol = pstepguardBase->pasegaPatrol;

    for (int i = 0; i < 17; ++i)
    {
        pstepguard->mpsgssga[i] = pstepguardBase->mpsgssga[i];
        pstepguard->mpsgssgp[i] = pstepguardBase->mpsgssgp[i];
    }

    pstepguard->pasegaSgs = pstepguardBase->pasegaSgs;
    pstepguard->csgmap = pstepguardBase->csgmap;

    for (int i = 0; i < 8; ++i)
        pstepguard->asgmap[i] = pstepguardBase->asgmap[i];

    pstepguard->pasegPrev = pstepguardBase->pasegPrev;
    pstepguard->uAsegaPrev = pstepguardBase->uAsegaPrev;
    pstepguard->psoPhysDefault = pstepguardBase->psoPhysDefault;
    pstepguard->posSearch = pstepguardBase->posSearch;
    pstepguard->radTargetSearch = pstepguardBase->radTargetSearch;
    pstepguard->tPatrolReturn = pstepguardBase->tPatrolReturn;
    pstepguard->tPatrolPrev = pstepguardBase->tPatrolPrev;
    pstepguard->svtPatrolPrev = pstepguardBase->svtPatrolPrev;
    pstepguard->rpk = pstepguardBase->rpk;
    pstepguard->oidPathzone = pstepguardBase->oidPathzone;
    pstepguard->ppathzone = pstepguardBase->ppathzone;
    pstepguard->cposPath = pstepguardBase->cposPath;
    pstepguard->iposPath = pstepguardBase->iposPath;

    for (int i = 0; i < 16; ++i)
        pstepguard->aposPath[i] = pstepguardBase->aposPath[i];

    pstepguard->posGoal = pstepguardBase->posGoal;
    pstepguard->posGoalPath = pstepguardBase->posGoalPath;
    pstepguard->fPathValid = pstepguardBase->fPathValid;
    pstepguard->tPathValid = pstepguardBase->tPathValid;
    pstepguard->cStun = pstepguardBase->cStun;
    pstepguard->fTumbleStun = pstepguardBase->fTumbleStun;
    pstepguard->dtFade = pstepguardBase->dtFade;
    pstepguard->pwaypoint = pstepguardBase->pwaypoint;
    pstepguard->csge = pstepguardBase->csge;

    for (int i = 0; i < 4; ++i)
        pstepguard->asge[i] = pstepguardBase->asge[i];

    pstepguard->pxpEffect = pstepguardBase->pxpEffect;
    pstepguard->vThrow = pstepguardBase->vThrow;
    pstepguard->csgda = pstepguardBase->csgda;

    for (int i = 0; i < 4; ++i)
        pstepguard->asgda[i] = pstepguardBase->asgda[i];

    pstepguard->fExplExplicit = pstepguardBase->fExplExplicit;
    pstepguard->oidExpl = pstepguardBase->oidExpl;
    pstepguard->pexpl = pstepguardBase->pexpl;
    pstepguard->fRwmExplicit = pstepguardBase->fRwmExplicit;
    pstepguard->oidRwm = pstepguardBase->oidRwm;
    pstepguard->prwm = pstepguardBase->prwm;
    pstepguard->fFirstFrame = pstepguardBase->fFirstFrame;
    pstepguard->uIdleStart = pstepguardBase->uIdleStart;
    pstepguard->fFlying = pstepguardBase->fFlying;
    pstepguard->dzFlying = pstepguardBase->dzFlying;
    pstepguard->radTilt = pstepguardBase->radTilt;
    pstepguard->clqSToUPursue = pstepguardBase->clqSToUPursue;
    pstepguard->fEnemyExternal = pstepguardBase->fEnemyExternal;
    pstepguard->fRebindEnemy = pstepguardBase->fRebindEnemy;
    pstepguard->psoEnemyExternal = pstepguardBase->psoEnemyExternal;
    pstepguard->oidEnemyExternal = pstepguardBase->oidEnemyExternal;
    pstepguard->psoSubscribe = pstepguardBase->psoSubscribe;
    pstepguard->ccoin = pstepguardBase->ccoin;
    pstepguard->fAnimatedJump = pstepguardBase->fAnimatedJump;
    pstepguard->fLockHeading = pstepguardBase->fLockHeading;
    pstepguard->fLockHeadingExternal = pstepguardBase->fLockHeadingExternal;
    pstepguard->fLockState = pstepguardBase->fLockState;
    pstepguard->fExternalAI = pstepguardBase->fExternalAI;
    pstepguard->fFreefall = pstepguardBase->fFreefall;
    pstepguard->fFreefallAseg = pstepguardBase->fFreefallAseg;
    pstepguard->dxFreefall = pstepguardBase->dxFreefall;
    pstepguard->dtFreefall = pstepguardBase->dtFreefall;
    pstepguard->lmRDtFreefall = pstepguardBase->lmRDtFreefall;
    pstepguard->tFreefallLand = pstepguardBase->tFreefallLand;
    pstepguard->posFreefallLand = pstepguardBase->posFreefallLand;
    pstepguard->vFreefallLand = pstepguardBase->vFreefallLand;
    pstepguard->uFling = pstepguardBase->uFling;
    pstepguard->fAutoFadeDying = pstepguardBase->fAutoFadeDying;
    pstepguard->tGiveUpSearch = pstepguardBase->tGiveUpSearch;
    pstepguard->tStopPatrol = pstepguardBase->tStopPatrol;
    pstepguard->tTaunt = pstepguardBase->tTaunt;
    pstepguard->tLostPlayer = pstepguardBase->tLostPlayer;
    pstepguard->pstepguardBase = pstepguardBase;
	pstepguard->ichkDead = ichkDead;
	pstepguard->psgg = psgg;
}

void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pstepguard, pbis);
    SnipAloObjects(pstepguard, 1, s_asnipStepguardLoad);
    LoadStepguardAnimations(pstepguard);
    LoadStepguardPhys(pstepguard);
}

void BindStepguard(STEPGUARD* pstepguard)
{
    BindAlo(pstepguard);

    // Resolve and bind pathzone if specified
    if (pstepguard->oidPathzone != OID_Nil) {
        pstepguard->ppathzone = (PATHZONE*)PloFindSwNearest(pstepguard->psw, pstepguard->oidPathzone, (LO*)pstepguard);
    }

    // Rebind enemy object if external enemy override is specified
    if (pstepguard->oidEnemyExternal != OID_Nil) {
        RebindStepguardEnemy(pstepguard);
    }

    // Bind explosion logic (EXPL), either inferred or explicit
    if (!pstepguard->fExplExplicit) {
        //InferExpl(&pstepguard->pexpl, pstepguard);
    }
    else if (pstepguard->oidExpl != OID_Nil) {
        pstepguard->pexpl = (EXPL*)PloFindSwObject(pstepguard->psw, 0x101, pstepguard->oidExpl, (LO*)pstepguard);
    }
}

void RebindStepguardEnemy(STEPGUARD* pstepguard)
{
    constexpr int kMaxResults = 32;
    LO* aplo[kMaxResults];

    // Find all objects matching the external enemy OID
    int count = CploFindSwObjects(pstepguard->psw, 5, pstepguard->oidEnemyExternal, nullptr, kMaxResults, aplo);
    if (count <= 0) {
        pstepguard->psoEnemyExternal = nullptr;
        pstepguard->fEnemyExternal = 1;
        return;
    }

    // Compute the test point from stepguard's position + DT_StepguardRebind * direction
    glm::vec3 pos = pstepguard->xf.pos;
    glm::vec3 dir(pstepguard->xf.v.x, pstepguard->xf.v.y, pstepguard->xf.v.z);
    glm::vec3 testPoint = pos + dir * DT_StepguardRebind;

    SO* bestMatch = nullptr;
    float bestDistSq = std::numeric_limits<float>::max();

    for (int i = 0; i < count; ++i) {
        SO* candidate = static_cast<SO*>(aplo[i]);

        // Candidate position + forward * DT_StepguardRebind
        glm::vec3 cpos = candidate->xf.pos;
        glm::vec3 cdir(candidate->xf.v.x, candidate->xf.v.y, candidate->xf.v.z);
        glm::vec3 cpoint = cpos + cdir * DT_StepguardRebind;

        float distSq = glm::distance2(testPoint, cpoint);

        // Prefer current enemy if distance is the same or slightly worse
        if (candidate == pstepguard->psoEnemyExternal)
            distSq *= 0.5f;

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestMatch = candidate;
        }
    }

    pstepguard->psoEnemyExternal = bestMatch;
    pstepguard->fEnemyExternal = 1;
}

void LoadStepguardAnimations(STEPGUARD* pstepguard)
{
    // Resolve ASEGs for static stepguard animation slots (SGA)
    for (int i = 0; i <= 0x10; ++i) {
        SGA* sga = &pstepguard->mpsgssga[i];

        if (sga->oidAseg != OID_Nil) {
            ASEG *paseg = PasegFindStepguard(pstepguard, sga->oidAseg);
            sga->paseg = paseg;
            sga->oidAseg = OID_Nil;
        }
    }

    // Resolve ASEGs for dynamic animation data (SGDA)
    for (int i = 0; i < pstepguard->csgda; ++i) {
        SGDA* sgda = &pstepguard->asgda[i];

        if (sgda->oidAseg != OID_Nil) {
            ASEG *paseg = PasegFindStepguard(pstepguard, sgda->oidAseg);
            sgda->paseg = paseg;
            sgda->oidAseg = OID_Nil;
        }
    }
}

ASEG* PasegFindStepguard(STEPGUARD* pstepguard, OID oidAseg)
{
    ASEG *paseg = (ASEG*)PloFindSwObject(pstepguard->psw, 0x101, oidAseg, (LO*)pstepguard->pstepguardBase);

    if (paseg != nullptr) 
    {
        paseg->fDefault = 0;
        StripAsegAlo(paseg, pstepguard);
        SnipLo(paseg);
    }
    return paseg;
}

void LoadStepguardPhys(STEPGUARD* pstepguard)
{
    for (int i = 0; i <= 0x10; ++i) 
    {
        SGP* physSlot = &pstepguard->mpsgssgp[i];

        if (physSlot->oidPhys != OID_Nil) 
        {
            SO* plo = (SO*)PloFindSwObject(pstepguard->psw, 0x101, physSlot->oidPhys, pstepguard);

            physSlot->psoPhys = plo;

            if (plo != nullptr) {
                SnipLo(plo);
            }
        }
    }
}

void PostStepguardLoad(STEPGUARD* pstepguard)
{
    PostStepLoad(pstepguard);
}

void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro)
{
	RenderStepSelf(pstepguard, pcm, pro);
}

int GetStepguardSize()
{
	return sizeof(STEPGUARD);
}

void DeleteStepguard(STEPGUARD* pstepguard)
{
	delete pstepguard;
}

SGG* NewSgg()
{
    return new SGG{};
}

void InitSgg(SGG* psgg)
{
    InitLo(psgg);
}

int GetSggSize()
{
    return sizeof(SGG);
}

void CloneSgg(SGG* psgg, SGG* psggBase)
{
    CloneLo(psgg, psggBase);

    psgg->posEnemyLast = psggBase->posEnemyLast;

    // Copy SGGS (assuming it's trivially copyable)
    psgg->sggs = psggBase->sggs;

    // Copy simple float values
    psgg->tSggs = psggBase->tSggs;

    // Copy integer values
    psgg->cpstepguard = psggBase->cpstepguard;
    psgg->coidStepguard = psggBase->coidStepguard;
    psgg->cpxfmSearch = psggBase->cpxfmSearch;
    psgg->coidXfmSearch = psggBase->coidXfmSearch;
    psgg->cpalarm = psggBase->cpalarm;
    psgg->fCallback = psggBase->fCallback;

    // Copy arrays (assuming shallow copy for the pointers)
    for (int i = 0; i < 16; ++i)
    {
        psgg->apstepguard[i] = psggBase->apstepguard[i];
        psgg->aoidStepguard[i] = psggBase->aoidStepguard[i];
        psgg->apxfmSearch[i] = psggBase->apxfmSearch[i];
        psgg->aoidXfmSearch[i] = psggBase->aoidXfmSearch[i];
    }

    // Copy pointer members (assuming shallow copy is sufficient)
    for (int i = 0; i < 4; ++i)
    {
        psgg->apalarm[i] = psggBase->apalarm[i];
    }

    // Copy other pointer members
    psgg->oidSync = psggBase->oidSync;
    psgg->pexcAttack = psggBase->pexcAttack;

    // Copy detect flag
    psgg->fDetect = psggBase->fDetect;
}

void BindSgg(SGG* psgg)
{

}

void DeleteSgg(SGG* psgg)
{
    delete psgg;
}

float DT_StepguardRebind = 0.34999999;
SNIP s_asnipStepguardLoad[1] =
{
    2, OID_stepguard_phys_patrol, offsetof(STEPGUARD, mpsgssgp[1].psoPhys)
};
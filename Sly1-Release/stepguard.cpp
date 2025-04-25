#include "stepguard.h"

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

void DeleteSgg(SGG* psgg)
{
	delete psgg;
}

STEPGUARD* NewStepguard()
{
	return new STEPGUARD{};
}

void InitStepGuard(STEPGUARD* pstepguard)
{
	InitStep(pstepguard);
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
    SGG* psgg = pstepguard->psgg;
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
    pstepguard->pstepguardBase = pstepguardBase->pstepguardBase;
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
    pstepguard->ichkDead = ichkDead;
    pstepguard->psgg = psgg;
}

void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pstepguard, pbis);
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

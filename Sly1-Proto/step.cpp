#include "step.h"

STEP* NewStep()
{
	return new STEP{};
}

void InitStep(STEP* pstep)
{
	InitPo(pstep);
}

int GetStepSize()
{
	return sizeof(STEP);
}

void UpdateStepXfWorld(STEP* pstep)
{
	UpdateSoXfWorld(pstep);
}

void RenderStepSelf(STEP* pstep, CM* pcm, RO* pro)
{
	RenderSoSelf(pstep, pcm, pro);
}

void CloneStep(STEP* pstep, STEP* pstepBase)
{
	ClonePo(pstep, pstepBase);

    pstep->paloLeftFoot = pstepBase->paloLeftFoot;
    pstep->paloRightFoot = pstepBase->paloRightFoot;
    pstep->paloLeftHand = pstepBase->paloLeftHand;
    pstep->paloRightHand = pstepBase->paloRightHand;
    pstep->paloLeftHeel = pstepBase->paloLeftHeel;
    pstep->paloRightHeel = pstepBase->paloRightHeel;
    pstep->paloLeftBall = pstepBase->paloLeftBall;
    pstep->paloRightBall = pstepBase->paloRightBall;

    // Copy simple float values
    pstep->radJoy = pstepBase->radJoy;
    pstep->uJoyDeflect = pstepBase->uJoyDeflect;
    pstep->radTarget = pstepBase->radTarget;
    pstep->uTarget = pstepBase->uTarget;
    pstep->zBaseTarget = pstepBase->zBaseTarget;

    // Copy glm::vec3 (trivially copyable)
    pstep->vTarget = pstepBase->vTarget;

    // Copy mat3 (trivially copyable)
    pstep->matTarget = pstepBase->matTarget;

    // Copy flags and int values
    pstep->fBaseXp = pstepBase->fBaseXp;
    pstep->tBaseXp = pstepBase->tBaseXp;
    pstep->uFootfall = pstepBase->uFootfall;
    pstep->tLastFootfall = pstepBase->tLastFootfall;
    pstep->dzBase = pstepBase->dzBase;
    pstep->iposBase = pstepBase->iposBase;

    // Copy glm::vec3 (for position and normal)
    pstep->posBaseXp = pstepBase->posBaseXp;
    pstep->normalBaseXp = pstepBase->normalBaseXp;

    // Copy pointers to ALO (assumed shallow copy)
    pstep->psoBaseXp = pstepBase->psoBaseXp;
    pstep->psoPhys = pstepBase->psoPhys;

    // Copy SMP structures (assuming they are trivially copyable)
    pstep->smpSpin = pstepBase->smpSpin;
    pstep->smpCompress = pstepBase->smpCompress;
    pstep->smpExpand = pstepBase->smpExpand;

    // Copy CLQ (assuming shallow copy is sufficient)
    pstep->clqDradToURun = pstepBase->clqDradToURun;

    // Copy gravity and related float values
    pstep->rGravity = pstepBase->rGravity;

    // Copy pointers to glm::vec3 arrays (assuming no deep copy is needed)
    pstep->pposBase = pstepBase->pposBase;
    pstep->pposBumper = pstepBase->pposBumper;

    // Copy other variables that hold data about footfall and base positions
    pstep->paloLeftFoot = pstepBase->paloLeftFoot;
    pstep->paloRightFoot = pstepBase->paloRightFoot;
    pstep->paloLeftHand = pstepBase->paloLeftHand;
    pstep->paloRightHand = pstepBase->paloRightHand;
    pstep->paloLeftHeel = pstepBase->paloLeftHeel;
    pstep->paloRightHeel = pstepBase->paloRightHeel;
    pstep->paloLeftBall = pstepBase->paloLeftBall;
    pstep->paloRightBall = pstepBase->paloRightBall;
}

void PostStepLoad(STEP* pstep)
{
    PostAloLoad(pstep);
}

void RotateStepToMat(STEP* pstep, glm::mat3& pmat)
{
	RotateSoToMat(pstep, pmat);
}

void DeleteStep(STEP* pstep)
{
	delete pstep;
}

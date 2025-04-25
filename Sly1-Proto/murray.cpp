#include "murray.h"

MURRAY* NewMurray()
{
	return new MURRAY{};
}

void InitMurray(MURRAY* pmurray)
{
	InitStepGuard(pmurray);
}

int GetMurraySize()
{
	return sizeof(MURRAY);
}

void CloneMurray(MURRAY* pmurray, MURRAY* pmurrayBase)
{
    CloneStepguard(pmurray, pmurrayBase);

    // Shallow copy fields
    pmurray->cpaloEnemy = pmurrayBase->cpaloEnemy;
    for (int i = 0; i < 4; ++i)
        pmurray->apaloEnemy[i] = pmurrayBase->apaloEnemy[i];
    pmurray->pxfmPatrol = pmurrayBase->pxfmPatrol;
    pmurray->pxfmPursue = pmurrayBase->pxfmPursue;
    pmurray->pasegCelebrateKey = pmurrayBase->pasegCelebrateKey;
    pmurray->pturret = pmurrayBase->pturret;
}

void DeleteMurray(MURRAY* pmurray)
{
	delete pmurray;
}

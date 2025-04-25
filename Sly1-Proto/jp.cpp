#include "jp.h"

JP* NewJp()
{
	return new JP{};
}

void InitJp(JP* pjp)
{
	InitStep(pjp);
}

int GetJpSize()
{
	return sizeof(JP);
}

void CloneJp(JP* pjp, JP* pjpBase)
{
    CloneStep(pjp, pjpBase);

    // Clone the members
    pjp->jps = pjpBase->jps;
    pjp->tJps = pjpBase->tJps;
    pjp->pasegaCur = pjpBase->pasegaCur;
    pjp->radPan = pjpBase->radPan;
    pjp->radTilt = pjpBase->radTilt;
    pjp->paloTurret = pjpBase->paloTurret;

    // Clone the ablrunRun array (assuming it is a simple array copy)
    for (int i = 0; i < 6; ++i)
    {
        pjp->ablrunRun[i] = pjpBase->ablrunRun[i];
    }

    // Clone the pasegblRun, pasegStand, pasegThrust, pasegFloat, pasegZap, pasegDead pointers
    pjp->pasegblRun = pjpBase->pasegblRun;
    pjp->pasegStand = pjpBase->pasegStand;
    pjp->pasegThrust = pjpBase->pasegThrust;
    pjp->pasegFloat = pjpBase->pasegFloat;
    pjp->pasegZap = pjpBase->pasegZap;
    pjp->pasegDead = pjpBase->pasegDead;

    // Clone the sffRun
    pjp->sffRun = pjpBase->sffRun;

    // Clone the ptargetCur pointer
    pjp->ptargetCur = pjpBase->ptargetCur;

    // Clone the ground-related variables
    pjp->fGround = pjpBase->fGround;
    pjp->tGround = pjpBase->tGround;
    pjp->posGround = pjpBase->posGround;

    // Clone the flash-related variables
    pjp->fFlash = pjpBase->fFlash;
    pjp->tFlash = pjpBase->tFlash;

    // Clone the charm-related variables
    pjp->tCharm = pjpBase->tCharm;
    pjp->tCharmPending = pjpBase->tCharmPending;
    pjp->fCharmEnabled = pjpBase->fCharmEnabled;

    // Clone the paloJt pointer
    pjp->paloJt = pjpBase->paloJt;

    // Clone the xpZap pointer
    pjp->pxpZap = pjpBase->pxpZap;
}

void RenderJpAll(JP* pjp, CM* pcm, RO* pro)
{
	RenderAloAll(pjp, pcm, pro);
}

void RenderJpSelf(JP* pjp, CM* pcm, RO* pro)
{
	RenderJpAll(pjp, pcm, pro);
}

void DeleteJp(JP *pjp)
{
	delete pjp;
}

#include "tank.h"

TANK* NewTank()
{
	return new TANK{};
}

void InitTank(TANK* ptank)
{
	InitStep(ptank);
}

int GetTankSize()
{
	return sizeof(TANK);
}

void CloneTank(TANK* ptank, TANK* ptankBase)
{
    CloneStep(ptank, ptankBase);

    ptank->tanks = ptankBase->tanks;
    ptank->tTanks = ptankBase->tTanks;
    ptank->fFlash = ptankBase->fFlash;
    ptank->tFlash = ptankBase->tFlash;
    ptank->tCharm = ptankBase->tCharm;
    ptank->tCharmPending = ptankBase->tCharmPending;
    ptank->fCharmEnabled = ptankBase->fCharmEnabled;
    ptank->fAllowEject = ptankBase->fAllowEject;
    ptank->pxpZap = ptankBase->pxpZap;
    ptank->paloJt = ptankBase->paloJt;
    ptank->paloGut = ptankBase->paloGut;
    ptank->pactadjGut = ptankBase->pactadjGut;
    ptank->paloHead = ptankBase->paloHead;
    ptank->pactadjHead = ptankBase->pactadjHead;
    ptank->psm = ptankBase->psm;
    ptank->psma = ptankBase->psma;
    ptank->ppntAnchor = ptankBase->ppntAnchor;
}

void PostTankLoad(TANK* ptank)
{
    PostStepLoad(ptank);
}

void UpdateTank(TANK* ptank, float dt)
{
    UpdateStep(ptank, dt);
}

void RenderTankAll(TANK* ptank, CM* pcm, RO* pro)
{
	RenderAloAll(ptank, pcm, pro);
}

void DeleteTank(TANK* ptank)
{
	delete ptank;
}
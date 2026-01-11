#include "water.h"

WATER* NewWater()
{
	return new WATER{};
}

void InitWater(WATER* pwater)
{
	InitSo(pwater);
	pwater->gBuoyancy = 1.0;
	pwater->gViscosity = 1.0;
	pwater->zpd.zpk = ZPK_Water;
}

int GetWaterSize()
{
	return sizeof(WATER);
}

void CloneWater(WATER* pwater, WATER* pwaterBase)
{
	CloneSo(pwater, pwaterBase);

	pwater->pxaTargets = pwaterBase->pxaTargets;
	pwater->pmapCurrent = pwaterBase->pmapCurrent;
	pwater->mrg = pwaterBase->mrg;
	pwater->vCurrent = pwaterBase->vCurrent;
	pwater->fSplash = pwaterBase->fSplash;
	pwater->fZap = pwaterBase->fZap;
	pwater->zpd = pwaterBase->zpd;
}

void PostWaterLoad(WATER* pwater)
{
	PostAloLoad(pwater);
}

void UpdateWater(WATER* pwater, float dt)
{
	UpdateSo(pwater, dt);
}

void DeleteWater(WATER *pwater)
{
	delete pwater;
}

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
	LO lo = *pwater;
	*pwater = *pwaterBase;
	memcpy(pwater, &lo, sizeof(LO));

	CloneLo(pwater, pwaterBase);

	ClearDl(&pwater->dlChild);

	pwater->pxa = nullptr;
	pwater->grfpvaXpValid = 0;
	pwater->pstso = nullptr;
}

void DeleteWater(WATER *pwater)
{
	delete pwater;
}

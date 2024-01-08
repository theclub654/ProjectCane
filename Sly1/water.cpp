#include "water.h"

void* NewWater()
{
	return new WATER;
}

void InitWater(WATER* pwater)
{
	InitSo(pwater);
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

void DeleteWater(LO* plo)
{
	delete (WATER*)plo;
}

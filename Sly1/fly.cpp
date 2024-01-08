#include "fly.h"

void* NewFly()
{
	return new FLY;
}

void InitFly(FLY* pfly)
{
	InitSo(pfly);
	AppendDlEntry(&pfly->psw->dlFly, pfly);
}

int GetFlySize()
{
	return sizeof(FLY);
}

void CloneFly(FLY* pfly, FLY* pflyBase)
{
	LO lo = *pfly;
	*pfly = *pflyBase;
	memcpy(pfly, &lo, sizeof(LO));

	CloneLo(pfly, pflyBase);

	ClearDl(&pfly->dlChild);

	pfly->pxa = nullptr;
	pfly->grfpvaXpValid = 0;
	pfly->pstso = nullptr;
}

void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pfly, pbis);
}

void DeleteFly(LO* plo)
{
	delete (FLY*)plo;
}

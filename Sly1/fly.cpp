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

void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pfly, pbis);
}

void DeleteFly(LO* plo)
{
	delete (FLY*)plo;
}

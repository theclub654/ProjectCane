#include "mgv.h"

void* NewMgv()
{
	return new MGV;
}

void InitMgv(MGV* pmgv)
{
	InitPo(pmgv);
	pmgv->cpoints = 200;
	pmgv->clives = 5;
}

void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pmgv, pbis);
}

void DeleteMgv(LO* plo)
{
	delete (MGV*)plo;
}

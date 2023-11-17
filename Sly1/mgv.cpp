#include "mgv.h"

void InitMgv(MGV* pmgv)
{
	InitPo(pmgv);
	pmgv->cpoints = 200;
	pmgv->clives = 5;
}

void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis)
{
	std::cout << "MGV Size: " << sizeof(MGV) << "\n";
	LoadSoFromBrx(pmgv, pbis);
}

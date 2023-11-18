#include "fly.h"

void InitFly(FLY* pfly)
{
	//std::cout << "FLY Size: " << sizeof(FLY) << "\n";
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

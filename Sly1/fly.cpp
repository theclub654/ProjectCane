#include "fly.h"

void InitFly(FLY* pfly)
{
	InitSo(pfly);
}

void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pfly, pbis);
}

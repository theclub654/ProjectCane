#include "hide.h"

void InitHbsk(HBSK* phbsk)
{
	//std::cout << "HBSK Size: " << sizeof(HBSK) << "\n";
	InitSo(phbsk);
}

void OnHbskAdd(HBSK* phbsk)
{
	OnSoAdd(phbsk);
}

void InitHshape(HSHAPE* phshape)
{
	//std::cout << "HSHAPE Size: " << sizeof(HSHAPE) << "\n";
	InitShape(phshape);
}

void OnHshapeAdd(HSHAPE* phshape)
{
	OnLoAdd(phshape);
}

void InitHpnt(HPNT* phpnt)
{
	//std::cout << "HPNT Size: " << sizeof(HPNT) << "\n";
	InitLo(phpnt);
}

void OnHpntAdd(HPNT* phpnt)
{
	OnLoAdd(phpnt);
}

void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(phbsk, pbis);
}

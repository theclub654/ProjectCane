#include "hide.h"

void InitHbsk(HBSK* phbsk)
{
	InitSo(phbsk);
}

void OnHbskAdd(HBSK* phbsk)
{
	OnSoAdd(phbsk);
}

void InitHshape(HSHAPE* phshape)
{
	InitShape(phshape);
}

void OnHshapeAdd(HSHAPE* phshape)
{
	OnLoAdd(phshape);
}

void InitHpnt(HPNT* phpnt)
{
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

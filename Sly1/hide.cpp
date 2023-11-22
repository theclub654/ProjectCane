#include "hide.h"

void* NewHbsk()
{
	return new HBSK;
}

void InitHbsk(HBSK* phbsk)
{
	InitSo(phbsk);
}

void OnHbskAdd(HBSK* phbsk)
{
	OnSoAdd(phbsk);
}

void DeleteHbsk(LO* plo)
{
	delete (HBSK*)plo;
}

void* NewHshape()
{
	return new HSHAPE;
}

void InitHshape(HSHAPE* phshape)
{
	InitShape(phshape);
}

void OnHshapeAdd(HSHAPE* phshape)
{
	OnLoAdd(phshape);
}

void DeleteHshape(LO* plo)
{
	delete(HSHAPE*)plo;
}

void* NewHpnt()
{
	return new HPNT;
}

void InitHpnt(HPNT* phpnt)
{
	InitLo(phpnt);
}

void DeleteHpnt(LO* plo)
{
	delete(HPNT*)plo;
}

void OnHpntAdd(HPNT* phpnt)
{
	OnLoAdd(phpnt);
}

void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(phbsk, pbis);
}

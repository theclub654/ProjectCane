#include "hide.h"

HBSK* NewHbsk()
{
	return new HBSK{};
}

void InitHbsk(HBSK* phbsk)
{
	InitSo(phbsk);
}

int GetHbskSize()
{
	return sizeof(HBSK);
}

void OnHbskAdd(HBSK* phbsk)
{
	OnSoAdd(phbsk);
}

void OnHbskRemove(HBSK* phbsk)
{
	OnSoRemove(phbsk);
	//RemoveDlEntry(&g_dlHbsk, phbsk);
}

void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(phbsk, pbis);
}

void CloneHbsk(HBSK* phbsk, HBSK* phbskBase)
{
	LO lo = *phbsk;
	*phbsk = *phbskBase;
	memcpy(phbsk, &lo, sizeof(LO));

	CloneLo(phbsk, phbskBase);

	ClearDl(&phbsk->dlChild);

	phbsk->pxa = nullptr;
	phbsk->grfpvaXpValid = 0;
	phbsk->pstso = nullptr;
}

void DeleteHbsk(LO* plo)
{
	delete (HBSK*)plo;
}

HSHAPE* NewHshape()
{
	return new HSHAPE{};
}

void InitHshape(HSHAPE* phshape)
{
	InitShape(phshape);
}

int GetHshapeSize()
{
	return sizeof(HSHAPE);
}

void OnHshapeAdd(HSHAPE* phshape)
{
	OnLoAdd(phshape);
}

void OnHshapeRemove(HSHAPE* phshape)
{
	OnLoRemove(phshape);
	//RemoveDlEntry(&g_dlHshape, phshape);
}

void CloneHshape(HSHAPE* phshape, HSHAPE* phshapeBase)
{
	LO lo = *phshape;
	*phshape = *phshapeBase;
	memcpy(phshape, &lo, sizeof(LO));

	CloneLo(phshape, phshapeBase);
}

void DeleteHshape(LO* plo)
{
	delete(HSHAPE*)plo;
}

HPNT* NewHpnt()
{
	return new HPNT{};
}

void InitHpnt(HPNT* phpnt)
{
	InitLo(phpnt);
}

int GetHpntSize()
{
	return sizeof(HPNT);
}

void OnHpntAdd(HPNT* phpnt)
{
	OnLoAdd(phpnt);
}

void OnHpntRemove(HPNT* phpnt)
{
	OnLoRemove(phpnt);
	//RemoveDlEntry(&g_dlHpnt, phpnt);
}

void CloneHpnt(HPNT* phpnt, HPNT* phpntBase)
{
	LO lo = *phpnt;
	*phpnt = *phpntBase;
	memcpy(phpnt, &lo, sizeof(LO));

	CloneLo(phpnt, phpntBase);
}

void DeleteHpnt(LO* plo)
{
	delete(HPNT*)plo;
}
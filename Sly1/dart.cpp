#include "dart.h"

void* NewDart()
{
	return new DART;
}

void InitDart(DART* pdart)
{
	InitSo(pdart);
}

int GetDartSize()
{
	return sizeof(DART);
}

void OnDartAdd(DART* pdart)
{
	OnSoAdd(pdart);
}

void CloneDart(DART* pdart, DART* pdartBase)
{
	LO lo = *pdart;
	*pdart = *pdartBase;
	memcpy(pdart, &lo, sizeof(LO));

	CloneLo(pdart, pdartBase);

	ClearDl(&pdart->dlChild);

	pdart->pxa = nullptr;
	pdart->grfpvaXpValid = 0;
	pdart->pstso = nullptr;
}

void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pdart, pbis);
}

void DeleteDart(LO* plo)
{
	delete(DART*)plo;
}

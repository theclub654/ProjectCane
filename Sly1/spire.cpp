#include "spire.h"

void* NewSpire()
{
	return new SPIRE{};
}

int GetSpireSize()
{
	return sizeof(SPIRE);
}

void OnSpireAdd(SPIRE* pspire)
{
	OnLoAdd(pspire);
	AppendDlEntry(&pspire->psw->dlSpire, pspire);
}

void OnSpireRemove(SPIRE* pspire)
{
	OnLoRemove(pspire);
	RemoveDlEntry(&pspire->psw->dlSpire, pspire);
}

void CloneSpire(SPIRE* pspire, SPIRE* pspireBase)
{
	LO lo = *pspire;
	*pspire = *pspireBase;
	memcpy(pspire, &lo, sizeof(LO));

	CloneLo(pspire, pspireBase);
}

void DeleteSpire(LO* plo)
{
	delete(SPIRE*)plo;
}
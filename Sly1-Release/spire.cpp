#include "spire.h"

SPIRE* NewSpire()
{
	return new SPIRE{};
}

void InitSwSpireDl(SW* psw)
{
	InitDl(&psw->dlSpire, offsetof(SPIRE, dleSpire));
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
	ClonePnt(pspire, pspireBase);

	pspire->dleSpire = pspireBase->dleSpire;
}

void DeleteSpire(SPIRE* pspire)
{
	delete pspire;
}
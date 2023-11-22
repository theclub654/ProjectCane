#include "spire.h"

void* NewSpire()
{
	return new SPIRE;
}

void OnSpireAdd(SPIRE* pspire)
{
	OnLoAdd(pspire);
	AppendDlEntry(&pspire->psw->dlSpire, pspire);
}

void DeleteSpire(LO* plo)
{
	delete(SPIRE*)plo;
}

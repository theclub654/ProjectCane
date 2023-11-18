#include "spire.h"

void OnSpireAdd(SPIRE* pspire)
{
	std::cout << "SPIRE Size: " << sizeof(SPIRE) << "\n";
	OnLoAdd(pspire);
	AppendDlEntry(&pspire->psw->dlSpire, pspire);
}

void DeleteSpire(LO* plo)
{
	delete(SPIRE*)plo;
}

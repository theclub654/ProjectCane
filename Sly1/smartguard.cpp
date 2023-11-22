#include "smartguard.h"

void* NewSmartguard()
{
	return new SMARTGUARD;
}

void InitSmartGuard(SMARTGUARD* psmartguard)
{
	InitStepGuard(psmartguard);
}

void DeleteSmartGuard(LO* plo)
{
	delete (SMARTGUARD*)plo;
}

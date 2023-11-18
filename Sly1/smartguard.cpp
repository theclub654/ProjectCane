#include "smartguard.h"

void InitSmartGuard(SMARTGUARD* psmartguard)
{
	InitStepGuard(psmartguard);
}

void DeleteSmartGuard(LO* plo)
{
	delete (SMARTGUARD*)plo;
}

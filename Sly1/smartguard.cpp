#include "smartguard.h"

void* CreateSmartguard()
{
	SMARTGUARD smartguard;
	return &smartguard;
}

void InitSmartGuard(SMARTGUARD* psmartguard)
{
	InitStepGuard(psmartguard);
}

void DeleteSmartGuard(LO* plo)
{
	delete (SMARTGUARD*)plo;
}

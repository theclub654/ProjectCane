#include "smartguard.h"

void* NewSmartguard()
{
	return new SMARTGUARD;
}

void InitSmartGuard(SMARTGUARD* psmartguard)
{
	InitStepGuard(psmartguard);
}

int GetSmartguardSize()
{
	return sizeof(SMARTGUARD);
}

void CloneSmartguard(SMARTGUARD* psmartguard, SMARTGUARD* psmartguardBase)
{
	LO lo = *psmartguard;
	*psmartguard = *psmartguardBase;
	memcpy(psmartguard, &lo, sizeof(LO));

	CloneLo(psmartguard, psmartguardBase);

	ClearDl(&psmartguard->dlChild);

	psmartguard->pxa = nullptr;
	psmartguard->grfpvaXpValid = 0;
	psmartguard->pstso = nullptr;
}

void DeleteSmartGuard(LO* plo)
{
	delete (SMARTGUARD*)plo;
}

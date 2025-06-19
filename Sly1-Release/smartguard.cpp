#include "smartguard.h"

SMARTGUARD* NewSmartguard()
{
	return new SMARTGUARD{};
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
    CloneStepguard(psmartguard, psmartguardBase);

    psmartguard->sgf = psmartguardBase->sgf;  // Copy SGF structure (assuming it's trivially copyable)
    psmartguard->sDetectNear = psmartguardBase->sDetectNear;  // Copy the float value
    psmartguard->zDetectNear = psmartguardBase->zDetectNear;  // Copy the float value
    psmartguard->sSneak = psmartguardBase->sSneak;  // Copy the float value
    psmartguard->zSneak = psmartguardBase->zSneak;  // Copy the float value
    psmartguard->oidSneakVol = psmartguardBase->oidSneakVol;  // Copy the OID value

    // Shallow copy of pointer to VOL and ALO (Assuming these are pointer types that need shallow copy)
    psmartguard->pvolSneak = psmartguardBase->pvolSneak;  // Shallow copy of pointer to VOL
    psmartguard->paloFlashTarget = psmartguardBase->paloFlashTarget;  // Shallow copy of pointer to ALO

    // Copying array of SGFT structures
    for (int i = 0; i < 17; i++) {
        psmartguard->mpsgssgft[i] = psmartguardBase->mpsgssgft[i];  // Shallow copy of array elements
    }

    // Shallow copy of EXC pointer (Assuming it's a pointer type)
    psmartguard->pexcSneak = psmartguardBase->pexcSneak;  // Shallow copy of pointer to EXC

    // Copying the integer for the search idle state
    psmartguard->cidleSearch = psmartguardBase->cidleSearch;
}

void DeleteSmartGuard(SMARTGUARD* psmartguard)
{
	delete psmartguard;
}

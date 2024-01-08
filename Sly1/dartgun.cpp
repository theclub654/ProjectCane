#include "dartgun.h"

void* NewDartgun()
{
	return new DARTGUN;
}

void InitDartgun(DARTGUN* pdartgun)
{
	InitBreak(pdartgun);
}

int GetDartgunSize()
{
	return sizeof(DARTGUN);
}

void CloneDartgun(DARTGUN* pdartgun, DARTGUN* pdartgunBase)
{
	LO lo = *pdartgun;
	*pdartgun = *pdartgunBase;
	memcpy(pdartgun, &lo, sizeof(LO));

	CloneLo(pdartgun, pdartgunBase);

	ClearDl(&pdartgun->dlChild);

	pdartgun->pxa = nullptr;
	pdartgun->grfpvaXpValid = 0;
	pdartgun->pstso = nullptr;
}

void DeleteDartgun(LO* plo)
{
	delete (DARTGUN*)plo;
}

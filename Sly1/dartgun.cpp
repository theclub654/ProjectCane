#include "dartgun.h"

void* NewDartgun()
{
	return new DARTGUN;
}

void InitDartgun(DARTGUN* pdartgun)
{
	InitBreak(pdartgun);
}

void DeleteDartgun(LO* plo)
{
	delete (DARTGUN*)plo;
}

#include "dartgun.h"

void InitDartgun(DARTGUN* pdartgun)
{
	//std::cout << "DARTGUN Size: " << sizeof(DARTGUN) << "\n";
	InitBreak(pdartgun);
}

void DeleteDartGun(LO* plo)
{
	delete (DARTGUN*)plo;
}

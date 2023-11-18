#include "water.h"

void InitWater(WATER* pwater)
{
	//std::cout << "WATER Size: " << sizeof(WATER) << "\n";
	InitSo(pwater);
}

void DeleteWater(LO* plo)
{
	delete (WATER*)plo;
}

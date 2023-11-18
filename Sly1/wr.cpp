#include "wr.h"

void InitWr(WR* pwr)
{
	//std::cout << "WR Size: " << sizeof(WR) << "\n";
	InitLo(pwr);
}

void DeleteWr(LO* plo)
{
	delete(WR*)plo;
}

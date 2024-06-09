#include "wr.h"

WR* NewWr()
{
	return new WR{};
}

void InitWr(WR* pwr)
{
	InitLo(pwr);
}

int  GetWrSize()
{
	return sizeof(WR);
}

void DeleteWr(LO* plo)
{
	delete(WR*)plo;
}

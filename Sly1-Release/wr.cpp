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

void DeleteWr(WR* pwr)
{
	delete pwr;
}

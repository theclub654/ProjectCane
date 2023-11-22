#include "wr.h"

void* NewWr()
{
	return new WR;
}

void InitWr(WR* pwr)
{
	InitLo(pwr);
}

void DeleteWr(LO* plo)
{
	delete(WR*)plo;
}

#include "hnd.h"

void* NewHnd()
{
	return new HND;
}

void InitHnd(HND* phnd)
{
	InitTarget(phnd);
}

void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis)
{
	LoadXfmFromBrx(phnd, pbis);
}

void DeleteHnd(LO* plo)
{
	delete(HND*)plo;
}

#include "hnd.h"

void* NewHnd()
{
	return new HND{};
}

void InitHnd(HND* phnd)
{
	InitTarget(phnd);
}

int GetHndSize()
{
	return sizeof(HND);
}

void CloneHnd(HND* phnd, HND* phndBase)
{
	LO lo = *phnd;
	*phnd = *phndBase;
	memcpy(phnd, &lo, sizeof(LO));

	CloneLo(phnd, phndBase);
}

void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis)
{
	LoadXfmFromBrx(phnd, pbis);
}

void DeleteHnd(LO* plo)
{
	delete(HND*)plo;
}

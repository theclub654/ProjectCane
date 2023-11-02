#include "hnd.h"

void InitHnd(HND* phnd)
{
	InitTarget(phnd);
}

void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis)
{
	LoadXfmFromBrx(phnd, pbis);
}

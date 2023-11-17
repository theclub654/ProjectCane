#include "hnd.h"

void InitHnd(HND* phnd)
{
	//std::cout << "HND Size: " << sizeof(HND) << "\n";
	InitTarget(phnd);
}

void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis)
{
	LoadXfmFromBrx(phnd, pbis);
}

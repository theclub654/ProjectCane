#include "binoc.h"

void* NewScan()
{
	return new SCAN;
}

void DeleteScan(LO* plo)
{
	delete(SCAN*)plo;
}

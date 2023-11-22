#include "frzg.h"

void* NewFrzg()
{
	return new FRZG;
}

void DeleteFrzg(LO* plo)
{
	delete(FRZG*)plo;
}

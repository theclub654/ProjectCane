#include "gomer.h"

void* CreateGomer()
{
	GOMER gomer;
	return &gomer;
}

void InitGomer(GOMER* pgomer)
{
	InitStepGuard(pgomer);
}

void DeleteGomer(LO* plo)
{
	delete (GOMER*)plo;
}

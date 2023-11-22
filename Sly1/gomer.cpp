#include "gomer.h"

void* NewGomer()
{
	return new GOMER;
}

void InitGomer(GOMER* pgomer)
{
	InitStepGuard(pgomer);
}

void DeleteGomer(LO* plo)
{
	delete (GOMER*)plo;
}

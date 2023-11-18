#include "gomer.h"

void InitGomer(GOMER* pgomer)
{
	InitStepGuard(pgomer);
}

void DeleteGomer(LO* plo)
{
	delete (GOMER*)plo;
}

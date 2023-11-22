#include "murray.h"

void* NewMurray()
{
	return new MURRAY;
}

void InitMurray(MURRAY* pmurray)
{
	InitStepGuard(pmurray);
}

void DeleteMurray(LO* plo)
{
	delete (MURRAY*)plo;
}

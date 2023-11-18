#include "murray.h"

void InitMurray(MURRAY* pmurray)
{
	InitStepGuard(pmurray);
}

void DeleteMurray(LO* plo)
{
	delete (MURRAY*)plo;
}

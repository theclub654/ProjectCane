#include "murray.h"

void* CreateMurray()
{
	MURRAY murray;
	return &murray;
}

void InitMurray(MURRAY* pmurray)
{
	InitStepGuard(pmurray);
}

void DeleteMurray(LO* plo)
{
	delete (MURRAY*)plo;
}

#include "step.h"

void* CreateStep()
{
	STEP step;
	return &step;
}

void InitStep(STEP* pstep)
{
	InitPo(pstep);
}

void DeleteStep(LO* plo)
{
	delete (STEP*)plo;
}

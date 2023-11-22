#include "step.h"

void* NewStep()
{
	return new STEP;
}

void InitStep(STEP* pstep)
{
	InitPo(pstep);
}

void DeleteStep(LO* plo)
{
	delete (STEP*)plo;
}

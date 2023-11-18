#include "step.h"

void InitStep(STEP* pstep)
{
	InitPo(pstep);
}

void DeleteStep(LO* plo)
{
	delete (STEP*)plo;
}

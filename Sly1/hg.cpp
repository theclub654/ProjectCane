#include "hg.h"

void InitHg(HG* phg)
{
	InitStep(phg);
}

void DeleteHg(LO* plo)
{
	delete (HG*)plo;
}

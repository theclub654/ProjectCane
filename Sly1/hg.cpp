#include "hg.h"

void* NewHg()
{
	return new HG;
}

void InitHg(HG* phg)
{
	InitStep(phg);
}

void DeleteHg(LO* plo)
{
	delete (HG*)plo;
}

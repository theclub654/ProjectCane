#include "hg.h"

void* NewHg()
{
	return new HG;
}

void InitHg(HG* phg)
{
	InitStep(phg);
}

int GetHgSize()
{
	return sizeof(HG);
}

void CloneHg(HG* phg, HG* phgBase)
{
	LO lo = *phg;
	*phg = *phgBase;
	memcpy(phg, &lo, sizeof(LO));

	CloneLo(phg, phgBase);

	ClearDl(&phg->dlChild);

	phg->pxa = nullptr;
	phg->grfpvaXpValid = 0;
	phg->pstso = nullptr;
}

void DeleteHg(LO* plo)
{
	delete (HG*)plo;
}

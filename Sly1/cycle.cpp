#include "cycle.h"

void* NewCycle()
{
	return new CYCLE{};
}

void InitCycle(CYCLE* pcycle)
{
	InitPo(pcycle);
}

int GetCycleSize()
{
	return sizeof(CYCLE);
}

void UpdateCycleXfWorld(CYCLE* pcycle)
{
	UpdateSoXfWorld(pcycle);
}

void CloneCycle(CYCLE* pcycle, CYCLE* pcycleBase)
{
	LO lo = *pcycle;
	*pcycle = *pcycleBase;
	memcpy(pcycle, &lo, sizeof(LO));

	CloneLo(pcycle, pcycleBase);

	ClearDl(&pcycle->dlChild);

	pcycle->pxa = nullptr;
	pcycle->grfpvaXpValid = 0;
	pcycle->pstso = nullptr;
}

void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro)
{
	RenderSoSelf(pcycle, pcm, pro);
}

void DeleteCycle(LO* plo)
{
	delete (CYCLE*)plo;
}

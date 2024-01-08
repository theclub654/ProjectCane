#include "barrier.h"

void* NewBarrier()
{
	return new BARRIER;
}

void InitBarrier(BARRIER* pbarrier)
{
	InitSo(pbarrier);
}

int GetBarrierSize()
{
	return sizeof(BARRIER);
}

void CloneBarrier(BARRIER* pbarrier, BARRIER* pbarrierBase)
{
	LO lo = *pbarrier;
	*pbarrier = *pbarrierBase;
	memcpy(pbarrier, &lo, sizeof(LO));

	CloneLo(pbarrier, pbarrierBase);

	ClearDl(&pbarrier->dlChild);

	pbarrier->pxa = nullptr;
	pbarrier->grfpvaXpValid = 0;
	pbarrier->pstso = nullptr;
}

void DeleteBarrier(LO* plo)
{
	delete (BARRIER*)plo;
}

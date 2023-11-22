#include "barrier.h"

void* NewBarrier()
{
	return new BARRIER;
}

void InitBarrier(BARRIER* pbarrier)
{
	InitSo(pbarrier);
}

void DeleteBarrier(LO* plo)
{
	delete (BARRIER*)plo;
}

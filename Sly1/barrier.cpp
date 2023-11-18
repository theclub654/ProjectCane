#include "barrier.h"

void InitBarrier(BARRIER* pbarrier)
{
	//std::cout << "BARRIER Size: " << sizeof(BARRIER) << "\n";
	InitSo(pbarrier);
}

void DeleteBarrier(LO* plo)
{
	delete (BARRIER*)plo;
}

#include "cycle.h"

void InitCycle(CYCLE* pcycle)
{
	std::cout << "CYCLE Size: " << sizeof(CYCLE) << "\n";
	InitPo(pcycle);
}

void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro)
{
	RenderSoSelf(pcycle, pcm, pro);
}

void DeleteCycle(LO* plo)
{
	delete (CYCLE*)plo;
}

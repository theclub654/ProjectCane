#include "cycle.h"

void InitCycle(CYCLE* pcycle)
{
	InitPo(pcycle);
}

void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro)
{
	RenderSoSelf(pcycle, pcm, pro);
}

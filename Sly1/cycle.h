#pragma once
#include "po.h"

class CYCLE : public PO
{
	public:
};

void InitCycle(CYCLE *pcycle);
void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro);
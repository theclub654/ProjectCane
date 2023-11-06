#pragma once
#include "step.h"

class TANK : public STEP
{
	public:
};

void InitTank(TANK* ptank);
void RenderTankAll(TANK* ptank, CM* pcm, RO* pro);
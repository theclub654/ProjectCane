#pragma once
#include "step.h"

class STEPGUARD : public STEP
{
	public:
};

void LoadStepGuardFromBrx(STEPGUARD *pstepguard, CBinaryInputStream *pbis);
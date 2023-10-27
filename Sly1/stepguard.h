#pragma once
#include "step.h"

class STEPGUARD : public STEP
{
	public:
};

class PUFFC : public STEPGUARD
{

};

void InitStepGuard(STEPGUARD* pstepguard);
void LoadStepGuardFromBrx(STEPGUARD *pstepguard, CBinaryInputStream *pbis);
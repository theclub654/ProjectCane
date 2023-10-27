#include "stepguard.h"

void InitStepGuard(STEPGUARD* pstepguard)
{
	InitStep(pstepguard);
}

void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pstepguard, pbis);
}

#include "stepguard.h"

void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pstepguard, pbis);
}

#include "stepguard.h"

void InitSgg(SGG* psgg)
{
	InitLo(psgg);
}

void InitStepGuard(STEPGUARD* pstepguard)
{
	InitStep(pstepguard);
}

void OnStepguardAdd(STEPGUARD* pstepguard)
{
	OnPoAdd(pstepguard);
}

void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pstepguard, pbis);
}

void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro)
{

}

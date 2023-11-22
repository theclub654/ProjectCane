#include "stepguard.h"

void* NewSgg()
{
	return new SGG;
}

void InitSgg(SGG* psgg)
{
	InitLo(psgg);
}

void DeleteSgg(LO* plo)
{
	delete(SGG*)plo;
}

void* NewStepguard()
{
	return new STEPGUARD;
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

void DeleteStepguard(LO* plo)
{
	delete (STEPGUARD*)plo;
}

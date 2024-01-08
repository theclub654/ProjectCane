#include "stepguard.h"

void* NewSgg()
{
	return new SGG;
}

void InitSgg(SGG* psgg)
{
	InitLo(psgg);
}

int GetSggSize()
{
	return sizeof(SGG);
}

void CloneSgg(SGG* psgg, SGG* psggBase)
{
	LO lo = *psgg;
	*psgg = *psggBase;
	memcpy(psgg, &lo, sizeof(LO));

	CloneLo(psgg, psggBase);
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

void CloneStepguard(STEPGUARD* pstepguard, STEPGUARD* pstepguardBase)
{
	LO lo = *pstepguard;
	*pstepguard = *pstepguardBase;
	memcpy(pstepguard, &lo, sizeof(LO));

	CloneLo(pstepguard, pstepguardBase);

	ClearDl(&pstepguard->dlChild);

	pstepguard->pxa = nullptr;
	pstepguard->grfpvaXpValid = 0;
	pstepguard->pstso = nullptr;
}

void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pstepguard, pbis);
}

void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro)
{

}

int GetStepguardSize()
{
	return sizeof(STEPGUARD);
}

void DeleteStepguard(LO* plo)
{
	delete (STEPGUARD*)plo;
}

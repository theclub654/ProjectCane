#include "step.h"

STEP* NewStep()
{
	return new STEP{};
}

void InitStep(STEP* pstep)
{
	InitPo(pstep);
}

int GetStepSize()
{
	return sizeof(STEP);
}

void UpdateStepXfWorld(STEP* pstep)
{
	UpdateSoXfWorld(pstep);
}

void RenderStepSelf(STEP* pstep, CM* pcm, RO* pro)
{
	RenderSoSelf(pstep, pcm, pro);
}

void CloneStep(STEP* pstep, STEP* pstepBase)
{
	LO lo = *pstep;
	*pstep = *pstepBase;
	memcpy(pstep, &lo, sizeof(LO));

	CloneLo(pstep, pstepBase);

	ClearDl(&pstep->dlChild);

	pstep->pxa = nullptr;
	pstep->grfpvaXpValid = 0;
	pstep->pstso = nullptr;
}

void RotateStepToMat(STEP* pstep, glm::mat3& pmat)
{
	RotateSoToMat(pstep, pmat);
}

void DeleteStep(STEP* pstep)
{
	delete pstep;
}

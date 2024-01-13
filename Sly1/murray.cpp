#include "murray.h"

void* NewMurray()
{
	return new MURRAY{};
}

void InitMurray(MURRAY* pmurray)
{
	InitStepGuard(pmurray);
}

int GetMurraySize()
{
	return sizeof(MURRAY);
}

void CloneMurray(MURRAY* pmurray, MURRAY* pmurrayBase)
{
	LO lo = *pmurray;
	*pmurray = *pmurrayBase;
	memcpy(pmurray, &lo, sizeof(LO));

	CloneLo(pmurray, pmurrayBase);

	ClearDl(&pmurray->dlChild);

	pmurray->pxa = nullptr;
	pmurray->grfpvaXpValid = 0;
	pmurray->pstso = nullptr;
}

void DeleteMurray(LO* plo)
{
	delete (MURRAY*)plo;
}

#include "tank.h"

void* NewTank()
{
	return new TANK{};
}

void InitTank(TANK* ptank)
{
	InitStep(ptank);
}

int GetTankSize()
{
	return sizeof(TANK);
}

void CloneTank(TANK* ptank, TANK* ptankBase)
{
	LO lo = *ptank;
	*ptank = *ptankBase;
	memcpy(ptank, &lo, sizeof(LO));

	CloneLo(ptank, ptankBase);

	ClearDl(&ptank->dlChild);

	ptank->pxa = nullptr;
	ptank->grfpvaXpValid = 0;
	ptank->pstso = nullptr;
}

void RenderTankAll(TANK* ptank, CM* pcm, RO* pro)
{

}

void DeleteTank(LO* plo)
{
	delete(TANK*)plo;
}
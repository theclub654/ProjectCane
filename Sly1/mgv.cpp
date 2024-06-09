#include "mgv.h"

MGV* NewMgv()
{
	return new MGV{};
}

void InitMgv(MGV* pmgv)
{
	InitPo(pmgv);
	pmgv->cpoints = 200;
	pmgv->clives = 5;
}

int GetMgvSize()
{
	return sizeof(MGV);
}

void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pmgv, pbis);
}

void CloneMgv(MGV* pmgv, MGV* pmgvBase)
{
	LO lo = *pmgv;
	*pmgv = *pmgvBase;
	memcpy(pmgv, &lo, sizeof(LO));

	CloneLo(pmgv, pmgvBase);

	ClearDl(&pmgv->dlChild);

	pmgv->pxa = nullptr;
	pmgv->grfpvaXpValid = 0;
	pmgv->pstso = nullptr;
}

void DeleteMgv(LO* plo)
{
	delete (MGV*)plo;
}

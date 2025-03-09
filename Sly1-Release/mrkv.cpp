#include "mrkv.h"

MRKV* NewMrkv()
{
	return new MRKV{};
}

void InitMrkv(MRKV* pmrkv)
{
	InitSo(pmrkv);
}

int GetMrkvSize()
{
	return sizeof(MRKV);
}

void CloneMrkv(MRKV* pmrkv, MRKV* pmrkvBase)
{
	LO lo = *pmrkv;
	*pmrkv = *pmrkvBase;
	memcpy(pmrkv, &lo, sizeof(LO));

	CloneLo(pmrkv, pmrkvBase);

	ClearDl(&pmrkv->dlChild);

	pmrkv->pxa = nullptr;
	pmrkv->grfpvaXpValid = 0;
	pmrkv->pstso = nullptr;
}

void DeleteMrkv(MRKV *pmrkv)
{
	delete pmrkv;
}

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
	CloneSo(pmrkv, pmrkvBase);

	pmrkv->normal = pmrkvBase->normal;
	pmrkv->lmAlpha = pmrkvBase->lmAlpha;
	pmrkv->sDepth = pmrkvBase->sDepth;
}

void DeleteMrkv(MRKV *pmrkv)
{
	delete pmrkv;
}

#include "mrkv.h"

MRKV* NewMrkv()
{
	return new MRKV{};
}

void InitMrkv(MRKV* pmrkv)
{
	InitSo(pmrkv);

	pmrkv->fNoXpsSelf = true;
	SetSoConstraints(pmrkv, CT_Locked, nullptr, CT_Locked, nullptr);
}

void* GetMrkvNormal(MRKV* pmrkv)
{
	return &pmrkv->normal;
}

void SetMrkvNormal(MRKV* pmrkv, glm::vec3 normal)
{
	pmrkv->normal = normal;
}

void* GetMrkvSDepth(MRKV* pmrkv)
{
	return &pmrkv->sDepth;
}

void SetMrkvSDepth(MRKV* pmrkv, float sDepth)
{
	pmrkv->sDepth = sDepth;
}

void* GetMrkvLmAlpha(MRKV* pmrkv)
{
	return &pmrkv->lmAlpha;
}

void SetMrkvLmAlpha(MRKV* pmrkv, LM lmAlpha)
{
	pmrkv->lmAlpha = lmAlpha;
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

#include "fly.h"

FLY* NewFly()
{
	return new FLY{};
}

void InitSwFlyDl(SW* psw)
{
	InitDl(&psw->dlFly, offsetof(FLY, dleFly));
}

void InitFly(FLY* pfly)
{
	InitSo(pfly);
	AppendDlEntry(&pfly->psw->dlFly, pfly);
}

int GetFlySize()
{
	return sizeof(FLY);
}

void CloneFly(FLY* pfly, FLY* pflyBase)
{
	LO lo = *pfly;
	*pfly = *pflyBase;
	memcpy(pfly, &lo, sizeof(LO));

	CloneLo(pfly, pflyBase);

	ClearDl(&pfly->dlChild);

	pfly->pxa = nullptr;
	pfly->grfpvaXpValid = 0;
	pfly->pstso = nullptr;
}

void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pfly, pbis);
}

void RenderFlySelf(FLY* pfly, CM* pcm, RO* pro)
{
	RenderSoSelf(pfly, pcm, pro);
	//pfly->pvtalo->pfnRenderAloAll(pfly, pcm, pro);
}

void DeleteFly(FLY *pfly)
{
	delete pfly;
}

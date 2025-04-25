#include "flash.h"

FLASH* NewFlash()
{
	return new FLASH{};
}

void InitFlash(FLASH* pflash)
{
	InitAlo(pflash);
}

int GetFlashSize()
{
	return sizeof(FLASH);
}

void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pflash, pbis);
	LoadTbspFromBrx(pbis);
}

void CloneFlash(FLASH* pflash, FLASH* pflashBase)
{
	CloneAlo(pflash, pflashBase);

	pflash->ctsurf = pflashBase->ctsurf;
	pflash->atsurf = pflashBase->atsurf;
	pflash->ctbsp = pflashBase->ctbsp;
	pflash->atbsp = pflashBase->atbsp;
	pflash->smpScale = pflashBase->smpScale;
	pflash->gScaleCur = pflashBase->gScaleCur;
	pflash->gScaleTarget = pflashBase->gScaleTarget;
}

void RenderFlashSelf(FLASH* pflash, CM* pcm, RO* pro)
{
	RenderAloSelf(pflash, pcm, pro);
}

void DeleteFlash(FLASH* pflash)
{
	delete pflash;
}

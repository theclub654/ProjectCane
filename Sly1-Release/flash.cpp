#include "flash.h"

FLASH* NewFlash()
{
	return new FLASH{};
}

void InitFlash(FLASH* pflash)
{
	InitAlo(pflash);

	pflash->smpScale.svFast = 1.0;
	pflash->gScaleTarget = 1.0;
	pflash->gScaleCur = 1.0;
	pflash->smpScale.dtFast = 0.1;
	pflash->smpScale.svSlow = 0.2;
}

int GetFlashSize()
{
	return sizeof(FLASH);
}

void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pflash, pbis);
	LoadTbspFromBrx(&pflash->ctsurf, pflash->atsurf, &pflash->ctbsp, pflash->atbsp, pbis);

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

void UpdateFlash(FLASH* pflash, float dt)
{
	UpdateAlo(pflash, dt);
	pflash->gScaleCur = GSmooth(pflash->gScaleCur, pflash->gScaleTarget, g_clock.dt, &pflash->smpScale, nullptr);
}

void RenderFlashSelf(FLASH *pflash, CM *pcm, RO *pro)
{
	glm::mat4 matScale{};
	RO ro{};

	LoadScaleMatrixScalar(nullptr, pflash->gScaleCur, &matScale);
	DupAloRo(pflash, pro, &ro);

	ro.model = ro.model * matScale;

	RenderAloSelf(pflash, pcm, &ro);
}

int FPosFlashWithin(FLASH* pflash, glm::vec3* ppos)
{
	if (pflash->gScaleCur < 0.0001f)
		return 0;

	glm::vec3 posLocal;
	ConvertAloPos(nullptr, (ALO*)pflash, ppos, &posLocal);

	posLocal *= 1.0f / pflash->gScaleCur;

	return FCheckTbspPoint(pflash->atbsp.data(), &posLocal);
}

void DeleteFlash(FLASH* pflash)
{
	delete pflash;
}

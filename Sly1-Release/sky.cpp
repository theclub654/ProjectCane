#include "sky.h"

SKY* NewSky()
{
	return new SKY{};
}

int GetSkySize()
{
	return sizeof(SKY);
}

void CloneSky(SKY* psky, SKY* pskyBase)
{
	CloneAlo(psky, pskyBase);

	psky->fFollowZ = pskyBase->fFollowZ;
}

void UpdateSky(SKY* psky, float dt)
{
	UpdateAlo(psky, dt);
	psky->pvtalo->pfnTranslateAloToPos(psky, g_pcm->pos);
}

void DeleteSky(SKY* psky)
{
	delete psky;
}

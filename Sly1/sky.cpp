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
	LO lo = *psky;
	*psky = *pskyBase;
	memcpy(psky, &lo, sizeof(LO));

	CloneLo(psky, pskyBase);

	ClearDl(&psky->dlChild);
}

void UpdateSky(SKY* psky, float dt)
{
	UpdateAlo(psky, dt);
	psky->pvtalo->pfnTranslateAloToPos(psky, g_pcm->pos);
}

void DeleteSky(LO* plo)
{
	delete(SKY*)plo;
}

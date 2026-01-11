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

void PostSkyLoad(SKY* psky)
{
	PostAloLoad(psky);
	*(unsigned long*)&psky->bitfield = *(unsigned long*)&psky->bitfield | 0x10000000000;
}

void UpdateSky(SKY* psky, float dt)
{
	UpdateAlo(psky, dt);

	// Target position starts at camera position
	glm::vec3 targetPos = g_pcm->pos;

	// If we don't follow Z, keep sky's existing world Z
	/*if (psky->fFollowZ == 0)
		targetPos.z = psky->xf.posWorld.z;*/

	// Move sky to target position
	if (psky->pvtalo && psky->pvtalo->pfnTranslateAloToPos)
		psky->pvtalo->pfnTranslateAloToPos(psky, targetPos);
}

void DeleteSky(SKY* psky)
{
	delete psky;
}

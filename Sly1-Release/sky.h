#pragma once
#include "alo.h"

class SKY : public ALO
{
	public:
	int fFollowZ;
};

SKY* NewSky();
int  GetSkySize();
void SetSkyFFollowZ(SKY* psky, int fFollowZ);
void*GetSkyFFollowZ(SKY* psky);
void CloneSky(SKY* psky, SKY* pskyBase);
void PostSkyLoad(SKY* psky);
void UpdateSky(SKY* psky, float dt);
void DeleteSky(SKY *psky);
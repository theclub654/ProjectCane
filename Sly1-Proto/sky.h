#pragma once
#include "alo.h"

class SKY : public ALO
{
	public:
		int fFollowZ;
};

SKY* NewSky();
int  GetSkySize();
void CloneSky(SKY* psky, SKY* pskyBase);
void UpdateSky(SKY* psky, float dt);
void DeleteSky(SKY *psky);
#pragma once
#include "alo.h"

class SKY : public ALO
{
	public:
		int fFollowZ;
};

void*NewSky();
int  GetSkySize();
void CloneSky(SKY* psky, SKY* pskyBase);
void UpdateSky(SKY* psky, float dt);
void DeleteSky(LO* plo);
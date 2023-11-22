#pragma once
#include "alo.h"

class SKY : public ALO
{
	public:
		int fFollowZ;
};

void* NewSky();
void DeleteSky(LO* plo);
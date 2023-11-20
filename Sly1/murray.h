#pragma once
#include "stepguard.h"

class MURRAY : public STEPGUARD
{
	public:
		int cpaloEnemy;
		struct ALO* apaloEnemy[4];
		struct XFM* pxfmPatrol;
		struct XFM* pxfmPursue;
		struct ASEG* pasegCelebrateKey;
		struct TURRET* pturret;
};

void* CreateMurray();
void InitMurray(MURRAY* pmurray);
void DeleteMurray(LO* plo);
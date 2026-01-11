#pragma once
#include "so.h"

class WATER : public SO
{
	public:
        struct XA* pxaTargets;
        struct MAP* pmapCurrent;
        MRG mrg;
        glm::vec3 vCurrent;
        int fSplash;
        int fZap;
        ZPD zpd;
};

WATER*NewWater();
void InitWater(WATER* pwater);
int  GetWaterSize();
void CloneWater(WATER* pwater, WATER* pwaterBase);
void PostWaterLoad(WATER* pwater);
void UpdateWater(WATER* pwater, float dt);
void DeleteWater(WATER* pwater);
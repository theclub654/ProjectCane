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

void InitWater(WATER* pwater);
void DeleteWater(LO* plo);
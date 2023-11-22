#pragma once
#include "step.h"

enum HGS
{
    HGS_Nil = -1,
    HGS_Passive = 0,
    HGS_Stand = 1,
    HGS_Run = 2,
    HGS_Glide = 3,
    HGS_Zap = 4,
    HGS_Dead = 5,
    HGS_Peek = 6,
    HGS_Max = 7
};

class HG : public STEP
{
	public:
        HGS hgs;
        float tHgs;
        struct ASEGA* pasegaCur;
        float swPan;
        float radBank;
        float radTilt;
        BLRUN ablrunRun[6];
        struct ASEGBL* pasegblRun;
        SFF sffRun;
        struct ASEG* pasegGlide;
};

void* NewHg();
void InitHg(HG* phg);
void DeleteHg(LO* plo);
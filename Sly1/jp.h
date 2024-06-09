#pragma once
#include "step.h"

enum JPS 
{
    JPS_Nil = -1,
    JPS_Passive = 0,
    JPS_Stand = 1,
    JPS_Run = 2,
    JPS_Thrust = 3,
    JPS_Float = 4,
    JPS_Zap = 5,
    JPS_Dead = 6,
    JPS_Peek = 7,
    JPS_Max = 8
};

class JP : public STEP
{
	public:
        JPS jps;
        float tJps;
        struct ASEGA* pasegaCur;
        float radPan;
        float radTilt;
        struct ALO* paloTurret;
        BLRUN ablrunRun[6];
        struct ASEGBL* pasegblRun;
        struct ASEG* pasegStand;
        struct ASEG* pasegThrust;
        struct ASEG* pasegFloat;
        struct ASEG* pasegZap;
        struct ASEG* pasegDead;
        SFF sffRun;
        struct TARGET* ptargetCur;
        int fGround;
        float tGround;
        glm::vec3 posGround;
        int fFlash;
        float tFlash;
        float tCharm;
        float tCharmPending;
        int fCharmEnabled;
        struct ALO* paloJt;
        struct XP* pxpZap;
};

JP*  NewJp();
void InitJp(JP* pjp);
int  GetJpSize();
void CloneJp(JP* pjp, JP* pjpBase);
void RenderJpAll(JP* pjp, CM* pcm, RO* pro);
#pragma once
#include "break.h"

enum CANS 
{
    CANS_Nil = -1,
    CANS_Auto = 0,
    CANS_Manual = 1,
    CANS_Passive = 2,
    CANS_Max = 3
};

class CAN : public BREAK
{
	public:
        CANS cans;
        struct RWM* prwm;
        struct SO* psoTurret;
        int fLeadTarget;
        LM lmRange;
        LM lmActivate;
        LM lmDeactivate;
        float radPanMax;
        float dradFireThreshold;
        float dtFireInterval;
        float tLastFire;
        float dzAim;
        float swTumble;
        SMP smpTracking;
        int fFireEnabled;
        int cuFireAccuracy;
        float auFireAccuracy[5];
        glm::vec3 posTarget;
        struct PNT* ppntMuzzle;
        struct SM* psm;
        struct SMA* psma;
        int cFired;
};

void*NewCan();
void InitCan(CAN *pcan);
int  GetCanSize();
void CloneCan(CAN* pcan, CAN* pcanBase);
void DeleteCan(LO* plo);
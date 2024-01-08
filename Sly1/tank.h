#pragma once
#include "step.h"

enum TANKS 
{
    TANKS_Nil = -1,
    TANKS_Passive = 0,
    TANKS_Active = 1,
    TANKS_Zap = 2,
    TANKS_Dead = 3,
    TANKS_Peek = 4,
    TANKS_Max = 5
};

class TANK : public STEP
{
	public:
        TANKS tanks;
        float tTanks;
        int fFlash;
        float tFlash;
        float tCharm;
        float tCharmPending;
        int fCharmEnabled;
        int fAllowEject;
        struct XP* pxpZap;
        struct ALO* paloJt;
        struct ALO* paloGut;
        struct ACTADJ* pactadjGut;
        struct ALO* paloHead;
        struct ACTADJ* pactadjHead;
        struct SM* psm;
        struct SMA* psma;
        struct PNT* ppntAnchor;
};

void*NewTank();
void InitTank(TANK* ptank);
int  GetTankSize();
void CloneTank(TANK* ptank, TANK* ptankBase);
void RenderTankAll(TANK* ptank, CM* pcm, RO* pro);
void DeleteTank(LO* plo);
#pragma once
#include "gomer.h"

enum UBPS
{
    UBPS_Nil = -1,
    UBPS_Hidden = 0,
    UBPS_Idle = 1,
    UBPS_ShowSafe = 2,
    UBPS_ShowDanger = 3,
    UBPS_Attack = 4,
    UBPS_SwapUp = 5,
    UBPS_SwapDown = 6,
    UBPS_PlaceBomb = 7,
    UBPS_Max = 8
};

class UBG : public GOMER
{
	public:
        struct SM* psmWorld;
        struct SMA* psmaWorld;
        OID oidPatrolGoal;
        int cJump;
        struct XFM* pxfmMiddle;
        struct SO* apsoPads[6];
        struct SO* psoLastDunk;
        int aichk[4];
        int nBossctr;
};

class UBP : public SO
{
	public:
        struct UBV* pubv;
        UBPS ubps;
        float tUbps;
        struct ACTVAL* pactval;
        struct BOMB* pbomb;
};

void InitUbg(UBG* pubg);
void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro);
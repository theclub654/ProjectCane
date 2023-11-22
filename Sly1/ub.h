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
enum UBVS
{
    UBVS_Nil = -1,
    UBVS_Idle = 0,
    UBVS_PlacePads = 1,
    UBVS_PlaceBombs = 2,
    UBVS_SwapPads = 3,
    UBVS_Move = 4,
    UBVS_Max = 5
};
struct SWAP 
{
    int row;
    int col1;
    int col2;
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

class UBV : public SO
{
    public:
        UBVS ubvs;
        float tUbvs;
        struct UBP* apubp[12];
        struct UBP* apubpBase[2];
        glm::vec3 dposPadRow;
        glm::vec3 dposPadCol;
        glm::vec3 dposBase;
        int cswap;
        int iswap;
        SWAP aswap[24];
        int mprowcolSafe[4];
};

void* NewUbg();
void InitUbg(UBG* pubg);
void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro);
void DeleteUbg(LO* plo);
void* NewUbp();
void DeleteUbp(LO* plo);
void* NewUbv();
void DeleteUbv(LO* plo);
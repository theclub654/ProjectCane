#pragma once
#include "alo.h"

enum DPRIZES {
    DPRIZES_Nil = -1,
    DPRIZES_Normal = 0,
    DPRIZES_Collect = 1,
    DPRIZES_Removed = 2,
    DPRIZES_Fall = 3,
    DPRIZES_Swirl = 4,
    DPRIZES_Stick = 5,
    DPRIZES_Lose = 6,
    DPRIZES_Attract = 7,
    DPRIZES_Max = 8
};

class DPRIZE : public ALO
{
	public:
        DPRIZES dprizes;
        float tDprizes;
        DPRIZES dprizesInit;
        OID oidInitialState;
        float dtInitialSkip;
        glm::vec3 posCenter;
        glm::vec3 vCenter;
        glm::vec3 dvCenter;
        float uGlintChance;
        int fLeft;
        float tGlint;
        int fNeverReuse;
        int fReuseCandidate;
        int fLastBounce;
        float svLastBounceMax;
        float svLastBounce;
        float sRadiusBounce;
        float sRadiusCollect;
        float rzBounce;
        float rxyBounce;
        float radSmooth;
        glm::vec3 normalSmooth;
        int fSwirlDone;
        DLE dle;
        int ichkCollected;
        float svcAttract;
        float cAttract;
};

class CHARM : public DPRIZE
{
    public:
};

class COIN : public DPRIZE
{
    public:
};

class KEY : public DPRIZE
{
    public:
};

class GOLD : public DPRIZE
{
    public:
};

void InitDprize(DPRIZE *pdprize);//NOT FINISHED
void InitCharm(CHARM* pcharm);
void InitCoin(COIN* pcoin);
void InitKey(KEY* pkey);
void InitGold(GOLD* pgold);
void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream *pbis);//NOT FINISHED
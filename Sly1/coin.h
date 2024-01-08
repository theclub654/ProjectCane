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
        struct SM* psm;
        struct SMA* psma;
        struct TARGET* ptarget;
        glm::vec3 posCenter;
        glm::vec3 vCenter;
        glm::vec3 dvCenter;
        float uGlintChance;
        struct PNT* ppntFrontGlint;
        struct PNT* ppntBackGlint;
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
        struct EXPL* pexplCollect;
        struct EXPL* pexplAttract;
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
        LM lmDtMaxLifetime;
        float tLose;
};

class KEY : public DPRIZE
{
    public:
};

class GOLD : public DPRIZE
{
    public:
        int fStartTimedChallenge;
};

void*NewDprize();
void InitDprize(DPRIZE *pdprize);//NOT FINISHED
int  GetDprizeSize();
void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis);
void CloneDprize(DPRIZE* pdprize, DPRIZE* pdprizeBase);
void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro);
void DeleteDprize(LO* plo);

void*NewCharm();
void InitCharm(CHARM* pcharm);
int  GetCharmSize();
void CloneCharm(CHARM* pcharm, CHARM* pcharmBase);
void DeleteCharm(LO* plo);

void*NewCoin();
void InitCoin(COIN* pcoin);
int  GetCoinSize();
void CloneCoin(COIN* pcoin, COIN* pcoinBase);
void DeleteCoin(LO* plo);

void*NewKey();
void InitKey(KEY* pkey);
int  GetKeySize();
void CloneKey(KEY* pkey, KEY* pkeyBase);
void DeleteKey(LO* plo);

void*NewGold();
void InitGold(GOLD* pgold);
int  GetGoldSize();
void CloneGold(GOLD* pgold, GOLD* pgoldBase);
void DeleteGold(LO* plo);
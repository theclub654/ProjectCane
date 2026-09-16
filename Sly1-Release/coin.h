#pragma once
#include "ctr.h"

enum DPRIZES 
{
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
    short oidInitialState;
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
    int fCollectible;
};

class CHARM : public DPRIZE
{
    public:
};

struct COINCTR : public CTR
{

};

class COIN : public DPRIZE
{
    public:
    LM lmDtMaxLifetime;
    float tLose;
};

struct LIFECTR : public CTR
{

};

class LIFETKN : public DPRIZE
{
    public:
    LM lmDtReuse;
    float dtReuseRemaining;
};

struct KEYCTR : public CTR
{

};

class KEY : public DPRIZE
{
    public:
};

struct GOLDCTR : public CTR
{

};

class GOLD : public DPRIZE
{
    public:
    int fStartTimedChallenge;
};

DPRIZE* NewDprize();
void InitSwDprizeDl(SW* psw);
void InitDprize(DPRIZE* pdprize);
void SetDprizeOidInitialState(DPRIZE* pdprize, OID oidInitialState);
void*GetDprizeOidInitialState(DPRIZE* pdprize);
void SetDprizeDtInitialSkip(DPRIZE* pdprize, float dtInitialSkip);
void*GetDprizeDtInitialSkip(DPRIZE* pdprize);
void SetDprizeDprizes(DPRIZE* pdprize, DPRIZES dprizes);
void*GetDprizeDprizes(DPRIZE* pdprize);
void SetDprizeFNeverReuse(DPRIZE* pdprize, int fNeverReuse);
void*GetDprizeFNeverReuse(DPRIZE* pdprize);
void SetDprizeSvcAttract(DPRIZE* pdprize, float svcAttract);
void*GetDprizeSvcAttract(DPRIZE* pdprize);
void SetDprizeFCollectible(DPRIZE* pdprize, int fCollectible);
void*GetDprizeFCollectible(DPRIZE* pdprize);
void*AccessDprizeOidInitialState(void* pv);
void*AccessDprizeDtInitialSkip(void* pv);
void*AccessDprizeDprizes(void* pv);
void*AccessDprizeFNeverReuse(void* pv);
void*AccessDprizeSvcAttract(void* pv);
void*AccessDprizeFCollectible(void* pv);
int  GetDprizeSize();
void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis);
void CloneDprize(DPRIZE* pdprize, DPRIZE* pdprizeBase);
void PostDprizeLoad(DPRIZE* pdprize);
void ProjectDprizeTransform(DPRIZE* pdprize, float dt, int fParentDirty);
int  FFilterDprizeObjects(DPRIZE* pdprize, SO* pso);
void CheckDprizeBounce(DPRIZE* pdprize);
void CheckDprizeCollect(DPRIZE* pdprize);
int  FAbsorbDprizeWkr(DPRIZE* pdprize, WKR* pwkr);
void UpdateDprize(DPRIZE* pdprize, float dt);
void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro);
void SetDprizeDprizes(DPRIZE* pdprize, DPRIZES dprizes);
void DeleteDprize(DPRIZE* pdprize);

int CpdprizeFindSwDprizes(SW* psw, CID cid, int cpdprizeMax, DPRIZE** apdprize);

CHARM* NewCharm();
void InitCharm(CHARM* pcharm);
int  GetCharmSize();
void CloneCharm(CHARM* pcharm, CHARM* pcharmBase);
void SetCharmDprizes(CHARM* pcharm, DPRIZES dprizes);
void DeleteCharm(CHARM* pcharm);

void StartupCoinCtr(COINCTR* pcoinctr);

COIN* NewCoin();
void InitCoin(COIN* pcoin);
int  GetCoinSize();
void CloneCoin(COIN* pcoin, COIN* pcoinBase);
void PostCoinCallback(COIN* pcoin);
void HandleCoinMessage(COIN* pcoin, MSGID msgid, SMA* psma);
void UpdateCoin(COIN* pcoin, float dt);
void CreateSwCharm(SW* psw);
void AddLife();
void OnCoinSmack(COIN* pcoin);
void SetCoinDprizes(COIN* pcoin, DPRIZES dprizes);
void DeleteCoin(COIN* pcoin);

void StartupLifeCtr(LIFECTR* plifectr);

LIFETKN* NewLifetkn();
void InitLifeTkn(LIFETKN* plifetkn);
void SetLifetknLmDtReuse(LIFETKN* plifetkn, glm::vec2 dtReuse);
void*GetLifetknLmDtReuse(LIFETKN* plifetkn);
int  GetLifetknSize();
void CloneLifetkn(LIFETKN* plifetkn, LIFETKN* plifetknBase);
void PostLifetknCallback(LIFETKN* plifetkn);
void HandleLifetknMessage(LIFETKN* plifetkn, MSGID msgid, SMA* psma);
void UpdateLifetkn(LIFETKN* plifetkn, float dt);
void AwardLife();
void SetLifetknDprizes(LIFETKN* plifetkn, DPRIZES dprizes);
void DeleteLifetkn(LIFETKN* plifetkn);

void StartupKeyCtr(KEYCTR* pkeyctr);

KEY* NewKey();
void InitKey(KEY* pkey);
int  GetKeySize();
void CloneKey(KEY* pkey, KEY* pkeyBase);
void SetKeyDprizes(KEY* pkey, DPRIZES dprizes);
void DeleteKey(KEY* pkey);

void StartupGoldCtr(GOLDCTR* pgoldctr);

// TIMER
GOLD* NewGold();
void InitGold(GOLD* pgold);
int  GetGoldSize();
void CloneGold(GOLD* pgold, GOLD* pgoldBase);
void PostGoldLoad(GOLD* pgold);
void PostGoldCallback(GOLD* pgold);
void HandleGoldMessage(GOLD* pgold, MSGID msgid, SMA* psma);
void UpdateGold(GOLD* pgold, float dt);
void SetGoldDprizes(GOLD* pgold, DPRIZES dprizes);
void DeleteGold(GOLD* pgold);

int  CpdprizeAttractSwDprizes(SW* psw, CID cid, glm::vec3* pposCenter, int cpdprizeMax, DPRIZE** apdprize);
void RemoveSwExtraneousCharms(SW* psw);

extern LIFECTR g_lifectr;
extern COINCTR g_coinctr;
extern GOLDCTR g_goldctr;
extern KEYCTR  g_keyctr;
extern float DT_DprizeSwirl;
extern float DT_DprizeStick;
extern CLQ s_clqUToRStick;
extern CLQ s_clqUToRSwirl;
extern glm::vec3 s_posTarget;
extern SMPA s_asmpaSwirl[3];
extern float RZ_DprizeSwirlLookAt;
extern SMP s_smpRadSmooth;
extern float R_DprizeDamping;
extern glm::vec3 s_dvDprizeGravity;
extern glm::vec3 s_vCenter;
extern glm::vec3 s_normalGlint;
extern float DT_DprizeGlintIntervalMin;
extern float DT_DprizeFallMax;
extern glm::vec3 s_vReject;
extern float S_DprizeCollectMax;
extern SNIP s_asnipDprize[5];
extern glm::vec3 s_posCharmScreen;
extern float DT_DprizeSkipEach;
extern LM s_lmXY;
extern LM s_lmZ;
extern int g_fGoldEnabled;

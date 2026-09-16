#pragma once
#include "stepguard.h"
#include "ctr.h"

class CRFODK;
class CRFODB;

struct CRFODKSLOT
{
    union
    {
        OID oid;
        ALO* palo;
    };                          // 0x00

    XFM* pxfmSpawn;             // 0x04, child OID 1014
    ASEG* pasegSpawn;           // 0x08, child OID 1015
    ASEGA* pasegaSpawn;         // 0x0c
    union
    {
        int fOccupied;
        CRFODB* pcrfodb;
        CRFODK* pcrfodk;
    };                          // original 0x10
};

struct CRFODKTUNE
{
    LM lmActiveMax;                    // 0x00
    float pad08[2];                    // 0x08
    CLQ clqProgressToActiveMax;        // 0x10

    LM lmSpawnDelayStart;              // 0x20
    LM lmSpawnDelayEnd;                // 0x28
    CLQ clqProgressToSpawnDelay;       // 0x30

    float uRespawnEnable;              // 0x40
    LM lmRespawnDelayStart;            // 0x44
    LM lmRespawnDelayEnd;              // 0x4c
    float pad54[3];                    // 0x54
    CLQ clqProgressToRespawnDelay;     // 0x60

    LM lmDifficultyValue;              // 0x70
    float pad78[2];                    // 0x78
    CLQ clqProgressToDifficulty;       // 0x80

    LM lmSecondaryDifficulty;          // 0x90
    float pad98[2];                    // 0x98
    CLQ clqProgressToSecondary;        // 0xa0
};

class CRFOD : public STEPGUARD
{
	public:
	DLE dleCrfod;
};

class CRFODB : public CRFOD
{
	public:
	int fDetectLatch;
	int cpoint;
	int icrfodb;
    CRFODKSLOT* pcrfodkSlot;
    bool fKillReported;
};

struct CRUSHERCTR : public CTR
{

};

class CRBRAIN : public ALO
{
	public:
	int cpoint;
	int cpointMax;
	float tContest;
};

class CRFODK : public CRFOD
{
	public:
	float svtPursue;
    CRFODKSLOT* pcrfodkSlot;
	int cKillValue;
    bool fKillReported;
};

struct CRFODKSPAWN : ALO
{
    int cKilled;               // 0x2d0
    int cKillGoal;             // 0x2d4, initialized to 50
    float tGameMax;            // 0x2d8, initialized to 90.0f
    int cslot;                 // 0x2dc
    CRFODKSLOT aslot[16];      // 0x2e0-0x41f

    RWM* prwmRespawn;          // 0x420
    RWM* prwmSpawn;            // 0x424
    SM* psm;                   // 0x428
    SMA* psma;                 // 0x42c
    OID oidState;              // 0x430

    float dtSpawnMin;          // 0x434
    float dtSpawnMax;          // 0x438
    float dtRespawnMin;        // 0x43c
    float dtRespawnMax;        // 0x440
    float difficultyValue;     // 0x444
    float anotherDifficulty;   // 0x448
    int cActiveMax;            // 0x44c

    int cRespawnMax;           // 0x450, initialized to 2
    int cSpawned;              // 0x454
    int cActive;               // 0x458
    float tRespawn;            // 0x45c
    float tSpawnNext;          // 0x460
    int fThresholdReached;     // 0x464

    uint8_t pad468[8];         // 0x468-0x46f
    CLQ clqSuckToTune;         // 0x470-0x47f

    CRFODKTUNE tuneNormal;     // 0x480-0x52f
    CRFODKTUNE tuneAssist;     // 0x530-0x5df
    CRFODKTUNE tuneCurrent;    // 0x5e0-0x68f
    int iDifficultyData;       // 0x690
};

void  StartupCrusherCtr(CRUSHERCTR* pcrusherctr);
float DtVisibleCrusherctr(CRUSHERCTR* pcrusherctr);

CRFODB*NewCrfodb();
void InitCrfodb(CRFODB *pcrfodb);
void SetCrfodbCpoint(CRFODB* pcrfodk, int cpoint);
void*GetCrfodbCpoint(CRFODB* pcrfodk);
int  GetCrfodbSize();
void CloneCrfodb(CRFODB* pcrfodb, CRFODB* pcrfodbBase);
void PostCrfodbLoad(CRFODB* pcrfodb);
void UpdateCrfodbGoal(CRFODB* pcrfodb, int fEnter);
int FDetectCrfodb(CRFODB* pcrfodb);
int  FAbsorbCrfodbWkr(CRFODB* pcrfodb, WKR* pwkr);
void OnCrfodbEnteringSgs(CRFODB* pcrfodb, SGS sgsPrev, ASEG* pasegTargetOverride);
void DeleteCrfodb(CRFODB* pcrfodb);

CRFOD*NewCrfod();
void InitSwCrfodDl(SW* psw);
void OnCrfodAdd(CRFOD* pcrfod);
void OnCrfodRemove(CRFOD* pcrfod);
void CloneCrfod(CRFOD* pcrfod, CRFOD* pcrfodBase);
int  GetCrfodSize();
void DeleteCrfodb(CRFODB* pcrfodb);

CRFODK*NewCrfodk();
int  GetCrfodkSize();
void CloneCrfodk(CRFODK* pcrfodk, CRFODK* pcrfodkBase);
SGS  SgsNextCrfodkAI(CRFODK* pcrfodk);
int  FDetectCrfodk(CRFODK* pcrfodk);
int  FAbsorbCrfodkWkr(CRFODK* pcrfodk, WKR *pwkr);
void OnCrfodkEnteringSgs(CRFODK* pcrfodk, SGS sgsPrev, ASEG* pasegTargetOverride);
void DeleteCrfodk(CRFODK* pcrfodk);

CRFODKSPAWN* NewCrfodkSpawn();
void InitCrfodkSpawn(CRFODKSPAWN* pspawn);
void CloneCrfodkSpawn(CRFODKSPAWN* pspawn, CRFODKSPAWN* pspawnBase);
void DeleteCrfodkSpawn(CRFODKSPAWN* pspawn);
void PostCrfodkSpawnerLoad(CRFODKSPAWN* pspawn);
void RecalcCrfodkSpawnerDifficulty(CRFODKSPAWN* pspawn);
void UpdateCrfodkSpawnerDeathDifficulty(CRFODKSPAWN* pspawn);
void HandleCrfodkSpawnerTimerExpiration(void* pv, int fActive);
void UpdateCrfodkSpawner(CRFODKSPAWN* pspawn, float dt);
void UpdateCrfodkSpawnerBlot(CRFODKSPAWN* pspawn);
CRFODKTUNE* PcrfodkSpawnerDifficultyData(CRFODKSPAWN* pspawn);
void AddCrfodkSpawnSlot(CRFODKSPAWN* pspawn, OID oidSlot);
void RespawnCrfodk(CRFODKSPAWN* pspawn);
void SpawnCrfodk(CRFODKSPAWN* pspawn);
void NotifyCrfodkDied(CRFODKSPAWN* pspawn);
void OnCrfodkSpawnedDestroyed(CRFODKSPAWN* pspawn, CRFODB* pcrfodb);
void HandleCrfodkSpawnMessage(CRFODKSPAWN* pspawn, MSGID msgid, void* pv);

CRBRAIN*NewCrbrain();
void InitCrbrain(CRBRAIN* pcrbrain);
int  GetCrbrainSize();
void CloneCrbrain(CRBRAIN* pcrbrain, CRBRAIN* pcrbrainBase);
void DeleteCrbrain(CRBRAIN* pcrbrain);

extern CRUSHERCTR g_crusherctr;
extern int s_icrfodbNext;
extern int CRAD_TryMax;
extern float DS_PredictCrfodb;
extern float U_PenaltyCbspIgnore;
extern float U_PenaltyCbsp;
extern float DT_PredictCrfodb;
extern float R_DistCrusher;
extern float U_PenaltyCrusher;
extern float R_DistCrfod;
extern float U_PenaltyCrfod;
extern float U_PenaltyRad;
extern float U_RadWorst;
extern CRFODKSPAWN* g_pcrfodkSpawner;

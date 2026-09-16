#pragma once
#include "stepguard.h"
#include "chkpnt.h"

class PUFFER : public PO
{
	public:
    class SM* psmShoot;
    class SMA* psmaShoot;
    float radTarget;
    float tLastGround;
    float sdvPuff;
    float sdvPuffMax;
    float svPuffMax;
    glm::vec3 normalGround;
    struct VOL* pvolCollect;
    struct WATER* pwater;
    struct VOL* pvolFire;
    struct PNT* appntFloor[4];
    class ALO* paloFireGun;
    class ALO* paloFireGunTarget;
    std::shared_ptr <ACTLA> pactlaFireGun;
    struct EMITTER* pemitterFire;
    struct EMITTER* pemitterSpray;
    struct PNT* ppntFireNatural;
    struct PUFFT* ppufft;
    class ALO* paloJt;
    class ALO* paloGut;
    struct ACTADJ* pactadjGut;
    class ALO* paloHead;
    struct ACTADJ* pactadjHead;
    int npuffcEaten;
    int npufftLit;
    int npufftMax;
    float tGameMax;
    int tPuffcEaten;
    DL dlRost0;
    DL dlRost1;
    std::vector <PUFFC*> appuffc;
    SM* psm;
    SMA* psma;
    int cpufftLit;
    int cpufftMax;
    float tNextPuffcEat;
    float tPufftFired;
    int cpufftLift;
    class EXPLO* pexploSplineTarget;
};

class PUFFB : public ALO
{
    public:
    int cppathzone;
    struct PATHZONE** appathzone;
    int* mpippathzonecpuffc;
    int cppuffv;
    struct PUFFV** appuffv;
    int cppuffc;
    struct PUFFC** appuffc;
    float tSpawnNext;
};

struct PUFFERWATERMSG
{
    WATER* pwater;
    ALO* palo;
};

class PUFFC : public STEPGUARD
{
	public:
	struct WATER* pwater;
	struct PATHZONE* ppathzoneNext;
	glm::vec3 posPathzoneNext;
	class PUFFV* ppuffvNext;
	struct PUFFB* ppuffb;
    int fStunFinished;
    int fNearPlayer;
    class ROST *prost;
};

class PUFFT : public SO
{
    public:
    int fLit;
    class PNT* ppntFire;
    class EMITTER* pemitterFire;
    class EXPLO* pexplo;
};

class PUFFV : public SO
{
	public:
    short oidPathzone;
    struct PATHZONE* ppathzone;
    short oidPuffvJump;
    struct PUFFV* ppuffvJump;
    int coidPuffvLand;
    short aoidPuffvLand[4];
    int cppuffvLand;
    struct PUFFV* appuffvLand[4];
    float tChosen;
};

PUFFER*NewPuffer();
void InitPuffer(PUFFER *ppuffer);
void SetPuffertGameMax(PUFFER* ppuffer, float tGameMax);
void*GetPufferPsmaShoot(PUFFER* ppuffer);
void*GetPufferPsma(PUFFER* ppuffer);
int  GetPufferSize();
void LoadPufferFromBrx(PUFFER* ppuffer, CBinaryInputStream* pbis);
void ClonePuffer(PUFFER* ppuffer, PUFFER* ppufferBase);
void PostPufferLoad(PUFFER* ppuffer);
void PresetPufferAccel(PUFFER* ppuffer, float dt);
int  FFilterPuffer(PUFFER* ppuffer, SO* psoOther);
void UpdatePuffer(PUFFER* ppuffer, float dt);
PUFFT* PpufftChoosePuffer(PUFFER* ppuffer);
void UpdatePufferDeathDifficulty(PUFFER* ppuffer);
void HandlePufferTimerExpiration(PUFFER* ppuffer, int fActive);
void OnPufferActive(PUFFER* ppuffer, int fActive);
void UpdatePufferActive(PUFFER* ppuffer, JOY* pjoy, float dt);
void UpdatePufferBlot(PUFFER* ppuffer);
void UpdatePufferPuffcEat(PUFFER* ppuffer);
void AdjustPufferNewXp(PUFFER* ppuffer, XP* pxp, int ixpd);
void AddPufferWaterAcceleration(PUFFER* ppuffer, WATER* pwater, float dt);
void HandlePufferMessage(PUFFER* ppuffer, MSGID msgid, void* pv);
void DeletePuffer(PUFFER* ppuffer);

PUFFB*NewPuffb();
int  GetPuffbSize();
void ClonePuffb(PUFFB* ppuffb, PUFFB* ppuffbBase);
void PostPuffbLoad(PUFFB* ppuffb);
void LandPuffb(PUFFER* ppuffer, PUFFC* ppuffc);
void UpdatePuffb(PUFFB* ppuffb, float dt);
void DeletePuffb(PUFFB* ppuffb);

PUFFV*NewPuffv();
void InitPuffv(PUFFV* ppuffv);
int  GetPuffvSize();
void ClonePuffv(PUFFV* ppuffv, PUFFV* ppuffvBase);
void PostPuffvLoad(PUFFV* ppuffv);
void DeletePuffv(PUFFV* ppuffv);

PUFFC*NewPuffc();
int  GetPuffcSize();
void ClonePuffc(PUFFC* ppuffc, PUFFC* ppuffcBase);
void PostPuffcLoad(PUFFC* ppuffc);
void PresetPuffcAccel(PUFFC* ppuffc, float dt);
SGS  SgsNextPuffcAI(PUFFC* ppuffc);
void UpdatePuffcGoal(PUFFC* ppufc, int fEnter);
void OnPuffcExitingSgs(PUFFC* ppuffc, SGS sgsNext);
void OnPuffcEnteringSgs(PUFFC* ppuffc, SGS sgsPrev, ASEG* pasegOverride);
void UpdatePuffcSgs(PUFFC* ppuffc);
void HandlePuffcMessage(PUFFC* ppuffc, MSGID msgid, void* pv);
void UpdatePuffc(PUFFC* ppuffc, float dt);
int  FDetectPuffc(PUFFC* ppuffc);
void DeletePuffc(PUFFC* ppuffc);

PUFFT*NewPufft();
int  GetPufftSize();
void ClonePufft(PUFFT* ppufft, PUFFT* ppufftBase);
void PostPufftLoad(PUFFT* ppufft);
void DeletePufft(PUFFT *ppufft);

extern float ELAS_Puffer;
extern float MU_Puffer;
extern float SDV_PuffcMax;
extern CLQ s_clqDradToULimit;
extern PUFFER* g_ppuffer;
extern int g_cPuffChargeMax;
extern float DT_PuffcEaten;
extern float S_PuffcReturnToPatrolMinDist;
extern float S_PuffcNearPlayerDistEnter;
extern float S_PuffcNearPlayerDistExit;
extern SNIP s_asnipLoadPuffer[1];
extern float SDV_PufferGroundMax;
extern float SV_PufferGroundMax;
extern SNIP s_asnipPostPufferLoad[13];
extern SMP s_smpPuffer;
extern CLQ s_clqDradToUPuff;
extern float DT_PufferPuffAfterGround;
extern CLQ s_clqUDtGroundToUPuff;
extern float SDV_PufferWaterMax;
extern float SV_PufferWaterMax;
extern float DZ_PointCheckMin;
extern float DZ_PointCheckMax;
extern float S_PufftChooseMax;
extern float DT_PufftChooseProj;
extern float DRAD_PufftChooseMax;
extern float D_T_Max;
extern SNIP s_asnipPostPufftLoad[3];
extern float DT_PufftIgniteDelay;

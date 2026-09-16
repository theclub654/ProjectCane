#pragma once
#include "expl.h"
#include "po.h"
#include "shape.h"
#include "emitter.h"
#include "ctr.h"

enum SVEK 
{
    SVEK_Nil = -1,
    SVEK_BoostCollect = 0,
    SVEK_BoostOpportunity = 1,
    SVEK_CheckPoint = 2,
    SVEK_Feature = 3,
    SVEK_Max = 4
};
enum SUVGK 
{
    SUVGK_Nil = -1,
    SUVGK_Race = 0,
    SUVGK_Chase = 1,
    SUVGK_Max = 2
};
enum SUVS 
{
    SUVS_Nil = -1,
    SUVS_Stop = 0,
    SUVS_Auto = 1,
    SUVS_Manual = 2,
    SUVS_Chase = 3,
    SUVS_Max = 4
};
struct SVRB
{
    CLQ clqDuToRsvAhead;
    CLQ clqDuToRsvBehind;
    LM lmRsvRandom;
    LM lmRsvBalance;
    float mpcpsuvuBoost[4];
};
struct SXP
{
    class ALO* paloWheel;
    class ALO* paloHub;
    struct PNT* ppntTop;
    int xsxp;
    glm::vec3 posCur;
    glm::vec3 posGround;
    glm::vec3 normalGround;
    glm::vec3 vGround;
    float swSpin;
    float radSpin;
    float cParticleDirt;
    float cParticleDust;
};
struct SVE 
{
    SVEK svek;
    struct VOL* pvol;
    int f;
    int isveCheck;
    float sTrack;
    struct SVE* psveCheckPrev;
    float mu;
    float svMax;
    struct EXPL* pexplDirt;
    struct EXPL* pexplDust;
};

struct LAPCTR : public CTR
{

};

struct BOOSTCTR : public CTR
{

};

struct PLACECTR : public CTR
{

};

class SUV : public PO
{
	public:
    float sRadiusFrontWheel;
    float sRadiusRearWheel;
    float svMax;
    float dyMax;
    CLQ clqTune;
    SVRB asvrb[2];
    SVRB svrb;
    class ALO* paloShadow;
    float radTarget;
    float svTarget;
    float radFront;
    int xsxp;
    SXP asxp[4];
    struct SHAPE* pshapeTrack;
    float sTrackMax;
    float uTrack;
    float sTrack;
    float dyTrack;
    float dyTarget;
    float dsTrackFinish;
    struct SHAPE* pshapeLine;
    float uLine;
    float tUpright;
    float tBoost;
    int cBoost;
    float rsvBalance;
    float rsvGoal;
    float tBalance;
    float muSxp;
    float tPunched;
    int csve;
    SVE asve[16];
    struct SVE* psveCheckFirst;
    struct SVE* psveCheckCur;
    struct SVE* psveFeatureCur;
    int cLap;
    int cLapMax;
    int nPlace;
    int nPlaceMax;
    float tPlace;
    struct EMITTER* pemitterBoost;
    struct EXPL* pexplDirt;
    struct EXPL* pexplDust;
    float cParticleDirt;
    float cParticleDust;
    class ALO* paloFrontAxle;
    class ALO* paloRearAxle;
    SUVGK suvgk;
    SUVS suvs;
    float tSuvs;
    struct PATHZONE* ppathzone;
    class SO* psoPrizeCur;
    struct SM* psmPuncher;
    struct SMA* psmaPuncher;
    class SO* psoPuncher;
    int cpsoIgnore;
    class SO* apsoIgnore[8];
    struct XP* pxpPuncher;
    struct MURRAY* pmurray;
    int fFreeWheeling;
    struct AMB* pambSkid;
    struct AMB* pambRunning;
    struct AMB* pambIdle;
    struct AMB* pambBoost;
    struct AMB* pambTireSlip;
    float volBoostTarget;
    float volBoostCur;
    float volTarget;
    float volCur;
    float dvolInc;
    float frqTarget;
    float frqCur;
    float dfrqInc;
    float dfrqTop;
    float dfrqTireSlip;
    float dfrqRunning;
    float dfrqIdle;
    float dfrqExtra;
    float uSuspensionLoad;
    CLQ clqProgressToRsv;
    LM lmRsvBalance;
    float uEngineSpeed;
};

SUV* NewSuv();
int  GetSuvSize();
void InitSuv(SUV *psuv);
void SetSuvSRadiusFrontWheel(SUV* psuv, float sRadiusFrontWheel);
void*GetSuvSRadiusFrontWheel(SUV* psuv);
void SetSuvSRadiusRearWheel(SUV* psuv, float sRadiusRearWheel);
void*GetSuvSRadiusRearWheel(SUV* psuv);
void SetSuvSvMax(SUV* psuv, float svMax);
void*GetSuvSvMax(SUV* psuv);
void SetSuvDyMax(SUV* psuv, float dyMax);
void*GetSuvDyMax(SUV* psuv);
void SetSuvCLapMax(SUV* psuv, int cLapMax);
void*GetSuvCLapMax(SUV* psuv);
void SetSuvPpathzone(SUV* psuv, PATHZONE* ppathzone);
void*GetSuvPpathzone(SUV* psuv);
void SetSuvSuvgk(SUV* psuv, SUVGK suvgk);
void*GetSuvSuvgk(SUV* psuv);
void SetSuvPexplDirt(SUV* psuv, EXPL* pexplDirt);
void*GetSuvPexplDirt(SUV* psuv);
void SetSuvPexplDust(SUV* psuv, EXPL* pexplDust);
void*GetSuvPexplDust(SUV* psuv);
void SetSuvPemitterBoost(SUV* psuv, EMITTER* pemitterBoost);
void*GetSuvPemitterBoost(SUV* psuv);
void SetSuvMuSxp(SUV* psuv, float muSxp);
void*GetSuvMuSxp(SUV* psuv);
void SetSuvCBoost(SUV* psuv, int cBoost);
void*GetSuvCBoost(SUV* psuv);
void SetSuvClqDuToRsvAhead(SUV* psuv, CLQ clq);
void*GetSuvClqDuToRsvAhead(SUV* psuv);
void SetSuvClqDuToRsvBehind(SUV* psuv, CLQ clq);
void*GetSuvClqDuToRsvBehind(SUV* psuv);
void SetSuvLmRsvRandom(SUV* psuv, LM lm);
void*GetSuvLmRsvRandom(SUV* psuv);
void SetSuvLmRsvBalance(SUV* psuv, LM lm);
void*GetSuvLmRsvBalance(SUV* psuv);
void CloneSuv(SUV* psuv, SUV* psuvBase);
void PostSuvLoad(SUV* psuv);
void PresetSuvAccel(SUV* psuv, float dt);
float GExcludeAlm(int clm, LM* alm, float g);
void UpdateSuvBalance(SUV* psuv);
float DsGetTrackRelative(float sMax, float s, float sOther);
int  FIsSuvAheadOf(SUV* psuv, SUV* psuvOther);
void UpdateSuvLine(SUV* psuv, int* pcpsuvInFront);
void UpdateSuvHeading(SUV* psuv);
void UpdateSuvWheels(SUV* psuv);
void UpdateSuvExpls(SUV* psuv);
void BoostSuv(SUV* psuv);
void HandleSuvRaceLoss(SUV* psuv);
void UpdateSuvVolumes(SUV* psuv);
void UpdateSuvSounds(SUV* psuv, float dt);
void UpdateSuvPuncher(SUV* psuv);
void UpdateSuv(SUV* psuv, float dt);
void UpdateSuvActive(SUV* psuv, JOY* pjoy);
void UpdateSuvBlots(SUV* psuv);
void UpdateSuvInternalXps(SUV* psuv);
void AddSuvCustomXps(SUV* psuv, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned);
void AdjustSuvNewXp(SUV* psuv, XP* pxp, int ixpd);
void UpdateSuvInfluences(SUV* psuv, RO* pro);
void RenderSuvSelf(SUV* psuv, CM* pcm, RO* pro);
void UpdateSuvBounds(SUV* psuv);
void CollectSuvPrize(SUV* psuv, PCK pck, ALO* paloOther);
void UpdateSuvShapes(SUV* psuv);
void UpdateSuvXfWorld(SUV* psuv);
void GetSuvCpdefi(SUV* psuv, float dt, CPDEFI* pcpdefi);
void OnSuvActive(SUV* psuv, int fActive);
void HandleSuvMessage(SUV* psuv, MSGID msgid, void* pv);

void*GetSuvTrack(SUV* psuv);
void SetSuvTrack(SUV* psuv, SHAPE* pshapeTrack);
void*GetSuvLine(SUV* psuv);
void SetSuvLine(SUV* psuv, SHAPE* pshapeLine);

void AddSuvCheckPoint(SUV* psuv, VOL* pvol);
void AddSuvFeature(SUV* psuv, VOL* pvol, float mu, float svMax, EXPL* pexplDirt, EXPL* pexplDust);
void*GetSuvSuvs(SUV* psuv);
void SetSuvSuvs(SUV* psuv, SUVS suvs);
void ResetSuv(SUV* psuv);
void DeleteSuv(SUV* psuv);

void StartupLapCtr(LAPCTR* plapctr);
void StartupBoostCtr(BOOSTCTR* pboostctr);
void StartupPlaceCtr(PLACECTR* placectr);

extern LAPCTR g_lapctr;
extern BOOSTCTR g_boostctr;
extern PLACECTR g_placectr;
extern float SV_SuvMinFriction;
static SVRB s_asvrb = 
{
    // CLQ clqDuToRsvAh
    { { 1.0f }, 3.0f, 0.0f, 0.0f },

    // CLQ clqDuToRsvBe
    { { 1.0f }, 5.0f, 0.0f, 0.0f },

    // LM lmRsvRandom
    { { -0.1f }, 0.1f },

    // LM lmRsvBalance
    { { 0.85f }, 1.15f },

    // float[4] mpcpsuvuBoost
    { 0.5f, 0.8f, 0.65f, 0.5f },
};
extern CLQ s_clqTune;
extern SNIP s_asnipSuv[20];
extern float DT_SuvUpright;
extern float DT_SuvPunched;
extern CLQ s_clqSuvUpright;
extern CLQ CLQ_DustFromSpeed;
extern float C_DustMin;
extern float C_DustMax;
extern CLQ SuvSpringVelocity;
extern float K_SuvSpringVelocity;
extern CLQ CLQ_SuvSpringPos;
extern float K_SuvSpringPos;
extern CLQ CLQ_AirborneUpright;
extern float G_SuvSuspensionLoadMax;
extern float DT_SuvBoost;
extern CLQ CLQ_SuvBoostAccel;
extern float R_SuvBoostAccelMin;
extern float R_SuvBoostAccelMax;
extern float SV_SuvTractionMax;
extern CLQ CLQ_SuvSpeedToDrive;
extern float SV_SuvSteerMax;
extern float K_SuvSteer;
extern CLQ CLQ_SuvBoostDrive;
extern float R_SuvDriveMin;
extern float R_SuvDriveMax;
extern CLQ CLQ_SuvFacingDrive;
extern float R_SuvFacingDriveMin;
extern float R_SuvFacingDriveMax;
extern CLQ CLQ_SuvAccelLimit;
extern float R_SuvAccelLimitMin;
extern float R_SuvAccelLimitMax;
extern SMP s_smpSuvAccel;
extern SMP s_smpSuvDecel;
extern CLQ CLQ_DirtFromAccel;
extern float C_DirtMin;
extern float C_DirtMax;
extern float G_SuvDownForce;
extern SMP s_smpBalance;
extern LM s_lmDtBalance;
extern CLQ CLQ_SuvDsToDyAvoid;
extern LM LM_SuvDyAvoid;
extern float DS_SuvHoming;
extern LM LM_SuvTrack;
extern SMP s_smpLine;
extern float DS_SuvNext;
extern float R_SuvTurnDrive;
extern float RAD_SuvWheelMax;
extern SMP s_smpWheelTurn;
extern CLQ CLQ_SuvDtToRDensity;
extern LM LM_SuvRadStick;
extern LM LM_SuvRadSpray;
extern float S_SuvSoundMax;
extern CLQ CLQ_SuvSuspensionToEngineBlend;
extern LM LM_SuvEngineBlend;
extern float R_SuvTargetEngineSpeed;
extern CLQ CLQ_SuvIdleFrq;
extern CLQ CLQ_SuvIdleVol;
extern LM LM_SuvIdleVol;
extern CLQ CLQ_SuvRunningFrq;
extern CLQ CLQ_SuvRunningVol;
extern LM LM_SuvRunningVol;
extern float R_SuvTireSlipX;
extern float S_SuvTireSlipMax;
extern CLQ CLQ_SuvSuspensionToTireSlip;
extern LM LM_SuvTireSlipScale;
extern CLQ CLQ_SuvTireSlipFrq;
extern CLQ CLQ_SuvTireSlipVol;
extern LM LM_SuvTireSlipVol;
extern SMP s_smpSuvEngineSpeed;
extern glm::vec3 s_vPuncher;
extern float R_SuvPuncherSvToSw;
extern float RV_SuvPuncherSelf;
extern SMP s_smpWheelFree;
extern int N_SuvChaseWin;
extern int g_fVehicleSpeedLimited;
extern float DT_SuvBoostRecharge;
extern SCORES* g_pscoresCur;

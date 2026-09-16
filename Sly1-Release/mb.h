#pragma once
#include "stepguard.h"

enum MBAK 
{
    MBAK_Nil = -1,
    MBAK_Gun = 0,
    MBAK_Grenade = 1,
    MBAK_Smash = 2,
    MBAK_Max = 3
};

struct MBGLBONE
{
    OID oidLbone;
    float g;
    struct LBONE *plbone;
};

class MBG : public STEPGUARD
{
	public:
    int fAbandonExternal;
    BLRUN ablrun[6];
    struct ASEGBL* pasegblRun;
    int fLeftFootAirborne;
    int fRightFootAirborne;
    class SM* psm;
    class SMA *psmaMbg;
    class XFM *pxfm;
    class RWM *prwmLeftFire;
    class RWM* prwmRightFire;
    int clBone;
    std::vector <MBGLBONE> ambglbone;
    int iAPhase;
    int fRenderBurnEffect;
    class ASEG* mpmbakpaseg[3];
    int oidSma;
    glm::vec3 posPredictedTarget;
};

class BHG : public STEPGUARD
{
    public:
    int cAbsorbedHits;
    ASEG *pasegHit;
    ASEG *pasegSweepCounterHit;
    ASEG *pasegSmashHit;
    ASEGA *pasegaCur;
    int fDetect;
};

class SCENTMAP : public ALO
{
	public:

	int cxMax;
	int cyMax;
	int cscp;
	struct SCP* ascp;
	struct SCP** mpixiypscpFirst;
	glm::vec3 posMin;
	glm::vec3 posMax;
};

MBG* NewMbg();
void InitMbg(MBG* pmbg);
int  GetMbgSize();
void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis);
void CloneMbg(MBG* pmbg, MBG* pmbgBase);
void PostMbgLoad(MBG* pmbg);
void DrawMbgHaloFire(RPL* prpl);
void RenderMbgSelf(MBG* pmbg, CM* pcm, RO* pro);
SO*  PsoHitTestCylinderObjects(GRFHTL grfhtl, glm::vec3* ppos0, glm::vec3* ppos1, float sRadius, std::vector<SO*>& apso, LSG* plsgBest);
int  FFilterMbgAttackObjects(MBG* pmbg, SO* psoOther);
int  FObstructedMbg(MBG* pmbg, glm::vec3* ppos0, glm::vec3* ppos1);
int  FCanMbgAttack(MBG* pmbg);
int  FDetectMbg(MBG* pmbg);
void OnMbgEnteringSgs(MBG* pmbg, SGS sgsPrev, ASEG* pasegOverride);
SGS  SgsNextMbgAi(MBG* pmbg);
void UpdateMbgAi(MBG* pmbg); 
void UpdateMbg(MBG* pmbg, float dt);
void UpdateMbgBossCounter(MBG *pmbg);
void UpdateMbgGoal(MBG* pmbg, int fEnter);
void UpdateMbgSgs(MBG* pmbg);
void OnMbgExitingSgs(MBG* pmbg, SGS sgsNext);
void HandleMbgMessage(MBG* pmbg, MSGID msgid, void* pv);
void GetMbgAttackBlend(MBG* pmbg, float* pgBlend);
void DeleteMbg(MBG* pmbg);

BHG* NewBhg();
void InitBhg(BHG* pbhg);
int  GetBhgSize();
void CloneBhg(BHG* pbhg, BHG* pbhgBase);
void PostBhgLoad(BHG* pbhg);
int  FDetectBhg(BHG* bhg);
void UpdateBhgGoal(BHG* pbhg, int fEnter);
void UpdateBhgSgs(BHG* pbhg);
void OnBhgEnteringSgs(BHG* pbhg, SGS sgsPrev, ASEG* pasegTargetOverride);
void UpdateBhg(BHG* pbhg, float dt);
void UpdateBhgBlot();
int  FAbsorbBhgWkr(BHG* pbhg, WKR* pwkr);
void SetBhgHitAnimations(BHG* pbhg, ASEG* pasegHit, ASEG* pasegSweepCounterHit, ASEG* pasegSmashHit);
void DeleteBhg(BHG* pbhg);

SCENTMAP*NewScentmap();
void InitScentmap(SCENTMAP* pscentmap);
int  GetScentmapSize();
void CloneScentmap(SCENTMAP* pscentmap, SCENTMAP* pscentmapBase);
void PostScentmapLoad(SCENTMAP* pscentmap);
void UpdateScentmap(SCENTMAP* pscentmap, float dt);
void DeleteScentmap(SCENTMAP* pscentmap);

extern SNIP s_asnipLoadMbg[2];
extern SNIP s_asnipPostMbgLoad[3];
extern OID  s_mpmbakoidAseg[3];
extern CID  s_acidReject[5];
extern float S_MbgVisCylinder;
extern MBGLBONE s_ambgLboneMbg[6];
extern float kMbgPursueNearSpeedBase;
extern float kMbgPursueNearSpeedLinear;
extern float kMbgPursueNearSpeedQuad;
extern float kMbgPursueFarSpeedBase;
extern float kMbgPursueFarSpeedLinear;
extern float kMbgPursueFarSpeedQuad;
extern float kMbgTurnBlendQuad;
extern float kMbgTurnBlendLinear;
extern float kMbgTurnBlendBase;
extern int s_mbgAiSmaOid[3];
extern SGS s_mbgAiSgs[3];
extern float kMbgFireFlatZOffset;
extern float kMbgFirePredictZOffset;
extern float DT_PredictMbg;

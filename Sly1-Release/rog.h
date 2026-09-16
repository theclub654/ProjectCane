#pragma once
#include "po.h"

enum ROVS 
{
    ROVS_Nil = -1,
    ROVS_Brake = 0,
    ROVS_Drive = 1,
    ROVS_Descend = 2,
    ROVS_Ascend = 3,
    ROVS_Max = 4
};
enum ROVTS 
{
    ROVTS_Nil = -1,
    ROVTS_Calm = 0,
    ROVTS_Firing = 1,
    ROVTS_Max = 2
};
enum ROHS 
{
    ROHS_Nil = -1,
    ROHS_Inactive = 0,
    ROHS_Spawn = 1,
    ROHS_Wander = 2,
    ROHS_Collect = 3,
    ROHS_Grab = 4,
    ROHS_Return = 5,
    ROHS_Exit = 6,
    ROHS_Happy = 7,
    ROHS_Sad = 8,
    ROHS_Die = 9,
    ROHS_Dead = 10,
    ROHS_Max = 11
};
enum ROCS 
{
    ROCS_Nil = -1,
    ROCS_Rest = 0,
    ROCS_Hit1 = 1,
    ROCS_Hit2 = 2,
    ROCS_Hit3 = 3,
    ROCS_Hit4 = 4,
    ROCS_Hit5 = 5,
    ROCS_Hit6 = 6,
    ROCS_Hit7 = 7,
    ROCS_Hit8 = 8,
    ROCS_Destroyed = 9,
    ROCS_Max = 10
};
enum ROSTS 
{
    ROSTS_Nil = -1,
    ROSTS_Close = 0,
    ROSTS_Open = 1,
    ROSTS_Max = 2
};
enum ROPS 
{
    ROPS_Nil = -1,
    ROPS_StayPut = 0,
    ROPS_Reveal = 1,
    ROPS_Collect = 2,
    ROPS_Collected = 3,
    ROPS_Max = 4
};
enum ROBK : short
{
    ROBK_Nil = -1,
    ROBK_Primary = 0,
    ROBK_Secondary = 1,
    ROBK_Tertiary = 2,
    ROBK_Max = 3
};
enum ROBS 
{
    ROBS_Nil = -1,
    ROBS_Descend = 0,
    ROBS_Instruct = 1,
    ROBS_Normal = 2,
    ROBS_Lose = 3,
    ROBS_Win = 4,
    ROBS_Ascend = 5,
    ROBS_Max = 6
};

struct RODD 
{
    float crocActiveMaxEasy;
    float crocActiveMaxHard;
    CLQ clqCrocActive;
    float crohActiveMaxEasy;
    float crohActiveMaxHard;
    CLQ clqCrohActive;
    LM lmDtAddRocEasy;
    LM lmDtAddRocHard;
    CLQ clqDtAddRoc;
    LM lmDtAddRohEasy;
    LM lmDtAddRohHard;
    CLQ clqDtAddRoh;
    float svRohMaxEasy;
    float svRohMaxHard;
    CLQ clqSvRoh;
    float dradRohMaxEasy;
    float dradRohMaxHard;
    CLQ clqDradRoh;
};

class ROV : public PO
{
	public:
    ROVS rovs;
    ROVTS rovts;
    float radJoyLeft;
    float uJoyDeflectLeft;
    float radJoyRight;
    float uJoyDeflectRight;
    float svMax;
    glm::vec3 vTarget;
    struct SM* psmBody;
    struct SMA* psmaBody;
    struct AMB* pambBody;
    class ALO* paloTurret;
    struct SM* psmTurret;
    struct SMA* psmaTurret;
    struct AMB* pambTurret;
    class EMITTER* pemitterLeftGun;
    class EMITTER* pemitterRightGun;
    class ALO* paloProp;
    int cpemitterProp;
    class EMITTER* apemitterProp[3];
    struct ROB* prob;
    struct ASEG* pasegAscend;
    struct ASEG* pasegDescend;
    struct ASEGA* pasegaDrive;
    int fRetractDrive;
};

class ROH : public SO
{
	public:
    ROHS rohs;
    float tRohs;
    struct ROB* prob;
    struct ROC* proc;
    struct ROST* prost;
    float svMax;
    float dradMax;
    glm::vec3 posWander;
    float tWanderNext;
    LM lmDtWander;
    struct ASEG* pasegMove;
    struct ASEG* pasegMoveCollect;
    struct ASEG* pasegaGrab;
    struct ASEG* pasegDie;
    struct ASEGA* pasega;
    struct SM* psmHappy;
    struct SM* psmSad;
    struct SMA* psma;
    struct EXPL* pexpl;
    DLE dleRob;
};

class ROC : public SO
{
	public:
    ROCS rocs;
    float tRocs;
    struct ROB* prob;
    struct ROH* proh;
    struct SM* psm;
    struct SMA* psma;
    int cHitMax;
    float dtRocsHitDown;
    float dtRocsHitUp;
    struct EXPL* pexpl;
    struct EMITTER* pemitterBubble;
    LM lmSvcBubble;
    CLQ clqBubble;
    DLE dleRob;
};

class ROST : public SO
{
	public:
    ROSTS rosts;
    float tRosts;
    struct ROB* prob;
    struct SM* psm;
    struct SMA* psma;
    struct PNT* ppnt;
    DLE dleRob;
};

class ROP : public SO
{
	public:
    ROPS rops;
    float tRops;
    struct ROB* prob;
    struct KEY* pkey;
    struct ASEG* pasegReveal;
    struct ASEGA* pasega;
    struct EXPL* pexpl;
};

class ROB : public ALO
{
	public:
    short oidCollectible;
    short oidHarvester;
    short oidVehicle;
    short oidPrize;
    int coidRost;
    short aoidRost[16];
    glm::vec3 posCenterArena;
    float sRadiusArenaTube;
    float sRadiusArenaRoc;
    float sRadiusArenaWander;
    float rAspectArena;
    float sCollectibleDropHeight;
    struct ROV* prov;
    struct ROP* prop;
    DL dlRocOwned;
    DL dlRocUnowned;
    DL dlRocInactive;
    int crocActive;
    DL dlRohActive;
    DL dlRohInactive;
    int crohActive;
    DL dlRostAvailable;
    DL dlRostUnavailable;
    int crocCreateMax;
    int crocCreated;
    int crocReturned;
    int crocDestroyed;
    CLQ clqTune;
    RODD arodd[2];
    RODD rodd;
    int irodd;
    int crocActiveMax;
    int crohActiveMax;
    LM lmDtAddRoc;
    float tAddRoc;
    LM lmDtAddRoh;
    float tAddRoh;
    float svRohMax;
    float dradRohMax;
    ROBK robk;
    ROBS robs;
    float tRobs;
    struct DIALOG* pdialog;
};

void ChooseRobWanderLocation(ROB* prob, ROH* proh);

ROV* NewRov();
void InitRov(ROV* prov);
void SetRovSvMax(ROV* prov, float svMax);
void*GetRovSvMax(ROV* prov);
void CloneRov(ROV* prov, ROV* provBase);
int  GetRovSize();
void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis);
void PostRovLoad(ROV* prov);
void UpdateRovActive(ROV* prov, JOY* pjoy, float dt);
void UpdateRov(ROV* prov, float dt);
void ReadRovJoystick(ROV* prov, JOY* pjoy);
ROVS RovsNextRov(ROV* prov);
void SetRovRovs(ROV* prov, ROVS rovs);
ROVTS RovtsNextRov(ROV* prov);
void SetRovRovts(ROV* prov, ROVTS rovts);
void HandleRovMessage(ROV* prov, MSGID msgid, void* pv);
void PresetRovAccel(ROV* prov, float dt);
void AdjustRovXpVelocity(ROV* prov, XP* pxp, int ixpd);
void AdjustRovNewXp(ROV* prov, XP* pxp, int ixpd);
void PropagateRovForce(ROV* prov, GRFSG grfsg, XP* pxp, int ixpd, DZ* pdz, FX* afx);
int  FIsRovSoundBase();
void DeleteRov(ROV *prov);

ROH* NewRoh();
void InitRoh(ROH* proh);
void SetRohLmDtWander(ROH* proh, LM lmDtWander);
void*GetRohLmDtWander(ROH* proh);
int  GetRohSize();
void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis);
void CloneRoh(ROH* proh, ROH* prohBase);
void PostRohLoad(ROH* proh);
void UpdateRoh(ROH* proh, float dt);
void MoveRohToTarget(ROH* proh, glm::vec3* targetPosition, int fBackwards);
ROHS RohsNextRoh(ROH* proh);
void SetRohRohs(ROH* proh, ROHS rohs);
int  FAbsorbRohWkr(ROH* proh, WKR* pwkr);
ROC* ProcContactRoh(ROH* proh);
void DeleteRoh(ROH *proh);

ROC* NewRoc();
void InitRoc(ROC* proc);
void SetRocCHitMax(ROC* proc, int cHitMax);
void SetRocDtRocsHitUp(ROC* proc, float dtRocsHitUp);
void*GetRocDtRocsHitUp(ROC* proc);
void SetRocDtRocsHitDown(ROC* proc, float dtRocsHitDown);
void*GetRocDtRocsHitDown(ROC* proc);
void SetRocClqBubble(ROC* proc, CLQ clqBubble);
void*GetRocClqBubble(ROC* proc);
int  GetRocSize();
void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis);
void CloneRoc(ROC* proc, ROC* procBase);
void PostRocLoad(ROC* proc);
void UpdateRoc(ROC* proc, float dt);
void PresetRocAccel(ROC* proc, float dt);
void AdjustRocNewXp(ROC* proc, XP* pxp, int ixpd);
int  FAbsorbRocWkr(ROC* proc, WKR* pwkr);
ROCS RocsNextRoc(ROC* proc);
void SetRocRocs(ROC* proc, ROCS rocs);
void DeleteRoc(ROC* proc);

ROST*NewRost();
void InitRost(ROST* prost);
int  GetRostSize();
void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis);
void CloneRost(ROST* prost, ROST* prostBase);
void PostRostLoad(ROST* prost);
void UpdateRost(ROST* prost, float dt);
ROSTS RostsNextRost(ROST* prost);
void SetRostRosts(ROST* prost, ROSTS rosts);
void DeleteRost(ROST *prost);

ROP* NewRop();
void InitRop(ROP* prop);
int  GetRopSize();
void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis);
void CloneRop(ROP* prop, ROP* probBase);
void PostRopLoad(ROP* prop);
void UpdateRop(ROP* prop, float dt);
void SetRopRops(ROP* prop, ROPS rops);
ROPS RopsNextRop(ROP* prop);
void DeleteRop(ROP *prop);

ROB* NewRob();
void InitRob(ROB* prob);
float* ProddCurRobCrocActiveMaxEasy(ROB* prob, ENSK ensk);
float* ProddCurRobCrocActiveMaxHard(ROB* prob, ENSK ensk);
CLQ* ProddCurRobClqCrocActive(ROB* prob, ENSK ensk);
float* ProddCurRobCrohActiveMaxEasy(ROB* prob, ENSK ensk);
float* ProddCurRobCrohActiveMaxHard(ROB* prob, ENSK ensk);
CLQ* ProddCurRobClqCrohActive(ROB* prob, ENSK ensk);
LM* ProddCurRobLmDtAddRocEasy(ROB* prob, ENSK ensk);
LM* ProddCurRobLmDtAddRocHard(ROB* prob, ENSK ensk);
CLQ* ProddCurRobClqDtAddRoc(ROB* prob, ENSK ensk);
LM* ProddCurRobLmDtAddRohEasy(ROB* prob, ENSK ensk);
LM* ProddCurRobLmDtAddRohHard(ROB* prob, ENSK ensk);
CLQ* ProddCurRobClqDtAddRoh(ROB* prob, ENSK ensk);
float* ProddCurRobSvRohMaxEasy(ROB* prob, ENSK ensk);
float* ProddCurRobSvRohMaxHard(ROB* prob, ENSK ensk);
CLQ* ProddCurRobClqSvRoh(ROB* prob, ENSK ensk);
float* ProddCurRobDradRohMaxEasy(ROB* prob, ENSK ensk);
float* ProddCurRobDradRohMaxHard(ROB* prob, ENSK ensk);
CLQ* ProddCurRobClqDradRoh(ROB* prob, ENSK ensk);
void SetRobPosCenterArena(ROB* prob, glm::vec3 posCenterArena);
void*GetRobPosCenterArena(ROB* prob);
void SetRobSRadiusArenaTube(ROB* prob, float sRadiusArenaTube);
void*GetRobSRadiusArenaTube(ROB* prob);
void SetRobSRadiusArenaRoc(ROB* prob, float aRadiusArenaRoc);
void*GetRobSRadiusArenaRoc(ROB* prob);
void SetRobSRadiusArenaWander(ROB* prob, float sRadiusArenaWander);
void*GetRobSRadiusArenaWander(ROB* prob);
void SetRobRAspectArena(ROB* prob, float rAspectArena);
void*GetRobRAspectArena(ROB* prob);
void SetRobSCollectibleDropHeight(ROB* prob, float sCollectibleDropHeight);
void*GetRobSCollectibleDropHeight(ROB* prob);
void SetRobCrocCreateMax(ROB* prob, int crocCreateMax);
void*GetRobCrocCreateMax(ROB* prob);
void SetRobRobk(ROB* prob, ROBK robk);
void*GetRobRobk(ROB* prob);
void SetRobIrodd(ROB* prob, int irodd);
void*GetRobIrodd(ROB* prob);
void SetRobClqTune(ROB* prob, glm::vec4 clqTune);
void*GetRobClqTune(ROB* prob);
int  GetRobSize();
void SetRobCollectible(ROB* prob, OID oidCollectible);
void SetRobHarvester(ROB* prob, OID oidHarvester);
void SetRobVehicle(ROB* prob, OID oidVehicle);
void SetRobPrize(ROB* prob, OID oidPrize);
void AddRobSpawnTunnel(ROB* prob, OID oidSpawnTunnel);
ROBK RobkCur();
void CloneRob(ROB* prob, ROB* probBase);
void BindRob(ROB* prob);
void PostRobLoad(ROB* prob);
void UpdateRob(ROB* prob, float dt);
void UpdateRobBlot(ROB* prob);
ROBS RobsNextRob(ROB* prob);
void SetRobRobs(ROB* prob, ROBS robs);
void AddRobRoc(ROB* prob);
void AddRobRoh(ROB* prob);
void AdjustRobDifficulty(ROB* prob, float dt);
void DestroyedRobRoc(ROB* prob, ROC* proc);
void SpawnedRobRoh(ROB* prob, ROH* proh);
void GrabbedRobRoh(ROB* prob, ROH* proh);
void DroppedRobRoh(ROB* prob, ROH* proh);
void ReturnedRobRoh(ROB* prob, ROH* proh);
void ExitedRobRoh(ROB* prob, ROH* proh);
void KilledRobRoh(ROB* prob, ROH* proh);
int  FChooseRobRoc(ROB* prob, ROH* proh);
void TakeRobRoc(ROB* prob, ROH* proh, ROC* proc);
int  FChooseRobRoh(ROB* prob, ROC* proc);
int  FChooseRobReturnPoint(ROB* prob, ROH* proh);
void ChooseRobWanderLocation(ROB* prob, ROH* proh);
RODD* ProddCurRob(ROB* prob, ENSK ensk);
void DeleteRob(ROB *prob);

extern SNIP s_asnipLoadRov[2];
extern SNIP s_asnipPostRovLoad[6];
extern SNIP s_asnipLoadRoh[6];
extern SMP  s_smpRovSpin;
extern SMP  s_smpRocSpin;
extern int s_mprocsoidState[10];
extern SNIP s_asnipLoadRoc;
extern SNIP s_asnipPostRocLoad;
extern SNIP s_asnipLoadRost;
extern int s_aoidStateNext[2];
extern SNIP s_asnipLoadRop;
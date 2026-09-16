#pragma once
#include "step.h"

enum SGS
{
    SGS_Nil = -1,
    SGS_Reset = 0,
    SGS_Patrol = 1,
    SGS_PatrolIdle = 2,
    SGS_PatrolWaypoint = 3,
    SGS_ReturnToPatrol = 4,
    SGS_Search = 5,
    SGS_SearchIdle = 6,
    SGS_Discover = 7,
    SGS_LostPlayer = 8,
    SGS_GiveUpSearch = 9,
    SGS_Taunt = 10,
    SGS_Dying = 11,
    SGS_Pursue = 12,
    SGS_PursueIdle = 13,
    SGS_Attack = 14,
    SGS_AttackIdle = 15,
    SGS_Stun = 16,
    SGS_Max = 17
};

enum SGAS
{
    SGAS_Nil = -1,
    SGAS_No = 0,
    SGAS_Yes = 1,
    SGAS_Force = 2,
    SGAS_Max = 3
};

struct STEP_GUARD_WATER_MSG
{
    void*pvUnknown;
    STEPGUARD* pstepguard;
};

struct STEP_GUARD_LABEL_MSG
{
    ASEGA* pasega;
    float tLocal;
    int label;
};

struct STEP_GUARD_WAYPOINT_MSG
{
    ASEGA* pasega;
    WAYPOINT* pwaypoint;
};

struct STEP_GUARD_WAYPOINT_TRANSITION_MSG
{
    WAYPOINT* pwaypoint;
    int unused;
    int transition;
};

struct STEP_GUARD_DAMAGED_MSG
{
    SO* psoSource;
    SO* psoAmmo;
};

struct SGA 
{
    OID oidAseg;
    struct ASEG* paseg;
};
struct SGMAP 
{
    OID aoid[4];
};
struct SGP 
{
    OID oidPhys;
    class SO* psoPhys;
};
enum RPK 
{
    RPK_Nil = -1,
    RPK_Closest = 0,
    RPK_Previous = 1,
    RPK_Max = 2
};
struct SGE 
{
    OID oidEffect;
    class SO* pso;
    ZPK zpk;
};
struct SGDA 
{
    int grftak;
    OID oidAseg;
    struct ASEG* paseg;
};
enum SGGS
{
    SGGS_Nil = -1,
    SGGS_Patrol = 0,
    SGGS_Search = 1,
    SGGS_Attack = 2,
    SGGS_Dead = 3,
    SGGS_Max = 4
};

struct DMG
{
    int mode;
    SO *psoSource;
};

class STEPGUARD : public STEP
{
	public:
    SGG* psgg;
    SGS sgs;
    float tSgs;
    int grfsgsc;
    SGS sgsNext;
    ALO* paloHead;
    LM lmAttackS;
    LM lmAttackT;
    float tAttackNext;
    float radAttackMax;
    ASEG* pasegPatrol;
    ASEGA* pasegaPatrol;
    SGA mpsgssga[17];
    ASEGA* pasegaSgs;
    int csgmap;
    SGMAP asgmap[8];
    ASEG* pasegPrev;
    float uAsegaPrev;
    SGP mpsgssgp[17];
    SO* psoPhysDefault;
    glm::vec3 posSearch;
    float radTargetSearch;
    float tPatrolReturn;
    float tPatrolPrev;
    float svtPatrolPrev;
    RPK rpk;
    short oidPathzone;
    PATHZONE* ppathzone;
    int cposPath;
    int iposPath;
    glm::vec3 aposPath[16];
    glm::vec3 posGoal;
    glm::vec3 posGoalPath;
    int fPathValid;
    float tPathValid;
    int cStun;
    int fTumbleStun;
    float dtFade;
    WAYPOINT* pwaypoint;
    int csge;
    SGE asge[4];
    XP* pxpEffect;
    glm::vec3 vThrow;
    int fBypassEffectHitSuppression;
    int csgda;
    SGDA asgda[4];
    int fExplExplicit;
    short oidExpl;
    EXPL* pexpl;
    int fRwmExplicit;
    short oidRwm;
    RWM* prwm;
    int ichkDead;
    int fFirstFrame;
    float uIdleStart;
    int fFlying;
    float dzFlying;
    float radTilt;
    CLQ clqSToUPursue;
    int fEnemyExternal;
    int fRebindEnemy;
    SO* psoEnemyExternal;
    short oidEnemyExternal;
    SO* psoEnemyBound;
    SO* psoSubscribe;
    int ccoin;
    glm::vec3 posBreakLocal;
    int fNoDifficultyUpdate;
    STEPGUARD* pstepguardBase;
    int fAnimatedJump;
    int fLockHeading;
    int fLockLook;
    int fLockState;
    int fExternalAI;
    int fFreefall;
    int fFreefallAseg;
    float dxFreefall;
    float dtFreefall;
    LM lmDtFreefall;
    float tFreefallLand;
    glm::vec3 posFreefallLand;
    glm::vec3 vFreefallLand;
    float uFling;
    int fAutoFadeDying;
    int fAbsorbDamage;
    int fFaceDamageDirection;
    LM lmDtTaunt;
    float muContact;
    OVR ovrSgs;
    float tGiveUpSearch;
    float tStopPatrol;
    float tTaunt;
    float tLostPlayer;
    int fIncapacitated;
    int fAlignToGround;
};

class SGG : public LO
{
	public:
    glm::vec3 posEnemyLast;
    SGGS sggs;
    float tSggs;
    int cpstepguard;
    struct STEPGUARD* apstepguard[16];
    int coidStepguard;
    short aoidStepguard[16];
    int cpxfmSearch;
    struct XFM* apxfmSearch[16];
    int coidXfmSearch;
    short aoidXfmSearch[16];
    int cpalarm;
    struct ALARM* apalarm[4];
    int fCallback;
    short oidSync;
    struct EXC* pexcAttack;
    int fDetect;
};

STEPGUARD*NewStepguard();
void InitStepGuard(STEPGUARD* pstepguard);
void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis);
void*GetStepguardLmAttackS(STEPGUARD* pstepguard);
void SetStepguardLmAttackS(STEPGUARD* pstepguard, LM lmAttackS);
void*GetStepguardLmAttackT(STEPGUARD* pstepguard);
void SetStepguardLmAttackT(STEPGUARD* pstepguard, LM lmAttackT);
void*GetStepguardRadAttackMax(STEPGUARD* pstepguard);
void SetStepguardRadAttackMax(STEPGUARD* pstepguard, float radAttackMax);
void*GetStepguardDtFade(STEPGUARD* pstepguard);
void SetStepguardDtFade(STEPGUARD* pstepguard, float dtFade);
void*GetStepguardCStun(STEPGUARD* pstepguard);
void SetStepguardCStun(STEPGUARD* pstepguard, int cStun);
void*GetStepguardFTumbleStun(STEPGUARD* pstepguard);
void SetStepguardFTumbleStun(STEPGUARD* pstepguard, int fTumbleStun);
void*GetStepguardLmDtFreefall(STEPGUARD* pstepguard);
void SetStepguardLmDtFreefall(STEPGUARD* pstepguard, LM lmDtFreefall);
void*GetStepguardFLockLook(STEPGUARD* pstepguard);
void SetStepguardFLockLook(STEPGUARD* pstepguard, int fLockLook);
void*GetStepguardOidEnemyExternal(STEPGUARD* pstepguard);
void SetStepguardOidEnemyExternal(STEPGUARD* pstepguard, int oidEnemyExternal);
void*GetStepguardFRebindEnemy(STEPGUARD* pstepguard);
void SetStepguardFRebindEnemy(STEPGUARD* pstepguard, int fRebindEnemy);
void*GetStepguardFFlying(STEPGUARD* pstepguard);
void SetStepguardFFlying(STEPGUARD* pstepguard, int fFlying);
void*GetStepguardDzFlying(STEPGUARD* pstepguard);
void SetStepguardDzFlying(STEPGUARD* pstepguard, float dzFlying);
void*GetStepguardClqSToUPursue(STEPGUARD* pstepguard);
void SetStepguardClqSToUPursue(STEPGUARD* pstepguard, CLQ clqSToUPursue);
void*GetStepguardRpk(STEPGUARD* pstepguard);
void SetStepguardRpk(STEPGUARD* pstepguard, RPK rpk);
void*GetStepguardCcoin(STEPGUARD* pstepguard);
void SetStepguardCcoin(STEPGUARD* pstepguard, int ccoin);
void*GetStepguardPosBreakLocal(STEPGUARD* pstepguard);
void SetStepguardPosBreakLocal(STEPGUARD* pstepguard, glm::vec3 posBreakLocal);
void*GetStepguardFNoDifficultyUpdate(STEPGUARD* pstepguard);
void SetStepguardFNoDifficultyUpdate(STEPGUARD* pstepguard, int fNoDifficultyUpdate);
void SetStepguardUIdleStart(STEPGUARD* pstepguard, float uIdleStart);
void*GetStepguardUFling(STEPGUARD* pstepguard);
void SetStepguardUFling(STEPGUARD* pstepguard, float uFling);
void*GetStepguardFAutoFadeDying(STEPGUARD* pstepguard);
void SetStepguardFAutoFadeDying(STEPGUARD* pstepguard, int fAutoFadeDying);
void*GetStepguardSgs(STEPGUARD* pstepguard);
void*GetStepguardFBypassEffectHitSuppression(STEPGUARD* pstepguard);
void SetStepguardFBypassEffectHitSuppression(STEPGUARD* pstepguard, int fBypassEffectHitSuppression);
void*GetStepguardLmDtTaunt(STEPGUARD* pstepguard);
void SetStepguardLmDtTaunt(STEPGUARD* pstepguard, LM lmDtTaunt);
void*GetStepguardFFaceDamageDirection(STEPGUARD* pstepguard);
void SetStepguardFFaceDamageDirection(STEPGUARD* pstepguard, int fFaceDamageDirection);
void*GetStepguardMuContact(STEPGUARD* pstepguard);
void SetStepguardMuContact(STEPGUARD* pstepguard, float muContact);
void*GetStepguardFAbsorbDamage(STEPGUARD* pstepguard);
void SetStepguardFAbsorbDamage(STEPGUARD* pstepguard, int fAbsorbDamage);
void*GetStepguardOvrOid(STEPGUARD* pstepguard);
void SetStepguardOvrOid(STEPGUARD* pstepguard, int oid);
void*GetStepguardFAlignToGround(STEPGUARD* pstepguard);
void SetStepguardFAlignToGround(STEPGUARD* pstepguard, int fAlignToGround);
void CloneStepguard(STEPGUARD* pstepguard, STEPGUARD* pstepguardBase);
void BindStepguard(STEPGUARD* pstepguard);
void PostStepguardLoadCallback(STEPGUARD* pstepguard, MSGID msgid, void*pv);
void PostStepguardLoad(STEPGUARD* pstepguard);
void SetStepguardSgs(STEPGUARD* pstepguard, SGS sgs, ASEG* pasegTargetOverride);
int  FIgnoreStepguardIntersection(STEPGUARD* pstepguard, SO* psoOther);
void OnStepguardAdd(STEPGUARD* pstepguard);
void OnStepguardRemove(STEPGUARD* pstepguard);
void PresetStepguardAccel(STEPGUARD* pstepguard, float dt);
float DradEnemyStepguard(STEPGUARD* pstepguard);
void MoveStepguardToGoal(STEPGUARD* pstepguard);
void SetStepguardTargetHeading(STEPGUARD* pstepguard, float radTarget, float radTilt);
void UpdateStepguardAI(STEPGUARD* pstepguard);
void UpdateStepguardDrive(STEPGUARD* pstepguard);
void GetStepguardOnFinalPathSegment(STEPGUARD* pstepguard, int* pfFinalSegment);
void UpdateStepguard(STEPGUARD* pstepguard, float dt);
SGS  SgsNextStepguardAI(STEPGUARD* pstepguard);
void SetStepguardGoal(STEPGUARD* pstepguard, glm::vec3* pposGoal);
int  FReachedStepguardGoal(STEPGUARD* pstepguard);
int  FFilterStepguardJump(STEPGUARD* pstepguard, SO* pso);
void UpdateStepguardPath(STEPGUARD* pstepguard);
void LookStepguardAtGoal(STEPGUARD* pstepguard);
void UpdateStepguardGoal(STEPGUARD* pstepguard, int fEnter);
void OnStepguardWaypointActive(STEPGUARD* pstepguard);
void UpdateStepguardSgs(STEPGUARD* pstepguard);
void SetStepguardSgsExternal(STEPGUARD* pstepguard, SGS sgs, ASEG* pasegTargetOverride);
void OnStepguardExitingSgs(STEPGUARD* pstepguard, SGS sgsNext);
void OnStepguardEnteringSgs(STEPGUARD* pstepguard, SGS sgsPrev, ASEG* pasegTargetOverride);
SGGS SggsGetStepguard(STEPGUARD* pstepguard);
int  FAbsorbStepguardWkr(STEPGUARD* pstepguard, WKR* pwkr);
int  FTakeStepguardDamage(STEPGUARD* pstepguard, ZPR* pzpr);
bool FAbsorbDamageStepguard(STEPGUARD* pstepguard);
int  FCanStepguardTauntDamage(STEPGUARD* pstepguard, SO* psoEnemy, SO* psoDamage);
void HandleStepguardGrfsgsc(STEPGUARD* pstepguard);
void DoStepguardFreefallJump(STEPGUARD* pstepguard);
void DoStepguardFreefallLanding(STEPGUARD* pstepguard);
void HandleStepguardMessage(STEPGUARD* pstepguard, MSGID msgid, void*pv);
SGAS SgasGetStepguard(STEPGUARD* pstepguard);
int  FCanStepguardAttack(STEPGUARD* pstepguard);
void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro);
int  FValidSgs(SGS sgs);
void UseStepguardAnimation(STEPGUARD* pstepguard, SGS sgs, OID oidAseg);
void UseStepguardAnimationImmediate(STEPGUARD* pstepguard, SGS sgs, OID oidAseg);
void UseStepguardDeathAnimation(STEPGUARD* pstepguard, GRFTAK grftak, OID oidAseg);
ASEG* PasegFindStepguard(STEPGUARD* pstepguard, OID oidAseg);
ASEG* GetStepguardSgsAseg(STEPGUARD* pstepguard, SGS sgs);
void LoadStepguardAnimations(STEPGUARD* pstepguard);
void UseStepguardExpl(STEPGUARD* pstepguard, OID oidExpl);
void UseStepguardRwm(STEPGUARD* pstepguard, OID oidRwm);
void UseStepguardPhys(STEPGUARD* pstepguard, SGS sgs, OID oidPhys);
void LoadStepguardPhys(STEPGUARD* pstepguard);
void AddStepguardEffect(STEPGUARD* pstepguard, OID oidEffect, ZPK zpk);
void SetStepguardPathzone(STEPGUARD* pstepguard, OID oidPathzone);
void*GetStepguardPathzone(STEPGUARD* pstepguard);
SO*  PsoEnemyStepguard(STEPGUARD* pstepguard);
SO*  GetStepguardEnemy(STEPGUARD* pstepguard);
void SetStepguardEnemyObject(STEPGUARD* pstepguard, SO* psoEnemy);
void RebindStepguardEnemy(STEPGUARD* pstepguard);
void SetStepguardEnemyBound(STEPGUARD* pstepguard, SO* pso);
void AdjustStepguardNewXp(STEPGUARD* pstepguard, XP* pxp, int ixpd);
void AdjustStepguardDz(STEPGUARD* pstepguard, GRFADJ grfadj, DZ* pdz, int ixpd, float dt);
void SetStepguardAttackAngleMax(STEPGUARD* pstepguard, float degAttackMax);
void*GetStepguardAttackAngleMax(STEPGUARD* pstepguard);
void AddStepguardAlarm(STEPGUARD* pstepguard, ALARM* palarm);
void EnsureSggAlarm(SGG* psgg, ALARM* palarm);
void MatchStepguardAnimationPhase(STEPGUARD* pstepguard, OID oid0, OID oid1, OID oid2, OID oid3);
void AddStepguardCustomXps(STEPGUARD* pstepguard, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxp);
void ApplyStepguardThrow(STEPGUARD* pstepguard, PO* ppo);
void UpdateStepguardEffect(STEPGUARD* pstepguard);
void SetStepguardPatrolAnimation(STEPGUARD* pstepguard, ASEG* pasegPatrol);
int  FInflictStepguardZap(STEPGUARD* pstepguard, XP* pxp, ZPR* pzpr);
int  FDetectStepguard(STEPGUARD* pstepguard);
int  FCheckStepguardEnemyHidden(STEPGUARD* pstepguard);
bool FStepguardLookLocked(STEPGUARD* pstepguard);
int  GetStepguardSize();
void DeleteStepguard(STEPGUARD* pstepguard);

SGG* NewSgg();
SGG* PsggNew(SW* psw);
void InitSgg(SGG* psgg);
int  GetSggSize();
void CloneSgg(SGG* psgg, SGG* psggBase);
void AddSggGuard(SGG* psgg, STEPGUARD* pstepguard);
void AddSggGuardName(SGG* psgg, OID oidGuard);
void AddSggSearchXfmName(SGG* psgg, OID oidXfm);
void RemoveSggGuard(SGG* psgg, STEPGUARD* pstepguard);
void BindSgg(SGG* psgg);
void PostSggLoadCallback(SGG* psgg, MSGID msgid, void*pv);
void EnsureSggCallback(SGG* psgg);
SO*  PsoEnemySgg(SGG* psgg);
void UpdateSggCallback(SGG* psgg, MSGID msgid, void*pv);
SGGS SggsNextSgg(SGG* psgg);
int  FDetectSgg(SGG* psgg);
int  FAbandonSggSearch(SGG* psgg);
void EnsureSggAlarm(SGG* psgg, ALARM* palarm);
void TriggerSggAlarms(SGG* psgg, ALTK altk);
void SetSggSggs(SGG* psgg, SGGS sggs);
void AssignSggSearchPoints(SGG* psgg);
void DeleteSgg(SGG* psgg);

extern SNIP s_asnipStepguardLoad[1];
extern SNIP s_asnipPostStepguardLoad[2];
extern float MU_StepguardDying;
extern float ELAS_StepguardDying;
extern float MU_StepguardBase;
extern CLQ s_clqDradToUFly;
extern CLQ s_clqStunSpring;
extern CLQ s_clqStunDamping;
extern CLQ s_clqUdtToU;
extern CLQ s_clqSdistToUdrad;
extern glm::vec3 kDefaultHitNormal;
extern LM g_lmDtGiveUpSearch;
extern LM g_lmDtLostPlayer;
extern LM g_lmDtDiscover;
extern float S_StepguardSearchSpacing;
extern LM g_lmUDradSearch;
extern LM g_lmUSSearch;
extern float R_StepguardElas;
extern float SV_StepguardThrowJtMin;
extern float SV_StepguardThrowJtMax;
extern CLQ s_clqSToUTilt;
extern LM  s_lmUTiltDz;
extern float RAD_StepguardTiltMax;
extern LM s_lmUTiltS;
extern CLQ s_clqDzToUTilt;
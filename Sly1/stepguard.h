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
struct SGA 
{
    enum OID oidAseg;
    struct ASEG* paseg;
};
struct SGMAP 
{
    OID aoid[4];
};
struct SGP 
{
    OID oidPhys;
    struct SO* psoPhys;
};
enum RPK {
    RPK_Nil = -1,
    RPK_Closest = 0,
    RPK_Previous = 1,
    RPK_Max = 2
};
struct SGE 
{
    OID oidEffect;
    struct SO* pso;
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

class STEPGUARD : public STEP
{
	public:
        struct SGG* psgg;
        SGS sgs;
        float tSgs;
        int grfsgsc;
        SGS sgsNext;
        struct ALO* paloHead;
        LM lmAttackS;
        LM lmAttackT;
        float tAttackNext;
        float radAttackMax;
        struct ASEG* pasegPatrol;
        struct ASEGA* pasegaPatrol;
        SGA mpsgssga[17];
        struct ASEGA* pasegaSgs;
        int csgmap;
        SGMAP asgmap[8];
        struct ASEG* pasegPrev;
        float uAsegaPrev;
        SGP mpsgssgp[17];
        struct SO* psoPhysDefault;
        glm::vec3 posSearch;
        float radTargetSearch;
        float tPatrolReturn;
        float tPatrolPrev;
        float svtPatrolPrev;
        RPK rpk;
        OID oidPathzone;
        struct PATHZONE* ppathzone;
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
        struct WAYPOINT* pwaypoint;
        int csge;
        SGE asge[4];
        struct XP* pxpEffect;
        glm::vec3 vThrow;
        int csgda;
        SGDA asgda[4];
        int fExplExplicit;
        OID oidExpl;
        struct EXPL* pexpl;
        int fRwmExplicit;
        OID oidRwm;
        struct RWM* prwm;
        int ichkDead;
        int fFirstFrame;
        float uIdleStart;
        int fFlying;
        float dzFlying;
        float radTilt;
        CLQ clqSToUPursue;
        int fEnemyExternal;
        int fRebindEnemy;
        struct SO* psoEnemyExternal;
        OID oidEnemyExternal;
        struct SO* psoSubscribe;
        int ccoin;
        struct STEPGUARD* pstepguardBase;
        int fAnimatedJump;
        int fLockHeading;
        int fLockHeadingExternal;
        int fLockState;
        int fExternalAI;
        int fFreefall;
        int fFreefallAseg;
        float dxFreefall;
        float dtFreefall;
        LM lmRDtFreefall;
        float tFreefallLand;
        glm::vec3 posFreefallLand;
        glm::vec3 vFreefallLand;
        float uFling;
        int fAutoFadeDying;
        float tGiveUpSearch;
        float tStopPatrol;
        float tTaunt;
        float tLostPlayer;
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
        OID aoidStepguard[16];
        int cpxfmSearch;
        struct XFM* apxfmSearch[16];
        int coidXfmSearch;
        OID aoidXfmSearch[16];
        int cpalarm;
        struct ALARM* apalarm[4];
        int fCallback;
        OID oidSync;
        struct EXC* pexcAttack;
        int fDetect;
};

void InitSgg(SGG* psgg);
void DeleteSgg(LO* plo);
void InitStepGuard(STEPGUARD* pstepguard);
void OnStepguardAdd(STEPGUARD* pstepguard);
void LoadStepGuardFromBrx(STEPGUARD *pstepguard, CBinaryInputStream *pbis);
void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro);
void DeleteStepguard(LO* plo);
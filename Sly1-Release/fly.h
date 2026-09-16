#pragma once
#include "so.h"

enum FLYK 
{
    FLYK_Nil = -1,
    FLYK_Insect = 0,
    FLYK_Pigeon = 1,
    FLYK_Max = 2
};
enum FLYS 
{
    FLYS_Nil = -1,
    FLYS_Wander = 0,
    FLYS_Dying = 1,
    FLYS_Dead = 2,
    FLYS_Attract = 3,
    FLYS_Stuck = 4,
    FLYS_Land = 5,
    FLYS_Ground = 6,
    FLYS_Max = 7
};

class FLY : public SO
{
	public:
    FLYK flyk;
    FLYS flys;
    float tFlys;
    float tFlysNext;
    FLYS flysInit;
    int cpaloRender;
    int ipaloRender;
    class ALO* apaloRender[16];
    int cpaloRenderGround;
    class ALO* apaloRenderGround[8];
    class ALO* paloRenderDead;
    struct EXPL* pexpl;
    struct TARGET* ptarget;
    DLE dleFly;
    struct AMB* pambBuzz;
    glm::vec3 posWanderNext;
    glm::vec3 vWanderNext;
    LM lmSCylinderRadius;
    LM lmSCylinderHeight;
    LM lmSvxyFlight;
    LM lmSvzFlight;
    short oidSoar;
    int ipaloRenderSoar;
    float uSoar;
    LM lmDtFlying;
    float dtFlyingFrame;
    float tFlyingFrameNext;
    float sGroundMax;
    LM lmDtGround;
    LM lmDtGroundFrame;
    LM lmDtGroundFramePause;
    float tGroundFrameNext;
    float tGroundTestLast;
    LM lmDtDroppings;
    struct EXPLS* pexplsDroppings;
    struct PNT* ppntDroppings;
    float tDroppingsNext;
    float sFlee;
    float tLandNext;
    int diRender;
    int ccoin;
    int fSuppressDifficultyUpdate;
};

FLY* NewFly();
void InitFly(FLY* pfly);
void SetFlyFlyk(FLY* pfly, FLYK flyk);
void*GetFlyFlyk(FLY* pfly);
void SetFlyFlysInit(FLY* pfly, FLYS flysInit);
void*GetFlyFlysInit(FLY* pfly);
void SetFlyFlys(FLY* pfly, FLYS flys);
void*GetFlyFlys(FLY* pfly);
void SetFlyLmSCylinderRadius(FLY* pfly, LM lmSCylinderRadius);
void*GetFlyLmSCylinderRadius(FLY* pfly);
void SetFlyLmSCylinderHeight(FLY* pfly, LM lmSCylinderHeight);
void*GetFlyLmSCylinderHeight(FLY* pfly);
void SetFlyLmSvxyFlight(FLY* pfly, LM lmSvxyFlight);
void*GetFlyLmSvxyFlight(FLY* pfly);
void SetFlyLmSvzFlight(FLY* pfly, LM lmSvzFlight);
void*GetFlyLmSvzFlight(FLY* pfly);

void*GetFlyOidSoar(FLY* pfly);
void SetFlyOidSoar(FLY* pfly, OID oidSoar);
void SetFlyUSoar(FLY* pfly, float uSoar);
void*GetFlyUSoar(FLY* pfly);
void SetFlyLmDtFlying(FLY* pfly, LM lmDtFlying);
void*GetFlyLmDtFlying(FLY* pfly);
void SetFlyDtFlyingFrame(FLY* pfly, float dtFlyingFrame);
void*GetFlyDtFlyingFrame(FLY* pfly);
void SetFlyLmDtGround(FLY* pfly, LM lmDtGround);
void*GetFlyLmDtGround(FLY* pfly);
void SetFlyLmDtGroundFrame(FLY* pfly, LM lmDtGroundFrame);
void*GetFlyLmDtGroundFrame(FLY* pfly);
void SetFlyLmDtGroundFramePause(FLY* pfly, LM lmDtGroundFramePause);
void*GetFlyLmDtGroundFramePause(FLY* pfly);
void SetFlyLmDtDroppings(FLY* pfly, LM lmDtDroppings);
void*GetFlyLmDtDroppings(FLY* pfly);
void SetFlySFlee(FLY* pfly, float sFlee);
void*GetFlySFlee(FLY* pfly);
void SetFlyCcoin(FLY* pfly, int ccoin);
void*GetFlyCcoin(FLY* pfly);
void SetFlyFSuppressDifficultyUpdate(FLY* pfly, int fSuppressDifficultyUpdate);
void*GetFlyFSuppressDifficultyUpdate(FLY* pfly);
int  GetFlySize();
void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis);
void CloneFly(FLY* pfly, FLY* pflyBase);
void FreezeFly(FLY* pfly, int fFreeze);
void PostFlyLoad(FLY* pfly);
void PresetFlyAccel(FLY* pfly, float dt);
void UpdateFly(FLY* pfly, float dt);
void RenderFlySelf(FLY* pfly, CM* pcm, RO* pro);
int  FAbsorbFlyWkr(FLY* pfly, WKR* pwkr);
void SetFlyFlys(FLY* pfly, FLYS flys);
void EmitFlyDroppings(FLY* pfly);
int  FFindFlyClosestLandingPos(FLY* pfly, glm::vec3* ppos);
FLYS FlysAttemptedFlyLanding(FLY* pfly);
int  FShouldFlyFlee(FLY* pfly);
int  FFilterFly(FLY* pfly, SO* pso);
int  FCheckFlyOpenSpaceBelow(FLY* pfly);
void DeleteFly(FLY *pfly);

extern CLQ s_clqArriveAlpha;
extern CLQ s_clqDyingAlpha;
extern LM s_lmSCylinderRadius;
extern LM s_lmSCylinderHeight;
extern LM s_lmSvxyFlight;
extern LM s_lmSvzFlight;
extern LM s_lmDtFlying;
extern LM s_lmDtGround;
extern LM s_lmDtGroundFrame;
extern LM s_lmDtGroundFramePause;
extern LM s_lmDtDroppings;
extern SNIP s_asnipFly[2];
extern SNIP s_asnipPostLoadFly[2];
extern CLQ s_clqFlyDampV;
extern CLQ s_clqFlyDampW;
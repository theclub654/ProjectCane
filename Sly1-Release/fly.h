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
        struct ALO* apaloRender[16];
        int cpaloRenderGround;
        struct ALO* apaloRenderGround[8];
        struct ALO* paloRenderDead;
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
        OID oidSoar;
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
};

FLY* NewFly();
void InitSwFlyDl(SW* psw);
void InitFly(FLY* pfly);
int  GetFlySize();
void CloneFly(FLY* pfly, FLY* pflyBase);
void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis);
void PostFlyLoad(FLY* pfly);
void UpdateFly(FLY* pfly, float dt);
void RenderFlySelf(FLY* pfly, CM* pcm, RO* pro);
void DeleteFly(FLY *pfly);
#pragma once

#include "emitshared.h"
#include "so.h"

struct EMITB;
struct EMITG;
struct RIP;
class CM;
struct LSG;

enum RIPGT 
{
    RIPGT_Nil = -1,
    RIPGT_Default = 0,
    RIPGT_Bounce = 1,
    RIPGT_Loot = 2,
    RIPGT_Max = 3
};

enum RIPT 
{
    RIPT_Nil = -1,
    RIPT_Rip = 0,
    RIPT_Shadow = 1,
    RIPT_Droplet = 2,
    RIPT_Bublet = 3,
    RIPT_Ripple = 4,
    RIPT_Puff = 5,
    RIPT_Dablet = 6,
    RIPT_Flake = 7,
    RIPT_Spark = 8,
    RIPT_Burst = 9,
    RIPT_Trail = 10,
    RIPT_Fireball = 11,
    RIPT_SmokeCloud = 12,
    RIPT_SmokeTrail = 13,
    RIPT_Debris = 14,
    RIPT_Orbit = 15,
    RIPT_Smack = 16,
    RIPT_Ray = 17,
    RIPT_Rose = 18,
    RIPT_Flying = 19,
    RIPT_Stuck = 20,
    RIPT_Leaf = 21,
    RIPT_Flame = 22,
    RIPT_Bullet = 23,
    RIPT_Shrapnel = 24,
    RIPT_Glint = 25,
    RIPT_Match = 26,
    RIPT_Emitter = 27,
    RIPT_Max = 28
};

#include "ripgdec.h"

struct RIP 
{
    union
    {
        struct VTRIP* pvtrip;
        struct VTDROPLET* pvtdroplet;
        struct VTBUBLET* pvtbublet;
        struct VTRIPPLE* pvtripple;
        struct VTPUFF* pvtpuff;
        struct VTDABLET* pvtdablet;
        struct VTFLAKE* pvtflake;
        struct VTSPARK* pvtspark;
        struct VTBURST* pvtburst;
        struct VTTRAIL* pvttrail;
        struct VTFIREBALL* pvtfireball;
        struct VTSMOKECLOUD* pvtsmokecloud;
        struct VTSMOKETRAIL* pvtsmoketrail;
        struct VTDEBRIS* pvtdebris;
        struct VTORBIT* pvtorbit;
        struct VTSMACK* pvtsmack;
        struct VTRAY* pvtray;
        struct VTROSE* pvtrose;
        struct VTFLYING* pvtflying;
        struct VTSTUCK* pvtstuck;
        struct VTLEAF* pvtleaf;
        struct VTFLAME* pvtflame;
        struct VTBULLET* pvtbullet;
        struct VTSHRAPNEL* pvtshrapnel;
        struct VTGLINT* pvtglint;
        struct VTMATCH* pvtmatch;
        struct VTREMIT* pvtremit;
    };
    RIPT ript;
    int cref;
    struct RIPG *pripg;
    DLE dle;
    float tCreated;
    float dtLifetime;
    class ALO* paloRender;
    class SO* psoTouch;
    CLQ clqScale;
    CLQ clqAlpha;
    glm::mat3 mat;
    glm::vec3 pos;
    glm::vec3 posPrev;
    glm::vec3 v;
    EMITDV emitdv;
    glm::vec3 w;
    float rwDamping;
    struct WR* pwr;
    // Original RIP slot layout stores a plain MQ pointer. Ownership lives in
    // SW so translated offset writes cannot corrupt a C++ shared_ptr here.
    MQ* pmqFirst;
};

class RIPG : public SO
{
	public:
    RIPGT ripgt;
    float sExpand;
    DL dlRip;
    class RIPG *pripgNext;
};

void InitRip(RIP* prip, glm::vec3* ppos, float scale, SO* psoTouch);

void ResetSwRipPool(SW* psw);
void DeleteSwRipPool(SW* psw);
RIPG*PripgNew(SW* psw, RIPGT ripgt);
RIPG*NewRipg();
RIP* PripNewRipg(RIPT ript, RIPG* pripg);
void InitRipg(RIPG *pripg);
int  GetRipgSize();
void CloneRipg(RIPG* pripg, RIPG* pripgBase);
void SetRipgEmitb(RIPG* pripg, EMITB* pemitb);
void OnRipgRemove(RIPG *pripg); 
void ProjectRipgTransform(RIPG* pripg, float dt, int fForce);
void UpdateRipg(RIPG* pripg, float dt);
void BounceRipgRips(RIPG* pripg);
void RenderRipgSelf(RIPG* pripg, CM* pcm, RO* pro);
void RemoveRip(RIP* prip);
void ReleaseRip(RIP* prip);
void TouchRip(RIP* prip, int fTouching);
void ForceRipFade(RIP* prip, float dtFade);
int  FBounceRip(RIP* prip, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal);
void ProjectRipTransform(RIP* prip, float dt);
void UpdateRipgBounds(RIPG* pripg);
void UpdateRip(RIP* prip, float dt);
int  FRenderRipPosMat(RIP* prip, CM* pcm, glm::vec3* ppos, glm::mat3* pmat);
void RenderRip(RIP* prip, CM* pcm);
void SubscribeRipObject(RIP* prip, LO* ploTarget);
void SubscribeRipStruct(RIP* prip, PFNMQ pfnmq, void* pvContext);
void UnsubscribeRipStruct(RIP* prip, PFNMQ pfnmq, void* pvContext);
void EmitRips(EMITB* pemitb, EMITG* pemitg, int crip, glm::vec3* apos, glm::vec3* av, float* atCreated, float* atDestroy);

void DeleteRipg(RIPG* pripg);

struct VTRIP
{
    void (*pfnInitRip)(RIP*, glm::vec3*, float, SO*) = InitRip;
    void (*pfnPostRipEmit)(RIP*, EMITB*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern float DT_RipFadeWater;
extern float R_RipSplashRadius;
extern float R_RipImpactWater;
extern float SV_RipBounceMin;
extern float DT_RipBounceFade;
extern CLQ g_clqAlphaRip;
extern VTRIP g_vtrip;

#pragma once
#include "xform.h"
#include "emitshared.h"
#include "rip.h"
#include "crv.h"

enum EMITRK 
{
    EMITRK_Nil = -1,
    EMITRK_Continuous = 0,
    EMITRK_ConstantCount = 1,
    EMITRK_Burst = 2,
    EMITRK_BurstOld = 3,
    EMITRK_Max = 4
};
enum EMITVK
{
    EMITVK_Nil = -1,
    EMITVK_Relative = 0,
    EMITVK_RelativeSpin = 1,
    EMITVK_NoRelative = 2,
    EMITVK_World = 3,
    EMITVK_Max = 4
};
enum EMITNK
{
    EMITNK_Nil = -1,
    EMITNK_Normal = 0,
    EMITNK_Radial = 1,
    EMITNK_CurveTangent = 2,
    EMITNK_CurveNormal = 3,
    EMITNK_Screen = 4,
    EMITNK_MeshNormal = 5,
    EMITNK_Max = 6
};
enum EMITOK 
{
    EMITOK_Nil = -1,
    EMITOK_Point = 0,
    EMITOK_Box = 1,
    EMITOK_Curve = 2,
    EMITOK_Skeleton = 3,
    EMITOK_Mesh = 4,
    EMITOK_Max = 5
};
enum EMITPK 
{
    EMITPK_Nil = -1,
    EMITPK_Blip = 0,
    EMITPK_Rip = 1,
    EMITPK_Max = 2
};
enum BLIPRK 
{
    BLIPRK_Nil = -1,
    BLIPRK_Mirror = 0,
    BLIPRK_NoMirror = 1,
    BLIPRK_Max = 2
};

struct BOX 
{
    glm::vec3 posMin;
    glm::vec3 posMax;
};
struct EMITCRV 
{
    std::shared_ptr <CRV> pcrv;
};

struct EMITTRI {
    int aipos[3];
    float sArea;
};

struct SKELP 
{
    short aoid[2];
    float agDensity[2];
    float asRadius[2];
    class ALO* apalo[2];
    float s;
    float gWeight;
};

struct SKEL
{
    int cskelp;
    std::vector <SKELP> askelp;
    float gTotalWeight;
};

struct EMITMESH 
{
    int cpos;
    std::vector <glm::vec3> apos;
    std::vector <glm::vec3> anormal;
    int cemittri;
    std::vector <EMITTRI> aemittri;
    float sTotalArea;
    glm::vec3 posCenter;
};
struct EMITX__bullet 
{
    int fDamage;
    int fRichochet;
};
struct EMITX__remit 
{
    OID oidExpls;
    float svcParticle;
};

struct EMITX__shrapnel 
{
    float sRadius;
    float elas;
    float mu;
};

struct EMITO 
{
    EMITOK emitok;
    glm::vec3 posOrigin;
    BOX boxOrigin;
    EMITCRV emitcrvOrigin;
    SKEL skelOrigin;
    EMITMESH emitmeshOrigin;
    EMITNK emitnk;
    glm::vec3 vec;
    LM lmSOffset;
    struct EMITOLXF *pemitolxf;
    struct ALO *paloReference;
    EMITVK emitvk;
    glm::vec3 vLocal;
};
struct EMITV 
{
    LM lmSv;
    float rSvz;
    glm::vec3 dv;
    float rvDamping;
    float swCurl;
    glm::vec3 normalCurl;
    EMITCNK emitcnk;
    LM lmTilt;
    int cParticlePerRing;
    float uRandomRad;
    float dtLifetime;
    LM lmDtSkip;
    LM lmDtBirth;
    CLQ clqAlpha;
};
enum BLIPMK
{
    BLIPMK_Nil = -1,
    BLIPMK_Accel = 0,
    BLIPMK_Constant = 1,
    BLIPMK_Spline = 2,
    BLIPMK_Max = 3
};
enum BLIPOK
{
    BLIPOK_Nil = -1,
    BLIPOK_Fixed = 0,
    BLIPOK_Flying = 1,
    BLIPOK_Rolling = 2,
    BLIPOK_Max = 3
};
struct EMITBLIP 
{
    BLIPMK blipmk;
    OID oidShader;
    CLQ clqScale;
    CLQ clqTexture;
    CLQ clqColor;
    int fShaderSpan;
    float dtShaderLoop;
    int fRandomFrame;
    int crgba;
    std::vector <glm::vec4> argba;
    int fColorRanges;
    short oidSplineTarget;
    struct EXPLO* pexploSplineTarget;
    BLIPOK blipok;
    float rSFlying;
    BLIPRK bliprk;
    LM lmSw;
    int fRandomRoll;
};
struct EMITRIP 
{
    RIPT ript;
    RIPT riptTrail;
    CLQ clqScale;
    LM lmGScale;
    class ALO* paloRender;
    class ALO* paloNextRender;
    class SO*  psoTouch;
    RIPGT ripgt;
    float sExpand;
};
struct EMITP 
{
    EMITPK emitpk;
    EMITBLIP emitblip;
    EMITRIP emitrip;
};
struct EMITB 
{
    int cref;
    EMITO emito;
    EMITV emitv;
    EMITP emitp;
    union EMITX 
    {
        struct EMITX__bullet bullet;
        struct EMITX__remit remit;
        struct EMITX__shrapnel shrapnel;
    }emitx;
    char* pchzName;
};

struct EMITG
{
    struct BLIPG** ppblipg;
    struct RIPG** ppripg;
    struct LO* ploSubscribe;
};

struct EMITGEN 
{
    std::vector <glm::vec3> apos;
    std::vector <glm::vec3> av;
    std::vector <float> atCreated; 
    std::vector <float> atDestroy;
    int fConvertPosVec;
};

struct EMITOLXF 
{
    glm::vec3 posLocal;
    glm::mat3 matLocal;
    glm::vec3 vLocal;
    glm::vec3 wLocal;
};

struct EMITVX 
{
    int cParticlePerRing;
    float dradTiltRing;
    float dradPanSlice;
    float radTiltMin;
    float radPanMin;
};

struct EXPLSO 
{
    GRFEXPLSO grfexplso;
    class ALO* paloReference;
    glm::vec3 vec;
    glm::vec3 posOrigin;
    float rScale;
    float sRadius;
    struct SO* psoTouch;
    struct EMITOLXF* pemitolxf;
    int cParticle;
};

class EMITTER : public ALO
{
    public:
    std::shared_ptr <EMITB> pemitb;
    EMITRK emitrk;
    int cParticle;
    LM lmSvcParticle;
    int fCountIsDensity;
    float uPauseProb;
    LM lmDtPause;
    float cParticleConstant;
    OID oidReference;
    OID oidRender;
    OID oidTouch;
    OID oidNextRender;
    int fAutoPause;
    OID oidShape;
    OID oidGroup;
    DL dlGroup;
    DLE dleGroup;
    float svcParticle;
    float dtRecalcSvc;
    float tRecalcSvc;
    float rDensity;
    float sBoxRadius;
    float uParticle;
    float tUnpause;
    struct RIPG* pripg;
    struct BLIPG* pblipg;
    int fValuesChanged;
};

EMITTER*NewEmitter();
void InitEmitter(EMITTER* pemitter);
int  GetEmitterSize();
void LoadEmitMeshFromBrx(EMITMESH *pemitmesh, CBinaryInputStream *pbis);
void LoadEmitblipColorsFromBrx(EMITBLIP* pemitblip, int crgba, CBinaryInputStream* pbis);
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);
void CloneEmitter(EMITTER* pemitter, EMITTER* pemitterBase);
void SetEmitterAutoPause(EMITTER* pemitter, int fAutoPause);
void*GetEmitterFAutoPause(EMITTER* pemitter);
EMITTER* PemitterEnsureEmitter(EMITTER* pemitter, ENSK ensk);
void AddEmitterSkeleton(EMITTER* pemitter, OID oid, OID oidOther, float sRadius, float gDensity, float sRadiusOther, float gDensityOther);
EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk);
EMITRK* PemitbEnsureEmitterEmitrk(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterlmSvcParticle(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmittercParticleConstant(EMITTER* pemitter, ENSK ensk);
float* PemitEnsureEmitteruPauseProb(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterlmDtPause(EMITTER* pemitter, ENSK ensk);
void GetEmitterEnabled(EMITTER* pemitter, int* pfEnabled);
void SetEmitterEnabled(EMITTER* pemitter, int fEnabled);
void SetEmitterFCountIsDensity(EMITTER* pemitter, int fCountIsDensity);
void*GetEmitterFCountIsDensity(EMITTER* pemitter);
void SetEmitterOidReference(EMITTER* pemitter, OID oidReference);
void*GetEmitterOidReference(EMITTER* pemitter);
void SetEmitterOidRender(EMITTER* pemitter, OID oidRender);
void*GetEmitterOidRender(EMITTER* pemitter);
void SetEmitterOidTouch(EMITTER* pemitter, OID oidTouch);
void*GetEmitterOidTouch(EMITTER* pemitter);
void SetEmitterOidNextRender(EMITTER* pemitter, OID oidNextRender);
void*GetEmitterOidNextRender(EMITTER* pemitter);
void SetEmitterOidGroup(EMITTER* pemitter, OID oidGroup);
void*GetEmitterOidGroup(EMITTER* pemitter);
void GetEmitterPaused(EMITTER* pemitter, int* pfPaused);
EMITOK* PemitbEnsureEmitterEmitok(EMITTER* pemitter, ENSK ensk);
glm::vec3* PemitbEnsureEmitterPosOrigin(EMITTER* pemitter, ENSK ensk);
glm::vec3* PemitbEnsureEmitterBoxOriginPosMin(EMITTER* pemitter, ENSK ensk);
glm::vec3* PemitbEnsureEmitterBoxOriginPosMax(EMITTER* pemitter, ENSK ensk);
void SetEmitterOidShape(EMITTER* pemitter, OID oidShape);
void* GetEmitterOidShape(EMITTER* pemitter);
EMITNK* PemitbEnsureEmitterEmitnk(EMITTER* pemitter, ENSK ensk);
glm::vec3* PemitbEnsureEmitterEmitoVec(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterlmSOffset(EMITTER* pemitter, ENSK ensk);
void* PemitbEnsureEmitterEmitvk(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterLmSv(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterRSvz(EMITTER* pemitter, ENSK ensk);
glm::vec3* PemitbEnsureEmitterDv(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterRvDamping(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterSwCurl(EMITTER* pemitter, ENSK ensk);
glm::vec3* PemitbEnsureEmitterNormalCurl(EMITTER* pemitter, ENSK ensk);
void* PemitbEnsureEmitterEmitcnk(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterLmTilt(EMITTER* pemitter, ENSK ensk);
int* PemitbEnsureEmitterCParticlePerRing(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterURandomRad(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterDtLifetime(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterLmDtSkip(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterLmDtBirth(EMITTER* pemitter, ENSK ensk);
CLQ* PemitbEnsureEmitterClqAlpha(EMITTER* pemitter, ENSK ensk);
void* PemitbEnsureEmitterEmitpk(EMITTER* pemitter, ENSK ensk);
void* PemitbEnsureEmitterRiptTrail(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterRipLmGScale(EMITTER* pemitter, ENSK ensk);
CLQ* PemitbEnsureEmitterRipClqScale(EMITTER* pemitter, ENSK ensk);
void* PemitbEnsureEmitterRipgt(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterSExpand(EMITTER* pemitter, ENSK ensk);
OID* PemitbEnsureEmitterOidShader(EMITTER* pemitter, ENSK ensk);
CLQ* PemitbEnsureEmitterBlipClqScale(EMITTER* pemitter, ENSK ensk);
CLQ* PemitbEnsureEmitterBlipClqTexture(EMITTER* pemitter, ENSK ensk);
CLQ* PemitbEnsureEmitterBlipClqColor(EMITTER* pemitter, ENSK ensk);
int* PemitbEnsureEmitterBlipFShaderSpan(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterBlipDtShaderLoop(EMITTER* pemitter, ENSK ensk);
int* PemitbEnsureEmitterBlipFRandomFrame(EMITTER* pemitter, ENSK ensk);
void* PemitbEnsureEmitterBlipmk(EMITTER* pemitter, ENSK ensk);
void* PemitbEnsureEmitterBlipok(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterBlipRSFlying(EMITTER* pemitter, ENSK ensk);
void* PemitbEnsureEmitterBliprk(EMITTER* pemitter, ENSK ensk);
LM* PemitbEnsureEmitterBlipLmSw(EMITTER* pemitter, ENSK ensk);
int* PemitbEnsureEmitterBlipFRandomRoll(EMITTER* pemitter, ENSK ensk);
int* PemitbEnsureEmitterBulletFDamage(EMITTER* pemitter, ENSK ensk);
int* PemitbEnsureEmitterBulletFRichochet(EMITTER* pemitter, ENSK ensk);
OID* PemitbEnsureEmitterRemitOidExpls(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterRemitSvcParticle(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterShrapnelSRadius(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterShrapnelElas(EMITTER* pemitter, ENSK ensk);
float* PemitbEnsureEmitterShrapnelMu(EMITTER* pemitter, ENSK ensk);

void PauseEmitter(EMITTER* pemitter, float dtPause);
void UnpauseEmitter(EMITTER* pemitter);
int  FPausedEmitter(EMITTER* emitter);
void SetEmitterParticleCount(EMITTER *pemitter, int cParticle);
void*GetEmitterParticleCount(EMITTER* pemitter);
void PauseEmitterIndefinite(EMITTER* pemitter);
void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro);
void BindEmitterCallback(EMITTER* pemitter, MSGID msgid, void* pvData);
void BindEmitter(EMITTER* pemitter);
void AddEmitoSkeleton(EMITO* pemito, OID oid, OID oidOther, float sRadius, float gDensity, float sRadiusOther, float gDensityOther, LO* ploContext);
void InitEmitb(EMITB* pemitb);
void SetEmitbRipt(EMITB* pemitb, RIPT ript);
void SetEmitterRipt(EMITTER* pemitter, RIPT ript);
void BindEmitb(EMITB* pemitb, LO* ploContext);
void SetEmitdvEmitb(EMITDV* pemitdv, EMITB* pemitb);
void CalculateEmitdvMatrix(EMITDV* pemitdv, float dt, glm::mat4* pmat4Dv);
void PostEmitterLoad(EMITTER* pemitter);
void HandleEmitterMessage(EMITTER* pemitter, MSGID msgid, void* pv);
void OnEmitterValuesChanged(EMITTER* pemitter);
void EmitParticles(int cParticle, EMITB* pemitb, EMITG* pemitg);
void EmitRipsSphere(glm::vec3* ppos, glm::vec3* pnormal, int crip, EMITRIP* pemitrip, EMITV* pemitv, RIPG** ppripg, LO* ploSubscribe);
void ModifyEmitterParticles(EMITTER* pemitter);
void OriginateParticles(int cParticle, EMITB* pemitb, EMITGEN* pemitgen);
void OriginateSplineSinkParticles(int cParticle, EMITB* pemitb, EMITGEN* emitgen, EMITGEN* emitgenTarget);
void CalculateEmitvx(int cParticlePerRing, LM* plmTilt, int cParticle, EMITVX* pemitvx);
void ChooseEmitoPos(EMITO* pemito, int iParticle, int cParticle, glm::vec3* pposRet, glm::vec3* pnormalRet);
void ChooseEmitvVelocityAge(EMITV* pemitv, EMITVX* pemitvx, EMITO* pemito, int iParticle, glm::vec3* ppos, glm::vec3* pnormal, glm::vec3* pv, float* ptCreated, float* ptDestroy);
void ChooseEmitVelocity(EMITVX* pemitvx, float uRandom, float rSvz, LM* plmSv, glm::vec3* pvecNormal, int iParticle, glm::vec3* pv);
void ConvertEmitoPosVec(EMITO* pemito, glm::vec3* ppos, glm::vec3* pv);
void UpdateEmitter(EMITTER* pemitter, float dt);
void InheritEmitterGrfzon(EMITTER* pemitter);
void DeleteEmitter(EMITTER *pemitter);

void StockSplashBig(glm::vec3* ppos, float gScale, SO* psoTouch);

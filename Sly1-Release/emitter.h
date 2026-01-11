#pragma once
#include "xform.h"
#include "rip.h"
#include "crv.h"

enum EMITCNK
{
	EMITCNK_Nil = -1,
	EMITCNK_World = 0,
	EMITCNK_Local = 1,
	EMITCNK_Max = 2
};
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
enum BLIPRK {
    BLIPRK_Nil = -1,
    BLIPRK_Mirror = 0,
    BLIPRK_NoMirror = 1,
    BLIPRK_Max = 2
};

struct EMITDV
{
    float rvDamping;
    glm::vec3 dv;
    float swCurl;
    glm::vec3 normalCurl;
    EMITCNK emitcnk;
    struct ALO* paloCurlRef;
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
struct SKEL 
{
    int cskelp;
    struct SKELP* askelp;
    float gTotalWeight;
};
struct EMITMESH 
{
    int cpos;
    std::vector <glm::vec3> apos;
    int cemittri;
    struct EMITTRI* aemittri;
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
    OID oidSplineTarget;
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
    struct ALO* paloRender;
    struct ALO* paloNextRender;
    struct SO* psoTouch;
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
class EXPL : public XFM
{
	public:
		class EXPLG *pexplgParent;
};
class EXPLG : public EXPL
{
	public:
		int cpexpl;
		EXPL *apexpl;
};
class EXPLO : public EXPL
{
	public:
        std::shared_ptr <EMITB> pemitb;;
		OID oidReference;
		OID oidShape;
};
class EXPLS : public EXPLO
{
	public:
        struct SFX* psfx;
        struct LM lmcParticle;
        OID oidRender;
        OID oidNextRender;
        OID oidTouch;
        float dtDelay;
        int fGrouped;
        struct RIPG* pripg;
        struct BLIPG* pblipg;
        float tExplodeNext;
        int fExplodeSiblings;
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

EXPLO*NewExplo();
void InitExplo(EXPLO* pexplo);
int  GetExploSize();
void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis);
void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase);
EMITOK* PemitbEnsureExploEmitok(EXPLO* pexplo, ENSK ensk);
void BindExplo(EXPLO* pexplo);
void DeleteExplo(EXPLO* pexplo);

EMITTER*NewEmitter();
void InitEmitter(EMITTER* pemitter);
int  GetEmitterSize();
void LoadEmitMeshFromBrx(EMITMESH *pemitmesh, CBinaryInputStream *pbis);
void LoadEmitblipColorsFromBrx(EMITBLIP* pemitblip, int crgba, CBinaryInputStream* pbis);
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);
void CloneEmitter(EMITTER* pemitter, EMITTER* pemitterBase);
void UnpauseEmitter(EMITTER* pemitter);
int FPausedEmitter(EMITTER* emitter);
EMITTER* PemitterEnsureEmitter(EMITTER* pemitter, ENSK ensk);
EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk);
EMITOK* PemitbEnsureEmitterEmitok(EMITTER* pemitter, ENSK ensk);
glm::vec3* PemitbEnsureEmitterEmitokVec(EMITTER* pemitter, ENSK ensk);
EMITRK* PemitbEnsureEmitterEmitrk(EMITTER* pemitter);
LM* PemitbEnsureEmitterlmSvcParticle(EMITTER* pemitter);
float* PemitbEnsureEmittercParticleConstant(EMITTER* pemitter);
float* PemitEnsureEmitteruPauseProb(EMITTER* pemitter);
LM* PemitbEnsureEmitterlmDtPause(EMITTER* pemitter);
void GetEmitterEnabled(EMITTER* pemitter, int* pfEnabled);
void SetEmitterEnabled(EMITTER* pemitter, int fEnabled);
int* GetEmitterfCountIsDensity(EMITTER* pemitter);
void SetEmitterfCountIsDensity(EMITTER* pemitter, bool fCountDensity);
void SetEmitterOidReference(EMITTER* pemitter, OID oidReference);
OID* GetEmitterOidReference(EMITTER* pemitter);
void* GetEmitterOidRender(EMITTER* pemitter);
void  SetEmitterOidRender(EMITTER* pemitter, OID oidRender);
void* GetEmitterOidTouch(EMITTER* pemitter);
void  SetEmitterOidTouch(EMITTER* pemitter, OID oidTouch);
void* GetEmitterOidNextRender(EMITTER* pemitter); 
void  SetEmitterOidNextRender(EMITTER* pemitter, OID oidNextRender);
void* GetEmitterOidGroup(EMITTER* pemitter);
void  SetEmitterOidGroup(EMITTER* pemitter, OID oidGroup);
void PauseEmitter(EMITTER* pemitter, float dtPause);
void GetEmitterPaused(EMITTER* pemitter, int* pfPaused);
void* GetEmitterOidShape(EMITTER* pemitter);
void  SetEmitterOidShape(EMITTER* pemitter, OID oidShape);
EMITNK* PemitbEnsureEmitterEmitnk(EMITTER* pemitter);
glm::vec3* PemitbEnsureEmitterEmitoVec(EMITTER* pemitter);
LM* PemitbEnsureEmitterlmSOffset(EMITTER* pemitter);


void SetEmitterParticleCount(EMITTER *pemitter, int cParticle);
void SetEmitterAutoPause(EMITTER* pemitter, int fAutoPause);
void PauseEmitterIndefinite(EMITTER* pemitter);
void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro);
void BindEmitter(EMITTER* pemitter);
void PostEmitterLoad(EMITTER* pemitter);
void UpdateEmitter(EMITTER* pemitter, float dt);
void DeleteEmitter(EMITTER *pemitter);

EXPL*NewExpl();
int  GetExplSize();
void CloneExpl(EXPL* pexpl, EXPL* pexplBase);
void PostExplLoad(EXPL* pexpl);
void DeleteExpl(EXPL* pexpl);

EXPLS*NewExpls();
void InitExpls(EXPLS* pexpls);
int  GetExplsSize();
void CloneExpls(EXPLS* pexpls, EXPLS* pexplsBase);
void BindExpls(EXPLS* pexpls);
void DeleteExpls(EXPLS* pexpls);

EXPLG*NewExplg();
int  GetExplgSize();
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);
void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase);
void BindExplg(EXPLG* pexplg);
void DeleteExplg(EXPLG* pexplg);
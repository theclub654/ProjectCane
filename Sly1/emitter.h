#pragma once
#include "xform.h"

enum EMITCNK
{
	EMITCNK_Nil = -1,
	EMITCNK_World = 0,
	EMITCNK_Local = 1,
	EMITCNK_Max = 2
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
enum EMITRK 
{
    EMITRK_Nil = -1,
    EMITRK_Continuous = 0,
    EMITRK_ConstantCount = 1,
    EMITRK_Burst = 2,
    EMITRK_BurstOld = 3,
    EMITRK_Max = 4
};

class EMITTER : public ALO
{
	public:
        struct EMITB* pemitb;
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
		struct EMITB* pemitb;
		OID oidreference;
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

void*NewExplo();
void InitExplo(EXPLO* pexplo);
int  GetExploSize();
void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis);
void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase);
void DeleteExplo(LO* plo);

void*NewEmitter();
void InitEmitter(EMITTER* pemitter);
int  GetEmitterSize();
void LoadEmitMeshFromBrx(CBinaryInputStream* pbis);
void LoadEmitblipColorsFromBrx(int crgba, CBinaryInputStream* pbis);
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);
void CloneEmitter(EMITTER* pemitter, EMITTER* pemitterBase);
void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro);
void DeleteEmitter(LO* plo);

void*NewExpl();
int  GetExplSize();
void CloneExpl(EXPL* pexpl, EXPL* pexplBase);
void DeleteExpl(LO* plo);

void*NewExpls();
void InitExpls(EXPLS* pexpls);
int  GetExplsSize();
void CloneExpls(EXPLS* pexpls, EXPLS* pexplsBase);
void DeleteExpls(LO* plo);

void*NewExplg();
int  GetExplgSize();
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);
void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase);
void DeleteExplg(LO* plo);
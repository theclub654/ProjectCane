#pragma once
#include "suv.h"
#include "break.h"
#include "acgb.h"

enum LGNS 
{
    LGNS_Nil = -1,
    LGNS_Passive = 0,
    LGNS_Active = 1,
    LGNS_Zap = 2,
    LGNS_Dead = 3,
    LGNS_Max = 4
};

struct LGNR : public BLOT
{
    glm::vec3 vScreen;
    glm::vec3 posScreen;
    struct TARGET* ptargetCur;
    struct SHD* pshd;
    float uAlpha;
};

class LGN : public SUV
{
	public:
    LGNS lgns;
    float tLgns;
    struct SM* psm;
    struct SMA* psma;
    class ALO* paloTurret;
    class ALO* paloPlatform;
    class ALO* paloJt;
    class ALO* paloGut;
    struct ACTADJ* pactadjGut;
    glm::vec3 posGutTurret;
    int fFlash;
    float tFlash;
    float tCharm;
    float tCharmPending;
    int fCharmEnabled;
    struct XP* pxpZap;
    class ALO *palo;
};

class LGNB : public SO
{
	public:
    struct SM* psm;
    struct SMA* psma;
    int fReset;
    float radPan;
    float radTilt;
    float tWkr;
    float tAttack;
    float dtAttackMax;
    glm::vec3 posPursueLocal;
    struct PATHZONE* ppathzone;
};

class SWP : public BREAK
{
	public:
    struct SHAPE* pshape;
    struct SM* psm;
    struct SMA* psma;
    SMP smpImpact;
    float sdvMax;
    float tMax;
    float svtLocal;
    float tLocal;
    std::shared_ptr <ACGB> pacgb;
    struct EXPL* pexplCrash;
};

LGN* NewLgn();
void InitLgn(LGN* plgn);
int  GetLgnSize();
void CloneLgn(LGN* plgn, LGN* plgnBase);
void PostLgnLoad(LGN* plgn);
void UpdateLgnActive(LGN* plgn, JOY* pjoy, float dt);
void UpdateLgn(LGN* plgn, float dt);
void UpdateLgnXfWorld(LGN* plgn);
void OnLgnActive(LGN* plgn, int fActive);
void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro);
void GetLgnCpdefi(LGN* plgn, float dt, CPDEFI* pcpdefi);
int  FInvulnerableLgn(LGN* plgn);
bool JthsCurrentLgn(LGN* plgn);
void UseLgnCharm(LGN* plgn);
void ApplyLgnThrow(LGN* plgn, STEP* pstep);
int  FTakeLgnDamage(LGN* plgn, ZPR* pzpr);
void HandleLgnMessage(LGN* plgn, MSGID msgid, void* pv);
void AdjustLgnXps(LGN* plgn);
void*GetLgnLgns(LGN* plgn);
void SetLgnLgns(LGN* plgn, LGNS lgns);
void DeleteLgn(LGN* plgn);

void StartupLgnr(LGNR* plgnr);
//GOTTA COME BACK TO THIS
void DrawLgnr(LGNR* plgnr);
void UpdateLgnrAim(LGNR* plgnr, JOY* pjoy);

LGNB*NewLgnb();
void InitLgnb(LGNB *plgnb);
int  GetLgnbSize();
void CloneLgnb(LGNB *plgnb, LGNB* plgnbBase);
void PostLgnbLoad(LGNB* plgnb);
void UpdateLgnb(LGNB* plgnb, float dt);
void DeleteLgnb(LGNB *plgnb);

SWP* NewSwp();
void InitSwp(SWP* pswp);
void SetSwpSmpImpactSvFast(SWP* pswp, float svFast);
void*GetSwpSmpImpactSvFast(SWP* pswp);
void SetSwpSmpImpactSvSlow(SWP* pswp, float svSlow);
void*GetSwpSmpImpactSvSlow(SWP* pswp);
void SetSwpSmpImpactDtFast(SWP* pswp, float dtFast);
void*GetSwpSmpImpactDtFast(SWP* pswp);
void SetSwpSdvMax(SWP* pswp, float sdvMax);
void*GetSwpSdvMax(SWP* pswp);
int  GetSwpSize();
void CloneSwp(SWP* pswp, SWP* pswpBase);
void PostSwpLoad(SWP* pswp);
void UpdateSwp(SWP* pswp, float dt);
void ProjectSwpTransform(SWP* pswp, float dt, int fParentDirty);
void SetSwpShape(SWP* pswp, SHAPE* pshape, float tMax);
void DeleteSwp(SWP* pswp);

extern LGNR g_lgnr;
extern SNIP s_asnipLgn[10];
extern float DT_LgnCharmBeforeEnabled;
extern float R_GutAdjust;
extern SMP s_smpLgnPlatformAim;
extern float R_LgnGutPosSpring;
extern float R_LgnGutRotSpring;
extern float DZ_LgnThrowJtApex;
extern float T_LgnCharmInitial;
extern SMP s_smpMat;
extern SNIP s_asnipSwp[2];
extern float S_SwpDetonate;
extern float DT_SwpSwoop;
extern float SVU_Joy;
extern SMP s_smp;

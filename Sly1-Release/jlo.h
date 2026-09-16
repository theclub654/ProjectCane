#pragma once
#include "button.h"

enum JLOS 
{
    JLOS_Nil = -1,
    JLOS_Jump = 0,
    JLOS_Land = 1,
    JLOS_Idle = 2,
    JLOS_Fire = 3,
    JLOS_Taunt = 4,
    JLOS_Max = 5
};
enum JLOCK 
{
    JLOCK_Nil = -1,
    JLOCK_Closest = 0,
    JLOCK_Random = 1,
    JLOCK_Max = 2
};
enum JLOMK 
{
    JLOMK_Nil = -1,
    JLOMK_Straight = 0,
    JLOMK_Lobbed = 1,
    JLOMK_Spline = 2,
    JLOMK_Max = 3
};

class JLO : public SO
{
	public:
    struct SM* psm;
    struct SMA* psma;
    struct JLOVOL* pjlovolCur;
    DL dlJlovol;
    struct PNT* ppntFeet;
    float dzFeet;
    struct RWM* prwm;
    float tFireNext;
    short oidJlovolStart;
    class ALO* paloReticle;
    class ALO* paloTracer;
    struct PNT* ppntTracerStart;
    struct ASEG* pasegReticlePop;
    struct ASEG* pasegReticleConverge;
    struct XFM* pxfmTarget;
    struct XFM* pxfmStatic;
    JLOS jlos;
    float tJlos;
    float radTarget;
    float dtLand;
    float tLand;
    float tStick;
    SMP smpSpin;
    struct EXC* pexc;
    float sStart;
    float sFull;
    int fDisableAutoSpin;
    class JLOC *pjlocFire;
    int fAmbientEnabled;
    int fStartAmbient;
    int cShotsUntilTargetJt;
};

class JLOC : public ALO
{
	public:
    struct XFM* apxfm[16];
    int cpxfm;
    JLOCK jlock;
    JLOMK jlomk;
    float dtFire;
    float dtMissile;
    float rAccelTime;
    float rMissileSpeed;
    float radTilt;
    float svLaunch;
    float svTarget;
};

class JLOVOL : public VOLBTN
{
	public:
    short oidLand;
    struct PNT* ppntLand;
    short oidJloc;
    struct JLOC* pjloc;
    DLE dleJlo;
};

JLOVOL*NewJlovol();
void InitJlovol(JLOVOL* pjlovol);
void*GetJlovolOidLand(JLOVOL* pjlovol);
void SetJlovolOidLand(JLOVOL* pjlovol, int oidLand);
void*GetJlovolPpntLand(JLOVOL* pjlovol);
void SetJlovolPpntLand(JLOVOL* pjlovol, PNT* ppntLand);
void*GetJlovolOidJloc(JLOVOL* pjlovol);
void SetJlovolOidJloc(JLOVOL* pjlovol, int oidJloc);
void*GetJlovolPjloc(JLOVOL* pjlovol);
void SetJlovolPjloc(JLOVOL* pjlovol, JLOC* pjloc);
int  GetJlovolSize();
void CloneJlovol(JLOVOL* pjlovol, JLOVOL* pjlovolBase);
void BindJlovol(JLOVOL* pjlovol);
void HandleJlovolMessage(JLOVOL* pjlovol, MSGID msgid, void* pv);
void DeleteJlovol(JLOVOL* pjlovol);

JLO* NewJlo();
void InitJlo(JLO* pjlo);
void SetJloOidJlovolStart(JLO* pjlo, OID oidJlovolStart);
void*GetJloOidJlovolStart(JLO* pjlo);
void SetJloFAmbientEnabled(JLO* pjlo, int fAmbientEnabled);
void*GetJloFAmbientEnabled(JLO* pjlo);
void SetJloSStart(JLO* pjlo, float sStart);
void*GetJloSStart(JLO* pjlo);
void SetJloSFull(JLO* pjlo, float sFull);
void*GetJloSFull(JLO* pjlo);
void SetJloFDisableAutoSpin(JLO* pjlo, int fDisableAutoSpin);
void*GetJloFDisableAutoSpin(JLO* pjlo);
int  GetJloSize();
void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);
void CloneJlo(JLO* pjlo, JLO* pjloBase);
void PostJloLoad(JLO* pjlo);
void SetJloStartJlovol(JLO* pjlo, OID oidJlovol);
void PresetJloAccel(JLO* pjlo, float dt);
void UpdateJlo(JLO* pjlo, float dt);
JLOS JlosNextJlo(JLO* pjlo);
void SetJloJlovol(JLO* pjlo, JLOVOL* pjlovol);
void FireJlo(JLO* pjlo);
void LandJlo(JLO* pjlo);
void JumpJlo(JLO* pjlo);
//GOTTA COME BACK TO THIS
void StartJloAmbient(JLO* pjlo);
void UpdateJloSound(JLO* pjlo);
void HandleJloMessage(JLO* pjlo, MSGID msgid, void* pv);
void SetJloJlos(JLO* pjlo, JLOS jlos);
void ActivateJlo(JLO* pjlo);
void DeactivateJlo();
void DeleteJlo(JLO *pjlo);

JLOC*NewJloc();
void InitJloc(JLOC* pjloc);
void SetJlocJlock(JLOC* pjloc, JLOCK jlock);
void*GetJlocJlock(JLOC* pjloc);
void SetJlocDtFire(JLOC* pjloc, float dtFire);
void*GetJlocDtFire(JLOC* pjloc);
void SetJlocDtMissile(JLOC* pjloc, float dtMissile);
void*GetJlocDtMissile(JLOC* pjloc);
void SetJlocJlomk(JLOC* pjloc, JLOMK jlomk);
void*GetJlocJlomk(JLOC* pjloc);
void SetJlocRAccelTime(JLOC* pjloc, float rAccelTime);
void*GetJlocRAccelTime(JLOC* pjloc);
void SetJlocRMissileSpeed(JLOC* pjloc, float rMissileSpeed);
void*GetJlocRMissileSpeed(JLOC* pjloc);
void SetJlocRadTilt(JLOC* pjloc, float radTilt);
void*GetJlocRadTilt(JLOC* pjloc);
void SetJlocSvLaunch(JLOC* pjloc, float svLaunch);
void*GetJlocSvLaunch(JLOC* pjloc);
void SetJlocSvTarget(JLOC* pjloc, float svTarget);
void*GetJlocSvTarget(JLOC* pjloc);
int  GetJlocSize();
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);
void CloneJloc(JLOC* pjloc, JLOC* pjlocBase);
void PostJlocLoad(JLOC* pjloc);
XFM* PxfmChooseJloc(JLOC* pjloc);
void DeleteJloc(JLOC* pjloc);

extern JLO *g_pjloCur;
extern int s_avagdescJlo[4];
extern SNIP s_asnipLoadJlo[5];
extern SNIP s_asnipPostJloLoad[2];
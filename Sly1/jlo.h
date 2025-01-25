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
        OID oidJlovolStart;
        struct ALO* paloReticle;
        struct ALO* paloTracer;
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
        OID oidLand;
        struct PNT* ppntLand;
        OID oidJloc;
        struct JLOC* pjloc;
        DLE dleJlo;
};

JLOVOL*NewJlovol();
void InitJlovol(JLOVOL* pjlovol);
int  GetJlovolSize();
void CloneJlovol(JLOVOL* pjlovol, JLOVOL* pjlovolBase);
void DeleteJlovol(JLOVOL* pjlovol);

JLO* NewJlo();
void InitJlo(JLO* pjlo);
int  GetJloSize();
void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);
void CloneJlo(JLO* pjlo, JLO* pjloBase);
void DeleteJlo(JLO *pjlo);

JLOC*NewJloc();
void InitJloc(JLOC* pjloc);
int  GetJlocSize();
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);
void CloneJloc(JLOC* pjloc, JLOC* pjlocBase);
void DeleteJloc(LO* plo);
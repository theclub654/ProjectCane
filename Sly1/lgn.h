#pragma once
#include "po.h"
#include "break.h"

enum LGNS 
{
    LGNS_Nil = -1,
    LGNS_Passive = 0,
    LGNS_Active = 1,
    LGNS_Zap = 2,
    LGNS_Dead = 3,
    LGNS_Max = 4
};

class LGN : public PO
{
	public:
        LGNS lgns;
        float tLgns;
        struct SM* psm;
        struct SMA* psma;
        struct ALO* paloTurret;
        struct ALO* paloPlatform;
        struct ALO* paloJt;
        struct ALO* paloGut;
        struct ACTADJ* pactadjGut;
        glm::vec3 posGutTurret;
        int fFlash;
        float tFlash;
        float tCharm;
        float tCharmPending;
        int fCharmEnabled;
        struct XP* pxpZap;
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
        struct ACGB* pacgb;
        struct EXPL* pexplCrash;
};

void*NewLgn();
void InitLgn(LGN* plgn);
int  GetLgnSize();
void CloneLgn(LGN* plgn, LGN* plgnBase);
void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro);
void DeleteLgn(LO* plo);

void*NewLgnb();
void InitLgnb(LGNB* plgnb);
int  GetLgnbSize();
void CloneLgnb(LGNB* plgnb, LGNB* plgnbBase);
void DeleteLgnb(LO* plo);

void*NewSwp();
void InitSwp(SWP* pswp);
int  GetSwpSize();
void CloneSwp(SWP* pswp, SWP* pswpBase);
void DeleteSwp(LO* plo);
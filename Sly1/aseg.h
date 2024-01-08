#pragma once
#include "lo.h"

enum SEGRPT 
{
    SEGRPT_Nil = -1,
    SEGRPT_Once = 0,
    SEGRPT_Loop = 1,
    SEGRPT_PingPong = 2,
    SEGRPT_Manual = 3,
    SEGRPT_Max = 4
};

class ASEG : public LO
{
	public:
        float tMax;
        int cchn;
        struct CHN* achn;
        OID oidRoot;
        SEGRPT segrpt;
        int fDefault;
        int fHandsOff;
        int fRealClock;
        int ceaApply;
        struct EA* aeaApply;
        int ceaFrame;
        struct EA* aeaFrame;
        int ceaRetract;
        struct EA* aeaRetract;
        DL dlAsega;
        int nPriority;
        float svtMaster;
        CLQ clqMasterSuck;
        LM lmMasterSuck;
        float svtMasterSuck;
        struct CHN* pchnStrip;
        int coidSearchRoot;
        enum OID* aoidSearchRoot;
};

class ASEGBL : public ASEG
{
    public:
        int cbBl;
        int cbl;
        struct BL* abl;
        int cmrsgc;
        struct MRSGC* amrsgc;
};

static int LoadAsegaCount;

void*NewAseg();
void InitAseg(ASEG* paseg);
int  GetAsegSize();
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);
void LoadAsegEventsFromBrx(CBinaryInputStream* pbis, int fFrame);
void CloneAseg(ASEG* paseg, ASEG* pasegBase);
void ApplyAseg(ASEG* paseg, ALO* paloAsegRoot, float tLocal, float svtLocal, int grfapl, ASEGA** ppasega);
void DeleteAseg(LO* plo);

void* NewAsegbl();
void DeleteAsegbl(LO* plo);
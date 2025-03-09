#pragma once
#include "lo.h"

class ASEGA : public BASIC
{
	public:
        struct ASEG* paseg;
        struct ALO* paloRoot;
        float dtPause;
        float tLocal;
        float svtLocal;
        float svtMaster;
        int ieaCur;
        char fHandsOff;
        struct ACTSEG* pactsegError;
        struct EA* peaPosError;
        struct EA* peaRotError;
        DLE dleSw;
        DLE dleAseg;
        DL dlActseg;
        struct AMB* pambContinuous;
        MRG mrg;
        struct ALO* apaloMrg[32];
        struct MQ* pmqFirst;
        int nPriority;
};

void InitSwAsegaDl(SW* psw);
void InitSwAsegaRealClockDl(SW* psw);
void InitSwAsegaPending(SW* psw);
int  GetAsegaSize();
void DeleteAsega(LO* plo);
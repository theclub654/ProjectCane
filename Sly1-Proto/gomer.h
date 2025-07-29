#pragma once
#include "stepguard.h"

class GOMER:public STEPGUARD
{
	public:
        float sAbandon;
        float dzAbandon;
        OID oidAbandon;
        struct VOL* pvolAbandon;
        LM lmSDetect;
        LM lmRadDetect;
        OID oidDetect;
        struct VOL* pvolDetect;
        int fDetectLatch;
        int fDetectLatchExternal;
        int fAbandonExternal;
};

GOMER*NewGomer();
void InitGomer(GOMER* pgomer);
int  GetGomerSize();
void CloneGomer(GOMER* pgomer, GOMER* pgomerBase);
void PostGomerLoad(GOMER* pgomer);
void RenderGomerSelf(GOMER* pgomer, CM* pcm, RO* pro);
void DeleteGomer(GOMER* pgomer);
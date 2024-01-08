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

void*NewGomer();
void InitGomer(GOMER* pgomer);
int  GetGomerSize();
void CloneGomer(GOMER* pgomer, GOMER* pgomerBase);
void DeleteGomer(LO* plo);
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

void* NewGomer();
void InitGomer(GOMER* pgomer);
void DeleteGomer(LO* plo);
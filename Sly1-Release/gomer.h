#pragma once
#include "stepguard.h"

class GOMER :public STEPGUARD
{
	public:
    float sAbandon;
    float dzAbandon;
    short oidAbandon;
    struct VOL* pvolAbandon;
    LM lmSDetect;
    LM lmRadDetect;
    short oidDetect;
    struct VOL* pvolDetect;
    int fDetectLatch;
    int fDetectLatchExternal;
    int fAbandonExternal;
};

GOMER*NewGomer();
void InitGomer(GOMER* pgomer);
void*GetGomerSAbandon(GOMER* pgomer);
void SetGomerSAbandon(GOMER* pgomer, float sAbandon);
void*GetGomerDzAbandon(GOMER* pgomer);
void SetGomerDzAbandon(GOMER* pgomer, float dzAbandon);
void*GetGomerOidAbandon(GOMER* pgomer);
void SetGomerOidAbandon(GOMER* pgomer, OID oidAbandon);
void*GetGomerLmSDetect(GOMER* pgomer);
void SetGomerLmSDetect(GOMER* pgomer, LM lmSDetect);
void*GetGomerLmRadDetect(GOMER* pgomer);
void SetGomerLmRadDetect(GOMER* pgomer, LM lmRadDetect);
void*GetGomerOidDetect(GOMER* pgomer);
void SetGomerOidDetect(GOMER* pgomer, OID oidDetect);
void*GetGomerPvolAbandon(GOMER* pgomer);
void SetGomerPvolAbandon(GOMER* pgomer, VOL* pvolAbandon);
int  GetGomerSize();
void CloneGomer(GOMER* pgomer, GOMER* pgomerBase);
void PostGomerLoad(GOMER* pgomer);
void RenderGomerSelf(GOMER* pgomer, CM* pcm, RO* pro);
int  FDetectGomer(GOMER* pgomer);
void DeleteGomer(GOMER* pgomer);
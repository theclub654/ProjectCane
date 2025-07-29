#include "gomer.h"

GOMER* NewGomer()
{
	return new GOMER{};
}

void InitGomer(GOMER* pgomer)
{
	InitStepGuard(pgomer);
}

int GetGomerSize()
{
	return sizeof(GOMER);
}

void CloneGomer(GOMER* pgomer, GOMER* pgomerBase)
{
    CloneStepguard(pgomer, pgomerBase);

    pgomer->sAbandon = pgomerBase->sAbandon;
    pgomer->dzAbandon = pgomerBase->dzAbandon;
    pgomer->oidAbandon = pgomerBase->oidAbandon;
    pgomer->pvolAbandon = pgomerBase->pvolAbandon;
    pgomer->lmSDetect = pgomerBase->lmSDetect;
    pgomer->lmRadDetect = pgomerBase->lmRadDetect;
    pgomer->oidDetect = pgomerBase->oidDetect;
    pgomer->pvolDetect = pgomerBase->pvolDetect;
    pgomer->fDetectLatch = pgomerBase->fDetectLatch;
    pgomer->fDetectLatchExternal = pgomerBase->fDetectLatchExternal;
    pgomer->fAbandonExternal = pgomerBase->fAbandonExternal;
}

void PostGomerLoad(GOMER* pgomer)
{
    PostStepguardLoad(pgomer);
}

void RenderGomerSelf(GOMER* pgomer, CM* pcm, RO* pro)
{
	RenderStepSelf(pgomer, pcm, pro);
}

void DeleteGomer(GOMER* pgomer)
{
	delete pgomer;
}

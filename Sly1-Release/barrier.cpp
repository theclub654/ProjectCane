#include "barrier.h"

BARRIER* NewBarrier()
{
	return new BARRIER{};
}

void InitBarrier(BARRIER* pbarrier)
{
	InitSo(pbarrier);
}

int GetBarrierSize()
{
	return sizeof(BARRIER);
}

void CloneBarrier(BARRIER* pbarrier, BARRIER* pbarrierBase)
{
    CloneSo(pbarrier, pbarrierBase);

    pbarrier->fAllow = pbarrierBase->fAllow;
    pbarrier->coid = pbarrierBase->coid;
    for (int i = 0; i < 4; ++i)
        pbarrier->aoid[i] = pbarrierBase->aoid[i];

    pbarrier->ccid = pbarrierBase->ccid;
    for (int i = 0; i < 4; ++i)
        pbarrier->acid[i] = pbarrierBase->acid[i];

    pbarrier->barwarp = pbarrierBase->barwarp;
    pbarrier->psoWarp = pbarrierBase->psoWarp;
    pbarrier->psoLastWarped = pbarrierBase->psoLastWarped;
}

void PostBarrierLoad(BARRIER* pbarrier)
{
    PostAloLoad(pbarrier);
}

void UpdateBarrier(BARRIER* pbarrier, float dt)
{
    UpdateSo(pbarrier, dt);
}

void DeleteBarrier(BARRIER* pbarrier)
{
	delete pbarrier;
}

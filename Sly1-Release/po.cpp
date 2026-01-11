#include "po.h"

PO* NewPo()
{
	return new PO{};
}

void InitPo(PO* ppo)
{
	InitSo(ppo);
}

void OnPoAdd(PO* ppo)
{
	OnSoAdd(ppo);
}

void OnPoRemove(PO* ppo)
{
	OnSoRemove(ppo);
}

void ClonePo(PO* ppo, PO* ppoBase)
{
	CloneSo(ppo, ppoBase);

	ppo->fPlayable = ppoBase->fPlayable;
	ppo->fLockBase = ppoBase->fLockBase;
	ppo->posBasePrev = ppoBase->posBasePrev;
	ppo->tWakeRipple = ppoBase->tWakeRipple;
	ppo->pambWake = ppoBase->pambWake;
	ppo->pzi = ppoBase->pzi;
	ppo->paloUseCharm = ppoBase->paloUseCharm;
}

void UpdatePo(PO* ppo, float dt)
{
	UpdateSo(ppo, dt);
}

int GetPoSize()
{
	return sizeof(PO);
}

void DeletePo(PO* ppo)
{
	delete ppo;
}

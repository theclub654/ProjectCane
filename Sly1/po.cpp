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
	LO lo = *ppo;
	*ppo = *ppoBase;
	memcpy(ppo, &lo, sizeof(LO));

	CloneLo(ppo, ppoBase);

	ClearDl(&ppo->dlChild);

	ppo->pxa = nullptr;
	ppo->grfpvaXpValid = 0;
	ppo->pstso = nullptr;
}

int GetPoSize()
{
	return sizeof(PO);
}

void DeletePo(PO* ppo)
{
	delete ppo;
}

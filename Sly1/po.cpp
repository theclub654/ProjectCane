#include "po.h"

void* NewPo()
{
	return new PO;
}

void InitPo(PO* ppo)
{
	InitSo(ppo);
}

void OnPoAdd(PO* ppo)
{
	OnSoAdd(ppo);
}

void DeletePo(LO* plo)
{
	delete(PO*)plo;
}

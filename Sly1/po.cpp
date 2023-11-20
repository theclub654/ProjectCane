#include "po.h"

void* CreatePo()
{
	PO po;
	return &po;
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

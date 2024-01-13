#include "suv.h"

void* NewSuv()
{
	return new SUV{};
}

void InitSuv(SUV* psuv)
{
	InitPo(psuv);
}

int GetSuvSize()
{
	return sizeof(SUV);
}

void UpdateSuvXfWorld(SUV* psuv)
{
	UpdateSoXfWorld(psuv);
}

void CloneSuv(SUV* psuv, SUV* psuvBase)
{
	LO lo = *psuv;
	*psuv = *psuvBase;
	memcpy(psuv, &lo, sizeof(LO));

	CloneLo(psuv, psuvBase);

	ClearDl(&psuv->dlChild);

	psuv->pxa = nullptr;
	psuv->grfpvaXpValid = 0;
	psuv->pstso = nullptr;
}

void DeleteSuv(LO* plo)
{
	delete (SUV*)plo;
}

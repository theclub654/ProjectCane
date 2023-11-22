#include "suv.h"

void* NewSuv()
{
	return new SUV;
}

void InitSuv(SUV* psuv)
{
	InitPo(psuv);
}

void DeleteSuv(LO* plo)
{
	delete (SUV*)plo;
}

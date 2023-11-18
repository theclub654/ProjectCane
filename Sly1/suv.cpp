#include "suv.h"

void InitSuv(SUV* psuv)
{
	//std::cout << "SUV Size: " << sizeof(SUV) << "\n";
	InitPo(psuv);
}

void DeleteSUV(LO* plo)
{
	delete (SUV*)plo;
}

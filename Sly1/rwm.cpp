#include "rwm.h"

void InitRwm(RWM* prwm)
{
	//std::cout << "RWM Size: " << sizeof(RWM) << "\n";
	InitLo(prwm);
}

void DeleteRwm(LO* plo)
{
	delete(RWM*)plo;
}

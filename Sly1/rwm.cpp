#include "rwm.h"

void* NewRwm()
{
	return new RWM;
}

void InitRwm(RWM* prwm)
{
	InitLo(prwm);
}

void DeleteRwm(LO* plo)
{
	delete(RWM*)plo;
}

#include "rwm.h"

RWM* NewRwm()
{
	return new RWM{};
}

void InitRwm(RWM* prwm)
{
	InitLo(prwm);
}

int GetRwmSize()
{
	return sizeof(RWM);
}

void DeleteRwm(RWM* prwm)
{
	delete prwm;
}

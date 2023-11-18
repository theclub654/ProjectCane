#include "alarm.h"

void InitAlarm(ALARM* palarm)
{
	InitSo(palarm);
}

void DeleteAlarm(LO* plo)
{
	delete(ALARM*)plo;
}

void DeleteAlbrk(LO* plo)
{
	delete (ALBRK*)plo;
}

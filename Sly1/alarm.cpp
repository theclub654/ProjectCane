#include "alarm.h"

void* NewAlarm()
{
	return new ALARM;
}

void InitAlarm(ALARM* palarm)
{
	InitSo(palarm);
}

void DeleteAlarm(LO* plo)
{
	delete(ALARM*)plo;
}

void* NewAlbrk()
{
	return new ALBRK;
}

void DeleteAlbrk(LO* plo)
{
	delete (ALBRK*)plo;
}

#include "alarm.h"

ALARM*NewAlarm()
{
	return new ALARM{};
}

void InitAlarm(ALARM* palarm)
{
	InitSo(palarm);
}

int GetAlarmSize()
{
	return sizeof(ALARM);
}

void CloneAlarm(ALARM* palarm, ALARM* palarmBase)
{
	LO lo = *palarm;
	*palarm = *palarmBase;
	memcpy(palarm, &lo, sizeof(LO));

	CloneLo(palarm, palarmBase);

	ClearDl(&palarm->dlChild);

	palarm->pxa = nullptr;
	palarm->grfpvaXpValid = 0;
	palarm->pstso = nullptr;
}

void DeleteAlarm(ALARM* palarm)
{
	delete palarm;
}

ALBRK*NewAlbrk()
{
	return new ALBRK{};
}

int GetAlbrkSize()
{
	return sizeof(ALBRK);
}

void CloneAlbrk(ALBRK* palbrk, ALBRK* palbrkBase)
{
	LO lo = *palbrk;
	*palbrk = *palbrkBase;
	memcpy(palbrk, &lo, sizeof(LO));

	CloneLo(palbrk, palbrkBase);

	ClearDl(&palbrk->dlChild);

	palbrk->pxa = nullptr;
	palbrk->grfpvaXpValid = 0;
	palbrk->pstso = nullptr;
}

void DeleteAlbrk(ALBRK* palbrk)
{
	delete palbrk;
}

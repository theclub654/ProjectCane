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
    int ichkDisabled = palarm->ichkDisabled;
    CloneSo(palarm, palarmBase);

    palarm->alarms = palarmBase->alarms;
    palarm->tAlarms = palarmBase->tAlarms;
    palarm->psm = palarmBase->psm;
    palarm->psma = palarmBase->psma;
    palarm->dtReset = palarmBase->dtReset;
    palarm->calbrks = palarmBase->calbrks;
    std::memcpy(palarm->aoidAlbrks, palarmBase->aoidAlbrks, sizeof(palarm->aoidAlbrks));
    palarm->coidSensors = palarmBase->coidSensors;
    std::memcpy(palarm->aoidSensors, palarmBase->aoidSensors, sizeof(palarm->aoidSensors));
    palarm->cpsensors = palarmBase->cpsensors;
    std::memcpy(palarm->apsensors, palarmBase->apsensors, sizeof(palarm->apsensors));
    palarm->coidStepguards = palarmBase->coidStepguards;
    std::memcpy(palarm->aoidStepguards, palarmBase->aoidStepguards, sizeof(palarm->aoidStepguards));
    palarm->calbrksDisabled = palarmBase->calbrksDisabled;
    palarm->pambSiren = palarmBase->pambSiren;
    palarm->pexc = palarmBase->pexc;
    palarm->fSilent = palarmBase->fSilent;
    palarm->crsmg = palarmBase->crsmg;
    std::memcpy(palarm->arsmg, palarmBase->arsmg, sizeof(palarm->arsmg));

    palarm->ichkDisabled = ichkDisabled;
}

void PostAlarmLoad(ALARM* palarm)
{
    PostAloLoad(palarm);
}

void UpdateAlarm(ALARM* palarm, float dt)
{
    UpdateSo(palarm, dt);
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
    CloneBrk(palbrk, palbrkBase);

    palbrk->palarm = palbrkBase->palarm;
}

void DeleteAlbrk(ALBRK* palbrk)
{
	delete palbrk;
}

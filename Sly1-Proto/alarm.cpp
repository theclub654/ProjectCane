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

void AddAlarmAlbrk(ALARM* palarm, OID oid)
{
    uint32_t calbrk = palarm->calbrks;

    if (calbrk < 4) {
        palarm->aoidAlbrks[calbrk] = oid;
        palarm->calbrks = calbrk++;
    }
}

void AddAlarmSensor(ALARM* palarm, OID oid)
{
    uint32_t coidSensors = palarm->coidSensors;

    if (coidSensors < 16) {
        palarm->aoidSensors[coidSensors] = oid;
        palarm->coidSensors = coidSensors++;
    }
}

void AddAlarmStepguard(ALARM* palarm, OID oid)
{
    uint32_t coidStepguards = palarm->coidStepguards;
    if (coidStepguards < 6) {
        palarm->aoidStepguards[coidStepguards] = oid;
        palarm->coidStepguards = coidStepguards++;
    }
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

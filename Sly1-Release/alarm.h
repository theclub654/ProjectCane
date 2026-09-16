#pragma once
#include "break.h"

enum ALARMS 
{
    ALARMS_Nil = -1,
    ALARMS_Enabled = 0,
    ALARMS_Disabled = 1,
    ALARMS_Triggered = 2,
    ALARMS_Max = 3
};

enum ALTK
{
    ALTK_Nil = -1,
    ALTK_Trigger = 0,
    ALTK_Untrigger = 1,
    ALTK_Disable = 2,
    ALTK_Max = 3
};

class ALARM : public SO
{
	public:
    ALARMS alarms;
    float tAlarms;
    struct SM* psm;
    struct SMA* psma;
    float dtReset;
    int calbrks;
    OID aoidAlbrks[4];
    int coidSensors;
    OID aoidSensors[16];
    int cpsensors;
    struct SENSOR* apsensors[16];
    int coidStepguards;
    OID aoidStepguards[6];
    int calbrksDisabled;
    struct AMB* pambSiren;
    struct EXC* pexc;
    int fSilent;
    int crsmg;
    RSMG arsmg[8];
    int ichkDisabled;
};

class ALBRK : public BREAK
{
    public:
    struct ALARM* palarm;
};

ALARM*NewAlarm();
void InitAlarm(ALARM* palarm);
void*GetAlarmFSilent(ALARM* palarm);
void SetAlarmFSilent(ALARM* palarm, int fSilent);
void*GetAlarmDtReset(ALARM* palarm);
void SetAlarmDtReset(ALARM* palarm, float dtReset);
void*GetAlarmAlarms(ALARM* palarm);
int  GetAlarmSize();
void CloneAlarm(ALARM* palarm, ALARM* palarmBase);
void PostAlarmLoad(ALARM* palarm);
void PostAlarmLoadCallbackHookup(ALARM* palarm, MSGID msgid, void* pv);
void SetAlarmAlarms(ALARM* palarm, ALARMS alarms);
void TriggerAlarm(ALARM* palarm, ALTK altk);
void DisableAlarmAlbrk(ALARM* palarm);
void EnableAlarmSensors(ALARM* palarm);
void DisableAlarmSensors(ALARM* palarm);
void NotifyAlarmSensorsOnTrigger(ALARM* palarm);
void AddAlarmAlbrk(ALARM* palarm, OID oid);
void AddAlarmSensor(ALARM* palarm, OID oid);
void AddAlarmStepguard(ALARM* palarm, OID oid);
void SetAlarmRsmg(ALARM* palarm, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal);
int  FGetAlarmSensorList(ALARM* palarm, void* pvstate);
void UpdateAlarm(ALARM* palarm, float dt);
void DeleteAlarm(ALARM* plo);

ALBRK*NewAlbrk(); 
int  GetAlbrkSize();
void CloneAlbrk(ALBRK* palbrk, ALBRK* palbrkBase);
void*GetAlbrkAlarm(ALBRK* plbrk);
void SetAlbrkAlarm(ALBRK* palbrk, ALARM* palarm);
void BreakAlbrk(ALBRK* palbrk);
void DeleteAlbrk(ALBRK* palbrk);
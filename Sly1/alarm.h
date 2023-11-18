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

class ALBRK : public BRK
{
    public:
        struct ALARM* palarm;
};

void InitAlarm(ALARM* palarm);
void DeleteAlarm(LO* plo);
void DeleteAlbrk(LO* plo);
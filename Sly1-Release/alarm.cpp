#include "alarm.h"
#include "stepguard.h"
#include "sensor.h"
#include "chkpnt.h"
#include "sm.h"
#include "button.h"
#include "spliceobj.h"
#include "asega.h"

ALARM*NewAlarm()
{
	return new ALARM{};
}

void InitAlarm(ALARM* palarm)
{
	InitSo(palarm);
    palarm->alarms = ALARMS_Nil;
    palarm->ichkDisabled = IchkAllocChkmgr(&g_chkmgr);
}

void* GetAlarmFSilent(ALARM* palarm)
{
    return &palarm->fSilent;
}

void SetAlarmFSilent(ALARM* palarm, int fSilent)
{
    palarm->fSilent = fSilent;
}

void* GetAlarmDtReset(ALARM* palarm)
{
    return &palarm->dtReset;
}

void SetAlarmDtReset(ALARM* palarm, float dtReset)
{
    palarm->dtReset = dtReset;
}

void* GetAlarmAlarms(ALARM* palarm)
{
    return &palarm->alarms;
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
    PostSwCallback(palarm->psw, (PFNMQ)PostAlarmLoadCallbackHookup, palarm, MSGID_callback, nullptr);
    PostAloLoad(palarm);

    DLI dli;
    dli.m_pdl = &palarm->dlChild;
    dli.m_ibDle = palarm->dlChild.ibDle;
    dli.m_pdliNext = s_pdliFirst;

    BASIC* pbasic = (BASIC*)palarm->dlChild.ploFirst;

    if (pbasic != nullptr) {
        dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(pbasic) + dli.m_ibDle);
    }

    s_pdliFirst = &dli;

    while (pbasic != nullptr) {
        if (palarm->psm == nullptr && FIsBasicDerivedFrom(pbasic, CID_SM) != 0) {
            palarm->psm = (SM*)pbasic;
        }

        pbasic = (BASIC*)*dli.m_ppv;

        if (pbasic != nullptr) {
            dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(pbasic) + dli.m_ibDle);
        }
    }

    if (palarm->psm != nullptr) {
        palarm->psma = PsmaApplySm(palarm->psm, (ALO*)palarm, OID_Nil, 1);
        SnipLo(palarm->psm);
    }

    s_pdliFirst = dli.m_pdliNext;
}

void PostAlarmLoadCallbackHookup(ALARM* palarm, MSGID msgid, void* pv)
{
    int validBreakCount = 0;

    for (int i = 0; i < palarm->calbrks; ++i) {
        auto* palbrk = static_cast<ALBRK*>(PloFindSwObject(palarm->psw, 260, palarm->aoidAlbrks[i], palarm));

        if (palbrk == nullptr) {
            continue;
        }

        if (!FIsBasicDerivedFrom(palbrk, CID_ALBRK)) {
            continue;
        }

        SetAlbrkAlarm(palbrk, palarm);
        ++validBreakCount;
    }

    palarm->calbrks = validBreakCount;

    for (int i = 0; i < palarm->coidSensors; ++i) {
        auto* sensor = static_cast <SENSOR*>(PloFindSwObject(palarm->psw, 260, palarm->aoidSensors[i], palarm));

        if (sensor == nullptr) {
            continue;
        }

        if (!FIsBasicDerivedFrom(sensor, CID_SENSOR)) {
            continue;
        }

        palarm->apsensors[palarm->cpsensors++] = sensor;
        sensor->pvtsensor->pfnSetSensorAlarm(sensor, palarm);
    }

    for (int i = 0; i < palarm->coidStepguards; ++i) {
        auto* stepguard = static_cast<STEPGUARD*>(PloFindSwObject(palarm->psw, 260, palarm->aoidStepguards[i], palarm));

        if (stepguard == nullptr) {
            continue;
        }

        if (!FIsBasicDerivedFrom(stepguard, CID_STEPGUARD)) {
            continue;
        }

        AddStepguardAlarm(stepguard, palarm);
    }

    if (FGetChkmgrIchk(&g_chkmgr, palarm->ichkDisabled))
        SetAlarmAlarms(palarm, ALARMS_Disabled);
    else
        SetAlarmAlarms(palarm, ALARMS_Enabled);

}

void SetAlarmAlarms(ALARM* palarm, ALARMS alarms)
{
    ALARMS alarmsPrev = palarm->alarms;

    if (alarmsPrev == alarms)
        return;

    if (alarmsPrev == ALARMS_Triggered) 
    {
        StopSound(palarm->pambSiren, 0);
        UnsetExcitement(palarm->pexc);
        TriggerRsmg(palarm->psw, palarm->crsmg, palarm->arsmg, palarm, 0);

        if (alarms == ALARMS_Disabled)
            OnGameAlarmDisabled(&g_game);
    }

    palarm->alarms = alarms;
    palarm->tAlarms = g_clock.t;

    switch (alarms) 
    {
        case ALARMS_Disabled:
        if (palarm->psma != nullptr)
            SeekSma(palarm->psma, (OID)544);

        DisableAlarmSensors(palarm);

        if (FGetChkmgrIchk(&g_chkmgr, palarm->ichkDisabled) == 0)
            HandleLoSpliceEvent(palarm, 3, 0, nullptr);
        else
            HandleLoSpliceEvent(palarm, 22, 0, nullptr);

        SetChkmgrIchk(&g_chkmgr, palarm->ichkDisabled);
        break;

        case ALARMS_Enabled:
        if (palarm->psma != nullptr)
        {
            SetSmaGoal(palarm->psma, (OID)538);

            // The ready transition is one frame long and its apply events
            // temporarily remove the idle render object.  Complete this
            // initial transition while the alarm is still active during
            // post-load; otherwise MRD can freeze a distant alarm halfway
            // through it and leave the idle model removed indefinitely.
            if (alarmsPrev == ALARMS_Nil && palarm->psma->pasegaCur != nullptr)
            {
                ASEGA* pasegaReady = palarm->psma->pasegaCur;
                ASEG* pasegReady = pasegaReady->paseg;

                if (pasegReady != nullptr)
                {
                    const float dtReady =
                        std::max(pasegReady->tMax - pasegaReady->tLocal, 0.0f) + 0.0001f;
                    UpdateAsega(pasegaReady, dtReady);
                }
            }
        }

        EnableAlarmSensors(palarm);
        break;

        case ALARMS_Triggered:
        if (palarm->psma != nullptr)
            SetSmaGoal(palarm->psma, (OID)543);

        if (palarm->fSilent == 0)
            StartSound((SFXID)21, &palarm->pambSiren, palarm, nullptr, 8000.0f, 5000.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

        palarm->pexc = PexcSetExcitement(107);
        NotifyAlarmSensorsOnTrigger(palarm);
        TriggerRsmg(palarm->psw, palarm->crsmg, palarm->arsmg, palarm, 1);
        HandleLoSpliceEvent(palarm, 2, 0, nullptr);
        OnGameAlarmTriggered(&g_game);
        break;

        default:
        break;
    }
}

void TriggerAlarm(ALARM* palarm, ALTK altk)
{
    ALARMS alarms;

    switch (altk) 
    {
        case ALTK_Trigger:
        alarms = ALARMS_Triggered;
        break;

        case ALTK_Untrigger:
        alarms = ALARMS_Enabled;
        break;

        case ALTK_Disable:
        alarms = ALARMS_Disabled;
        break;

        default:
        return;
    }

    SetAlarmAlarms(palarm, alarms);
}

void DisableAlarmAlbrk(ALARM* palarm)
{
    palarm->calbrksDisabled = palarm->calbrksDisabled + 1;
}

void EnableAlarmSensors(ALARM* palarm)
{
    for (int i = 0; i < palarm->cpsensors; ++i) 
    {
        SENSOR* psensor = palarm->apsensors[i];

        if (psensor->sensorsInitial == SENSORS_Nil && psensor->pvtsensor->pfnEnableSensor != nullptr)
            psensor->pvtsensor->pfnEnableSensor(psensor, SENSM_SenseOnly);
    }
}

void DisableAlarmSensors(ALARM* palarm)
{
    for (int i = 0; i < palarm->cpsensors; ++i) 
    {
        SENSOR* psensor = palarm->apsensors[i];

        if (psensor->pvtsensor->pfnDisableSensor != nullptr)
            psensor->pvtsensor->pfnDisableSensor(psensor);
    }
}

void NotifyAlarmSensorsOnTrigger(ALARM* palarm)
{
    for (int i = 0; i < palarm->cpsensors; ++i) 
    {
        SENSOR* psensor = palarm->apsensors[i];

        if (psensor->pvtsensor->pfnOnSensorAlarmTriggered != nullptr)
            psensor->pvtsensor->pfnOnSensorAlarmTriggered(psensor);
    }
}

void AddAlarmAlbrk(ALARM* palarm, OID oid)
{
    if (palarm->calbrks < 4) {
        palarm->aoidAlbrks[palarm->calbrks++] = oid;
    }
}

void AddAlarmSensor(ALARM* palarm, OID oid)
{
    if (palarm->coidSensors < 16) {
        palarm->aoidSensors[palarm->coidSensors++] = oid;
    }
}

void AddAlarmStepguard(ALARM* palarm, OID oid)
{
    if (palarm->coidStepguards < 6) {
        palarm->aoidStepguards[palarm->coidStepguards++] = oid;
    }
}

void SetAlarmRsmg(ALARM* palarm, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal)
{
    FAddRsmg(palarm->arsmg, 8, &palarm->crsmg, fOnTrigger, oidRoot, oidSM, oidGoal);
}

int FGetAlarmSensorList(ALARM* palarm, void* pvstate)
{
    for (int i = 0; i < palarm->cpsensors; ++i) 
    {
        if (FAppendSpliceListElement(pvstate, &palarm->apsensors[i]) == 0) 
            return 0;
    }

    return 1;
}

void UpdateAlarm(ALARM* palarm, float dt)
{
    UpdateSo(palarm, dt);

    if (palarm->alarms == ALARMS_Triggered && palarm->dtReset != 0.0f) 
    {
        if (palarm->dtReset < g_clock.t - palarm->tAlarms) 
        {
            SetAlarmAlarms(palarm, ALARMS_Enabled);
            return;
        }
    }

    if (palarm->calbrks > 0 && palarm->calbrksDisabled == palarm->calbrks) 
    {
        SetAlarmAlarms(palarm, ALARMS_Disabled);
        return;
    }

    if (palarm->alarms == ALARMS_Triggered && g_pjt->jts == JTS_Celebrate) 
    {
        for (int i = 0; i < palarm->calbrks; ++i) 
        {
            ALBRK* palbrk = (ALBRK*)PloFindSwObject(palarm->psw, 260, palarm->aoidAlbrks[i], palarm);

            if (palbrk != nullptr && FIsBasicDerivedFrom((BASIC*)palbrk, CID_ALBRK) != 0)
                palbrk->pvtalbrk->pfnBreakAlbrk(palbrk);
        }
    }
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

void* GetAlbrkAlarm(ALBRK* plbrk)
{
    return &plbrk->palarm;
}

void SetAlbrkAlarm(ALBRK* palbrk, ALARM* palarm)
{
    palbrk->palarm = palarm;
}

void BreakAlbrk(ALBRK* palbrk)
{
    if (palbrk->fBroken == 0) 
        DisableAlarmAlbrk(palbrk->palarm);

    BreakBrk(palbrk);
}

void DeleteAlbrk(ALBRK* palbrk)
{
	delete palbrk;
}

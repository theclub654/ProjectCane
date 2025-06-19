#pragma once
#include "blot.h"

enum TIMERS
{
    TIMERS_Nil = -1,
    TIMERS_Expired = 0,
    TIMERS_Paused = 1,
    TIMERS_Running = 2,
    TIMERS_Max = 3
};

struct TIMER : public BLOT
{

    TIMERS timers;
    float tTimers;
    float dt;
    float svt;
    int nSecondsLast;
    int nTenthsLast;
    float dtThreshold;
    glm::vec4 rgbaThreshold;
    PFNTN pfntnThreshold;
    int fThreshold;
    float dtExpire;
    PFNTN pfntnExpire;
    int fStopped;
};

void StartupTimer(TIMER *ptimer);
void PostTimerLoad(TIMER *ptimer);
void SetTimerTimers(TIMER *ptimer, TIMERS timers);
void UpdateTimer(TIMER* ptimer);
void DrawTimer(TIMER* ptimer);

extern TIMER g_timer;
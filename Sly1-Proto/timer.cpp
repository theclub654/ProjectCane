#include "timer.h"

void StartupTimer(TIMER* ptimer)
{
    ptimer->pvttimer = &g_vttimer;
}

void PostTimerLoad(TIMER* ptimer)
{
    PostBlotLoad(ptimer);

    ptimer->pfont = g_pfontScreenCounters;
    ptimer->nTenthsLast = -1;
    ptimer->dtThreshold = 3.402823e+38;
    ptimer->fStopped = 1;
    ptimer->nSecondsLast = -1;
    SetTimerTimers(ptimer, TIMERS_Expired);
}

void SetTimerTimers(TIMER* ptimer, TIMERS timers)
{
    if (ptimer->timers != timers)
    {
        ptimer->timers = timers;
        ptimer->tTimers = g_clock.t;
    }
}

void UpdateTimer(TIMER* ptimer)
{

}

void DrawTimer(TIMER* ptimer)
{

}

TIMER g_timer;
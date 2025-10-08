#include "timer.h"

void StartupTimer(TIMER* ptimer)
{
    ptimer->pvttimer = &g_vttimer;
}

void PostTimerLoad(TIMER* ptimer)
{
    PostBlotLoad(ptimer);

    ptimer->pfont = &g_afontBrx[2];
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
    if (ptimer->achzDraw[0] == '\0')
        return;

    // Setup textbox
    CTextBox tbx;
    tbx.SetPos(ptimer->x, ptimer->y);
    tbx.SetSize(ptimer->dx, ptimer->dy);
    tbx.SetTextColor(&ptimer->rgba);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    // Optional edge rect
    if (ptimer->pte && ptimer->pte->m_pfont)
    {
        ptimer->pte->m_pfont->EdgeRect(ptimer->pte, &tbx);
    }

    // Apply font scale
    float scale = ptimer->rFontScale;
    ptimer->pfont->PushScaling(scale, scale);

    // Draw 'T' separately
    char achzStopwatch[2] = { ptimer->achzDraw[0], '\0' };
    ptimer->pfont->DrawPchz(achzStopwatch, &tbx);

    // Advance X position
    float dx = ptimer->pfont->DxFromPchz(achzStopwatch);
    tbx.m_x = ptimer->x + dx;
    tbx.m_y = ptimer->y;

    // Optional threshold color override
    if (ptimer->fThreshold)
    {
        tbx.SetTextColor(&ptimer->rgbaThreshold);
    }

    // Draw the remaining string (starting after 'T')
    ptimer->pfont->DrawPchz(ptimer->achzDraw + 1, &tbx);

    ptimer->pfont->PopScaling();
}

TIMER g_timer;
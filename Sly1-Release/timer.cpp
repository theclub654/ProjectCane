#include "timer.h"
#include <cstdio>

void StartupTimer(TIMER* ptimer)
{
    ptimer->pvttimer = &g_vttimer;
}

void PostTimerLoad(TIMER* ptimer)
{
    PostBlotLoad(ptimer);

    ptimer->pfont = PfontFromFont(2);
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

void StopTimer(TIMER* ptimer)
{
    ptimer->fStopped = 1;
    SetTimerTimers(ptimer, TIMERS_Expired);
}

void RebuildTimerAchzDraw(TIMER* ptimer, float dt)
{
    const float dtPrevious = ptimer->dt;
    const float absPrevious = std::fabs(dtPrevious);

    const int secondsPrevious = static_cast<int>(absPrevious);
    const int tenthsPrevious = static_cast<int>((absPrevious - static_cast<float>(secondsPrevious)) * 10.0f);

    if (ptimer->timers == TIMERS_Running)
    {
        if (ptimer->svt != 0.0f)
        {
            if (ptimer->svt < 0.0f && dtPrevious >= 0.0f && dtPrevious < 10.0f)
            {
                if (tenthsPrevious != ptimer->nTenthsLast)
                {
                    if (tenthsPrevious == 9)
                        StartSound(static_cast<SFXID>(126), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                    else if (tenthsPrevious == 4)
                        StartSound(static_cast<SFXID>(127), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                }
            }
            else if (secondsPrevious != ptimer->nSecondsLast)
            {
                const SFXID sfxid = static_cast<SFXID>((secondsPrevious & 1) == 0 ? 115 : 114);
                StartSound(sfxid, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            }
        }
    }

    ptimer->nTenthsLast = tenthsPrevious;
    ptimer->nSecondsLast = secondsPrevious;

    if (g_clock.fEnabled && ptimer->timers == TIMERS_Running && (g_grfjoyt & 2U) != 0)
    {
        const float rate = ptimer->svt;

        ptimer->dt += dt * rate;

        bool crossedThreshold = false;
        bool expired = false;

        if (rate < 0.0f)
        {
            crossedThreshold = ptimer->dtThreshold != FLT_MAX && ptimer->dt <= ptimer->dtThreshold;
            expired = ptimer->dt <= ptimer->dtExpire;
        }
        else if (rate > 0.0f)
        {
            crossedThreshold = ptimer->dtThreshold != FLT_MAX && ptimer->dt >= ptimer->dtThreshold;
            expired = ptimer->dt >= ptimer->dtExpire;
        }

        if (crossedThreshold && !ptimer->fThreshold)
        {
            ptimer->fThreshold = true;

            if (ptimer->pfntnThreshold != nullptr)
                ptimer->pfntnThreshold(ptimer, 0);
        }

        if (expired)
        {
            ptimer->dt = ptimer->dtExpire;

            StartSound(static_cast<SFXID>(1), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            SetTimerTimers(ptimer, TIMERS_Expired);

            if (ptimer->pfntnExpire != nullptr)
                ptimer->pfntnExpire(ptimer, 1);
        }
    }

    const float absCurrent = std::fabs(ptimer->dt);
    const int secondsCurrent = static_cast<int>(absCurrent);
    const int tenthsCurrent = static_cast<int>(absCurrent * 10.0f);

    int major = 0;
    int minor = 0;
    char separator = '\0';
    const char* zeroPadding = "";
    bool rebuild = false;

    if (absCurrent < 10.0f && tenthsCurrent != static_cast<int>(absPrevious * 10.0f))
    {
        major = tenthsCurrent / 10;
        minor = tenthsCurrent % 10;
        separator = '.';
        rebuild = true;
    }
    else
    {
        const bool secondsChanged = secondsCurrent != secondsPrevious;
        const bool justPaused = ptimer->timers == TIMERS_Paused && g_clock.t - ptimer->tTimers <= g_clock.dt;

        if (secondsChanged || justPaused)
        {
            major = secondsCurrent / 60;
            minor = secondsCurrent % 60;
            separator = ':';
            zeroPadding = minor < 10 ? "0" : "";
            rebuild = true;
        }
    }

    if (!rebuild)
        return;

    std::snprintf(ptimer->achzDraw, sizeof(ptimer->achzDraw), "%c%d%c%s%d", 'T', major, separator, zeroPadding, minor);

    ptimer->pfont->PushScaling(ptimer->rFontScale, ptimer->rFontScale);

    const float dx = ptimer->pfont->DxFromPchz(ptimer->achzDraw);
    const float dy = static_cast<float>(ptimer->pfont->m_dyUnscaled) * ptimer->pfont->m_ryScale;

    ResizeBlot(ptimer, dx, dy);
    ptimer->pfont->PopScaling();
}

void SetTimer(TIMER* ptimer, float dt)
{
    ptimer->pfntnThreshold = nullptr;
    ptimer->fThreshold = false;

    ptimer->dtExpire = 0.0f;
    ptimer->pfntnExpire = nullptr;

    ptimer->fStopped = false;

    ptimer->rgba = glm::vec4(1.0);

    ptimer->svt = -1.0f;
    ptimer->nTenthsLast = -1;
    ptimer->dtThreshold = std::numeric_limits<float>::max();
    ptimer->nSecondsLast = -1;
    ptimer->dt = dt;

    SetTimerTimers(ptimer, TIMERS_Paused);
    RebuildTimerAchzDraw(ptimer, 0.0f);
    ptimer->pvtblot->pfnShowBlot(ptimer);
}

void SetTimerSpeed(TIMER* ptimer, float svt)
{
    ptimer->svt = svt;
}

void SetTimerThreshold(TIMER* ptimer, float dt, uint32_t rgba, PFNTN pfntn)
{
    constexpr float scale = 1.0f / 255.0f;

    ptimer->pfntnThreshold = pfntn;
    ptimer->dtThreshold = dt;

    ptimer->rgbaThreshold = glm::vec4(
        static_cast<float>((rgba >> 0) & 0xFF) * scale,
        static_cast<float>((rgba >> 8) & 0xFF) * scale,
        static_cast<float>((rgba >> 16) & 0xFF) * scale,
        static_cast<float>((rgba >> 24) & 0xFF) * scale
    );
}

void SetTimerExpire(TIMER* ptimer, float dt, PFNTN pfntn)
{
    ptimer->pfntnExpire = pfntn;
    ptimer->dtExpire = dt;
}

void StartTimer(TIMER* ptimer)
{
    SetTimerTimers(ptimer, TIMERS_Running);
}

void UpdateTimer(TIMER* ptimer)
{
    UpdateBlot(ptimer);

    if (ptimer->blots == BLOTS_Hidden)
        return;

    if (ptimer->timers != TIMERS_Expired)
    {
        RebuildTimerAchzDraw(ptimer, g_clock.dt);
        return;
    }

    const bool canHide = static_cast<unsigned int>(ptimer->blots + BLOTS_Nil) < 2;

    if (!canHide)
        return;

    const bool expirationDelayFinished = g_clock.t - ptimer->tTimers >= 1.0f;

    if (ptimer->fStopped || expirationDelayFinished)
        ptimer->pvtblot->pfnHideBlot(ptimer);
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
        ptimer->pte->m_pfont->EdgeRect(ptimer->pte, &tbx);

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
        tbx.SetTextColor(&ptimer->rgbaThreshold);

    // Draw the remaining string (starting after 'T')
    ptimer->pfont->DrawPchz(ptimer->achzDraw + 1, &tbx);

    ptimer->pfont->PopScaling();
}

TIMER g_timer;

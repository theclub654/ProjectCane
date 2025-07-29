#include "clock.h"

void StartupClock()
{
    g_clock.tickFrame = glfwGetTime();
}

void SetClockRate(float rt)
{
    g_rtClock = rt;
    SetClockEnabled(&g_clock, (uint32_t)(0.0 < rt));
}

void MarkClockTick(CLOCK* pclock)
{
    double currentTime = glfwGetTime();
    float deltaTime = static_cast<float>(currentTime - pclock->tickFrame);

    // Clamp to min/max frame time
    deltaTime = std::clamp(deltaTime, 0.01666667f, 0.03333334f);
    pclock->dtReal = deltaTime;

    float scaledDelta = pclock->fEnabled ? deltaTime : 0.0f;

    float adjustedDelta = scaledDelta * g_rtClockDebug * g_rtClockPowerUp * g_rtClock;

    if (adjustedDelta >= 0.01666667f) {
        pclock->dtReal = adjustedDelta;
    }

    pclock->tickFrame = currentTime;
    pclock->t += adjustedDelta;
    pclock->dtPrev = pclock->dt;
    pclock->dt = adjustedDelta;
    pclock->tReal += pclock->dtReal;
}

void SetClockEnabled(CLOCK* pclock, int fEnabled)
{
    pclock->fEnabled = fEnabled;
}

void ResetClock(CLOCK* pclock, float t)
{
    pclock->t = t;
}

CLOCK g_clock;
uint32_t s_tickLastRaw = 0;
uint32_t s_tickWrapCount = 0;
std::atomic<uint32_t> Count{ 0 };
float g_rtClockDebug = 1.0;
float g_rtClockPowerUp = 1.0;
float g_rtClock = 1.0;
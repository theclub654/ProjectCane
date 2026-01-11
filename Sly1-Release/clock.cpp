#include "clock.h"

void StartupClock()
{
    // Initialize the last raw tick count using atomic Count
    s_tickLastRaw = static_cast<TICK>(Count.load());

    // Initialize the frame tick
    g_clock.tickFrame = TickNow();
}

void SetClockRate(float rt)
{
    g_rtClock = rt;
    SetClockEnabled(&g_clock, (uint32_t)(0.0 < rt));
}

TICK TickNow()
{
    uint32_t current = Count.load();  // Simulating hardware counter read

    // Detect rollover (32-bit counter wrapping around)
    if (current < s_tickLastRaw) {
        ++s_tickWrapCount;
    }

    s_tickLastRaw = current;

    // Combine wrap count and current tick to simulate a 64-bit timer
    return (static_cast<uint64_t>(s_tickWrapCount) << 32) | current;
}

void MarkClockTick(CLOCK* pclock)
{
    TICK now = TickNow();
    uint32_t u = static_cast<uint32_t>(now - static_cast<TICK>(pclock->tickFrame));

    float dtTicks;
    if (static_cast<int32_t>(u) < 0) {
        // exact unsigned->float trick from the original
        float f = static_cast<float>((u >> 1) | (u & 1u));
        dtTicks = f + f;
    }
    else {
        dtTicks = static_cast<float>(u);
    }

    float dt = dtTicks * 3.390842e-09f;

    // clamp to [1/60, 1/30]
    if (dt < 0.00833333) dt = 0.00833333;
    else if (dt > 0.03333334f) dt = 0.03333334f;

    pclock->dtReal = dt;

    float enabledDt = (pclock->fEnabled != 0) ? dt : 0.0f;
    float adjusted = enabledDt * g_rtClockDebug * g_rtClockPowerUp * g_rtClock;

    if (adjusted >= 0.00833333)
        pclock->dtReal = adjusted;

    pclock->tickFrame = now;
    pclock->t += adjusted;
    pclock->dtPrev = pclock->dt;
    pclock->dt = adjusted;
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
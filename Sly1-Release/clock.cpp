#include "clock.h"

uint32_t s_tickLastRaw = 0;
uint32_t s_tickWrapCount = 0;
std::atomic<uint32_t> Count{ 0 };
float g_rtClockDebug = 1.0;
float g_rtClockPowerUp = 1.0;
float g_rtClock = 1.0;

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
    uint64_t currentTick = TickNow();
    int64_t deltaTicks = static_cast<int64_t>(currentTick - pclock->tickFrame);

    float deltaTime;
    if (deltaTicks < 0) {
        // Handling possible underflow or wraparound
        uint64_t corrected = static_cast<uint64_t>((deltaTicks & 1) | (deltaTicks >> 1));
        float f = static_cast<float>(corrected);
        deltaTime = f + f;
    }
    else {
        deltaTime = static_cast<float>(deltaTicks);
    }

    // Convert to seconds: tick * tick-to-sec (3.390842e-09 is probably 1/tickFrequency)
    deltaTime *= 3.390842e-09f;

    // Clamp to min/max frame time
    deltaTime = std::clamp(deltaTime, 0.01666667f, 0.03333334f);
    pclock->dtReal = deltaTime;
    
    float scaledDelta = 0.0f;
    if (pclock->fEnabled) {
        scaledDelta = deltaTime;
    }

    float adjustedDelta = scaledDelta * g_rtClockDebug * g_rtClockPowerUp * g_rtClock;

    if (adjustedDelta >= 0.01666667f) {
        pclock->dtReal = adjustedDelta;
    }

    pclock->tickFrame = currentTick;
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

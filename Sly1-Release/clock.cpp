#include "clock.h"
#include <chrono>
#include <algorithm>
#include <cmath>

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
    using HostClock = std::chrono::steady_clock;
    static const HostClock::time_point s_epoch = HostClock::now();

    // MarkClockTick converts ticks with 3.390842e-9, matching the PS2's
    // 294.912 MHz timing base. Count was never advanced by the PC port, so
    // using it made every frame fall back to the minimum timestep.
    constexpr double kTicksPerSecond = 294912000.0;
    const std::chrono::duration<double> elapsed = HostClock::now() - s_epoch;
    return static_cast<TICK>(elapsed.count() * kTicksPerSecond);
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

    float kDtMin;
    if (g_fRetailTiming)
    {
        // Retail Timing: behave like the PS2. The console's frame loop is locked
        // to NTSC vsync (59.94 Hz), so its measured step is always a whole
        // number of fields. Snap the PC's slightly noisy frame time to the
        // nearest field so every step is identical to the console's, then
        // apply retail's clamp (one to two 1/60 s frames, clock.c).
        constexpr float kDtField = 1001.0f / 60000.0f;
        const float fields = std::max(1.0f, std::round(dt / kDtField));
        dt = fields * kDtField;
        kDtMin = 1.0f / 60.0f;
    }
    else
    {
        // Keep the engine's minimum update step synchronized with the selected
        // presentation rate. Uncapped (0): the frame pacer stops at 1000 FPS,
        // so 1 ms is the floor.
        kDtMin = 1.0f / static_cast<float>(g_targetFrameRate > 0 ? g_targetFrameRate : 1000);
    }
    if (dt < kDtMin) dt = kDtMin;
    else if (dt > 0.03333334f) dt = 0.03333334f;

    pclock->dtReal = dt;

    float enabledDt = (pclock->fEnabled != 0) ? dt : 0.0f;
    float adjusted = enabledDt * g_rtClockDebug * g_rtClockPowerUp * g_rtClock;

    if (adjusted >= kDtMin)
        pclock->dtReal = adjusted;

    pclock->tickFrame = now;
    pclock->t += adjusted;
    pclock->dtPrev = pclock->dt;
    pclock->dt = adjusted;
    pclock->tReal += pclock->dtReal;
}

void ResyncClockTick(CLOCK* pclock)
{
    if (pclock != nullptr)
        pclock->tickFrame = TickNow();
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
// Safe first-run fallback. StartupSaveData replaces this with the frame rate
// saved from the pause-menu video settings before the frame pacer starts.
int g_targetFrameRate = 60;
// Retail Timing (Options > Gameplay). QOL.ini overrides this at startup.
bool g_fRetailTiming = true;

double EffectiveFrameRateHz()
{
    if (g_fRetailTiming)
        return 60000.0 / 1001.0;   // NTSC field rate, like the PS2
    return g_targetFrameRate > 0 ? static_cast<double>(g_targetFrameRate) : 1000.0;
}

#pragma once
#include <iostream>
#include <algorithm>

typedef uint64_t TICK;

struct CLOCK
{
    bool fEnabled;
    float t;
    float dt;
    float dtPrev;
    float tReal;
    float dtReal;
    TICK tickFrame;
};

void StartupClock();
void SetClockRate(float rt);
TICK TickNow();
void MarkClockTick(CLOCK* pclock);
void ResyncClockTick(CLOCK* pclock);
void SetClockEnabled(CLOCK* pclock, int fEnabled);
void ResetClock(CLOCK* pclock, float t);

extern CLOCK g_clock;
extern uint32_t s_tickLastRaw;
extern uint32_t s_tickWrapCount;
extern std::atomic<uint32_t> Count;
extern float g_rtClockDebug;
extern float g_rtClockPowerUp;
extern float g_rtClock;
extern int g_targetFrameRate;
// Retail Timing: game updates exactly as on the PS2 (see qol.h).
extern bool g_fRetailTiming;
// The rate the frame pacer runs at: 59.94 with Retail Timing, else the option.
double EffectiveFrameRateHz();

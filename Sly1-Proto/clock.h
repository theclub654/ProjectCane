#pragma once
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>

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
void SetClockEnabled(CLOCK* pclock, int fEnabled);
void ResetClock(CLOCK* pclock, float t);

extern CLOCK g_clock;
extern uint32_t s_tickLastRaw;
extern uint32_t s_tickWrapCount;
extern std::atomic<uint32_t> Count;
extern float g_rtClockDebug;
extern float g_rtClockPowerUp;
extern float g_rtClock;
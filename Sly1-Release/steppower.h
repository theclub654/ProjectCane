#pragma once
#include "pzo.h"

void SetFsp(FSP fsp);
void UpdateJtActivePowerUp(JT* pjt, JOY* pjoy);
int  IRotatePowerUp(JOY* pjoy, int iCur, int iMax, GRFVAULT* mpigrfvault);
void SetJtStun(JT* pjt, int fStun);

static const char* s_mpfspszPowerUp[7] =
{
    "&2T&.: Dive",
    "&2T&.: Roll",
    "&2T&.: Slow",
    "&2T&.: Mine",
    "&2T&.: Fast",
    "&2T&.: Decoy",
    "&2T&.: Stun"
};

static GRFVAULT s_agrfvaultFsp[FSP_Max] =
{
    static_cast<GRFVAULT>(0x0012), // FSP_Dive
    static_cast<GRFVAULT>(0x0004), // FSP_Ball (base roll)
    static_cast<GRFVAULT>(0x1008), // FSP_SlowDownClock
    static_cast<GRFVAULT>(0x0040), // FSP_Mine
    static_cast<GRFVAULT>(0x0080), // FSP_SpeedUpClock
    static_cast<GRFVAULT>(0x0200), // FSP_Decoy
    static_cast<GRFVAULT>(0x8000)  // FSP_Stun
};
static float RT_JtClockSpeedUp = 1.5;
static float RT_JtClockSlowDown = 0.25;
static SMP s_smpSpeedUpClock = { 2.0, 0.0, 0.1 };

extern FSP g_fsp;

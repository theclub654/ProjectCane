#pragma once
#include "sw.h"

// Update all objects in SW
void UpdateSw(SW* psw, float dt);
void UpdateSwObjects(SW* psw, float dt);
void UpdateSwDarkening(SW* psw, float dt);

extern SMP g_smpDarkenIn;
extern SMP g_smpDarkenOut;
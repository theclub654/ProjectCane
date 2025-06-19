#pragma once
#include "transition.h"

enum WIPES
{
    WIPES_Nil = -1,
    WIPES_Idle = 0,
    WIPES_WipingOut = 1,
    WIPES_Black = 2,
    WIPES_WipingIn = 3,
    WIPES_Max = 4
};
enum WIPEK
{
    WIPEK_Nil = -1,
    WIPEK_Fade = 0,
    WIPEK_Keyhole = 1,
    WIPEK_WorldMap = 2,
    WIPEK_Frozen = 3,
    WIPEK_Max = 4
};

struct WIPE
{
    WIPES wipes;
    float tWipes;
    float uBlack;
    TRANS trans;
    WIPEK wipek;
    TRANS transButton;
    WIPEK wipekButton;
};

void InitWipe(WIPE* pwipe);

extern WIPE g_wipe;
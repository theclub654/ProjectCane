#pragma once
#include "lo.h"

struct FADER
{
    struct ALO* palo;
    float uAlpha;
    float duAlpha;
    DLE dleFader;
};

void InitSwFaderDl(SW* psw);
void InitSwRealClockFader(SW* psw);

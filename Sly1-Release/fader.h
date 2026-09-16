#pragma once
#include "lo.h"

struct FADER
{
    class ALO* palo;
    float uAlpha;
    float duAlpha;
    DLE dleFader;
};

void InitSwFaderDl(SW* psw);
void InitSwRealClockFader(SW* psw);
std::shared_ptr<FADER> PfaderNew(ALO* palo);
void UpdateFader(FADER* pfader, float dt);
void RemoveFader(FADER* pfader);
#pragma once
#include "blot.h"

struct ATTRACT : public BLOT
{
    int fJoyValid;
    int fReshow;
    char* pchzReshow;
};

void StartupAttract(ATTRACT* pattract);
void PostAttractLoad(ATTRACT* pattract);
void SetAttractAchzDraw(ATTRACT* pattract, char* pchz);
void SetAttractBlots(ATTRACT* pattract, BLOTS blots);
void UpdateAttract(ATTRACT* pattract);
void DrawAttract(ATTRACT* pattract);

extern ATTRACT g_attract;
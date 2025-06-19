#pragma once
#include "ctr.h"

struct BOSSCTR : public CTR
{
	float gDisplay;
};

void StartupBossCtr(BOSSCTR* bossctr);
void PostBossctrLoad(BOSSCTR* pbossctr);
void DrawBossCtr(BOSSCTR* pbossctr);

extern BOSSCTR g_bossctr;
extern SMP s_smpBossctrSlide;
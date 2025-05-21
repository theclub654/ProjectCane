#pragma once
#include "ctr.h"

struct BOSSCTR : public CTR
{

};

void StartupBossCtr(BOSSCTR* bossctr);
void DrawBossCtr(BOSSCTR* pbossctr);

extern BOSSCTR g_bossctr;
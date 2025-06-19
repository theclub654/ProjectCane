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
extern CTextEdge g_teAttract;
extern const char *g_aachzAttract[2];
extern float RX_Attract;
extern float RY_Attract;
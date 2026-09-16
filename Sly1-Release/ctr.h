#pragma once
#include "blot.h"
#include "lo.h"

// COUNTER
struct CTR : public BLOT
{
    int nDisplay;
    int* pnActual;
    int* pnTotal;
    float dgDisplayMax;
    float uRemain;
    PFNSMACK pfnsmack;
    void* pv;
};

struct PERCENTCTR : public CTR
{

};

// GOTTA COME BACK TO THIS
void PostCtrLoad(CTR *pctr);
float DtVisibleCtr(CTR* pctr);
void UpdateCtr(CTR* pctr);
void RebuildCtrAchzDraw(CTR *pctr);
void DrawCtr(CTR *pctr);

void StartupPercentCtr(PERCENTCTR* ppercentctr);

extern PERCENTCTR g_percentctr; 
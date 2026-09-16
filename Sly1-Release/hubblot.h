#pragma once
#include "blot.h"

struct HUBBLOT : public BLOT
{
	public:
	class HUBSEL* phubsel;
	int   fWaitVag;
	float tHintLast;
};

void StartupHubBlot(HUBBLOT* phublot);
void PostHubBlotLoad(HUBBLOT* phubblot);
void EnableHubCamera(HUBBLOT* phubblot);
//GOTTA COME BACK TO THIS WHEN DONE SOUND
void UpdateHubBlotActive(HUBBLOT* phubblot, JOY* pjoy);
//GOTTA COME BACK TO THIS WHEN DONE SOUND
void OnHubBlotActive(HUBBLOT* phubblot, int fActive);
void DrawHubBlot(HUBBLOT* phubblot);

struct VTHUBBLOT
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostHubBlotLoad)(HUBBLOT*) = PostHubBlotLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnHubBlotActive)(HUBBLOT*, int) = OnHubBlotActive;
    void (*pfnUpdateHubBlotActive)(HUBBLOT*, JOY*) = UpdateHubBlotActive;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawHubBlot)(HUBBLOT*) = DrawHubBlot;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTHUBBLOT g_vthubblot;
extern CTextEdge g_teHubBlot;
extern HUBBLOT g_hubblot;
extern float g_dtHubHint;
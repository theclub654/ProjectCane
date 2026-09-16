#pragma once
#include "prompt.h"
#include "letterbox.h"
#include "dl.h"
#include "asega.h"

enum UIS 
{
    UIS_Nil = -1,
    UIS_Splash = 0,
    UIS_Attract = 1,
    UIS_Hub = 2,
    UIS_Playing = 3,
    UIS_Pausing = 4,
    UIS_Paused = 5,
    UIS_Unpausing = 6,
    UIS_WorldMap = 7,
    UIS_Wiping = 8,
    UIS_Max = 9
};

struct UI
{
    UIS uis;
    float tUis;
    float rvolMusicPrepause;
    float rvolEffectsPrepause;
    BLOT *apblotActive[8];
    int cpblotActive;
    UIS uisPlaying;
};

void StartupUi();
void InitUi(UI *pui);
void PostUiLoad(UI* pui);
void SetUiUis(UI *pui, int uis);
void SetUiUPause(UI *pui, float uPause);
bool FCanShowGameplayBlot();
bool FCanDrawBinoc();
bool FCanDrawTv();
bool FCanRenderTv();
void PushUiActiveBlot(UI* pui, BLOT* pblot);
void PopUiActiveBlot(UI* pui);
int  FDebugmenuActive(UI* pui);
void UpdateUi(UI *pui);
void DrawUiSelf(UI* pui);
void DrawUiSplash(UI* pui);
void DrawUi(UI *pui);
void RenderUi(UI* pui);
void ResetUi(UI* pui);

extern UI g_ui;
extern const char *suckerPunchProd;
extern float RX_UiLegal;
extern float RY_UiLegal;
extern float R_UiPresents;
extern const char *g_aachzLegal[];
extern const char *g_aachzPresents[];

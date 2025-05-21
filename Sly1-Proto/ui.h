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
    UIS_Playing = 2,
    UIS_Pausing = 3,
    UIS_Paused = 4,
    UIS_Unpausing = 5,
    UIS_WorldMap = 6,
    UIS_Wiping = 7,
    UIS_Max = 8
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
void SetUiUis(UI *pui, UIS uis);
void SetUiUPause(UI *pui, float uPause);
void PushUiActiveBlot(UI* pui, BLOT* pblot);
void PopUiActiveBlot(UI* pui);
int  FDebugmenuActive(UI* pui);
void UpdateUi(UI *pui);
void DrawUi(UI *pui);
void ResetUi(UI* pui);

extern UI g_ui;
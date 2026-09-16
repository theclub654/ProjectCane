#pragma once
#include "blot.h"
#include "game.h"

inline constexpr int SAVE_SLOT_COUNT = 6;

enum SAVEMENUSTATE
{
    SAVE_MENU_STATE_Nil = 0,
    SAVE_MENU_STATE_Selecting = 1,
    SAVE_MENU_STATE_Closing = 2
};

struct SAVEBLOT : public BLOT
{
    int fSaveComplete;
};

struct VIDEOSAVESETTINGS
{
    int fogType;
    int msaaEnabled;
    int msaaSamples;
    int frameRate;
    int internalResolutionHeight;
    float drawDistance;
    int windowMode;
    int vsyncEnabled;
    int aspectMode;
};

struct SAVEDATA
{
    SAVEMENUSTATE state;
    float stateStartTime;
    GS saveData[SAVE_SLOT_COUNT];
    GS* pgsCurrentSave;
    GS* pgsAttractSave;
    GS* pgsSelectedSave;
    VIDEOSAVESETTINGS videoSettings[SAVE_SLOT_COUNT];
    bool hasVideoSettings[SAVE_SLOT_COUNT];
};

void StartupSaveBlot(SAVEBLOT* psaveblot);
void PostAutoSaveLoad(SAVEBLOT* psaveblot);
void SetSaveBlots(SAVEBLOT* psaveblot, BLOTS blots);
void DrawAutoSave(SAVEBLOT* psaveblot);

void StartupSaveData(SAVEDATA* psaveData);
bool SaveCurrentGameToDisk(SAVEDATA* psaveData);
bool DeleteSaveSlotFromDisk(SAVEDATA* psaveData, int slot);
void AutosaveCurrentGame(SAVEDATA* psaveData);
void SetSaveManagerState(SAVEDATA* psavedata, SAVEMENUSTATE state);
int  LoadCurrentSave(SAVEDATA* psaveData);

extern SAVEDATA g_saveData;
extern SAVEBLOT g_autosave;

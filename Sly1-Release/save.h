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
    float guiScale;
    int guiStyle;
    int keyboardBindings[BTN_MAX];
    int gamepadBindings[BTN_MAX];
    int disableControllerInputWhenUnfocused;
    int lastSaveSlot;
    int stereoEnabled;
    float musicVolume;
    float soundEffectsVolume;
    float dialogueVolume;
    int mouseCameraEnabled;
    int cutsceneSkipControlsEnabled;
};

struct SAVEDATA
{
    SAVEMENUSTATE state;
    float stateStartTime;
    GS saveData[SAVE_SLOT_COUNT];
    GS* pgsCurrentSave;
    GS* pgsAttractSave;
    GS* pgsSelectedSave;
};

void StartupSaveBlot(SAVEBLOT* psaveblot);
void PostAutoSaveLoad(SAVEBLOT* psaveblot);
void SetSaveBlots(SAVEBLOT* psaveblot, BLOTS blots);
void DrawAutoSave(SAVEBLOT* psaveblot);

void StartupSaveData(SAVEDATA* psaveData);
bool SaveSystemSettings();
// Options > Reset to Default: video, sound and control bindings back to the built-in defaults.
void ResetSystemSettingsToDefault();

// Per-setting defaults (changed-setting markers and single-setting reset).
enum SYSOPT
{
    SYSOPT_FrameRate = 0,
    SYSOPT_InternalResolution,
    SYSOPT_DrawDistance,
    SYSOPT_WindowMode,
    SYSOPT_Vsync,
    SYSOPT_AspectRatio,
    SYSOPT_Msaa,
    SYSOPT_Fog,
    SYSOPT_GuiScale,
    SYSOPT_GuiStyle,
    SYSOPT_Stereo,
    SYSOPT_DisableControllerUnfocused,
    SYSOPT_Max
};
bool FSystemOptionDefault(SYSOPT opt);
void ResetSystemOption(SYSOPT opt);
int DefaultKeyboardBinding(int button);
int DefaultGamepadBinding(int button);
// Display-change confirmation: snapshot and restore of the video/system settings.
VIDEOSAVESETTINGS CaptureCurrentVideoSettings();
void RestoreVideoSettings(const VIDEOSAVESETTINGS& settings);
// Copies all settings files to "Settings Backups\\<date_time>" next to the exe.
// pchzFolder receives that folder (relative to the exe) for display.
bool ExportSettingsBackup(char* pchzFolder, size_t cchFolder);
// Lists backups (newest first) as "Import: <date time>" labels; returns the count.
int ListSettingsBackups(char (*pachzNames)[64], int cMax);
// Copies backup [index] (from the last ListSettingsBackups) over the current
// settings files and applies video / sound / bindings. Other systems are
// reloaded by the caller.
bool ImportSettingsBackup(int index);
bool SaveCurrentGameToDisk(SAVEDATA* psaveData);
bool DeleteSaveSlotFromDisk(SAVEDATA* psaveData, int slot);
void AutosaveCurrentGame(SAVEDATA* psaveData);
void SetSaveManagerState(SAVEDATA* psavedata, SAVEMENUSTATE state);
int  LoadCurrentSave(SAVEDATA* psaveData);

extern SAVEDATA g_saveData;
extern SAVEBLOT g_autosave;

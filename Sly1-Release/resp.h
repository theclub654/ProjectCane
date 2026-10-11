#pragma once
#include "mods.h"
#include "menustyle.h"
#include "mouseinput.h"
#include "qol.h"

// -----------------------------------------------------------------------------
// Prompt groups
// -----------------------------------------------------------------------------

enum PRP : int
{
    PRP_Nil = -1,
    PRP_Basic = 0,
    PRP_Memcard = 1,
    PRP_Cd = 2,
    PRP_Max = 3
};

// -----------------------------------------------------------------------------
// Release response IDs
// -----------------------------------------------------------------------------

enum RESPK : int
{
    RESPK_Nil = -1,

    RESPK_Yes = 0,
    RESPK_No = 1,
    RESPK_Continue = 2,
    RESPK_ReturnToGame = 3,
    RESPK_Map = 4,
    RESPK_Options = 5,
    RESPK_NewGame = 6,
    RESPK_LoadGame = 7,
    RESPK_SaveGame = 8,
    RESPK_Quit = 9,
    RESPK_Slot1 = 10,
    RESPK_Slot2 = 11,
    RESPK_Slot3 = 12,
    RESPK_Vibration = 13,
    RESPK_CenterScreen = 14,
    RESPK_Music = 15,
    RESPK_Speakers = 16,
    RESPK_Back = 17,
    RESPK_RestartRace = 18,
    RESPK_RestartSprint = 19,
    RESPK_RestartLevel = 20,
    RESPK_ExitLevel = 21,
    RESPK_ExitToHideout = 22,
    RESPK_Controls = 23,
    RESPK_BinocInvert = 24,
    RESPK_TurretInvert = 25,
    RESPK_JetpackInvert = 26,
    RESPK_Msaa = 27,
    RESPK_FrameRate = 28,
    RESPK_Video = 29,
    RESPK_InternalResolution = 30,
    RESPK_Fog = 31,
    RESPK_DrawDistance = 32,
    RESPK_WindowMode = 33,
    RESPK_Vsync = 34,
    RESPK_AspectRatio = 35,
    RESPK_Slot4 = 36,
    RESPK_Slot5 = 37,
    RESPK_Slot6 = 38,
    RESPK_ExitToDesktop = 39,
    RESPK_GuiScale = 40,
	RESPK_CameraInvert = 41,
	RESPK_KeyboardMapping = 42,
	RESPK_ControllerMapping = 43,
	RESPK_KeyUp = 44,
	RESPK_KeyDown,
	RESPK_KeyLeft,
	RESPK_KeyRight,
	RESPK_KeySquare,
	RESPK_KeyCircle,
	RESPK_KeyCross,
	RESPK_KeyTriangle,
	RESPK_KeyStart,
	RESPK_KeySelect,
	RESPK_KeyL1,
	RESPK_KeyR1,
	RESPK_KeyL2,
	RESPK_KeyR2,
	RESPK_KeyL3,
	RESPK_KeyR3,
	RESPK_GuiStyle,
	RESPK_DisableControllerWhenUnfocused,
	RESPK_Sound,
	RESPK_MusicVolume,
	RESPK_SfxVolume,
	RESPK_DialogueVolume,
	RESPK_Dlss,
	RESPK_DlssPreset,
	RESPK_Mods,
	RESPK_ModEnable,
	RESPK_ModLoadState,
	RESPK_ModSaveState,
	RESPK_ModSlot,
	RESPK_ModFfSpeed,
	RESPK_ModBindSave,        // order matches MODACT
	RESPK_ModBindLoad,
	RESPK_ModBindPrevSlot,
	RESPK_ModBindNextSlot,
	RESPK_ModBindFfHold,
	RESPK_ModBindFfToggle,
	RESPK_FrameGen,
	RESPK_FpsCounter,
	RESPK_Fsr,
	RESPK_FsrSharpness,
	RESPK_Fov,
	RESPK_Anisotropy,
	RESPK_CameraSensitivity,
	RESPK_ModStatesMenu,         // 87 "Save States..."
	RESPK_ModTimerMenu,          // 88 "Speedrun Timer..."
	RESPK_ModCombosMenu,         // 89 "Button Combos..."
	RESPK_ModBindResetTimer,     // 90
	RESPK_TimerEnable,           // 91 .. 98 follow MODTIMEROPT order
	RESPK_TimerMode,
	RESPK_TimerLevel,
	RESPK_TimerSplit,
	RESPK_TimerMs,
	RESPK_TimerPosition,
	RESPK_TimerAutoStart,
	RESPK_TimerReset,
	RESPK_PauseOnFocusLoss,      // 99
	RESPK_MenuStyle,             // 100 "Customization..."
	RESPK_MsTextMenu,            // 101
	RESPK_MsBoxMenu,             // 102
	RESPK_MsBorderMenu,          // 103
	RESPK_MsHighlightMenu,       // 104
	RESPK_MsEffectsMenu,         // 105
	RESPK_MsReset,               // 106
	RESPK_MsOpt0,                // 107 .. one per MSOPT (menustyle.h)

	RESPK_MouseMenu = RESPK_MsOpt0 + MSOPT_Max,   // 144 "Mouse Settings..."
	RESPK_MouseOpt0,                               // 145 .. one per MOUSEOPT (mouseinput.h)

	RESPK_ResetDefaults = RESPK_MouseOpt0 + MOUSEOPT_Max,   // 153 Options > "Reset to Default"
	RESPK_BackupsMenu,           // 154 Options > "Import / Export Settings..."
	RESPK_ExportNow,             // 155 "Export Current Settings"
	RESPK_Backup0,               // 156 .. one per listed backup

	RESPK_KeyFfHold = RESPK_Backup0 + 8,   // 164 Mouse & Keyboard Mapping: mod / QOL hotkeys
	RESPK_KeyFfToggle,           // 165
	RESPK_KeyScreenshot,         // 166
	RESPK_ControllerMenu,        // 167 Controls > "Controller Settings"
	RESPK_GameplayMenu,          // 168 Options > "Gameplay"
	RESPK_HdrGame,               // 169 section headers (not selectable)
	RESPK_HdrSettings,
	RESPK_HdrData,
	RESPK_HdrCamera,
	RESPK_HdrDevices,
	RESPK_HdrGeneral,
	RESPK_HdrHud,
	RESPK_HdrSpeedrun,           // 176
	RESPK_HdrFirst = RESPK_HdrGame,
	RESPK_HdrLast = RESPK_HdrSpeedrun,

	RESPK_ButtonIcons = RESPK_HdrLast + 1,   // 177 Controller Settings > "Button Icons"

	// From ProjectCane 0.4.0. Mouse Camera is upstream's own mouse-look toggle;
	// ours (Controls > Mouse Settings) replaces it, so it is never listed.
	RESPK_MouseCamera,                       // 178 (unused)
	RESPK_CutsceneSkipControls,              // 179 Gameplay > "Dialog Skip Controls"
	RESPK_RetailTiming,                      // 180 Gameplay > "Retail Timing"

	RESPK_Max
};

// -----------------------------------------------------------------------------
// Release prompt IDs
// -----------------------------------------------------------------------------

enum PRK : int
{
    PRK_Nil = -1,

    PRK_PauseMenu = 0,
    PRK_GameOver = 1,
    PRK_QuitConfirm = 2,

    PRK_MemcardMissing = 3,
    PRK_MemcardFormatConfirm = 4,
    PRK_MemcardFormatting = 5,
    PRK_MemcardFormatError = 6,
    PRK_MemcardCreateConfirm = 7,
    PRK_MemcardCreating = 8,
    PRK_MemcardCreateError = 9,
    PRK_MemcardCardFull = 10,
    PRK_MemcardChooseSaveSlot = 11,
    PRK_MemcardSlotSaving = 12,
    PRK_MemcardSlotSaved = 13,
    PRK_MemcardOverwriteConfirm = 14,
    PRK_Unknown15 = 15,
    PRK_MemcardChooseNewSlot = 16,
    PRK_MemcardChooseLoadSlot = 17,
    PRK_MemcardEraseConfirm = 18,
    PRK_MemcardErasing = 19,
    PRK_MemcardAutoNotify = 20,
    PRK_Unknown21 = 21,
    PRK_Unknown22 = 22,

    PRK_OptionsMenu = 23,
    PRK_CenterScreen = 24,
    PRK_SpecialRetry = 25,
    PRK_TryAgain = 26,
    PRK_MtsComplete = 27,
    PRK_MtsExpired = 28,
    PRK_MtsBestTime = 29,
    PRK_MtsFailedBestTime = 30,
    PRK_ControlsMenu = 31,
    PRK_Unknown32 = 32,
    PRK_VideoMenu = 33,
	PRK_ExitToDesktopConfirm = 34,
	PRK_KeyboardMapping = 35,
	PRK_ControllerMapping = 36,
	PRK_SoundMenu = 37,

	PRK_ModsMenu = 38,
	PRK_ModsStates = 39,
	PRK_ModsTimer = 40,
	PRK_ModsCombos = 41,
	PRK_Custom = 42,
	PRK_CustText = 43,
	PRK_CustBox = 44,
	PRK_CustBorder = 45,
	PRK_CustHighlight = 46,
	PRK_CustEffects = 47,
	PRK_Mouse = 48,
	PRK_ResetConfirm = 49,
	PRK_ResetExport = 50,
	PRK_Backups = 51,
	PRK_ImportConfirm = 52,
	PRK_ControllerSettings = 53,
	PRK_VideoGraphics = 54,      // Video tabs: PRK_VideoMenu = Display
	PRK_VideoUpscaling = 55,
	PRK_Gameplay = 56,
	PRK_DisplayConfirm = 57,     // "Keep these display settings?" (auto-revert)
	PRK_Max = 58
};

// -----------------------------------------------------------------------------
// Prompt data structures
// -----------------------------------------------------------------------------

struct PRD
{
    const char* pchz;
    float rScaleTitle;
    float rScaleRespk;
    int fVertical;
    int crespk;
    RESPK* arespk;
};

struct RESPD
{
    int cpchz;
    const char* const* apchz;
};

// -----------------------------------------------------------------------------
// Prompt title strings
// -----------------------------------------------------------------------------

inline const char* pchzMemcardSaveCreate =
"There is no Memory Card (PS2) inserted in\n"
"Memory Card Slot 1\n\n"
"Sly Cooper requires 49 KB of free space\n\n"
"Please insert a Memory Card (PS2) containing sufficient space "
"or you will not be able to save your progress\n";

// The release has a separate prompt at PRK 4 before PRK_MemcardFormatting.
// Replace this text if the exact retail string is recovered later.
inline const char* pchzFormatMemcardConfirm =
"The Memory Card (PS2) in Memory Card Slot 1 is not formatted.\n\n"
"Do you want to format it?";

inline const char* pchzFormatMemcard =
"\nFormatting the Memory Card (PS2) in Memory Card Slot 1\n\n"
"Please do not turn off your system\n";

inline const char* pchzErrorFormatMemcard =
"An error occurred while formatting the Memory Card (PS2) "
"in Memory Card Slot 1\n\n"
"You will not be able to save your progress\n";

inline const char* pchzNoSaveFileFound =
"No Sly Cooper Save File on the Memory Card (PS2) inserted "
"in Memory Card Slot 1\n\n"
"Sly Cooper requires 49 KB of free space\n\n"
"If you do not create a Sly Cooper Save File then you will not "
"be able to save your progress\n\n"
"Do you want to create a Sly Cooper Save File?";

inline const char* pchzCreateSaveFile =
"\nCreating the Sly Cooper Save File on the System "
"in Memory Card Slot 1\n\n"
"Please do not turn off your system or remove the Memory Card (PS2)\n";

inline const char* pchzErrorCreatingSave =
"An error occurred while creating the Sly Cooper Save File on "
"the Memory Card (PS2) in Memory Card Slot 1\n\n"
"Your progress is not being saved\n";

inline const char* pchzNotEnoughSpaceSave =
"The Memory Card (PS2) inserted in Memory Card Slot 1 does not "
"have enough free space to create a Sly Cooper Save File\n\n"
"Sly Cooper requires 49 KB of free space\n\n"
"Please insert a Memory Card (PS2) containing sufficient space "
"or you will not be able to save your progress\n";

inline const char* pchzSaving =
"\nSaving...\n\n"
"Please do not turn off your system or remove the Memory Card (PS2)\n";

inline const char* pchzErrorWriteSave =
"An error occurred while writing to the Sly Cooper Save File on "
"the Memory Card (PS2) in Memory Card Slot 1\n\n"
"Your progress is not being saved\n";

inline const char* pchzErasingSave =
"\nErasing...\n\n"
"Please do not turn off your system\n";

inline const char* pchzIconAppear =
"When the following icon appears,\n"
"your progress is being saved\n\n"
"^15&2a&.^.\n\n"
"While this icon is on screen, please\n"
"do not turn off your system\n";

inline const char* pchzBeatMts =
"CONGRATULATIONS!\n\n"
"You have beaten the\n"
"Master Thief Sprint!\n\n"
"Return to this level at any time\n"
"to hear the Bonus Commentary!";

inline const char* pchzMtsExpired =
"TIME EXPIRED\n\n"
"Would you like to retry\n"
"the Master Thief Sprint?\n\n";

inline const char* pchzMtsTimeBeat =
"CONGRATULATIONS!\n\n"
"You beat your best\n"
"Master Thief Sprint time!\n\n"
"Try Again?";

inline const char* pchzFailedTimeBeat =
"You didn't beat your best\n"
"Master Thief Sprint time\n\n"
"Try again?";

// -----------------------------------------------------------------------------
// Fixed response arrays
// -----------------------------------------------------------------------------

inline RESPK s_arespkYesNo[2] =
{
    RESPK_Yes,
    RESPK_No
};

inline RESPK s_arespkContinue[1] =
{
    RESPK_Continue
};

inline RESPK s_arespkSlots[7] =
{
    RESPK_Slot1,
    RESPK_Slot2,
    RESPK_Slot3,
    RESPK_Slot4,
    RESPK_Slot5,
    RESPK_Slot6,
    RESPK_Back
};

inline const RESPK s_arespkControlsMenuAll[10] =
{
	RESPK_HdrCamera,
	RESPK_CameraInvert,
    RESPK_BinocInvert,
    RESPK_TurretInvert,
    RESPK_JetpackInvert,
	RESPK_HdrDevices,
	RESPK_MouseMenu,
	RESPK_ControllerMenu,
	RESPK_KeyboardMapping,
    RESPK_Back
};

inline RESPK s_arespkControlsMenu[12]{};

// Controls > Controller Settings (Controller Mapping is left out when no
// gamepad is connected; built in SetPromptPrk).
inline const RESPK s_arespkControllerMenuAll[5] =
{
	RESPK_CameraSensitivity,
	RESPK_ButtonIcons,
	RESPK_DisableControllerWhenUnfocused,
	RESPK_ControllerMapping,
	RESPK_Back
};

inline RESPK s_arespkControllerMenu[5]{};

inline RESPK s_arespkKeyboardMapping[17] =
{
	RESPK_KeyUp, RESPK_KeyDown, RESPK_KeyLeft, RESPK_KeyRight,
	RESPK_KeySquare, RESPK_KeyCircle, RESPK_KeyCross, RESPK_KeyTriangle,
	RESPK_KeyStart, RESPK_KeySelect, RESPK_KeyL1, RESPK_KeyR1,
	RESPK_KeyL2, RESPK_KeyR2, RESPK_KeyL3, RESPK_KeyR3,
	RESPK_Back
};

// Mouse & Keyboard Mapping: the 16 game buttons plus the PC-only hotkeys.
inline RESPK s_arespkKbMouseMapping[20] =
{
	RESPK_KeyUp, RESPK_KeyDown, RESPK_KeyLeft, RESPK_KeyRight,
	RESPK_KeySquare, RESPK_KeyCircle, RESPK_KeyCross, RESPK_KeyTriangle,
	RESPK_KeyStart, RESPK_KeySelect, RESPK_KeyL1, RESPK_KeyR1,
	RESPK_KeyL2, RESPK_KeyR2, RESPK_KeyL3, RESPK_KeyR3,
	RESPK_KeyFfHold, RESPK_KeyFfToggle, RESPK_KeyScreenshot,
	RESPK_Back
};

// -----------------------------------------------------------------------------
// Pause menu
// -----------------------------------------------------------------------------

inline const RESPK s_arespkPauseMenuAll[9] =
{
    RESPK_ReturnToGame,
    RESPK_RestartRace,
    RESPK_RestartSprint,
    RESPK_RestartLevel,
    RESPK_Map,
    RESPK_Options,
    RESPK_ExitLevel,
    RESPK_Quit,
    RESPK_ExitToDesktop
};

inline RESPK s_arespkPauseMenu[9]{};

// -----------------------------------------------------------------------------
// Options menu
// -----------------------------------------------------------------------------

inline const RESPK s_arespkOptionsMenuAll[] =
{
    RESPK_HdrGame,
    RESPK_NewGame,
    RESPK_LoadGame,
    RESPK_SaveGame,
    RESPK_HdrSettings,
    RESPK_GameplayMenu,
    RESPK_Video,
    RESPK_Sound,
    RESPK_Controls,
    RESPK_Mods,
    RESPK_MenuStyle,
    RESPK_HdrData,
    RESPK_BackupsMenu,
    RESPK_ResetDefaults,
    RESPK_Back
};

inline RESPK s_arespkOptionsMenu[std::size(s_arespkOptionsMenuAll)]{};

// Video tabs (built in SetPromptPrk; rows that can't apply right now are left out).
inline const RESPK s_arespkVideoDisplayAll[] =
{
    RESPK_WindowMode, RESPK_AspectRatio, RESPK_InternalResolution, RESPK_FrameRate, RESPK_Vsync, RESPK_Back
};
inline const RESPK s_arespkVideoGraphicsAll[] =
{
    RESPK_DrawDistance, RESPK_Fov, RESPK_Anisotropy, RESPK_Msaa, RESPK_Fog, RESPK_Back
};
inline const RESPK s_arespkVideoUpscalingAll[] =
{
    RESPK_Dlss, RESPK_DlssPreset, RESPK_FrameGen, RESPK_Fsr, RESPK_FsrSharpness, RESPK_Back
};
inline RESPK s_arespkVideoDisplay[std::size(s_arespkVideoDisplayAll)]{};
inline RESPK s_arespkVideoGraphics[std::size(s_arespkVideoGraphicsAll)]{};
inline RESPK s_arespkVideoUpscaling[std::size(s_arespkVideoUpscalingAll)]{};

// Options > Gameplay
inline RESPK s_arespkGameplayMenu[] =
{
    RESPK_HdrGeneral, RESPK_PauseOnFocusLoss, RESPK_Vibration,
    RESPK_HdrHud, RESPK_FpsCounter, RESPK_GuiStyle, RESPK_GuiScale,
    RESPK_HdrSpeedrun, RESPK_RetailTiming, RESPK_TimerEnable, RESPK_CutsceneSkipControls,
    RESPK_Back
};

// -----------------------------------------------------------------------------
// Dynamic response buffers
// -----------------------------------------------------------------------------

inline char g_achzRespk9[64]{};
inline char g_achzRespk10[64]{};
inline char g_achzRespk11[64]{};
inline char g_achzRespk12[64]{};
inline char g_achzSaveSlot4[64]{};
inline char g_achzSaveSlot5[64]{};
inline char g_achzSaveSlot6[64]{};
inline char g_achzRespk13[64]{};
inline char g_achzRespk15[64]{};
inline char g_achzRespk16[64]{};
inline char g_achzRespk24[64]{};
inline char g_achzRespk25[64]{};
inline char g_achzRespk26[64]{};
inline char g_achzRespk27[64]{};
inline char g_achzRespk28[64]{};
inline char g_achzRespk30[64]{};
inline char g_achzRespk31[64]{};
inline char g_achzRespk32[64]{};
inline char g_achzRespk33[64]{};
inline char g_achzRespk34[64]{};
inline char g_achzRespk35[64]{};
inline char g_achzRespk40[64]{};
inline char g_achzRespk41[64]{};
inline char g_achzRespk60[64]{};
inline char g_achzRespk61[64]{};
inline char g_achzRespk63[64]{};
inline char g_achzRespk64[64]{};
inline char g_achzRespk65[64]{};
inline char g_achzRespk66[64]{};
inline char g_achzRespk67[64]{};
inline char g_achzRespk80[64]{};
inline char g_achzRespk81[64]{};
inline char g_achzRespk82[64]{};
inline char g_achzRespk83[64]{};
inline char g_achzRespk84[64]{};
inline char g_achzRespk85[64]{};
inline char g_achzRespk86[64]{};
inline char g_achzRespk99[64]{};
inline char g_achzButtonIcons[64]{};
// "Keep these display settings?" title (countdown updated every frame).
inline char g_achzDisplayConfirm[96]{ "Keep these display settings?" };
// Options > Import / Export Settings: one label per listed backup.
inline char g_achzBackup[8][64]{};
// Mouse & Keyboard Mapping labels for the PC hotkeys (FF hold, FF toggle, screenshot).
inline char g_aachzExtraBindings[3][64]{};
// Built by SetPromptPrk: Export, up to 8 backups, Back.
inline RESPK s_arespkBackupsMenu[10]{};
// Gameplay > dialog skip controls (ProjectCane 0.4.0 option).
inline char g_achzCutsceneSkip[64]{};
// Gameplay > Retail Timing (PS2 59.94 Hz game updates).
inline char g_achzRetailTiming[64]{};
inline char g_aachzKeyboardBindings[BTN_MAX][64]{};

// -----------------------------------------------------------------------------
// Variant response strings
// -----------------------------------------------------------------------------

inline const char* const s_apchzRespk9[2] =
{
    "Quit Game",
    "Save and Quit"
};

inline const char* const s_apchzRespk13[2] =
{
    "Vibrations: On",
    "Vibrations: Off"
};

inline const char* const s_apchzRespk15[2] =
{
    "Music: On",
    "Music: Off"
};

inline const char* const s_apchzRespk16[2] =
{
    "Speakers: Stereo",
    "Speakers: Mono"
};

inline const char* const s_apchzRespk24[2] =
{
    "Binoc-u-com: &2(&. Up = Look Down",
    "Binoc-u-com: &2(&. Up = Look Up"
};

inline const char* const s_apchzRespk25[2] =
{
    "Turrets: &2(&. Up = Aim Down",
    "Turrets: &2(&. Up = Aim Up"
};

inline const char* const s_apchzRespk26[2] =
{
    "Jet Pack: &2(&. Up = Fly Down",
    "Jet Pack: &2(&. Up = Fly Up"
};

inline RESPK s_arespkSoundMenu[] =
{
	RESPK_Speakers, RESPK_MusicVolume, RESPK_SfxVolume,
	RESPK_DialogueVolume, RESPK_Back
};

// Mods tabs: General / Save States / Timer / Combos.
inline RESPK s_arespkModsMenu[] =
{
	RESPK_ModEnable, RESPK_ModFfSpeed,
	RESPK_Back
};

inline RESPK s_arespkModsStatesMenu[] =
{
	RESPK_ModLoadState, RESPK_ModSaveState, RESPK_ModSlot, RESPK_Back
};

inline RESPK s_arespkModsTimerMenu[] =
{
	RESPK_TimerEnable, RESPK_TimerMode, RESPK_TimerLevel, RESPK_TimerSplit,
	RESPK_TimerMs, RESPK_TimerPosition, RESPK_TimerAutoStart, RESPK_TimerReset,
	RESPK_Back
};


// ---- Customization menus (menustyle.h) ----
// Customization tabs: General / Text / Background / Border / Highlight / Effects.
inline RESPK s_arespkCustomMenu[] =
{
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Theme),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Layout),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_PromptBar),
	RESPK_MsReset,
	RESPK_Back
};

inline RESPK s_arespkCustTextMenu[] =
{
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Font),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_TextSize),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_TitleSize),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_TextR),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_TextG),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_TextB),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_TitleColor),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_TextShadow),
	RESPK_Back
};

inline RESPK s_arespkCustBoxMenu[] =
{
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BoxStyle),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BoxR),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BoxG),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BoxB),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BoxOpacity),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BoxRainbow),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BoxWidth),
	RESPK_Back
};

inline RESPK s_arespkCustBorderMenu[] =
{
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Border),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BorderR),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BorderG),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BorderB),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BorderOpacity),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BorderThickness),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BorderRainbow),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_BorderPulse),
	RESPK_Back
};

inline RESPK s_arespkCustHighlightMenu[] =
{
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_HlR),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_HlG),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_HlB),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_HlPulse),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_HlBounce),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_HlCursor),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Divider),
	RESPK_Back
};

inline RESPK s_arespkCustEffectsMenu[] =
{
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Position),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_OpenAnim),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_ScreenDim),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Sounds),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Brightness),
	static_cast<RESPK>(RESPK_MsOpt0 + MSOPT_Contrast),
	RESPK_Back
};

inline RESPK s_arespkMouseMenu[] =
{
	static_cast<RESPK>(RESPK_MouseOpt0 + MOUSEOPT_Camera),
	static_cast<RESPK>(RESPK_MouseOpt0 + MOUSEOPT_Sensitivity),
	static_cast<RESPK>(RESPK_MouseOpt0 + MOUSEOPT_AimSensitivity),
	static_cast<RESPK>(RESPK_MouseOpt0 + MOUSEOPT_InvertX),
	static_cast<RESPK>(RESPK_MouseOpt0 + MOUSEOPT_InvertY),
	static_cast<RESPK>(RESPK_MouseOpt0 + MOUSEOPT_RawInput),
	static_cast<RESPK>(RESPK_MouseOpt0 + MOUSEOPT_WheelZoom),
	static_cast<RESPK>(RESPK_MouseOpt0 + MOUSEOPT_Menus),
	RESPK_Back
};

// Order matches MODACT (UpdatePromptActive relies on it while capturing).
inline RESPK s_arespkModsCombosMenu[] =
{
	RESPK_ModBindSave, RESPK_ModBindLoad, RESPK_ModBindPrevSlot, RESPK_ModBindNextSlot,
	RESPK_ModBindFfHold, RESPK_ModBindFfToggle, RESPK_ModBindResetTimer, RESPK_Back
};

inline const char* const s_apchzRespk41[2] =
{
	"Camera: &2(&. Right = Turn Left",
	"Camera: &2(&. Right = Turn Right"
};

// -----------------------------------------------------------------------------
// Response string lookup table
// -----------------------------------------------------------------------------

inline const char* s_mprespkachz[RESPK_Max] =
{
    "Yes",                    // 0  RESPK_Yes
    "No",                     // 1  RESPK_No
    "Continue",               // 2  RESPK_Continue
    "Return to Game",         // 3  RESPK_ReturnToGame
    "View Map",               // 4  RESPK_Map
    "Options",             // 5  RESPK_Options
    "New Game",            // 6  RESPK_NewGame
    "Load Game",           // 7  RESPK_LoadGame
    "Save Game",           // 8  RESPK_SaveGame
    g_achzRespk9,             // 9  RESPK_Quit
    g_achzRespk10,            // 10 RESPK_Slot1
    g_achzRespk11,            // 11 RESPK_Slot2
    g_achzRespk12,            // 12 RESPK_Slot3
    g_achzRespk13,            // 13 RESPK_Vibration
    "Center Screen",       // 14 RESPK_CenterScreen
    g_achzRespk15,            // 15 RESPK_Music
    g_achzRespk16,            // 16 RESPK_Speakers
    "Back",                   // 17 RESPK_Back
    "Restart Race",           // 18 RESPK_RestartRace
    "Restart Sprint",         // 19 RESPK_RestartSprint
    "Restart Level",          // 20 RESPK_RestartLevel
    "Exit Level",             // 21 RESPK_ExitLevel
    "Exit to Hideout",        // 22 RESPK_ExitToHideout
    "Controls",            // 23 RESPK_Controls
    g_achzRespk24,            // 24 RESPK_BinocInvert
    g_achzRespk25,            // 25 RESPK_TurretInvert
    g_achzRespk26,            // 26 RESPK_JetpackInvert
    g_achzRespk27,            // 27 RESPK_Msaa
    g_achzRespk28,            // 28 RESPK_FrameRate
    "Video",              // 29 RESPK_Video
    g_achzRespk30,            // 30 RESPK_InternalResolution
    g_achzRespk31,            // 31 RESPK_Fog
    g_achzRespk32,            // 32 RESPK_DrawDistance
    g_achzRespk33,            // 33 RESPK_WindowMode
    g_achzRespk34,            // 34 RESPK_Vsync
    g_achzRespk35,            // 35 RESPK_AspectRatio
    g_achzSaveSlot4,          // 36 RESPK_Slot4
    g_achzSaveSlot5,          // 37 RESPK_Slot5
    g_achzSaveSlot6,          // 38 RESPK_Slot6
    "Exit to Desktop",        // 39 RESPK_ExitToDesktop
    g_achzRespk40,            // 40 RESPK_GuiScale
	g_achzRespk41,             // 41 RESPK_CameraInvert
	"Mouse && Keyboard Mapping", // 42 RESPK_KeyboardMapping ("&&" = literal '&')
	"Controller Mapping",  // 43 RESPK_ControllerMapping
	g_aachzKeyboardBindings[0], g_aachzKeyboardBindings[1],
	g_aachzKeyboardBindings[2], g_aachzKeyboardBindings[3],
	g_aachzKeyboardBindings[4], g_aachzKeyboardBindings[5],
	g_aachzKeyboardBindings[6], g_aachzKeyboardBindings[7],
	g_aachzKeyboardBindings[8], g_aachzKeyboardBindings[9],
	g_aachzKeyboardBindings[10], g_aachzKeyboardBindings[11],
	g_aachzKeyboardBindings[12], g_aachzKeyboardBindings[13],
	g_aachzKeyboardBindings[14], g_aachzKeyboardBindings[15],
	g_achzRespk60,            // 60 RESPK_GuiStyle
	g_achzRespk61,            // 61 RESPK_DisableControllerWhenUnfocused
	"Sound",              // 62 RESPK_Sound
	g_achzRespk63, g_achzRespk64, g_achzRespk65,
	g_achzRespk66,            // 66 RESPK_Dlss
	g_achzRespk67,            // 67 RESPK_DlssPreset
	"Mods",                // 68 RESPK_Mods
	g_achzModsMenu[0],        // 69 RESPK_ModEnable
	g_achzModsMenu[1],        // 70 RESPK_ModLoadState
	g_achzModsMenu[2],        // 71 RESPK_ModSaveState
	g_achzModsMenu[3],        // 72 RESPK_ModSlot
	g_achzModsMenu[4],        // 73 RESPK_ModFfSpeed
	g_achzModsMenu[5], g_achzModsMenu[6], g_achzModsMenu[7],   // 74-76 bindings
	g_achzModsMenu[8], g_achzModsMenu[9], g_achzModsMenu[10],  // 77-79 bindings
	g_achzRespk80,            // 80 RESPK_FrameGen
	g_achzRespk81,            // 81 RESPK_FpsCounter
	g_achzRespk82,            // 82 RESPK_Fsr
	g_achzRespk83,            // 83 RESPK_FsrSharpness
	g_achzRespk84,            // 84 RESPK_Fov
	g_achzRespk85,            // 85 RESPK_Anisotropy
	g_achzRespk86,            // 86 RESPK_CameraSensitivity
	"Save States",         // 87 RESPK_ModStatesMenu
	"Speedrun Timer",      // 88 RESPK_ModTimerMenu
	"Button Combos",       // 89 RESPK_ModCombosMenu
	g_achzModsMenu[5 + MODACT_ResetTimer],              // 90 RESPK_ModBindResetTimer
	g_achzModsMenu[kModsMenuTimer + 0],                 // 91 RESPK_TimerEnable
	g_achzModsMenu[kModsMenuTimer + 1],                 // 92 RESPK_TimerMode
	g_achzModsMenu[kModsMenuTimer + 2],                 // 93 RESPK_TimerLevel
	g_achzModsMenu[kModsMenuTimer + 3],                 // 94 RESPK_TimerSplit
	g_achzModsMenu[kModsMenuTimer + 4],                 // 95 RESPK_TimerMs
	g_achzModsMenu[kModsMenuTimer + 5],                 // 96 RESPK_TimerPosition
	g_achzModsMenu[kModsMenuTimer + 6],                 // 97 RESPK_TimerAutoStart
	g_achzModsMenu[kModsMenuTimer + 7],                 // 98 RESPK_TimerReset
	g_achzRespk99,            // 99 RESPK_PauseOnFocusLoss
	"Customization",          // 100
	"Text",                   // 101
	"Background",             // 102
	"Border",                 // 103
	"Highlight",              // 104
	"Effects",                // 105
	"Reset to Default",          // 106
	g_achzMenuStyle[0],          // 107
	g_achzMenuStyle[1],          // 108
	g_achzMenuStyle[2],          // 109
	g_achzMenuStyle[3],          // 110
	g_achzMenuStyle[4],          // 111
	g_achzMenuStyle[5],          // 112
	g_achzMenuStyle[6],          // 113
	g_achzMenuStyle[7],          // 114
	g_achzMenuStyle[8],          // 115
	g_achzMenuStyle[9],          // 116
	g_achzMenuStyle[10],         // 117
	g_achzMenuStyle[11],         // 118
	g_achzMenuStyle[12],         // 119
	g_achzMenuStyle[13],         // 120
	g_achzMenuStyle[14],         // 121
	g_achzMenuStyle[15],         // 122
	g_achzMenuStyle[16],         // 123
	g_achzMenuStyle[17],         // 124
	g_achzMenuStyle[18],         // 125
	g_achzMenuStyle[19],         // 126
	g_achzMenuStyle[20],         // 127
	g_achzMenuStyle[21],         // 128
	g_achzMenuStyle[22],         // 129
	g_achzMenuStyle[23],         // 130
	g_achzMenuStyle[24],         // 131
	g_achzMenuStyle[25],         // 132
	g_achzMenuStyle[26],         // 133
	g_achzMenuStyle[27],         // 134
	g_achzMenuStyle[28],         // 135
	g_achzMenuStyle[29],         // 136
	g_achzMenuStyle[30],         // 137
	g_achzMenuStyle[31],         // 138
	g_achzMenuStyle[32],         // 139
	g_achzMenuStyle[33],         // 140
	g_achzMenuStyle[34],         // 141
	g_achzMenuStyle[35],         // 142
	g_achzMenuStyle[36],         // 143
	g_achzMenuStyle[37],         // MSOPT_Layout
	g_achzMenuStyle[38],         // MSOPT_PromptBar
	"Mouse Settings",         // 144 RESPK_MouseMenu
	g_achzMouseOpt[0],           // 145
	g_achzMouseOpt[1],           // 146
	g_achzMouseOpt[2],           // 147
	g_achzMouseOpt[3],           // 148
	g_achzMouseOpt[4],           // 149
	g_achzMouseOpt[5],           // 150
	g_achzMouseOpt[6],           // 151
	g_achzMouseOpt[7],           // 152
	"Reset to Default",          // 153 RESPK_ResetDefaults
	"Import / Export Settings", // 154 RESPK_BackupsMenu
	"Export Current Settings",   // 155 RESPK_ExportNow
	g_achzBackup[0],             // 156
	g_achzBackup[1],
	g_achzBackup[2],
	g_achzBackup[3],
	g_achzBackup[4],
	g_achzBackup[5],
	g_achzBackup[6],
	g_achzBackup[7],             // 163
	g_aachzExtraBindings[0],     // 164 RESPK_KeyFfHold
	g_aachzExtraBindings[1],     // 165 RESPK_KeyFfToggle
	g_aachzExtraBindings[2],     // 166 RESPK_KeyScreenshot
	"Controller Settings",       // 167 RESPK_ControllerMenu
	"Gameplay",                  // 168 RESPK_GameplayMenu
	"Game",                      // 169 RESPK_HdrGame
	"Settings",                  // RESPK_HdrSettings
	"Data",                      // RESPK_HdrData
	"Camera",                    // RESPK_HdrCamera
	"Devices",                   // RESPK_HdrDevices
	"General",                   // RESPK_HdrGeneral
	"HUD",                       // RESPK_HdrHud
	"Speedrun",                  // 176 RESPK_HdrSpeedrun
	g_achzButtonIcons,           // 177 RESPK_ButtonIcons
	"Mouse Camera",              // 178 RESPK_MouseCamera (unused)
	g_achzCutsceneSkip,         // 179 RESPK_CutsceneSkipControls
	g_achzRetailTiming,         // 180 RESPK_RetailTiming
};
static_assert(MSOPT_Max == 39, "update s_mprespkachz for MSOPT");
static_assert(MOUSEOPT_Max == 8, "update s_mprespkachz for MOUSEOPT");

// -----------------------------------------------------------------------------
// Response variant metadata
// -----------------------------------------------------------------------------

inline RESPD s_arespd[RESPK_Max] =
{
    { 0, nullptr },          // 0  RESPK_Yes
    { 0, nullptr },          // 1  RESPK_No
    { 0, nullptr },          // 2  RESPK_Continue
    { 0, nullptr },          // 3  RESPK_ReturnToGame
    { 0, nullptr },          // 4  RESPK_Map
    { 0, nullptr },          // 5  RESPK_Options
    { 0, nullptr },          // 6  RESPK_NewGame
    { 0, nullptr },          // 7  RESPK_LoadGame
    { 0, nullptr },          // 8  RESPK_SaveGame
    { 2, s_apchzRespk9 },    // 9  RESPK_Quit
    { 0, nullptr },          // 10 RESPK_Slot1
    { 0, nullptr },          // 11 RESPK_Slot2
    { 0, nullptr },          // 12 RESPK_Slot3
    { 2, s_apchzRespk13 },   // 13 RESPK_Vibration
    { 0, nullptr },          // 14 RESPK_CenterScreen
    { 2, s_apchzRespk15 },   // 15 RESPK_Music
    { 2, s_apchzRespk16 },   // 16 RESPK_Speakers
    { 0, nullptr },          // 17 RESPK_Back
    { 0, nullptr },          // 18 RESPK_RestartRace
    { 0, nullptr },          // 19 RESPK_RestartSprint
    { 0, nullptr },          // 20 RESPK_RestartLevel
    { 0, nullptr },          // 21 RESPK_ExitLevel
    { 0, nullptr },          // 22 RESPK_ExitToHideout
    { 0, nullptr },          // 23 RESPK_Controls
    { 2, s_apchzRespk24 },   // 24 RESPK_BinocInvert
    { 2, s_apchzRespk25 },   // 25 RESPK_TurretInvert
    { 2, s_apchzRespk26 },   // 26 RESPK_JetpackInvert
    { 0, nullptr },          // 27 RESPK_Msaa
    { 0, nullptr },          // 28 RESPK_FrameRate
    { 0, nullptr },          // 29 RESPK_Video
    { 0, nullptr },          // 30 RESPK_InternalResolution
    { 0, nullptr },          // 31 RESPK_Fog
    { 0, nullptr },          // 32 RESPK_DrawDistance
    { 0, nullptr },          // 33 RESPK_WindowMode
    { 0, nullptr },          // 34 RESPK_Vsync
    { 0, nullptr },          // 35 RESPK_AspectRatio
    { 0, nullptr },          // 36 RESPK_Slot4
    { 0, nullptr },          // 37 RESPK_Slot5
    { 0, nullptr },          // 38 RESPK_Slot6
    { 0, nullptr },          // 39 RESPK_ExitToDesktop
    { 0, nullptr },          // 40 RESPK_GuiScale
	{ 2, s_apchzRespk41 },    // 41 RESPK_CameraInvert
	{ 0, nullptr },           // 42 RESPK_KeyboardMapping
	{ 0, nullptr },           // 43 RESPK_ControllerMapping
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr },           // 60 RESPK_GuiStyle
	{ 0, nullptr },           // 61 RESPK_DisableControllerWhenUnfocused
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr },           // 66 RESPK_Dlss
	{ 0, nullptr },           // 67 RESPK_DlssPreset
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },   // 68-71 mods
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },   // 72-75 mods
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },   // 76-79 mods
	{ 0, nullptr }, { 0, nullptr },                                   // 80-81 frame gen, fps
	{ 0, nullptr }, { 0, nullptr },                                   // 82-83 fsr, fsr sharpness
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr },                   // 84-86 fov, aniso, sensitivity
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },   // 87-90 mods submenus, reset binding
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },   // 91-94 timer
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },   // 95-98 timer
	{ 0, nullptr },                                                   // 99 pause on focus loss
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
	{ 0, nullptr }, { 0, nullptr }, { 0, nullptr },                   // 178-180
};

// -----------------------------------------------------------------------------
// Prompt descriptor table
//
// There must be exactly one entry for every numeric PRK from 0 through 32.
// -----------------------------------------------------------------------------

inline PRD s_mpprkprd[PRK_Max] =
{
    { "Paused", 1.0f, 1.0f, 1, 9, s_arespkPauseMenu },                                         // 0  PRK_PauseMenu
    { "Game Over. Try again?", 1.0f, 1.0f, 0, 2, s_arespkYesNo },                              // 1  PRK_GameOver
    { "Really Quit?", 1.0f, 1.0f, 0, 2, s_arespkYesNo },                                      // 2  PRK_QuitConfirm
    { pchzMemcardSaveCreate, 0.7f, 1.0f, 0, 2, s_arespkYesNo },                               // 3  PRK_MemcardMissing
    { pchzFormatMemcardConfirm, 0.7f, 1.0f, 0, 2, s_arespkYesNo },                             // 4  PRK_MemcardFormatConfirm
    { pchzFormatMemcard, 0.7f, 1.0f, 1, 0, nullptr },                                         // 5  PRK_MemcardFormatting
    { pchzErrorFormatMemcard, 0.7f, 1.0f, 1, 1, s_arespkContinue },                            // 6  PRK_MemcardFormatError
    { pchzNoSaveFileFound, 0.7f, 1.0f, 0, 2, s_arespkYesNo },                                 // 7  PRK_MemcardCreateConfirm
    { pchzCreateSaveFile, 0.7f, 1.0f, 1, 0, nullptr },                                        // 8  PRK_MemcardCreating
    { pchzErrorCreatingSave, 0.7f, 1.0f, 1, 1, s_arespkContinue },                            // 9  PRK_MemcardCreateError
    { pchzNotEnoughSpaceSave, 0.7f, 1.0f, 1, 1, s_arespkContinue },                           // 10 PRK_MemcardCardFull
    { "Save Game", 1.0f, 0.7f, 1, 7, s_arespkSlots },                                         // 11 PRK_MemcardChooseSaveSlot
    { pchzSaving, 0.7f, 1.0f, 1, 0, nullptr },                                                // 12 PRK_MemcardSlotSaving
    { "Game Saved", 1.0f, 1.0f, 1, 1, s_arespkContinue },                                     // 13 PRK_MemcardSlotSaved
    { "Do you want to overwrite this game?", 0.7f, 1.0f, 0, 2, s_arespkYesNo },               // 14 PRK_MemcardOverwriteConfirm
    { pchzErrorWriteSave, 0.7f, 1.0f, 1, 1, s_arespkContinue },                               // 15 PRK_Unknown15 (write error)
    { "New Game", 1.0f, 0.7f, 1, 7, s_arespkSlots },                                          // 16 PRK_MemcardChooseNewSlot
    { "Load Game", 1.0f, 0.7f, 1, 7, s_arespkSlots },                                         // 17 PRK_MemcardChooseLoadSlot
    { "Do you want to erase this game?", 0.7f, 1.0f, 0, 2, s_arespkYesNo },                   // 18 PRK_MemcardEraseConfirm
    { pchzErasingSave, 0.7f, 1.0f, 1, 0, nullptr },                                           // 19 PRK_MemcardErasing
    { pchzIconAppear, 0.7f, 1.0f, 1, 1, s_arespkContinue },                                   // 20 PRK_MemcardAutoNotify
    { nullptr, 1.0f, 1.0f, 1, 0, nullptr },                                                   // 21 PRK_Unknown21
    { nullptr, 1.0f, 1.0f, 1, 0, nullptr },                                                   // 22 PRK_Unknown22
    { "Options", 1.0f, 1.0f, 1, static_cast<int>(std::size(s_arespkOptionsMenu)), s_arespkOptionsMenu }, // 23 PRK_OptionsMenu
    { "&2T&. : Cancel\n&2+&. : Move\n&2X&. : Accept", 0.8f, 1.0f, 1, 0, nullptr },            // 24 PRK_CenterScreen
    { "Try again?", 1.0f, 1.0f, 0, 2, s_arespkYesNo },                                        // 25 PRK_SpecialRetry
    { "Try again?", 1.0f, 1.0f, 0, 2, s_arespkYesNo },                                        // 26 PRK_TryAgain
    { pchzBeatMts, 0.8f, 1.0f, 0, 1, s_arespkContinue },                                      // 27 PRK_MtsComplete
    { pchzMtsExpired, 0.8f, 1.0f, 0, 2, s_arespkYesNo },                                      // 28 PRK_MtsExpired
    { pchzMtsTimeBeat, 0.8f, 1.0f, 0, 2, s_arespkYesNo },                                     // 29 PRK_MtsBestTime
    { pchzFailedTimeBeat, 0.8f, 1.0f, 0, 2, s_arespkYesNo },                                  // 30 PRK_MtsFailedBestTime
    { "Controls", 1.0f, 0.75f, 1, static_cast<int>(std::size(s_arespkControlsMenu)), s_arespkControlsMenu }, // 31 PRK_ControlsMenu
    { nullptr, 1.0f, 1.0f, 1, 0, nullptr },                                                   // 32 PRK_Unknown32
	{ "Video", 1.0f, 0.8f, 1, 0, s_arespkVideoDisplay },                                    // 33 PRK_VideoMenu (Display tab, built in SetPromptPrk)
	{ "Are you sure?", 1.0f, 1.0f, 0, 2, s_arespkYesNo },                                     // 34 PRK_ExitToDesktopConfirm
	{ "Mouse && Keyboard Mapping", 0.8f, 0.55f, 1, static_cast<int>(std::size(s_arespkKbMouseMapping)), s_arespkKbMouseMapping }, // 35
	{ "Controller Mapping", 0.8f, 0.55f, 1, static_cast<int>(std::size(s_arespkKeyboardMapping)), s_arespkKeyboardMapping }, // 36
	{ "Sound", 1.0f, 0.8f, 1, static_cast<int>(std::size(s_arespkSoundMenu)), s_arespkSoundMenu }, // 37
	{ "Mods", 1.0f, 0.85f, 1, static_cast<int>(std::size(s_arespkModsMenu)), s_arespkModsMenu }, // 38 PRK_ModsMenu
	{ "Mods", 1.0f, 0.85f, 1, static_cast<int>(std::size(s_arespkModsStatesMenu)), s_arespkModsStatesMenu }, // 39 PRK_ModsStates
	{ "Mods", 1.0f, 0.75f, 1, static_cast<int>(std::size(s_arespkModsTimerMenu)), s_arespkModsTimerMenu }, // 40 PRK_ModsTimer
	{ "Mods", 1.0f, 0.75f, 1, static_cast<int>(std::size(s_arespkModsCombosMenu)), s_arespkModsCombosMenu }, // 41 PRK_ModsCombos
	{ "Customization", 1.0f, 0.85f, 1, static_cast<int>(std::size(s_arespkCustomMenu)), s_arespkCustomMenu }, // 42 PRK_Custom
	{ "Customization", 1.0f, 0.8f, 1, static_cast<int>(std::size(s_arespkCustTextMenu)), s_arespkCustTextMenu }, // 43 PRK_CustText
	{ "Customization", 1.0f, 0.8f, 1, static_cast<int>(std::size(s_arespkCustBoxMenu)), s_arespkCustBoxMenu }, // 44 PRK_CustBox
	{ "Customization", 1.0f, 0.8f, 1, static_cast<int>(std::size(s_arespkCustBorderMenu)), s_arespkCustBorderMenu }, // 45 PRK_CustBorder
	{ "Customization", 1.0f, 0.8f, 1, static_cast<int>(std::size(s_arespkCustHighlightMenu)), s_arespkCustHighlightMenu }, // 46 PRK_CustHighlight
	{ "Customization", 1.0f, 0.8f, 1, static_cast<int>(std::size(s_arespkCustEffectsMenu)), s_arespkCustEffectsMenu }, // 47 PRK_CustEffects
	{ "Mouse", 1.0f, 0.8f, 1, static_cast<int>(std::size(s_arespkMouseMenu)), s_arespkMouseMenu }, // 48 PRK_Mouse
	{ "Reset ALL settings to default?", 1.0f, 1.0f, 0, 2, s_arespkYesNo },                // 49 PRK_ResetConfirm
	{ "Export settings before reset?", 1.0f, 1.0f, 0, 2, s_arespkYesNo },                 // 50 PRK_ResetExport
	{ "Import / Export Settings", 1.0f, 0.8f, 1, 0, s_arespkBackupsMenu },                   // 51 PRK_Backups (built in SetPromptPrk)
	{ "Replace current settings with this backup?", 1.0f, 1.0f, 0, 2, s_arespkYesNo },      // 52 PRK_ImportConfirm
	{ "Controller Settings", 1.0f, 0.8f, 1, 0, s_arespkControllerMenu },                    // 53 PRK_ControllerSettings (built in SetPromptPrk)
	{ "Video", 1.0f, 0.8f, 1, 0, s_arespkVideoGraphics },                                   // 54 PRK_VideoGraphics (built in SetPromptPrk)
	{ "Video", 1.0f, 0.8f, 1, 0, s_arespkVideoUpscaling },                                  // 55 PRK_VideoUpscaling (built in SetPromptPrk)
	{ "Gameplay", 1.0f, 0.8f, 1, static_cast<int>(std::size(s_arespkGameplayMenu)), s_arespkGameplayMenu }, // 56 PRK_Gameplay
	{ g_achzDisplayConfirm, 1.0f, 1.0f, 0, 2, s_arespkYesNo },                              // 57 PRK_DisplayConfirm
};

static_assert(sizeof(s_mpprkprd) / sizeof(s_mpprkprd[0]) == PRK_Max);

#pragma once

// -----------------------------------------------------------------------------
// Prompt groups
// -----------------------------------------------------------------------------

enum PRP
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

enum RESPK
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

    RESPK_Max = 39
};

// -----------------------------------------------------------------------------
// Release prompt IDs
// -----------------------------------------------------------------------------

enum PRK
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

    PRK_Max = 34
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

inline RESPK s_arespkControlsMenu[4] =
{
    RESPK_BinocInvert,
    RESPK_TurretInvert,
    RESPK_JetpackInvert,
    RESPK_Back
};

// -----------------------------------------------------------------------------
// Pause menu
// -----------------------------------------------------------------------------

inline const RESPK s_arespkPauseMenuAll[8] =
{
    RESPK_ReturnToGame,
    RESPK_RestartRace,
    RESPK_RestartSprint,
    RESPK_RestartLevel,
    RESPK_Map,
    RESPK_Options,
    RESPK_ExitLevel,
    RESPK_Quit
};

inline RESPK s_arespkPauseMenu[8]{};

// -----------------------------------------------------------------------------
// Options menu
// -----------------------------------------------------------------------------

inline const RESPK s_arespkOptionsMenuAll[] =
{
    RESPK_NewGame,
    RESPK_LoadGame,
    RESPK_SaveGame,
    RESPK_Music,
    RESPK_Speakers,
    RESPK_Vibration,
    RESPK_Fog,
    RESPK_Video,
    RESPK_Controls,
    RESPK_Back
};

inline RESPK s_arespkOptionsMenu[std::size(s_arespkOptionsMenuAll)]{};

inline RESPK s_arespkVideoMenu[] =
{
    RESPK_Msaa,
    RESPK_FrameRate,
    RESPK_InternalResolution,
    RESPK_DrawDistance,
    RESPK_WindowMode,
    RESPK_Vsync,
    RESPK_AspectRatio,
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
    "Options...",             // 5  RESPK_Options
    "New Game...",            // 6  RESPK_NewGame
    "Load Game...",           // 7  RESPK_LoadGame
    "Save Game...",           // 8  RESPK_SaveGame
    g_achzRespk9,             // 9  RESPK_Quit
    g_achzRespk10,            // 10 RESPK_Slot1
    g_achzRespk11,            // 11 RESPK_Slot2
    g_achzRespk12,            // 12 RESPK_Slot3
    g_achzRespk13,            // 13 RESPK_Vibration
    "Center Screen...",       // 14 RESPK_CenterScreen
    g_achzRespk15,            // 15 RESPK_Music
    g_achzRespk16,            // 16 RESPK_Speakers
    "Back",                   // 17 RESPK_Back
    "Restart Race",           // 18 RESPK_RestartRace
    "Restart Sprint",         // 19 RESPK_RestartSprint
    "Restart Level",          // 20 RESPK_RestartLevel
    "Exit Level",             // 21 RESPK_ExitLevel
    "Exit to Hideout",        // 22 RESPK_ExitToHideout
    "Controls...",            // 23 RESPK_Controls
    g_achzRespk24,            // 24 RESPK_BinocInvert
    g_achzRespk25,            // 25 RESPK_TurretInvert
    g_achzRespk26,            // 26 RESPK_JetpackInvert
    g_achzRespk27,            // 27 RESPK_Msaa
    g_achzRespk28,            // 28 RESPK_FrameRate
    "Video...",              // 29 RESPK_Video
    g_achzRespk30,            // 30 RESPK_InternalResolution
    g_achzRespk31,            // 31 RESPK_Fog
    g_achzRespk32,            // 32 RESPK_DrawDistance
    g_achzRespk33,            // 33 RESPK_WindowMode
    g_achzRespk34,            // 34 RESPK_Vsync
    g_achzRespk35,            // 35 RESPK_AspectRatio
    g_achzSaveSlot4,          // 36 RESPK_Slot4
    g_achzSaveSlot5,          // 37 RESPK_Slot5
    g_achzSaveSlot6           // 38 RESPK_Slot6
};

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
    { 0, nullptr }           // 38 RESPK_Slot6
};

// -----------------------------------------------------------------------------
// Prompt descriptor table
//
// There must be exactly one entry for every numeric PRK from 0 through 32.
// -----------------------------------------------------------------------------

inline PRD s_mpprkprd[PRK_Max] =
{
    { "Paused", 1.0f, 1.0f, 1, 8, s_arespkPauseMenu },                                         // 0  PRK_PauseMenu
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
    { "Controls", 1.0f, 0.8f, 1, 4, s_arespkControlsMenu },                                   // 31 PRK_ControlsMenu
    { nullptr, 1.0f, 1.0f, 1, 0, nullptr },                                                   // 32 PRK_Unknown32
    { "Video", 1.0f, 1.0f, 1, static_cast<int>(std::size(s_arespkVideoMenu)), s_arespkVideoMenu } // 33 PRK_VideoMenu
};

static_assert(sizeof(s_mpprkprd) / sizeof(s_mpprkprd[0]) == PRK_Max);

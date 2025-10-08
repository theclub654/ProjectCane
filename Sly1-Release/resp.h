#pragma once
#include <string>

enum PRP
{
    PRP_Nil = -1,
    PRP_Basic = 0,
    PRP_Memcard = 1,
    PRP_Cd = 2,
    PRP_Max = 3
};
enum RESPK
{
    RESPK_Nil = -1,
    RESPK_Yes = 0,
    RESPK_No = 1,
    RESPK_Continue = 2,
    RESPK_Map = 3,
    RESPK_Options = 4,
    RESPK_Save = 5,
    RESPK_Exit = 6,
    RESPK_Quit = 7,
    RESPK_Slot1 = 8,
    RESPK_Slot2 = 9,
    RESPK_Slot3 = 10,
    RESPK_GameOptions = 11,
    RESPK_SoundOptions = 12,
    RESPK_Max = 13
};
enum PRK
{
    PRK_Nil = -1,
    PRK_PauseMenu = 0,
    PRK_ExitLevel = 1,
    PRK_GameOver = 2,
    PRK_QuitConfirm = 3,
    PRK_MemcardMissing = 4,
    PRK_MemcardFormatConfirm = 5,
    PRK_MemcardFormatting = 6,
    PRK_MemcardFormatError = 7,
    PRK_MemcardCreateConfirm = 8,
    PRK_MemcardCreating = 9,
    PRK_MemcardCreateError = 10,
    PRK_MemcardCardFull = 11,
    PRK_MemcardChooseSaveSlot = 12,
    PRK_MemcardSlotSaving = 13,
    PRK_MemcardSlotSaved = 14,
    PRK_MemcardChooseLoadSlot = 15,
    PRK_MemcardChooseEraseSlot = 16,
    PRK_MemcardEraseConfirm = 17,
    PRK_MemcardAutoNotify = 18,
    PRK_CdRemoved = 19,
    PRK_OptionsMenu = 20,
    PRK_AttractMenu = 21,
    PRK_Max = 22
};

struct PRD
{
    const char *pchz;
    float rScale;
    int fVertical;
    int crespk;
    RESPK* arespk;
};

inline const char* errorcreatingFileStr = "An error occurred while creating the Sly Cooper Save File on the Memory Card (PS2) in Memory Card Slot 1\n\nYou will not be able to save your progress\n";
inline const char* creatingSaveFileStr = "Creating the Sly Cooper Save File on the Memory Card (PS2) in Memory Card Slot 1";
inline const char* noSaveFileStr = "No Sly Cooper Save File on the Memory Card(PS2) inserted in Memory Card Slot 1\n\nIf you do not create a Sly Cooper Save File then you will not be able to save your progress\n\nWould you like to create a Sly Cooper Save File ? ";
inline const char* pauseStr = "Paused";

inline RESPK s_arespkOptionsMenu[2] =
{
    RESPK_GameOptions, RESPK_SoundOptions,
};

inline RESPK s_arespkSlots[3] =
{
    RESPK_Slot1, RESPK_Slot2, RESPK_Slot3,
};

inline RESPK s_arespkContinue[1] =
{
    RESPK_Continue,
};

inline RESPK s_arespkYesNo[2] = 
{
    RESPK_Yes, RESPK_No,
};

inline RESPK s_arespkPauseMenu[6] =
{
    RESPK_Yes, RESPK_Yes, RESPK_Yes,
    RESPK_Yes, RESPK_Yes, RESPK_Yes
};

inline RESPK s_arespkPauseMenuAll[6] =
{
    RESPK_Continue,
    RESPK_Map,
    RESPK_Options,
    RESPK_Save,
    RESPK_Exit,
    RESPK_Quit
};

inline const char* s_mprespkachz[13] =
{
    "Yes", "No", "Return to Game", "View Map",
    "Options", "Save", "Exit Level",
    "Quit Game", "Game 1", "Game 2",
    "Game 3", "Game Options", "Sound Options"
};

inline PRD s_mpprkprd[22] =
{
    "", 1.0, 1, 6, s_arespkPauseMenu,
    "Game Over. Exit level?", 1.0, 0, 2, s_arespkYesNo,
    "Game Over", 1.0, 0, 1, s_arespkContinue,
    "Really Quit?", 1.0, 0, 2, s_arespkYesNo,
    //GOTTA COME BACK TO THIS
    "", 0.69999999, 1, 1, s_arespkContinue,
    //GOTTA COME BACK TO THIS
    "", 0.69999999, 0, 2, s_arespkYesNo,
    //GOTTA COME BACK TO THIS
    "", 1.0, 1, 0, nullptr,
    //GOTTA COME BACK TO THIS
    "", 0.69999999, 1, 1, nullptr,
    noSaveFileStr, 0.69999999, 0, 2, s_arespkYesNo,
    creatingSaveFileStr, 1.0, 1, 0, nullptr,
    errorcreatingFileStr, 0.69999999, 1, 1, s_arespkContinue,
    "Not Enough Space on hard drive to save file", 0.69999999, 1, 1, s_arespkContinue,
    "Save Game?", 1.0, 1, 3, s_arespkSlots,
    "Saving...", 1.0, 1, 0, nullptr,
    "Game Saved", 1.0, 1, 1, s_arespkContinue,
    "Choose Game", 1.0, 1, 3, s_arespkSlots,
    "Erase Game?", 1.0, 1, 3, s_arespkSlots,
    "Really Erase?", 1.0, 0, 2, s_arespkYesNo,
    "Sly Cooper will now auto-save", 1.0, 1, 1, s_arespkContinue,
    "", 0.8, 1, 0, nullptr,
    "", 1.0, 1, 2, s_arespkOptionsMenu,
    "Choose Game", 1.0, 1, 3, s_arespkSlots,
};
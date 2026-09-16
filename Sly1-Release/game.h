#pragma once
#include "dec.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

typedef int GRFGS;
typedef int GRFVAULT;
typedef int GRFCHT;

struct DIFFICULTY;

template <std::size_t BitCount>
struct CBitArray
{
    int m_cbitSet;
    std::uint8_t m_ab[(BitCount + 7) / 8];
};

enum GAMEWORLD 
{
    GAMEWORLD_Nil = -1,
    GAMEWORLD_Intro = 0,
    GAMEWORLD_Underwater = 1,
    GAMEWORLD_Muggshot = 2,
    GAMEWORLD_Voodoo = 3,
    GAMEWORLD_Snow = 4,
    GAMEWORLD_Clockwerk = 5,
    GAMEWORLD_Max = 6
};

enum WORLDLEVEL
{
    WORLDLEVEL_Nil = -1,
    WORLDLEVEL_Approach = 0,
    WORLDLEVEL_Hub = 1,
    WORLDLEVEL_1 = 2,
    WORLDLEVEL_2 = 3,
    WORLDLEVEL_3 = 4,
    WORLDLEVEL_4 = 5,
    WORLDLEVEL_5 = 6,
    WORLDLEVEL_6 = 7,
    WORLDLEVEL_Boss = 8,
    WORLDLEVEL_Max = 9
};

enum FWS // World State Flags
{
    FWS_Visited = 1,
    FWS_Lock_0 = 2,
    FWS_Lock_1 = 4,
    FWS_Lock_2 = 8,
    FWS_Lock_3 = 16,
    GRFWS_Lock_All = 30,
    FWS_Complete = 32
};

enum FLS // Level state flags
{
    FLS_Visited = 1,
    FLS_KeyCollected = 2,
    FLS_Secondary = 4,
    FLS_Tertiary = 8,
    FLS_BossDefeated = 16
};

enum FGS // Game state flags
{
    FGS_FirstClue = 1,
    FGS_HalfClues = 2,
    FGS_AllClues = 4,
    FGS_FirstVault = 8,
    FGS_SecondVault = 16
};

struct FCL {
    int isector;
    int cb;
};

struct CFileLocation {
    struct FCL fcl;
};

struct LEVELINFOBRX
{
    CFileLocation fileInfo; // 0x00
    uint32_t searchValue;       // 0x08
    uint32_t field2_0x0C;       // 0x0C
    uint32_t field3_0x10;       // 0x10
    uint32_t lsnKey;            // 0x14
    uint32_t searchCipher;      // 0x18
    uint32_t sizeKey;           // 0x1C

    uint32_t SearchKey() const
    {
        return searchValue ^ searchCipher;
    }
};

struct LEVELINFO
{
    int levelID;
    std::string levelName;
    FLS task;
    uint32_t searchKey;
};

struct GAME
{
    int cAlarmsTriggered;
};

struct LEVELDATA
{
    int key;
    int value;
};

struct LS 
{ /* Level State */
    GRFLS grfls; /* Level state flags */
    float dt; /* Time spent in level (seconds) */
    float dtTimedBest; /* Best MTS time */
    float uSuck; /* Player suck value for level */
    float uSuckRaw;
    int afDialogPlayed[12]; /*  Dialog played flags */
    LEVELDATA alevelData[4]; /* Per-level Splice key/value data */
    CBitArray <64> abitClue;
    CBitArray <8> abitChkpnt;
};

static_assert(offsetof(LS, alevelData) == 0x44);
static_assert(offsetof(LS, abitClue) == 0x64);

struct WS // World State
{
    LS als[9]; /* Level saves for world */
    int ckey; /* Count of keys collected on world */
    int cvault; /* Count of vaults opened on world */
    int ctimed; /* Count of MTSs completed on world */
    float dt; /* Time spent playing world */
    int fws; /* World save flags */
};

struct VAULTPROG
{
    int worldLevel;      // +0x20 packed world/level, or -1
    uint32_t grflsRequired;
};

struct GS 
{ /* Keeps track of the current game state */
    int gsv;
    int cbThis; /* Size of this struct in bytes */
    int nChecksum; /* Checksum is the size of the savedata */
    float dt; /* Time spent in save file (secs) */
    WS aws[6]; /* Saves for each world */
    GAMEWORLD gameWorldCur; /* Current world */
    WORLDLEVEL worldLevelCur; /* Current level */
    int clife; /* Lives count */
    int ccharm; /* Charms count */
    int ccoin; /* Coins count */
    GRFGS grfgs; /* Settings flags? */
    GRFVAULT grfvault;
    int unlockedCutscenes;
    int fgs;
    int nPowerupLast;
};

void StartupGame();
LEVELINFO* PlevelinfoFromSearchKey(uint32_t searchKey);
LEVELINFO* PlevelinfoFromLevelName(const std::string& levelName);
LEVELINFO* PlevelinfoFromLevel(std::string worldname);
const char* PchzFriendlyFromLevelId(int levelId);
LEVELINFO* PlevelInfoFromId(int levelID);
LEVELINFO* GetLevelInfo(int levelID);
uint32_t GetLevelId(std::string& world);
FLS TasksFromLevelID(int levelID);
void TallyWorldTaskTotals(GAMEWORLD gameWorld, int* pcKeys, int* pcVaults, int* pcMts);
uint32_t GetGameProgress();
void UnlockWorldCutscene(GAMEWORLD gameworld);
void SetWorldComplete(GAMEWORLD gameworld);
void UnlockProgressRewards(uint32_t progressType);

void InitGameState(GS* pgs);

void SetupGame(std::string& world, GRFTRANS grftrans);
void UpdateGameState(float dt);
LS*  PlsFromWorldLevelKey(uint32_t key);
GRFLS GrflsFromWid(WID wid);
void UnloadGame();
void RetryGame();
void StartGame();
void ReloadCurrentLevel();
int  CalculatePercentCompletion(const GS* pgs);
void SetCcharm(int ccharm);
int  FCharmAvailable();
int* PfLookupDialog(LS* pls, int oid);
void OnGameLoad(GAME* pgame);
void ResetGameAlarmsTriggered(GAME* pgame);
void OnGameAlarmTriggered(GAME* pgame);
void OnGameAlarmDisabled(GAME* pgame);
uint32_t GetAvailableVaultFlags();
GRFVAULT GrfvaultFromGameWorld(GAMEWORLD gameWorld);
int CcharmMost();
void HandlePlayerDeath();


void PlayEndingFromCompletionFlag(int completionFlag);

bool FWouldCompleteVaultSet(GRFVAULT grfvault);
//GOTTA COME BACK TO THIS
void UnlockVaultProgressRewards(int fgs);

extern GS  g_gsCur;
extern GS *g_pgsCur;
extern WS *g_pwsCur;
extern LS *g_plsCur;
extern GRFCHT g_grfcht;
extern int g_fInfiniteCharms;
extern int g_grfDebugCheats;
extern GAME g_game;
extern int g_mtsEligibility;
extern LEVELINFO g_levelTable[46];

extern int g_percentCompletion;
extern int g_fDebugMode;

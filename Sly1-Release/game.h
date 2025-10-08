#pragma once

typedef int GRFGS;
typedef int GRFVAULT;

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
    GRFWS_Lock_All = 30
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

struct LS 
{ /* Level State */
    FLS fls; /* Level state flags */
    float dt; /* Time spent in level (seconds) */
    float dtTimedBest; /* Best MTS time */
    float uSuck; /* Player suck value for level */
    //float suck_0x10 ? ;
    int afDialogPlayed[12]; /*  Dialog played flags */
    //struct SceneVar a_scene_vars[4];
    int cclue; /* Count of bottles collected */
    unsigned int fclue; /* Which bottles have been collected */
};

struct WS // World State
{
    LS als[9]; /* Level saves for world */
    int ckey; /* Count of keys collected on world */
    int cvault; /* Count of vaults opened on world */
    int ctimed; /* Count of MTSs completed on world */
    float dt; /* Time spent playing world */
    FWS fws; /* World save flags */
};

struct GS 
{ /* Keeps track of the current game state */
    int gsv;
    int cbThis; /* Size of this struct in bytes */
    int nChecksum; /* Checksum is the size of the savedata */
    float dt; /* Time spent in save file (secs) */
    WS aws[6]; /* Saves for each world */
    GAMEWORLD gameworldCur; /* Current world */
    WORLDLEVEL worldlevelCur; /* Current level */
    int clife; /* Lives count */
    int ccharm; /* Charms count */
    int ccoin; /* Coins count */
    GRFGS grfgs; /* Settings flags? */
    GRFVAULT grfvault;
    int unlocked_cutscenes;
    FGS fgs;
    int nPowerupLast;
};

extern GS *g_pgsCur;
extern WS *g_pwsCur;
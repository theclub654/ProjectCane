#include "game.h"
#include "sw.h"
#include "Input.h"
#include "chkpnt.h"
#include "debug.h"
#include "fmv.h"

void StartupGame()
{
    StartGame();
}

LEVELINFO* PlevelinfoFromSearchKey(uint32_t searchKey)
{
    for (LEVELINFO& level : g_levelTable)
    {
        if (level.searchKey == searchKey)
            return &level;
    }

    return nullptr;
}

LEVELINFO* PlevelinfoFromLevelName(
    const std::string& levelName)
{
    const std::string requestedName =
        std::filesystem::path(levelName).stem().string();

    for (LEVELINFO& levelInfo : g_levelTable)
    {
        if (_stricmp(levelInfo.levelName.c_str(), requestedName.c_str()) == 0)
            return &levelInfo;

        if (levelInfo.levelID == 0x003 &&
            _stricmp(requestedName.c_str(), "Paris") == 0)
        {
            return &levelInfo;
        }
    }

    return nullptr;
}

LEVELINFO* PlevelinfoFromLevel(std::string worldname)
{
    for (LEVELINFO& levelInfo : g_levelTable)
    {
        if (worldname == levelInfo.levelName)
            return &levelInfo;
    }

    return nullptr;
}

const char* PchzFriendlyFromLevelId(int levelId)
{
    LEVELINFO* plevelInfo = PlevelInfoFromId(levelId);
    return plevelInfo != nullptr ? plevelInfo->levelName.c_str() : "<none>";
}

LEVELINFO* PlevelInfoFromId(int levelID)
{
    for (LEVELINFO& levelInfo : g_levelTable)
    {
        if (levelInfo.levelID == levelID)
            return &levelInfo;
    }

    return nullptr;
}

LEVELINFO* GetLevelInfo(int levelID)
{
    return PlevelInfoFromId(levelID);
}

uint32_t GetLevelId(std::string& world)
{
    LEVELINFO* plevelInfo = PlevelinfoFromLevel(world);
    return plevelInfo ? plevelInfo->levelID : UINT32_MAX;
}

FLS TasksFromLevelID(int levelID)
{
    const LEVELINFO* pLevelInfo = PlevelInfoFromId(levelID);

    if (pLevelInfo == nullptr)
        return static_cast<FLS>(0);

    return pLevelInfo->task;
}

void TallyWorldTaskTotals(GAMEWORLD gameWorld, int* pcKeys, int* pcVaults, int* pcMts)
{
    *pcKeys = 0;
    *pcVaults = 0;
    *pcMts = 0;

    for (const LEVELINFO& levelInfo : g_levelTable)
    {
        if ((levelInfo.levelID >> 8) != gameWorld)
            continue;

        const uint32_t tasks = static_cast<uint32_t>(levelInfo.task);

        if ((tasks & FLS_KeyCollected) != 0)
            ++*pcKeys;

        if ((tasks & FLS_Secondary) != 0)
            ++*pcVaults;

        if ((tasks & FLS_Tertiary) != 0)
            ++*pcMts;
    }
}

uint32_t GetGameProgress()
{
    uint32_t progress = FLS_KeyCollected | FLS_Secondary | FLS_Tertiary;

    for (const LEVELINFO& levelInfo : g_levelTable)
    {
        const LS* pls = PlsFromWorldLevelKey(levelInfo.levelID);

        progress &= ~static_cast<uint32_t>(levelInfo.task) | static_cast<uint32_t>(pls->grfls);
    }

    for (int gameWorld = GAMEWORLD_Underwater; gameWorld <= GAMEWORLD_Clockwerk; ++gameWorld)
    {
        if ((g_pgsCur->aws[gameWorld].fws & 0x20U) == 0)
            progress &= ~static_cast<uint32_t>(FLS_KeyCollected);
    }

    return progress;
}

void UnlockWorldCutscene(GAMEWORLD gameworld)
{
    switch (gameworld) 
    {
        case GAMEWORLD_Underwater:
        g_pgsCur->unlockedCutscenes = g_pgsCur->unlockedCutscenes | 16;
        return;
        case GAMEWORLD_Muggshot:
        g_pgsCur->unlockedCutscenes = g_pgsCur->unlockedCutscenes | 64;
        return;
        case GAMEWORLD_Voodoo:
        g_pgsCur->unlockedCutscenes = g_pgsCur->unlockedCutscenes | 256;
        return;
        case GAMEWORLD_Snow:
        g_pgsCur->unlockedCutscenes = g_pgsCur->unlockedCutscenes | 1024;
        return;
        case GAMEWORLD_Clockwerk:
        g_pgsCur->unlockedCutscenes = g_pgsCur->unlockedCutscenes | 4096;
        return;
    }
}

void SetWorldComplete(GAMEWORLD gameworld)
{
    g_pgsCur->aws[gameworld].fws = (FWS)(g_pgsCur->aws[gameworld].fws | 0x20);

    switch (gameworld)
    {
        case GAMEWORLD_Underwater:
        g_pgsCur->unlockedCutscenes |= 0x20;
        break;

        case GAMEWORLD_Muggshot:
        g_pgsCur->unlockedCutscenes |= 0x80;
        break;

        case GAMEWORLD_Voodoo:
        g_pgsCur->unlockedCutscenes |= 0x200;
        g_pgsCur->grfvault |= 0x10000;
        break;

        case GAMEWORLD_Snow:
        g_pgsCur->unlockedCutscenes |= 0x800;
        break;

        case GAMEWORLD_Clockwerk:
        UnlockVaultProgressRewards(2);
        break;
    }
}

void UnlockProgressRewards(uint32_t progressType)
{
    switch (progressType)
    {
        case 0x02:
        g_pgsCur->unlockedCutscenes |= 0xA002U;
        g_pgsCur->fgs |= 0x0002U;
        break;

        case 0x04:
        if ((GetGameProgress() & 0x04U) != 0)
            g_pgsCur->unlockedCutscenes |= 0xC000U;
        break;

        case 0x08:
        if ((GetGameProgress() & 0x08U) != 0)
        {
            g_pgsCur->unlockedCutscenes |= 0x000CU;
            g_pgsCur->fgs |= 0x000CU;
        }
        break;
    }
}

void InitGameState(GS* pgs)
{
    memset(pgs, 0, sizeof(GS));
    pgs->gameWorldCur = GAMEWORLD_Intro;
    pgs->grfvault = 0;
    pgs->gsv = 18;
    //pgs->cbSaveData = 6656;
    pgs->worldLevelCur = WORLDLEVEL_2;
    pgs->clife = 5;
    pgs->nPowerupLast = -1;
    ApplyVibrationSetting(pgs);
}

void SetupGame(std::string& world, GRFTRANS grftrans)
{
    const int levelID = GetLevelId(world);

    GAMEWORLD gameWorld = GAMEWORLD_Intro;
    WORLDLEVEL worldLevel = WORLDLEVEL_Approach;

    if (levelID != WORLDLEVEL_Nil)
    {
        gameWorld = static_cast<GAMEWORLD>(levelID >> 8);
        worldLevel = static_cast<WORLDLEVEL>(levelID & 0xFF);
    }

    g_pwsCur = &g_pgsCur->aws[gameWorld];
    g_plsCur = &g_pwsCur->als[worldLevel];

    if (g_pgsCur->clife < 0 || (grftrans & 0x02U) != 0)
        RetryGame();
    else if ((grftrans & 0x01U) != 0)
        RestoreChkmgrFromCheckpoint(&g_chkmgr);
    else if ((grftrans & 0x04U) != 0)
        UnloadGame();
    else
        ResetChkmgrCheckPoints(&g_chkmgr);

    g_gameWorldPrev = g_pgsCur->gameWorldCur;

    g_pgsCur->gameWorldCur = gameWorld;
    g_pgsCur->worldLevelCur = worldLevel;

    g_pwsCur->fws |= 0x01;
    g_plsCur->grfls |= FLS_Visited;
}

void UpdateGameState(float dt)
{
    g_pgsCur->dt += dt;
    g_pwsCur->dt += dt;
    g_plsCur->dt += dt;
}

LS* PlsFromWorldLevelKey(uint32_t key)
{
    if (key == -1)
        return nullptr;

    return &g_pgsCur->aws[key >> 8].als[key & 0xff];
}

GRFLS GrflsFromWid(WID wid)
{
    const LS* pls = PlsFromWorldLevelKey(wid);
    return pls != nullptr ? pls->grfls : 0;
}

void UnloadGame()
{
    InitGameState(g_pgsCur);
    g_saveData.pgsCurrentSave = nullptr;
    OnGameLoad(&g_game);
    OnDifficultyGameLoad(&g_difficulty);
    g_grfcht = 0;
    g_gameWorldPrev = GAMEWORLD_Nil;
    RetryGame();
}

void RetryGame()
{
    g_pgsCur->clife = 5;
    g_pgsCur->ccharm = 0;

    ResetChkmgrCheckPoints(&g_chkmgr);
}

void StartGame()
{
    UnloadGame();

    if (g_fDebugMode < 1)
        WipeToWorldWarp("Splash.brx", (OID)-1, WIPEK_Fade);
}

void ReloadCurrentLevel()
{
    const int levelID =
        (static_cast<int>(g_pgsCur->gameWorldCur) << 8) |
        static_cast<int>(g_pgsCur->worldLevelCur);

    LEVELINFO* const pLevelInfo = GetLevelInfo(levelID);

    OnGameLoad(&g_game);

    if (pLevelInfo == nullptr)
        return;

    const char* const pchzWorld = pLevelInfo->levelName.c_str();

    if (g_ui.uisPlaying == UIS_Attract && g_pgsCur->dt == 0.0f)
    {
        const bool sameWorld =
            std::strcmp(pchzWorld, g_transition.m_worldCurrent.c_str()) == 0;

        if (sameWorld)
        {
            if (g_wipe.wipes != WIPES_Idle)
                return;

            SetAttractVolume((g_pgsCur->grfgs & 0x80) != 0);
            SetAttractSoundOption((g_pgsCur->grfgs & 0x40) != 0);

            ASEG* const paseg = static_cast<ASEG*>(
                PloFindSwObject(g_psw, 261, (OID)1172, nullptr));

            if (paseg != nullptr)
            {
                ASEGA* pasega = static_cast<ASEGA*>(paseg->dlAsega.pvFirst);

                while (pasega != nullptr)
                {
                    ASEGA* const pasegaNext = *reinterpret_cast<ASEGA**>( reinterpret_cast<uint8_t*>(pasega) + paseg->dlAsega.ibDle);

                    RetractAsega(pasega);
                    pasega = pasegaNext;
                }
            }

            g_ui.uisPlaying = UIS_Nil;
            SetUiUis(&g_ui, static_cast<UIS>(3));
            return;
        }
    }

    WipeToWorldWarp(pchzWorld, OID_Nil, WIPEK_Keyhole);
}

int CalculatePercentCompletion(const GS* pgs)
{
    int cTasksTotal = 0;
    int cTasksComplete = 0;

    for (const LEVELINFO& levelInfo : g_levelTable)
    {
        const int gameWorld = levelInfo.levelID >> 8;
        const int worldLevel = levelInfo.levelID & 0xFF;

        // Intro levels do not count toward completion.
        if (gameWorld == GAMEWORLD_Intro)
            continue;

        const LS& levelState = pgs->aws[gameWorld].als[worldLevel];
        const uint32_t completed = levelState.grfls;
        const uint32_t required =
            static_cast<uint32_t>(levelInfo.task);

        // Visiting every non-intro level counts as one task.
        ++cTasksTotal;

        if ((completed & 0x01U) != 0)
            ++cTasksComplete;

        // Count each task declared by the level table.
        for (uint32_t taskFlag = 0x02U;
            taskFlag <= 0x08U;
            taskFlag <<= 1U)
        {
            if ((required & taskFlag) == 0)
                continue;

            ++cTasksTotal;

            if ((completed & taskFlag) != 0)
                ++cTasksComplete;
        }
    }

    // Each of the five main worlds contributes one additional objective.
    for (int gameWorld = GAMEWORLD_Underwater;
        gameWorld <= GAMEWORLD_Clockwerk;
        ++gameWorld)
    {
        ++cTasksTotal;

        if ((pgs->aws[gameWorld].fws & 0x20U) != 0)
            ++cTasksComplete;
    }

    if (cTasksComplete == 0)
        return 0;

    if (cTasksComplete == cTasksTotal)
        return 100;

    const int percent = (cTasksComplete * 100) / cTasksTotal;

    // Any nonzero incomplete progress is displayed as 1–99%.
    return std::clamp(percent, 1, 99);
}

void SetCcharm(int ccharm)
{
    g_pgsCur->ccharm = ccharm;
}

int FCharmAvailable()
{
    if (g_pgsCur->ccharm < 1) {
        if (g_fInfiniteCharms != 0) {
            return 1;
        }
        if ((g_grfcht & 2U) == 0) {
            return 0;
        }
    }

    return 1;
}

int* PfLookupDialog(LS* pls, int oid)
{
    constexpr int oidDialogFirst = 827;
    constexpr int cDialog = 12;

    const int index = static_cast<int>(oid) - oidDialogFirst;

    if (index < 0 || index >= cDialog)
        return nullptr;

    return &pls->afDialogPlayed[index];
}

void OnGameLoad(GAME* pgame)
{
    memset(pgame, 0, sizeof(GAME));
}

void ResetGameAlarmsTriggered(GAME* pgame)
{
    pgame->cAlarmsTriggered = 0;
}

void OnGameAlarmTriggered(GAME* pgame)
{
    pgame->cAlarmsTriggered++;
}

void OnGameAlarmDisabled(GAME* pgame)
{
    pgame->cAlarmsTriggered--;
}

uint32_t GetAvailableVaultFlags()
{
    return g_pgsCur->grfvault & g_psw->grfvault;
}

GRFVAULT GrfvaultFromGameWorld(GAMEWORLD gameWorld)
{
    switch (gameWorld)
    {
        case GAMEWORLD_Underwater:
        return static_cast<GRFVAULT>(1U << 29);

        case GAMEWORLD_Muggshot:
        return static_cast<GRFVAULT>(1U << 30);

        case GAMEWORLD_Voodoo:
        return static_cast<GRFVAULT>(1U << 31);

        case GAMEWORLD_Snow:
        return static_cast<GRFVAULT>(1U << 28);

        default:
        return static_cast<GRFVAULT>(0);
    }
}

int CcharmMost()
{
    return 2;
}

void HandlePlayerDeath()
{
    if (g_pgsCur == nullptr)
        return;

    --g_pgsCur->clife;

    CancelSwDialogPlaying(g_psw);
    ResetCommentaryAudio();

    if (g_pgsCur->worldLevelCur == WORLDLEVEL_Hub)
    {
        LEVELINFO* plevelCurrent = g_transition.m_plevelCurrent;

        if (plevelCurrent == nullptr && !g_transition.m_worldCurrent.empty())
            plevelCurrent = PlevelinfoFromLevelName(g_transition.m_worldCurrent);

        TRANS trans{};
        trans.fSet = 1;
        trans.plevel = plevelCurrent;
        trans.oidWarp = g_transition.m_oidWarp;
        trans.oidWarpContext = g_transition.m_oidWarpContext;
        trans.grftrans = static_cast<GRFTRANS>(0x10);

        ActivateWipe(&g_wipe, &trans, WIPEK_Keyhole);
    }
    else if (g_pgsCur->clife < 0)
        SetPrompt(&g_prompt, PRP_Basic, PRK_GameOver);
    else
        ReturnChkmgrToCheckpoint(&g_chkmgr);

    OnDifficultyPlayerDeath(&g_difficulty, 1.0f);
}

void PlayEndingFromCompletionFlag(int completionFlag)
{
    int endingCutscene = -1;
    if (completionFlag == 4)
        endingCutscene = 14;
    else if (completionFlag == 2)
        endingCutscene = 13;

    // The retail target at 0x00247AD0 is the Hideout level descriptor.
    LEVELINFO* hideout = PlevelinfoFromSearchKey(9);
    if (hideout != nullptr)
        WipeToWorldWarp(hideout, OID_Nil, WIPEK_Keyhole);

    if (endingCutscene != -1)
    {
        // PlayAndUnlockMovie(endingCutscene, 15): QueueCutscene performs the
        // unlock and supports two queued movies in their playback order.
        const CUTSCENEINFO* ending = PcutsceneinfoFromId(endingCutscene);
        const CUTSCENEINFO* credits = PcutsceneinfoFromId(15);
        if (ending != nullptr)
            QueueCutscene(ending->identityId);
        if (credits != nullptr)
            QueueCutscene(credits->identityId);
    }
}

bool FWouldCompleteVaultSet(GRFVAULT grfvault)
{
    const uint32_t grfVaultValid = 0xF000FFFF;

    if ((g_pgsCur->grfvault & grfVaultValid) == grfVaultValid)
        return false;

    if ((GetGameProgress() & 2) != 0)
        return false;

    return ((g_pgsCur->grfvault | grfvault) & grfVaultValid) == grfVaultValid;
}

void UnlockVaultProgressRewards(int fgs)
{
    FGS fgsCompletion;
    uint32_t grfCutscenes;

    if (fgs == FGS_AllClues) {
        fgsCompletion = (FGS) GetGameProgress();

        if ((fgsCompletion & FGS_AllClues) != 0)
            g_pgsCur->unlockedCutscenes |= 0xc000;

        return;
    }

    if (fgs == FGS_HalfClues) 
    {
        g_pgsCur->unlockedCutscenes |= 0xa002;
        g_pgsCur->fgs = (FGS)(g_pgsCur->fgs | FGS_HalfClues);
        return;
    }

    if (fgs == FGS_FirstVault) {
        fgsCompletion = (FGS)GetGameProgress();

        if ((fgsCompletion & FGS_FirstVault) == 0)
            return;

        g_pgsCur->unlockedCutscenes |= 0xc;
        g_pgsCur->fgs = (FGS)(g_pgsCur->fgs | FGS_AllClues | FGS_FirstVault);
    }
}

GS  g_gsCur;
GS* g_pgsCur = &g_gsCur;
WS* g_pwsCur;
LS* g_plsCur;
GRFCHT g_grfcht = 0;
int g_fInfiniteCharms = 0;
int g_grfDebugCheats = 0;
GAME g_game;
int g_mtsEligibility = 0;
LEVELINFO g_levelTable[46] =
{
    { 0x002, "Splash",                    static_cast<FLS>(0x00), 21 },
    { 0x003, "Paris, France",             static_cast<FLS>(0x00), 10 },
    { 0x004, "The Hideout",               static_cast<FLS>(0x00),  9 },

    { 0x100, "A Stealthy Approach",       static_cast<FLS>(0x0E), 37 },
    { 0x101, "Prowling the Grounds",      static_cast<FLS>(0x00), 38 },
    { 0x102, "High Class Heist",          static_cast<FLS>(0x0E), 33 },
    { 0x103, "Into the Machine",          static_cast<FLS>(0x0E), 36 },
    { 0x104, "A Cunning Disguise",        static_cast<FLS>(0x0E), 32 },
    { 0x105, "The Fire Down Below",       static_cast<FLS>(0x0E), 31 },
    { 0x106, "Treasure in the Depths",    static_cast<FLS>(0x02), 39 },
    { 0x107, "The Gunboat Graveyard",     static_cast<FLS>(0x0E), 40 },
    { 0x108, "The Eye of the Storm",      static_cast<FLS>(0x00), 34 },

    { 0x200, "A Rocky Start",             static_cast<FLS>(0x0E), 12 },
    { 0x201, "Muggshot's Turf",           static_cast<FLS>(0x00), 15 },
    { 0x202, "Boneyard Casino",           static_cast<FLS>(0x0E), 14 },
    { 0x203, "Murray's Big Gamble",       static_cast<FLS>(0x02), 18 },
    { 0x204, "At the Dog Track",          static_cast<FLS>(0x02), 19 },
    { 0x205, "Two to Tango",              static_cast<FLS>(0x0E), 16 },
    { 0x206, "Straight to the Top",       static_cast<FLS>(0x0E), 17 },
    { 0x207, "Back Alley Heist",          static_cast<FLS>(0x0E), 20 },
    { 0x208, "Last Call",                 static_cast<FLS>(0x00), 13 },

    { 0x300, "The Dread Swamp Path",      static_cast<FLS>(0x0E), 41 },
    { 0x301, "The Swamp's Dark Center",   static_cast<FLS>(0x00), 45 },
    { 0x302, "The Lair of the Beast",     static_cast<FLS>(0x0E), 49 },
    { 0x303, "A Grave Undertaking",       static_cast<FLS>(0x0E), 44 },
    { 0x304, "Piranha Lake",              static_cast<FLS>(0x02), 47 },
    { 0x305, "Descent into Danger",       static_cast<FLS>(0x0E), 48 },
    { 0x306, "A Ghastly Voyage",          static_cast<FLS>(0x02), 46 },
    { 0x307, "Down Home Cooking",         static_cast<FLS>(0x02), 43 },
    { 0x308, "A Deadly Dance",            static_cast<FLS>(0x00), 42 },

    { 0x400, "A Perilous Ascent",         static_cast<FLS>(0x0E), 22 },
    { 0x401, "Inside the Stronghold",     static_cast<FLS>(0x00), 25 },
    { 0x402, "Flaming Temple of Flame",   static_cast<FLS>(0x0E), 27 },
    { 0x403, "The Unseen Foe",            static_cast<FLS>(0x0E), 23 },
    { 0x404, "The King of the Hill",      static_cast<FLS>(0x02), 28 },
    { 0x405, "Rapid Fire Assault",        static_cast<FLS>(0x02), 30 },
    { 0x406, "Duel by the Dragon",        static_cast<FLS>(0x0E), 26 },
    { 0x407, "A Desperate Race",          static_cast<FLS>(0x02), 29 },
    { 0x408, "Flame Fu!",                 static_cast<FLS>(0x00), 24 },

    { 0x500, "A Hazardous Path",          static_cast<FLS>(0x00),  8 },
    { 0x502, "Burning Rubber",            static_cast<FLS>(0x00),  7 },
    { 0x503, "A Daring Rescue",           static_cast<FLS>(0x00),  6 },
    { 0x504, "Bentley Comes Through",     static_cast<FLS>(0x00),  2 },
    { 0x505, "A Temporary Truce",         static_cast<FLS>(0x00),  5 },
    { 0x506, "Sinking Peril",             static_cast<FLS>(0x00),  4 },
    { 0x508, "A Strange Reunion",         static_cast<FLS>(0x00),  3 }
};

int g_percentCompletion;
int g_fDebugMode = 0;
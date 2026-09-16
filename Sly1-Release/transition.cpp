#include "transition.h"
#include "coin.h"
#include "bbmark.h"
#include "difficulty.h"
#include "chkpnt.h"
#include "hubblot.h"
#include <Windows.h>

namespace
{
const char* WorldDirectoryName(int levelId)
{
    switch (levelId >> 8)
    {
        case GAMEWORLD_Underwater: return "Tide of Terror";
        case GAMEWORLD_Muggshot: return "Sunset Snake Eyes";
        case GAMEWORLD_Voodoo: return "Vicious Voodoo";
        case GAMEWORLD_Snow: return "Fire in the sky";
        case GAMEWORLD_Clockwerk: return "The Cold Heart of Hate";
        default: return nullptr;
    }
}

std::filesystem::path ExecutableDirectory()
{
    wchar_t executablePath[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executablePath, MAX_PATH);

    if (length == 0 || length >= MAX_PATH)
        return std::filesystem::current_path();

    return std::filesystem::path(executablePath).parent_path();
}

std::filesystem::path FindWorldsDirectory(const std::string& currentFile)
{
    const std::filesystem::path selectedPath(currentFile);

    for (std::filesystem::path path = selectedPath.parent_path(); !path.empty(); path = path.parent_path())
    {
        if (_stricmp(path.filename().string().c_str(), "Worlds") == 0)
            return path;

        if (path == path.root_path())
            break;
    }

    const std::filesystem::path executableDirectory = ExecutableDirectory();
    const std::filesystem::path candidates[] =
    {
        selectedPath.parent_path() / "Worlds",
        executableDirectory / "Worlds",
        std::filesystem::current_path() / "Worlds",
        std::filesystem::current_path() / "Sly1-Release" / "Worlds"
    };

    for (const std::filesystem::path& candidate : candidates)
    {
        if (std::filesystem::is_directory(candidate))
            return candidate;
    }

    return executableDirectory / "Worlds";
}

std::filesystem::path LevelPath(const LEVELINFO& levelInfo, const std::string& currentFile)
{
    std::filesystem::path directory = FindWorldsDirectory(currentFile);

    if (const char* worldDirectory = WorldDirectoryName(levelInfo.levelID))
        directory /= worldDirectory;

    const char* levelName = levelInfo.levelID == 0x003
        ? "Paris"
        : levelInfo.levelName.c_str();

    return directory / (std::string(levelName) + ".brx");
}
}

void CTransition::ResetWorld(int nParam)
{
    SetMasterVolume(0.0f);

    if (nParam == 1)
        ReturnChkmgrToCheckpoint(&g_chkmgr);
    else if (m_plevelCurrent != nullptr)
        Set(m_plevelCurrent, OID_Nil, OID_Nil, static_cast<GRFTRANS>(0));
    else
        std::printf("RESET WORLD ERROR: no current level\n");
}

void CTransition::Set(LEVELINFO* plevel, OID oidWarp, OID oidWarpContext, GRFTRANS grftrans)
{
    LEVELINFO* plevelTarget =
        plevel != nullptr
        ? plevel
        : m_plevelCurrent;

    if (m_fPending != 0)
        return;

    if (plevelTarget == nullptr)
    {
        std::printf("TRANSITION SET ERROR: no target or current level\n");

        return;
    }

    m_fPending = 1;
    m_plevelPending = plevelTarget;
    m_worldPending = plevelTarget->levelName;
    m_oidWarp = oidWarp;
    m_oidWarpContext = oidWarpContext;
    m_grftrans = grftrans;
}

void CTransition::Execute(std::string& file)
{
    SetPhase(PHASE_Load);

    bool fLoadSucceeded = true;

    LEVELINFO* plevelTarget = m_plevelPending != nullptr ? m_plevelPending : m_plevelCurrent;

    if (plevelTarget == nullptr)
    {
        std::printf("TRANSITION EXECUTE ERROR: no pending or current level\n");
        fLoadSucceeded = false;
    }

    std::string fileNew;

    if (fLoadSucceeded)
    {
        fileNew = LevelPath(*plevelTarget, file).string();

        const bool fWorldChanged = m_plevelCurrent == nullptr || m_plevelCurrent->searchKey != plevelTarget->searchKey;

        CBinaryInputStream bis(fileNew);

        if (!bis.file.is_open())
        {
            std::printf("TRANSITION EXECUTE ERROR: could not open '%s'\n", fileNew.c_str());
            fLoadSucceeded = false;
        }
        else
        {
            DeleteWorld(g_psw);

            bis.S32Read();
            bis.S32Read();

            g_psw = nullptr;

            //SetupBulkDataFromBrx(!fWorldChanged, &bis);

            ResetClock(&g_clock, 0.0f);
            AddGrfusr(128);
            SetupGame(plevelTarget->levelName, m_grftrans);

            g_psw = static_cast<SW*>(PloNew(CID_SW, nullptr, nullptr, OID__WORLD, OID_Nil));

            if (g_psw != nullptr)
                g_psw->pvtlo->pfnLoadLoFromBrx(g_psw, &bis);
            else
                fLoadSucceeded = false;

            bis.Close();

            if (fLoadSucceeded)
            {
                m_plevelCurrent = plevelTarget;
                m_worldCurrent = plevelTarget->levelName;
                file = fileNew;
            }

            if (fLoadSucceeded && g_pgsCur != nullptr && (g_pgsCur->grfgs & 0x100U) != 0 && g_saveData.pgsCurrentSave != nullptr && (g_saveData.pgsCurrentSave->gameWorldCur != g_pgsCur->gameWorldCur || g_saveData.pgsCurrentSave->worldLevelCur != g_pgsCur->worldLevelCur))
                AutosaveCurrentGame(&g_saveData);
        }
    }

    if (fLoadSucceeded && g_psw != nullptr)
    {
        ResetCm(g_pcm, 1);
        UpdateSounds();
        TeleportSwPlayer(g_psw, m_oidWarp, m_oidWarpContext);
        ResetGameAlarmsTriggered(&g_game);
        EnableHubCamera(&g_hubblot);
        RecalcSwXpAll(g_psw, 0);
        UpdateCmLast(g_pcm, 0, g_clock.dtReal);

        if ((static_cast<uint32_t>(m_grftrans) & 0x10U) != 0)
            g_lifectr.pvtblot->pfnShowBlot(&g_lifectr);

        SetClockRate(1.0f);
    }

    InitializeSoundOptions();
    SetMvgkRvol(g_psw->uMusicVolume, 1, 1);

    m_plevelPending = nullptr;
    m_worldPending.clear();
    m_fPending = 0;

    //ClearPhase(PHASE_Load);
}

CTransition g_transition;

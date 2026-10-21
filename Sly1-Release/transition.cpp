#include "transition.h"
#include "coin.h"
#include "bbmark.h"
#include "difficulty.h"
#include "chkpnt.h"
#include "hubblot.h"
#include "spu_reverb_xapo.h"
#include "runtime_error.h"
#include "content.h"
#include "../platform/platform.h"

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

namespace
{
std::uint32_t Crc32(const std::uint8_t* data, std::size_t size)
{
    std::uint32_t crc = 0xffffffffu;
    for (std::size_t index = 0; index < size; ++index)
    {
        crc ^= data[index];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

bool FValidPcBrx(const std::filesystem::path& path)
{
    constexpr uint32_t kPcBrxMagic = 0x52424350u; // "PCBR" little-endian
    std::ifstream input(path, std::ios::binary);
    uint32_t magic = 0;
    uint32_t version = 0;
    return input.read(reinterpret_cast<char*>(&magic), sizeof(magic)) &&
        input.read(reinterpret_cast<char*>(&version), sizeof(version)) &&
        magic == kPcBrxMagic && (version == 17 || version == 18 || version == 19);
}

bool FValidWorldsDirectory(const std::filesystem::path& directory)
{
    return FValidPcBrx(directory / "Splash" / "Splash.brx") ||
        FValidPcBrx(directory / "Splash" / "English" / "Splash.brx") ||
        FValidPcBrx(directory / "Splash.brx");
}

std::filesystem::path AdoptWorldsDirectory(const std::filesystem::path& directory)
{
    RefreshContentPackage(directory.parent_path());
    return directory;
}

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

std::filesystem::path FindWorldsDirectory(const std::string& currentFile)
{
    const std::filesystem::path selectedPath(currentFile);

    RefreshContentPackage(selectedPath);
    if (ContentPackage().manifestLoaded)
    {
        const std::filesystem::path worlds = ContentPackage().root / "Worlds";
        if (FValidWorldsDirectory(worlds))
            return worlds;
    }

    for (std::filesystem::path path = selectedPath.parent_path(); !path.empty(); path = path.parent_path())
    {
        if (_stricmp(path.filename().string().c_str(), "Worlds") == 0 &&
            FValidWorldsDirectory(path))
            return AdoptWorldsDirectory(path);

        if (path == path.root_path())
            break;
    }

    const std::filesystem::path executableDirectory = cane::DataDirectory();
    const std::filesystem::path candidates[] =
    {
        selectedPath.parent_path() / "Worlds",
        executableDirectory / "Worlds",
        std::filesystem::current_path() / "Worlds",
        std::filesystem::current_path() / "Sly1-Release" / "Worlds",
        std::filesystem::current_path() / "x64" / "Debug" / "Worlds",
        std::filesystem::current_path() / "x64" / "Release" / "Worlds",
        executableDirectory.parent_path().parent_path() / "x64" / "Debug" / "Worlds",
        executableDirectory.parent_path().parent_path() / "x64" / "Release" / "Worlds",
        executableDirectory.parent_path().parent_path().parent_path() / "x64" / "Debug" / "Worlds",
        executableDirectory.parent_path().parent_path().parent_path() / "x64" / "Release" / "Worlds"
    };

    for (const std::filesystem::path& candidate : candidates)
    {
        if (FValidWorldsDirectory(candidate))
            return AdoptWorldsDirectory(candidate);
    }

    // The extractor allows any output directory. A user can therefore select
    // a folder inside the portable package and end up with one additional
    // directory between the AppImage and Worlds. Search only a few levels and
    // require a valid current-format Splash level so stale retail/older PCBRX
    // trees are never selected accidentally.
    const std::filesystem::path searchRoots[] =
    {
        executableDirectory,
        selectedPath.parent_path(),
        std::filesystem::current_path()
    };
    for (const std::filesystem::path& root : searchRoots)
    {
        std::error_code error;
        if (!std::filesystem::is_directory(root, error))
            continue;
        std::filesystem::recursive_directory_iterator it(root,
            std::filesystem::directory_options::skip_permission_denied, error);
        const std::filesystem::recursive_directory_iterator end;
        while (it != end)
        {
            if (error)
            {
                error.clear();
                it.increment(error);
                continue;
            }
            if (it.depth() >= 3)
                it.disable_recursion_pending();
            if (it->is_directory(error) &&
                _stricmp(it->path().filename().string().c_str(), "Worlds") == 0 &&
                FValidWorldsDirectory(it->path()))
                return AdoptWorldsDirectory(it->path());
            it.increment(error);
        }
    }

    return executableDirectory / "Worlds";
}

std::filesystem::path LevelPath(const LEVELINFO& levelInfo, const std::string& currentFile)
{
    const char* levelName = levelInfo.levelID == 0x003
        ? "Paris"
        : levelInfo.levelName.c_str();

    // The debug file picker supplies the exact file the user selected. Honor
    // it when its name matches the pending level instead of reconstructing a
    // different path and silently opening another retail BRX.
    const std::filesystem::path selectedPath(currentFile);
    if (std::filesystem::is_regular_file(selectedPath))
    {
        const std::string selectedStem = selectedPath.stem().string();
        const bool selectedLanguageMatches = !FPalContent() ||
            selectedPath.parent_path().filename() == levelName ||
            _stricmp(selectedPath.parent_path().filename().string().c_str(),
                ContentLanguageDirectory()) == 0;
        if (selectedLanguageMatches &&
            (selectedStem == levelName ||
             selectedStem == std::string(levelName) + "_Rebuilt"))
            return selectedPath;
    }

    std::filesystem::path directory = FindWorldsDirectory(currentFile);

    if (const char* worldDirectory = WorldDirectoryName(levelInfo.levelID))
        directory /= worldDirectory;

    const std::filesystem::path fileName = std::string(levelName) + ".brx";
    std::filesystem::path organizedPath = directory / levelName;
    organizedPath /= fileName;
    if (std::filesystem::is_regular_file(organizedPath))
        return organizedPath;

    if (FPalContent())
    {
        // Compatibility with the pre-v19 PAL layout, where each language kept
        // a separate v17 BRX below the level directory.
        const std::filesystem::path legacyLocalizedPath = directory / levelName /
            ContentLanguageDirectory() / fileName;
        if (std::filesystem::is_regular_file(legacyLocalizedPath))
            return legacyLocalizedPath;

        std::printf("PAL CONTENT ERROR: missing combined level '%s' at '%s'\n",
            levelName,
            cane::Utf8(organizedPath).c_str());
        return organizedPath;
    }

    // Keep existing extracted trees usable while transitioning to the
    // per-level directory layout.
    return directory / fileName;
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
    LEVELINFO* plevelTarget = plevel != nullptr ? plevel : m_plevelCurrent;

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

	// Every transition reaches Execute, including player-death checkpoint
	// returns which do not pass through ResetWorld. Silence the mastering voice
	// before deleting the old world's sources and wet graph so no final hardware
	// buffer can expose a reverb tail. InitializeSoundOptions restores the user's
	// output level after the destination world has finished loading.
	SetMasterVolume(0.0f);

    bool fLoadSucceeded = true;
    const std::string filePrevious = file;

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

        CBinaryInputStream bis;

        try
        {
			bis.OpenFile(fileNew);
        }
        catch (const std::exception& exception)
        {
            std::printf("TRANSITION EXECUTE ERROR: %s\n", exception.what());
            fLoadSucceeded = false;
        }

        if (!fLoadSucceeded || !bis.file.is_open())
        {
            if (fLoadSucceeded)
				std::printf("TRANSITION EXECUTE ERROR: could not open '%s'\n", fileNew.c_str());

            fLoadSucceeded = false;
        }
        else
        {
            // Sound-bank and music setup happen inside LoadSwFromBrx. Publish
            // the destination BRX path before parsing so those loaders can
            // resolve media beside the extractor output instead of searching
            // only relative to the executable's working directory.
            file = fileNew;

            const uint32_t brxHeader = bis.U32Read();
            const uint32_t brxVersion = bis.U32Read();
            constexpr uint32_t kPcBrxMagic = 0x52424350u; // "PCBR" little-endian
            const bool validVersion = brxVersion == 17 || brxVersion == 18 || brxVersion == 19;
            if (brxHeader != kPcBrxMagic || !validVersion)
            {
                ReportRuntimeError("Load", "Level '%s' has the wrong PCBRX version for this content region. Re-extract it with the current File Extractor GUI.", fileNew.c_str());
                bis.Close();
                fLoadSucceeded = false;
            }
            if (!fLoadSucceeded)
                file = filePrevious;
            else
            {
                std::vector<std::uint8_t> languagePayload;
                if (brxVersion == 18 || brxVersion == 19)
                {
                    constexpr std::uint32_t kChunkedPalMagic = 0x4B484350u; // "PCHK"
                    constexpr std::uint32_t kUnifiedMagic = 0x494E5550u; // "PUNI"
                    const std::size_t containerHeaderSize = brxVersion == 19 ? 20u : 12u;
                    const bool containerHeaderAvailable = bis.Remaining() >= containerHeaderSize;
                    std::uint32_t formatMagic = 0;
                    std::uint32_t schema = 0;
                    std::uint32_t languageCount = 0;
                    std::uint32_t defaultLanguage = 0;
                    std::uint32_t chunkCount = 0;
                    if (containerHeaderAvailable)
                    {
                        formatMagic = bis.U32Read();
                        schema = brxVersion == 19 ? bis.U32Read() : 0;
                        languageCount = bis.U32Read();
                        defaultLanguage = brxVersion == 19 ? bis.U32Read() : 0;
                        chunkCount = bis.U32Read();
                    }
                    const std::size_t chunkDescriptorSize = brxVersion == 19
                        ? sizeof(std::uint32_t) * 2 : sizeof(std::uint32_t);
                    const std::size_t maximumChunkCount = bis.Remaining() / chunkDescriptorSize;
                    const bool validContainerHeader = containerHeaderAvailable &&
                        (brxVersion == 18
                            ? formatMagic == kChunkedPalMagic && languageCount == 5
                            : formatMagic == kUnifiedMagic && schema == 1 &&
                                languageCount >= 1 && languageCount <= 5 && defaultLanguage < 5);
                    if (!validContainerHeader || chunkCount == 0 ||
                        chunkCount > maximumChunkCount)
                    {
                        ReportRuntimeError("Load", "Level '%s' has an invalid language container.", fileNew.c_str());
                        fLoadSucceeded = false;
                    }
                    else
                    {
                        struct LanguageEntry
                        {
                            std::uint32_t language{};
                            std::uint64_t size{};
                            std::vector<std::uint32_t> chunks;
                        };
                        std::vector<LanguageEntry> languages;
                        languages.reserve(languageCount);
                        std::uint32_t languageMask = 0;
                        for (std::uint32_t languageIndex = 0; languageIndex < languageCount; ++languageIndex)
                        {
                            const std::size_t fixedEntrySize = brxVersion == 19 ? 16u : 12u;
                            if (bis.Remaining() < fixedEntrySize)
                            {
                                fLoadSucceeded = false;
                                break;
                            }
                            const std::uint32_t language = brxVersion == 19
                                ? bis.U32Read() : languageIndex;
                            const std::uint64_t sizeLow = bis.U32Read();
                            const std::uint64_t sizeHigh = bis.U32Read();
                            const std::uint64_t payloadSize = sizeLow | (sizeHigh << 32);
                            const std::uint32_t referenceCount = bis.U32Read();
                            if (language >= 5 || (languageMask & (1u << language)) != 0 ||
                                referenceCount == 0 ||
                                referenceCount > bis.Remaining() / sizeof(std::uint32_t))
                            {
                                fLoadSucceeded = false;
                                break;
                            }
                            languageMask |= 1u << language;
                            LanguageEntry entry{ language, payloadSize, {} };
                            entry.chunks.reserve(referenceCount);
                            for (std::uint32_t reference = 0; reference < referenceCount; ++reference)
                                entry.chunks.push_back(bis.U32Read());
                            languages.push_back(std::move(entry));
                        }

                        if (fLoadSucceeded && (languageMask & (1u << defaultLanguage)) == 0)
                            fLoadSucceeded = false;
                        const std::uint32_t preferredLanguage = FJapaneseContent()
                            ? (ContentVoiceLanguage() == CONTENT_LANGUAGE::English ? 1u : 0u)
                            : static_cast<std::uint32_t>(ContentPackage().preferredLanguage);
                        const std::uint32_t selectedLanguage = preferredLanguage < 5 &&
                            (languageMask & (1u << preferredLanguage)) != 0
                                ? preferredLanguage : defaultLanguage;
                        const auto selected = std::find_if(languages.begin(), languages.end(),
                            [selectedLanguage](const LanguageEntry& entry)
                            {
                                return entry.language == selectedLanguage;
                            });
                        if (fLoadSucceeded && selected == languages.end())
                            fLoadSucceeded = false;

                        struct ChunkDescriptor
                        {
                            std::size_t offset{};
                            std::uint32_t size{};
                            std::uint32_t crc{};
                        };
                        std::vector<ChunkDescriptor> chunks;
                        chunks.reserve(chunkCount);
                        for (std::uint32_t chunk = 0; fLoadSucceeded && chunk < chunkCount; ++chunk)
                        {
                            if (bis.Remaining() < chunkDescriptorSize)
                            {
                                fLoadSucceeded = false;
                                break;
                            }
                            const std::uint32_t chunkSize = bis.U32Read();
                            const std::uint32_t chunkCrc = brxVersion == 19 ? bis.U32Read() : 0;
                            if (chunkSize > bis.Remaining())
                            {
                                fLoadSucceeded = false;
                                break;
                            }
                            chunks.push_back({ bis.GetOffset(), chunkSize, chunkCrc });
                            bis.Seek(bis.GetOffset() + chunkSize);
                        }

                        if (fLoadSucceeded && selected->size >
                            static_cast<std::uint64_t>((std::numeric_limits<int>::max)()))
                            fLoadSucceeded = false;
                        if (fLoadSucceeded)
                        {
                            languagePayload.reserve(static_cast<std::size_t>(selected->size));
                            for (const std::uint32_t chunk : selected->chunks)
                            {
                                if (chunk >= chunks.size() || languagePayload.size() > selected->size ||
                                    chunks[chunk].size > selected->size - languagePayload.size())
                                {
                                    fLoadSucceeded = false;
                                    break;
                                }
                                const std::size_t oldSize = languagePayload.size();
                                languagePayload.resize(oldSize + chunks[chunk].size);
                                bis.Seek(chunks[chunk].offset);
                                bis.ReadBytes(languagePayload.data() + oldSize, chunks[chunk].size);
                                if (brxVersion == 19 &&
                                    Crc32(languagePayload.data() + oldSize, chunks[chunk].size) !=
                                        chunks[chunk].crc)
                                {
                                    fLoadSucceeded = false;
                                    break;
                                }
                            }
                            if (languagePayload.size() != selected->size)
                                fLoadSucceeded = false;
                        }

                        if (!fLoadSucceeded)
                        {
                            ReportRuntimeError("Load", "Level '%s' has invalid language data.", fileNew.c_str());
                        }
                        else
                        {
                            if (languagePayload.size() < 8)
                            {
                                ReportRuntimeError("Load", "Level '%s' contains a truncated language payload.", fileNew.c_str());
                                fLoadSucceeded = false;
                            }
                        }
                        if (fLoadSucceeded)
                        {
                            bis.OpenMemory(static_cast<int>(languagePayload.size()), languagePayload.data());
                            const std::uint32_t payloadMagic = bis.U32Read();
                            const std::uint32_t payloadVersion = bis.U32Read();
                            if (payloadMagic != kPcBrxMagic || payloadVersion != 17)
                            {
                                ReportRuntimeError("Load", "Level '%s' contains an invalid language payload.", fileNew.c_str());
                                fLoadSucceeded = false;
                            }
                            else
                            {
                                const bool hasLocalizationFooter = languagePayload.size() >= 8 &&
                                    languagePayload[languagePayload.size() - 8] == 'P' &&
                                    languagePayload[languagePayload.size() - 7] == 'L' &&
                                    languagePayload[languagePayload.size() - 6] == 'O' &&
                                    languagePayload[languagePayload.size() - 5] == 'C';
                                if (hasLocalizationFooter &&
                                    !LoadContentLocalization(languagePayload.data(), languagePayload.size()))
                                {
                                    ReportRuntimeError("Load", "Level '%s' contains invalid localization data.", fileNew.c_str());
                                    fLoadSucceeded = false;
                                }
                                else
                                {
                                    if (!hasLocalizationFooter)
                                        ClearContentLocalization();
                                    // Commit global language state only after
                                    // every container, chunk, payload, and PLOC
                                    // check has succeeded. A rejected level must
                                    // not alter the currently running world.
                                    ConfigureContentLanguages(languageMask,
                                        static_cast<CONTENT_LANGUAGE>(defaultLanguage));
                                    std::printf("Loading unified PCBRX v%u (%s payload): %s\n",
                                        brxVersion, FJapaneseContent()
                                            ? ContentAudioLanguageDirectory()
                                            : ContentLanguageDirectory(), fileNew.c_str());
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (FPalContent() && LoadContentLocalization(fileNew))
                    {
                        // Legacy PAL v17 files are language-specific and carry
                        // one PLOC footer apiece. Keep all five choices exposed;
                        // LevelPath selects the corresponding language BRX.
                        ConfigureContentLanguages((1u << 5) - 1,
                            CONTENT_LANGUAGE::English);
                        std::printf("Loading localized legacy PCBRX v%u (%s): %s\n",
                            brxVersion, ContentLanguageDirectory(), fileNew.c_str());
                    }
                    else
                    {
                        ConfigureContentLanguages(1u, CONTENT_LANGUAGE::English);
                        ClearContentLocalization();
                        std::printf("Loading rebuilt PCBRX v%u: %s\n", brxVersion, fileNew.c_str());
                    }
                }

                if (!fLoadSucceeded)
                {
                    bis.Close();
                    file = filePrevious;
                }
                else
                {

                DeleteWorld(g_psw);

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
                }
                else
                    file = filePrevious;

                if (fLoadSucceeded && g_pgsCur != nullptr && (g_pgsCur->grfgs & 0x100U) != 0 && g_saveData.pgsCurrentSave != nullptr && (g_saveData.pgsCurrentSave->gameWorldCur != g_pgsCur->gameWorldCur || g_saveData.pgsCurrentSave->worldLevelCur != g_pgsCur->worldLevelCur))
                    AutosaveCurrentGame(&g_saveData);
                }
            }
        }
    }

    if (fLoadSucceeded && g_psw != nullptr)
    {
        ResetCm(g_pcm, 1);
        UpdateSounds();
        //TeleportSwPlayer(g_psw, (OID)446, m_oidWarpContext);
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

    // The language menu stages its choice before this reload so payload
    // selection and localized media resolution can use it. Persist only a
    // successful load; otherwise restore the selection that owns the old world.
    FinishContentLanguageChange(fLoadSucceeded && g_psw != nullptr);

    if (g_psw != nullptr)
        SetMvgkRvol(g_psw->uMusicVolume, 1, 1);

    m_plevelPending = nullptr;
    m_worldPending.clear();
    m_fPending = 0;
    //ClearPhase(PHASE_Load);
}

CTransition g_transition;

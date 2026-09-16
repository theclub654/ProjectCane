#include "save.h"
#include "clock.h"
#include "cm.h"
#include "gl.h"
#include "glob.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <Windows.h>

namespace
{
constexpr char kSaveMagic[8] = { 'P', 'C', 'A', 'N', 'E', 'S', 'A', 'V' };
constexpr uint32_t kSaveVersion = 2;

struct SaveFileHeader
{
    char magic[8];
    uint32_t version;
    uint32_t payloadSize;
    uint32_t checksum;
};

struct SavePayload
{
    GS gameState;
    VIDEOSAVESETTINGS video;
};

uint32_t SaveChecksum(const void* data, size_t size)
{
    const auto* bytes = static_cast<const uint8_t*>(data);
    uint32_t hash = 2166136261U;

    for (size_t i = 0; i < size; ++i)
    {
        hash ^= bytes[i];
        hash *= 16777619U;
    }

    return hash;
}

VIDEOSAVESETTINGS CaptureVideoSettings()
{
    VIDEOSAVESETTINGS settings{};
    settings.fogType = g_fogType;
    settings.msaaEnabled = g_fMsaa ? 1 : 0;
    settings.msaaSamples = g_msaaSamples;
    settings.frameRate = g_targetFrameRate;
    settings.internalResolutionHeight = g_internalResolutionHeight;
    settings.drawDistance = g_drawDistanceMultiplier;
    settings.windowMode = static_cast<int>(g_windowMode);
    settings.vsyncEnabled = g_fVsync ? 1 : 0;
    settings.aspectMode = static_cast<int>(g_gl.aspectMode);
    return settings;
}

GLFWmonitor* CurrentMonitor()
{
    if (g_gl.window != nullptr)
    {
        if (GLFWmonitor* monitor = glfwGetWindowMonitor(g_gl.window))
            return monitor;
    }

    return glfwGetPrimaryMonitor();
}

void ValidateVideoSettings(VIDEOSAVESETTINGS& settings)
{
    settings.fogType = std::clamp(settings.fogType, 0, 2);
    settings.msaaEnabled = settings.msaaEnabled != 0;
    settings.msaaSamples = std::clamp(settings.msaaSamples, 1, 16);
    settings.frameRate = (std::max)(settings.frameRate, 30);
    settings.drawDistance = std::clamp(settings.drawDistance, 0.75f, 3.0f);
    settings.windowMode = std::clamp(settings.windowMode,
        static_cast<int>(WindowMode_Windowed), static_cast<int>(WindowMode_Fullscreen));
    settings.vsyncEnabled = settings.vsyncEnabled != 0;
    settings.aspectMode = std::clamp(settings.aspectMode,
        static_cast<int>(FitToScreen), static_cast<int>(Fixed_16_10));

    const int validInternalResolutions[] = { 0, 720, 1080, 1440, 2160 };
    if (std::find(std::begin(validInternalResolutions), std::end(validInternalResolutions),
        settings.internalResolutionHeight) == std::end(validInternalResolutions))
    {
        settings.internalResolutionHeight = 0;
    }

    GLFWmonitor* monitor = CurrentMonitor();
    const GLFWvidmode* mode = monitor != nullptr ? glfwGetVideoMode(monitor) : nullptr;
    if (mode != nullptr)
    {
        if (settings.frameRate > mode->refreshRate)
            settings.frameRate = (std::max)(mode->refreshRate, 30);

        if (settings.internalResolutionHeight > mode->height)
        {
            settings.internalResolutionHeight = 0;
            for (int height : validInternalResolutions)
            {
                if (height <= mode->height)
                    settings.internalResolutionHeight = height;
            }
        }
    }

    GLint maxSamples = 1;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    settings.msaaSamples = (std::clamp)(
        settings.msaaSamples, 1, (std::max)(static_cast<int>(maxSamples), 1));
    if (maxSamples < 2)
        settings.msaaEnabled = 0;
}

void ApplySavedVideoSettings(VIDEOSAVESETTINGS settings)
{
    ValidateVideoSettings(settings);

    ApplyWindowModeSettings(static_cast<WindowMode>(settings.windowMode));
    g_targetFrameRate = settings.frameRate;
    g_internalResolutionHeight = settings.internalResolutionHeight;
    g_fVsync = settings.vsyncEnabled != 0;
    g_fMsaa = settings.msaaEnabled != 0;
    g_msaaSamples = settings.msaaSamples;
    g_fogType = settings.fogType;
    g_drawDistanceMultiplier = settings.drawDistance;

    ApplyAspectRatioSettings(static_cast<AspectMode>(settings.aspectMode));
    ApplyMsaaSettings();
    glfwSwapInterval(g_fVsync ? 1 : 0);

    glGlobShader.Use();
    glUniform1i(glslFogType, g_fogType);

    if (g_pcm != nullptr)
        SetCmMrdRatio(g_pcm, settings.drawDistance);
}

std::filesystem::path SaveDirectory()
{
    wchar_t executablePath[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    std::filesystem::path base = length != 0
        ? std::filesystem::path(executablePath).parent_path()
        : std::filesystem::current_path();
    return base / L"Saves";
}

std::filesystem::path SaveSlotPath(int slot)
{
    return SaveDirectory() / (L"slot" + std::to_wstring(slot + 1) + L".sav");
}

bool WriteSaveSlot(int slot, const GS& save, const VIDEOSAVESETTINGS& video)
{
    std::error_code ec;
    std::filesystem::create_directories(SaveDirectory(), ec);
    if (ec)
        return false;

    const std::filesystem::path path = SaveSlotPath(slot);
    const std::filesystem::path temporary = path.wstring() + L".tmp";
    const SavePayload payload{ save, video };
    const SaveFileHeader header{ { 'P', 'C', 'A', 'N', 'E', 'S', 'A', 'V' }, kSaveVersion,
        static_cast<uint32_t>(sizeof(payload)), SaveChecksum(&payload, sizeof(payload)) };

    std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
    if (!stream)
        return false;

    stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
    stream.write(reinterpret_cast<const char*>(&payload), sizeof(payload));
    stream.flush();
    if (!stream)
        return false;
    stream.close();

    return MoveFileExW(temporary.c_str(), path.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
}

bool ReadSaveSlot(int slot, GS* save, VIDEOSAVESETTINGS* video, bool* hasVideo)
{
    if (save == nullptr || video == nullptr || hasVideo == nullptr)
        return false;

    std::ifstream stream(SaveSlotPath(slot), std::ios::binary);
    SaveFileHeader header{};
    if (!stream.read(reinterpret_cast<char*>(&header), sizeof(header)) ||
        std::memcmp(header.magic, kSaveMagic, sizeof(kSaveMagic)) != 0)
    {
        *save = {};
        *video = {};
        *hasVideo = false;
        return false;
    }

    if (header.version == 1 && header.payloadSize == sizeof(GS))
    {
        GS loaded{};
        if (!stream.read(reinterpret_cast<char*>(&loaded), sizeof(loaded)) ||
            SaveChecksum(&loaded, sizeof(loaded)) != header.checksum)
            return false;

        *save = loaded;
        *video = {};
        *hasVideo = false;
        return true;
    }

    if (header.version == kSaveVersion && header.payloadSize == sizeof(SavePayload))
    {
        SavePayload payload{};
        if (!stream.read(reinterpret_cast<char*>(&payload), sizeof(payload)) ||
            SaveChecksum(&payload, sizeof(payload)) != header.checksum)
            return false;

        *save = payload.gameState;
        *video = payload.video;
        *hasVideo = true;
        return true;
    }

    *save = {};
    *video = {};
    *hasVideo = false;
    return false;
}
}

void StartupSaveBlot(SAVEBLOT* psaveblot)
{
    g_autosave.pvtsaveblot = &g_vtsaveblot;
}

void PostAutoSaveLoad(SAVEBLOT* psaveblot)
{
    PostBlotLoad(psaveblot);

    SetBlotDtAppear(psaveblot, 0.25f);
    SetBlotDtDisappear(psaveblot, 0.25f);
    SetBlotDtVisible(psaveblot, 0.0f);

    psaveblot->pfont = PfontFromFont(1);
    SetBlotFontScale(psaveblot, 0.69999999);

    char achzPrompt[2] =
    {
        GetAnimatedPromptCharacter(),
        '\0'
    };

    SetBlotAchzDraw(psaveblot, achzPrompt);

    // Keep the size calculated for the glyph, but initially draw no text.
    psaveblot->achzDraw[0] = '\0';
}

void SetSaveBlots(SAVEBLOT* psaveblot, BLOTS blots)
{
    if (blots == BLOTS_Hidden) 
        psaveblot->fSaveComplete = 0;

    SetBlotBlots(psaveblot, blots);
}

void DrawAutoSave(SAVEBLOT* psaveblot)
{
    if (psaveblot->blots == BLOTS_Hidden)
        return;

    const float dtVisible = g_clock.tReal - psaveblot->tBlots;

    if (psaveblot->blots == BLOTS_Visible && g_letterbox.blots == BLOTS_Visible)
        psaveblot->fSaveComplete = dtVisible < 2.0f;

    float yOffset = 0.0f;

    if (psaveblot->fSaveComplete)
        yOffset = (1.0f - g_letterbox.uOn) * 66.40001f;
    else if (psaveblot->blots == BLOTS_Visible && dtVisible > 1.0f)
        HideBlot(psaveblot);

    CTextBox tbx;
    tbx.SetPos(psaveblot->x, psaveblot->y + yOffset);
    tbx.SetSize(psaveblot->dx, psaveblot->dy);
    tbx.SetTextColor(&psaveblot->rgba);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    const char chPrompt = GetAnimatedPromptCharacter();
    const int percent = CalculatePercentCompletion(g_pgsCur);

    char achz[32];
    std::snprintf(achz, sizeof(achz), "%c %d%%", chPrompt, percent);

    psaveblot->pfont->PushScaling(psaveblot->rFontScale, psaveblot->rFontScale);

    CRichText richText(achz, psaveblot->pfont);
    richText.Draw(&tbx, nullptr);

    psaveblot->pfont->PopScaling();
}

void AutosaveCurrentGame(SAVEDATA* psaveData)
{
    if (!SaveCurrentGameToDisk(psaveData))
        return;

    g_autosave.fSaveComplete = 0;
    g_autosave.pvtblot->pfnShowBlot(&g_autosave);
}

void StartupSaveData(SAVEDATA* psaveData)
{
    if (psaveData == nullptr)
        return;

    *psaveData = {};

    for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
        ReadSaveSlot(slot, &psaveData->saveData[slot], &psaveData->videoSettings[slot],
            &psaveData->hasVideoSettings[slot]);

    for (GS& save : psaveData->saveData)
    {
        if (save.dt > 0.0f)
        {
            psaveData->pgsAttractSave = &save;
            break;
        }
    }
}

bool SaveCurrentGameToDisk(SAVEDATA* psaveData)
{
    if (psaveData == nullptr || psaveData->pgsCurrentSave == nullptr || g_pgsCur == nullptr)
        return false;

    const ptrdiff_t slot = psaveData->pgsCurrentSave - psaveData->saveData;
    if (slot < 0 || slot >= SAVE_SLOT_COUNT)
        return false;

    GS snapshot = *g_pgsCur;
    // Existing slot menus use dt == 0 to mean "empty". Preserve that
    // convention while still allowing a brand-new game to be saved before
    // its first gameplay tick.
    if (snapshot.dt <= 0.0f)
        snapshot.dt = 0.000001f;
    snapshot.cbThis = sizeof(GS);
    snapshot.nChecksum = sizeof(GS);

    const VIDEOSAVESETTINGS video = CaptureVideoSettings();
    if (!WriteSaveSlot(static_cast<int>(slot), snapshot, video))
        return false;

    psaveData->saveData[slot] = snapshot;
    psaveData->videoSettings[slot] = video;
    psaveData->hasVideoSettings[slot] = true;
    psaveData->pgsCurrentSave = &psaveData->saveData[slot];
    if (psaveData->pgsAttractSave == nullptr)
        psaveData->pgsAttractSave = psaveData->pgsCurrentSave;
    return true;
}

bool DeleteSaveSlotFromDisk(SAVEDATA* psaveData, int slot)
{
    if (psaveData == nullptr || slot < 0 || slot >= SAVE_SLOT_COUNT)
        return false;

    std::error_code ec;
    const std::filesystem::path path = SaveSlotPath(slot);
    std::filesystem::remove(path, ec);

    // A missing file already represents an empty slot, so only an actual
    // filesystem error should make the erase operation fail.
    if (ec)
        return false;

    GS* erasedSave = &psaveData->saveData[slot];
    if (psaveData->pgsCurrentSave == erasedSave)
        psaveData->pgsCurrentSave = nullptr;
    if (psaveData->pgsAttractSave == erasedSave)
        psaveData->pgsAttractSave = nullptr;
    if (psaveData->pgsSelectedSave == erasedSave)
        psaveData->pgsSelectedSave = nullptr;

    *erasedSave = {};
    psaveData->videoSettings[slot] = {};
    psaveData->hasVideoSettings[slot] = false;

    if (psaveData->pgsAttractSave == nullptr)
    {
        for (GS& save : psaveData->saveData)
        {
            if (save.dt > 0.0f)
            {
                psaveData->pgsAttractSave = &save;
                break;
            }
        }
    }

    return true;
}

void SetSaveManagerState(SAVEDATA* psavedata, SAVEMENUSTATE state)
{
    if (state != psavedata->state) {
        // Keep the chosen PC slot attached after the menu closes so level
        // completion, vault, clue, and coin autosaves continue writing to it.
        psavedata->state = state;
        psavedata->stateStartTime = g_clock.tReal;
    }
}

int LoadCurrentSave(SAVEDATA* psaveData)
{
    if (psaveData == nullptr || psaveData->pgsCurrentSave == nullptr || g_pgsCur == nullptr)
        return false;

    const GS* src = psaveData->pgsCurrentSave;
    if (src->dt <= 0.0f || src->gameWorldCur < GAMEWORLD_Intro ||
        src->gameWorldCur >= GAMEWORLD_Max || src->worldLevelCur < 0 ||
        src->worldLevelCur >= 9)
        return false;

    // GS is entirely persistent value data; it contains no runtime pointers.
    *g_pgsCur = *src;
    const ptrdiff_t slot = src - psaveData->saveData;
    if (slot >= 0 && slot < SAVE_SLOT_COUNT && psaveData->hasVideoSettings[slot])
        ApplySavedVideoSettings(psaveData->videoSettings[slot]);
    ApplyVibrationSetting(g_pgsCur);
    return 1;
}

SAVEDATA g_saveData;
SAVEBLOT g_autosave;
VTSAVEBLOT g_vtsaveblot;

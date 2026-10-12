#include "save.h"
#include "clock.h"
#include "cm.h"
#include "gl.h"
#include "gui_layout.h"
#include "glob.h"
#include "debug.h"
#include "sound.h"
#include "mouseinput.h"
#include "../platform/platform.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <Windows.h>

namespace
{
constexpr char kSaveMagic[8] = { 'P', 'C', 'A', 'N', 'E', 'S', 'A', 'V' };
constexpr uint32_t kSaveVersion = 3;
constexpr char kSettingsMagic[8] = { 'P', 'C', 'A', 'N', 'E', 'S', 'Y', 'S' };
constexpr uint32_t kSettingsVersion = 10;
int s_lastSaveSlot = -1;

struct VideoSaveSettingsV1
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
};

struct VideoSaveSettingsV2
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
};

struct VideoSaveSettingsV3
{
    VideoSaveSettingsV2 video;
    int keyboardBindings[BTN_MAX];
};

struct VideoSaveSettingsV4
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
    int keyboardBindings[BTN_MAX];
    int gamepadBindings[BTN_MAX];
};

struct VideoSaveSettingsV5
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
};

struct VideoSaveSettingsV6
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
};

struct VideoSaveSettingsV7
{
	VideoSaveSettingsV6 base;
	int lastSaveSlot;
};

struct VideoSaveSettingsV8
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
};

struct VideoSaveSettingsV9
{
	VideoSaveSettingsV8 base;
	int mouseCameraEnabled;
};

struct SaveFileHeader
{
    char magic[8];
    uint32_t version;
    uint32_t payloadSize;
    uint32_t checksum;
};

struct SavePayloadV2
{
    GS gameState;
    VideoSaveSettingsV1 video;
};

void SetDefaultAudioSettings(VIDEOSAVESETTINGS& settings);

VIDEOSAVESETTINGS UpgradeVideoSettings(const VideoSaveSettingsV1& oldSettings)
{
    VIDEOSAVESETTINGS settings{};
    settings.fogType = oldSettings.fogType;
    settings.msaaEnabled = oldSettings.msaaEnabled;
    settings.msaaSamples = oldSettings.msaaSamples;
    settings.frameRate = oldSettings.frameRate;
    settings.internalResolutionHeight = oldSettings.internalResolutionHeight;
    settings.drawDistance = oldSettings.drawDistance;
    settings.windowMode = oldSettings.windowMode;
    settings.vsyncEnabled = oldSettings.vsyncEnabled;
    settings.aspectMode = oldSettings.aspectMode;
    settings.guiScale = 1.0f;
	settings.guiStyle = GuiStyle_PS2;
	std::copy(g_keyboardBindings.begin(), g_keyboardBindings.end(), settings.keyboardBindings);
	std::copy(g_gamepadBindings.begin(), g_gamepadBindings.end(), settings.gamepadBindings);
    settings.lastSaveSlot = -1;
	SetDefaultAudioSettings(settings);
    return settings;
}

VIDEOSAVESETTINGS UpgradeVideoSettings(const VideoSaveSettingsV2& oldSettings)
{
	VIDEOSAVESETTINGS settings{};
	std::memcpy(&settings, &oldSettings, sizeof(oldSettings));
	std::copy(g_keyboardBindings.begin(), g_keyboardBindings.end(), settings.keyboardBindings);
	std::copy(g_gamepadBindings.begin(), g_gamepadBindings.end(), settings.gamepadBindings);
	settings.guiStyle = GuiStyle_PS2;
	settings.lastSaveSlot = -1;
	SetDefaultAudioSettings(settings);
	return settings;
}

VIDEOSAVESETTINGS UpgradeVideoSettings(const VideoSaveSettingsV3& oldSettings)
{
	VIDEOSAVESETTINGS settings = UpgradeVideoSettings(oldSettings.video);
	std::copy(std::begin(oldSettings.keyboardBindings), std::end(oldSettings.keyboardBindings),
		settings.keyboardBindings);
	std::copy(g_gamepadBindings.begin(), g_gamepadBindings.end(), settings.gamepadBindings);
	settings.guiStyle = GuiStyle_PS2;
	settings.lastSaveSlot = -1;
	SetDefaultAudioSettings(settings);
	return settings;
}

VIDEOSAVESETTINGS UpgradeVideoSettings(const VideoSaveSettingsV4& oldSettings)
{
	VIDEOSAVESETTINGS settings{};
	settings.fogType = oldSettings.fogType;
	settings.msaaEnabled = oldSettings.msaaEnabled;
	settings.msaaSamples = oldSettings.msaaSamples;
	settings.frameRate = oldSettings.frameRate;
	settings.internalResolutionHeight = oldSettings.internalResolutionHeight;
	settings.drawDistance = oldSettings.drawDistance;
	settings.windowMode = oldSettings.windowMode;
	settings.vsyncEnabled = oldSettings.vsyncEnabled;
	settings.aspectMode = oldSettings.aspectMode;
	settings.guiScale = oldSettings.guiScale;
	settings.guiStyle = GuiStyle_PS2;
	std::copy(std::begin(oldSettings.keyboardBindings), std::end(oldSettings.keyboardBindings),
		settings.keyboardBindings);
	std::copy(std::begin(oldSettings.gamepadBindings), std::end(oldSettings.gamepadBindings),
		settings.gamepadBindings);
	settings.lastSaveSlot = -1;
	SetDefaultAudioSettings(settings);
	return settings;
}

VIDEOSAVESETTINGS UpgradeVideoSettings(const VideoSaveSettingsV5& oldSettings)
{
	VIDEOSAVESETTINGS settings{};
	std::memcpy(&settings, &oldSettings, sizeof(oldSettings));
	settings.disableControllerInputWhenUnfocused = 0;
	settings.lastSaveSlot = -1;
	SetDefaultAudioSettings(settings);
	return settings;
}

VIDEOSAVESETTINGS UpgradeVideoSettings(const VideoSaveSettingsV6& oldSettings)
{
	VIDEOSAVESETTINGS settings{};
	std::memcpy(&settings, &oldSettings, sizeof(oldSettings));
	settings.lastSaveSlot = -1;
	SetDefaultAudioSettings(settings);
	return settings;
}

void SetDefaultAudioSettings(VIDEOSAVESETTINGS& settings)
{
	settings.stereoEnabled = 1;
	settings.musicVolume = 1.0f;
	settings.soundEffectsVolume = 1.0f;
	settings.dialogueVolume = 1.0f;
}

struct SettingsFileHeader
{
    char magic[8];
    uint32_t version;
    uint32_t payloadSize;
    uint32_t checksum;
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
    settings.guiScale = g_guiScale;
	settings.guiStyle = static_cast<int>(g_guiStyle);
	std::copy(g_keyboardBindings.begin(), g_keyboardBindings.end(), settings.keyboardBindings);
	std::copy(g_gamepadBindings.begin(), g_gamepadBindings.end(), settings.gamepadBindings);
    settings.disableControllerInputWhenUnfocused = g_fDisableControllerInputWhenUnfocused ? 1 : 0;
    settings.lastSaveSlot = s_lastSaveSlot;
	settings.stereoEnabled = FUserStereoEnabled() ? 1 : 0;
	settings.musicVolume = GetUserMusicVolume();
	settings.soundEffectsVolume = GetUserSfxVolume();
	settings.dialogueVolume = GetUserDialogueVolume();
	settings.mouseCameraEnabled = g_fMouseCamera ? 1 : 0;
	settings.cutsceneSkipControlsEnabled = g_fSkipBinocDialogs ? 1 : 0;
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
    // 0 = Uncapped; anything else is at least 30.
    if (settings.frameRate != 0)
        settings.frameRate = (std::max)(settings.frameRate, 30);
    settings.drawDistance = std::clamp(settings.drawDistance, 0.75f, 3.0f);
    settings.windowMode = std::clamp(settings.windowMode,
        static_cast<int>(WindowMode_Windowed), static_cast<int>(WindowMode_Fullscreen));
    settings.vsyncEnabled = settings.vsyncEnabled != 0;
    settings.aspectMode = std::clamp(settings.aspectMode,
        static_cast<int>(FitToScreen), static_cast<int>(PS2_16_9));
	settings.guiStyle = std::clamp(settings.guiStyle,
		static_cast<int>(GuiStyle_PS2), static_cast<int>(GuiStyle_Modern));
    settings.disableControllerInputWhenUnfocused =
        settings.disableControllerInputWhenUnfocused != 0;
	settings.mouseCameraEnabled = settings.mouseCameraEnabled != 0;
	settings.cutsceneSkipControlsEnabled = settings.cutsceneSkipControlsEnabled != 0;
    settings.lastSaveSlot = std::clamp(settings.lastSaveSlot, -1, SAVE_SLOT_COUNT - 1);
	settings.stereoEnabled = settings.stereoEnabled != 0;
	settings.musicVolume = std::clamp(settings.musicVolume, 0.0f, 1.0f);
	settings.soundEffectsVolume = std::clamp(settings.soundEffectsVolume, 0.0f, 1.0f);
	settings.dialogueVolume = std::clamp(settings.dialogueVolume, 0.0f, 1.0f);
    // Each presentation exposes two deliberate sizes. Both share 100%, so
    // malformed or legacy values safely normalize to the nearest valid size.
    if (settings.guiStyle == GuiStyle_Modern)
        settings.guiScale = settings.guiScale >= 1.125f ? 1.25f : 1.0f;
    else
        settings.guiScale = settings.guiScale < 0.875f ? 0.75f : 1.0f;
	for (int button = 0; button < BTN_MAX; ++button)
	{
		const int key = settings.keyboardBindings[button];
		if (!FMouseBinding(key) && (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST || key == GLFW_KEY_ESCAPE))
			settings.keyboardBindings[button] = g_keyboardBindings[button];
	}
	for (int button = 0; button < BTN_MAX; ++button)
	{
		const int binding = settings.gamepadBindings[button];
		if (!((binding >= 0 && binding <= GLFW_GAMEPAD_BUTTON_LAST) ||
			binding == GAMEPAD_BINDING_LEFT_TRIGGER || binding == GAMEPAD_BINDING_RIGHT_TRIGGER))
		{
			settings.gamepadBindings[button] = g_gamepadBindings[button];
		}
	}

    const int validInternalResolutions[] = { 0, 360, 480, 720, 1080, 1440, 2160 };
    if (std::find(std::begin(validInternalResolutions), std::end(validInternalResolutions),
        settings.internalResolutionHeight) == std::end(validInternalResolutions))
    {
        settings.internalResolutionHeight = 0;
    }

    GLFWmonitor* monitor = CurrentMonitor();
    const GLFWvidmode* mode = monitor != nullptr ? glfwGetVideoMode(monitor) : nullptr;
    if (mode != nullptr)
    {
        if (settings.frameRate != 0 && settings.frameRate > mode->refreshRate)
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
    g_guiScale = settings.guiScale;
	g_guiStyle = static_cast<GuiStyle>(settings.guiStyle);
	std::copy(std::begin(settings.keyboardBindings), std::end(settings.keyboardBindings),
		g_keyboardBindings.begin());
	std::copy(std::begin(settings.gamepadBindings), std::end(settings.gamepadBindings),
		g_gamepadBindings.begin());
    g_fDisableControllerInputWhenUnfocused =
        settings.disableControllerInputWhenUnfocused != 0;
	g_fMouseCamera = settings.mouseCameraEnabled != 0;
	g_fSkipBinocDialogs = settings.cutsceneSkipControlsEnabled != 0;
    s_lastSaveSlot = settings.lastSaveSlot;
	SetUserStereoEnabled(settings.stereoEnabled != 0);
	SetUserMusicVolume(settings.musicVolume);
	SetUserSfxVolume(settings.soundEffectsVolume);
	SetUserDialogueVolume(settings.dialogueVolume);

    ApplyAspectRatioSettings(static_cast<AspectMode>(settings.aspectMode));
    if (g_gl.renderHeight != g_internalResolutionHeight &&
        g_internalResolutionHeight != 0)
    {
        // A saved supersampling mode may be valid on one GPU but too expensive
        // on another. Keep the working native-sized framebuffer in that case.
        g_internalResolutionHeight = 0;
        ApplyInternalResolutionSettings();
    }
    ApplyMsaaSettings();
    glfwSwapInterval(g_fVsync ? 1 : 0);

    glGlobShader.Use();
    glUniform1i(glslFogType, g_fogType);

    if (g_pcm != nullptr)
        SetCmMrdRatio(g_pcm, settings.drawDistance);
}

// Built-in defaults (captured at startup before any saved settings are applied).
VIDEOSAVESETTINGS s_videoDefaults{};
bool s_fHaveVideoDefaults = false;

// The two public functions below are defined outside the file's anonymous
// namespace so other files (prompt.cpp) can link to them.
} // namespace

namespace
{
// Apply + save, then re-run the resize path so HUD/menu layout follows GUI
// scale / style / aspect changes.
void ApplyAndRefreshVideoSettings(VIDEOSAVESETTINGS settings)
{
    settings.lastSaveSlot = s_lastSaveSlot;
    ApplySavedVideoSettings(settings);
    SaveSystemSettings();
    if (g_gl.window != nullptr)
    {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(g_gl.window, &width, &height);
        if (width > 0 && height > 0)
            FrameBufferSizeCallBack(g_gl.window, width, height);
    }
}
}

VIDEOSAVESETTINGS CaptureCurrentVideoSettings()
{
    return CaptureVideoSettings();
}

void RestoreVideoSettings(const VIDEOSAVESETTINGS& settings)
{
    ApplyAndRefreshVideoSettings(settings);
}

bool FSystemOptionDefault(SYSOPT opt)
{
    if (!s_fHaveVideoDefaults)
        return true;
    const VIDEOSAVESETTINGS cur = CaptureVideoSettings();
    const VIDEOSAVESETTINGS& def = s_videoDefaults;
    switch (opt)
    {
    case SYSOPT_FrameRate: return cur.frameRate == def.frameRate;
    case SYSOPT_InternalResolution: return cur.internalResolutionHeight == def.internalResolutionHeight;
    case SYSOPT_DrawDistance: return std::abs(cur.drawDistance - def.drawDistance) < 0.001f;
    case SYSOPT_WindowMode: return cur.windowMode == def.windowMode;
    case SYSOPT_Vsync: return cur.vsyncEnabled == def.vsyncEnabled;
    case SYSOPT_AspectRatio: return cur.aspectMode == def.aspectMode;
    case SYSOPT_Msaa: return cur.msaaEnabled == def.msaaEnabled && (cur.msaaEnabled == 0 || cur.msaaSamples == def.msaaSamples);
    case SYSOPT_Fog: return cur.fogType == def.fogType;
    case SYSOPT_GuiScale: return std::abs(cur.guiScale - def.guiScale) < 0.001f;
    case SYSOPT_GuiStyle: return cur.guiStyle == def.guiStyle;
    case SYSOPT_Stereo: return cur.stereoEnabled == def.stereoEnabled;
    case SYSOPT_DisableControllerUnfocused: return cur.disableControllerInputWhenUnfocused == def.disableControllerInputWhenUnfocused;
    default: return true;
    }
}

void ResetSystemOption(SYSOPT opt)
{
    if (!s_fHaveVideoDefaults)
        return;
    VIDEOSAVESETTINGS cur = CaptureVideoSettings();
    const VIDEOSAVESETTINGS& def = s_videoDefaults;
    switch (opt)
    {
    case SYSOPT_FrameRate: cur.frameRate = def.frameRate; break;
    case SYSOPT_InternalResolution: cur.internalResolutionHeight = def.internalResolutionHeight; break;
    case SYSOPT_DrawDistance: cur.drawDistance = def.drawDistance; break;
    case SYSOPT_WindowMode: cur.windowMode = def.windowMode; break;
    case SYSOPT_Vsync: cur.vsyncEnabled = def.vsyncEnabled; break;
    case SYSOPT_AspectRatio: cur.aspectMode = def.aspectMode; break;
    case SYSOPT_Msaa: cur.msaaEnabled = def.msaaEnabled; cur.msaaSamples = def.msaaSamples; break;
    case SYSOPT_Fog: cur.fogType = def.fogType; break;
    case SYSOPT_GuiScale: cur.guiScale = def.guiScale; break;
    case SYSOPT_GuiStyle: cur.guiStyle = def.guiStyle; cur.guiScale = def.guiScale; break;
    case SYSOPT_Stereo: cur.stereoEnabled = def.stereoEnabled; break;
    case SYSOPT_DisableControllerUnfocused: cur.disableControllerInputWhenUnfocused = def.disableControllerInputWhenUnfocused; break;
    default: return;
    }
    ApplyAndRefreshVideoSettings(cur);
}

int DefaultKeyboardBinding(int button)
{
    return (button >= 0 && button < BTN_MAX) ? s_videoDefaults.keyboardBindings[button] : 0;
}

int DefaultGamepadBinding(int button)
{
    return (button >= 0 && button < BTN_MAX) ? s_videoDefaults.gamepadBindings[button] : -1;
}

void ResetSystemSettingsToDefault()
{
    if (!s_fHaveVideoDefaults)
        return;
    // lastSaveSlot is not a setting: ApplyAndRefreshVideoSettings keeps it.
    ApplyAndRefreshVideoSettings(s_videoDefaults);
}

namespace
{
std::filesystem::path SystemSettingsPath();   // defined below (anonymous namespace)
}

// Copies every settings file into "Settings Backups\<date_time>" next to the
// exe. To restore, copy the files back (system_settings.dat goes in Saves).
bool ExportSettingsBackup(char* pchzFolder, size_t cchFolder)
{
    SaveSystemSettings();

    wchar_t executablePath[MAX_PATH]{};
    if (GetModuleFileNameW(nullptr, executablePath, MAX_PATH) == 0)
        return false;
    const std::filesystem::path exeDir = std::filesystem::path(executablePath).parent_path();

    SYSTEMTIME time{};
    GetLocalTime(&time);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%04d-%02d-%02d_%02d-%02d-%02d",
        time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

    const std::filesystem::path folder = exeDir / L"Settings Backups" / stamp;
    std::error_code ec;
    std::filesystem::create_directories(folder, ec);
    if (ec)
        return false;

    const wchar_t* const apwchzIni[] = { L"DLSS.ini", L"FSR.ini", L"QOL.ini", L"Mods.ini", L"MenuStyle.ini", L"Mouse.ini" };
    int copied = 0;
    for (const wchar_t* pwchz : apwchzIni)
    {
        const std::filesystem::path source = exeDir / pwchz;
        if (std::filesystem::exists(source, ec) &&
            std::filesystem::copy_file(source, folder / pwchz, std::filesystem::copy_options::overwrite_existing, ec))
            ++copied;
    }
    const std::filesystem::path system = SystemSettingsPath();
    if (std::filesystem::exists(system, ec) &&
        std::filesystem::copy_file(system, folder / system.filename(), std::filesystem::copy_options::overwrite_existing, ec))
        ++copied;

    // Options stored in the game save rather than a settings file.
    {
        std::ofstream options(folder / L"GameOptions.ini");
        if (options)
        {
            options << "[Game]\n";
            if (g_pgsCur != nullptr)
                options << "InvertFlags=" << (g_pgsCur->grfgs & 0x1E20) << "\n";
            options << "Vibration=" << vibrationSetting << "\n";
        }
    }

    std::ofstream readme(folder / L"README.txt");
    if (readme)
    {
        readme << "ProjectCane settings exported before \"Reset to Default\".\n"
               << "To restore: close the game, copy the .ini files back next to the exe,\n"
               << "and copy system_settings.dat into the Saves folder.\n";
    }

    if (pchzFolder != nullptr && cchFolder > 0)
        std::snprintf(pchzFolder, cchFolder, "Settings Backups\\%s", stamp);
    return copied > 0;
}

namespace
{
std::filesystem::path SaveDirectory()
{
    return cane::UserDirectory() / "Saves";
}

std::filesystem::path SaveSlotPath(int slot)
{
    return SaveDirectory() / ("slot" + std::to_string(slot + 1) + ".sav");
}

std::filesystem::path SystemSettingsPath()
{
    return SaveDirectory() / "system_settings.dat";
}

bool WriteSaveSlot(int slot, const GS& save)
{
    std::error_code ec;
    std::filesystem::create_directories(SaveDirectory(), ec);
    if (ec)
        return false;

    const std::filesystem::path path = SaveSlotPath(slot);
    std::filesystem::path temporary = path;
    temporary += ".tmp";
    const GS payload = save;
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

    return cane::AtomicReplace(temporary, path);
}

bool ReadSaveSlot(int slot, GS* save, VIDEOSAVESETTINGS* legacyVideo, bool* hasLegacyVideo)
{
    if (save == nullptr || legacyVideo == nullptr || hasLegacyVideo == nullptr)
        return false;

    std::ifstream stream(SaveSlotPath(slot), std::ios::binary);
    SaveFileHeader header{};
    if (!stream.read(reinterpret_cast<char*>(&header), sizeof(header)) ||
        std::memcmp(header.magic, kSaveMagic, sizeof(kSaveMagic)) != 0)
    {
        *save = {};
        *legacyVideo = {};
        *hasLegacyVideo = false;
        return false;
    }

    if (header.version == 1 && header.payloadSize == sizeof(GS))
    {
        GS loaded{};
        if (!stream.read(reinterpret_cast<char*>(&loaded), sizeof(loaded)) ||
            SaveChecksum(&loaded, sizeof(loaded)) != header.checksum)
            return false;

        *save = loaded;
        *legacyVideo = {};
        *hasLegacyVideo = false;
        return true;
    }

    if (header.version == 2 && header.payloadSize == sizeof(SavePayloadV2))
    {
        SavePayloadV2 payload{};
        if (!stream.read(reinterpret_cast<char*>(&payload), sizeof(payload)) ||
            SaveChecksum(&payload, sizeof(payload)) != header.checksum)
            return false;

        *save = payload.gameState;
        *legacyVideo = UpgradeVideoSettings(payload.video);
        *hasLegacyVideo = true;
        return true;
    }

    if (header.version == kSaveVersion && header.payloadSize == sizeof(GS))
    {
        GS loaded{};
        if (!stream.read(reinterpret_cast<char*>(&loaded), sizeof(loaded)) ||
            SaveChecksum(&loaded, sizeof(loaded)) != header.checksum)
            return false;

        *save = loaded;
        *legacyVideo = {};
        *hasLegacyVideo = false;
        return true;
    }

    *save = {};
    *legacyVideo = {};
    *hasLegacyVideo = false;
    return false;
}

bool LoadSystemSettings(VIDEOSAVESETTINGS* settings)
{
    if (settings == nullptr)
        return false;

    std::ifstream stream(SystemSettingsPath(), std::ios::binary);
    SettingsFileHeader header{};
    if (!stream.read(reinterpret_cast<char*>(&header), sizeof(header)) ||
        std::memcmp(header.magic, kSettingsMagic, sizeof(kSettingsMagic)) != 0)
    {
        *settings = {};
        return false;
    }

    if (header.version == 1 && header.payloadSize == sizeof(VideoSaveSettingsV1))
    {
        VideoSaveSettingsV1 oldSettings{};
        if (!stream.read(reinterpret_cast<char*>(&oldSettings), sizeof(oldSettings)) ||
            SaveChecksum(&oldSettings, sizeof(oldSettings)) != header.checksum)
        {
            *settings = {};
            return false;
        }

        *settings = UpgradeVideoSettings(oldSettings);
        return true;
    }

	if (header.version == 2 && header.payloadSize == sizeof(VideoSaveSettingsV2))
	{
		VideoSaveSettingsV2 oldSettings{};
		if (!stream.read(reinterpret_cast<char*>(&oldSettings), sizeof(oldSettings)) ||
			SaveChecksum(&oldSettings, sizeof(oldSettings)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		*settings = UpgradeVideoSettings(oldSettings);
		return true;
	}

	if (header.version == 3 && header.payloadSize == sizeof(VideoSaveSettingsV3))
	{
		VideoSaveSettingsV3 oldSettings{};
		if (!stream.read(reinterpret_cast<char*>(&oldSettings), sizeof(oldSettings)) ||
			SaveChecksum(&oldSettings, sizeof(oldSettings)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		*settings = UpgradeVideoSettings(oldSettings);
		return true;
	}

	if (header.version == 4 && header.payloadSize == sizeof(VideoSaveSettingsV4))
	{
		VideoSaveSettingsV4 oldSettings{};
		if (!stream.read(reinterpret_cast<char*>(&oldSettings), sizeof(oldSettings)) ||
			SaveChecksum(&oldSettings, sizeof(oldSettings)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		*settings = UpgradeVideoSettings(oldSettings);
		return true;
	}

	if (header.version == 5 && header.payloadSize == sizeof(VideoSaveSettingsV5))
	{
		VideoSaveSettingsV5 oldSettings{};
		if (!stream.read(reinterpret_cast<char*>(&oldSettings), sizeof(oldSettings)) ||
			SaveChecksum(&oldSettings, sizeof(oldSettings)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		*settings = UpgradeVideoSettings(oldSettings);
		return true;
	}

	if (header.version == 6 && header.payloadSize == sizeof(VideoSaveSettingsV6))
	{
		VideoSaveSettingsV6 oldSettings{};
		if (!stream.read(reinterpret_cast<char*>(&oldSettings), sizeof(oldSettings)) ||
			SaveChecksum(&oldSettings, sizeof(oldSettings)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		*settings = UpgradeVideoSettings(oldSettings);
		return true;
	}

	if (header.version == 7 && header.payloadSize == sizeof(VideoSaveSettingsV7))
	{
		VideoSaveSettingsV7 oldV7{};
		if (!stream.read(reinterpret_cast<char*>(&oldV7), sizeof(oldV7)) ||
			SaveChecksum(&oldV7, sizeof(oldV7)) != header.checksum)
		{
			*settings = {};
			return false;
		}
		std::memcpy(settings, &oldV7, sizeof(oldV7));
		SetDefaultAudioSettings(*settings);
		return true;
	}

	if (header.version == 8 && header.payloadSize == sizeof(VideoSaveSettingsV8))
	{
		VideoSaveSettingsV8 oldV8{};
		if (!stream.read(reinterpret_cast<char*>(&oldV8), sizeof(oldV8)) ||
			SaveChecksum(&oldV8, sizeof(oldV8)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		std::memcpy(settings, &oldV8, sizeof(oldV8));
		settings->mouseCameraEnabled = 0;
		return true;
	}

	if (header.version == 9 && header.payloadSize == sizeof(VideoSaveSettingsV9))
	{
		VideoSaveSettingsV9 oldV9{};
		if (!stream.read(reinterpret_cast<char*>(&oldV9), sizeof(oldV9)) ||
			SaveChecksum(&oldV9, sizeof(oldV9)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		std::memcpy(settings, &oldV9, sizeof(oldV9));
		settings->cutsceneSkipControlsEnabled = 0;
		return true;
	}

    if (header.version != kSettingsVersion || header.payloadSize != sizeof(*settings) ||
        !stream.read(reinterpret_cast<char*>(settings), sizeof(*settings)) ||
        SaveChecksum(settings, sizeof(*settings)) != header.checksum)
    {
        *settings = {};
        return false;
    }

    return true;
}
}

bool SaveSystemSettings()
{
    std::error_code ec;
    std::filesystem::create_directories(SaveDirectory(), ec);
    if (ec)
        return false;

    const VIDEOSAVESETTINGS settings = CaptureVideoSettings();
    const SettingsFileHeader header{ { 'P', 'C', 'A', 'N', 'E', 'S', 'Y', 'S' },
        kSettingsVersion, static_cast<uint32_t>(sizeof(settings)),
        SaveChecksum(&settings, sizeof(settings)) };
    const std::filesystem::path path = SystemSettingsPath();
    std::filesystem::path temporary = path;
    temporary += ".tmp";
    std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
    if (!stream)
        return false;

    stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
    stream.write(reinterpret_cast<const char*>(&settings), sizeof(settings));
    stream.flush();
    if (!stream)
        return false;
    stream.close();

    return cane::AtomicReplace(temporary, path);
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
    if (g_fShowAutosaveIcon && psaveblot->blots == BLOTS_Hidden)
    {
        psaveblot->fSaveComplete = 0;
        ShowBlot(psaveblot);
    }

    if (psaveblot->blots == BLOTS_Hidden)
        return;

    const float dtVisible = g_clock.tReal - psaveblot->tBlots;

    if (!g_fShowAutosaveIcon &&
        psaveblot->blots == BLOTS_Visible && g_letterbox.blots == BLOTS_Visible)
        psaveblot->fSaveComplete = dtVisible < 2.0f;

    float yOffset = 0.0f;

    if (psaveblot->fSaveComplete)
        yOffset = (1.0f - g_letterbox.uOn) * 66.40001f;
    else if (!g_fShowAutosaveIcon &&
        psaveblot->blots == BLOTS_Visible && dtVisible > 1.0f)
        HideBlot(psaveblot);

    const GuiScale guiScale = GetGuiScale();
    const float dxAutosave = psaveblot->dx * guiScale.x;
    const float dyAutosave = psaveblot->dy * guiScale.y;

    // Retail positions this bottom-anchored blot after measuring its scaled
    // text. ProjectCane keeps the authored blot dimensions, so preserve the
    // already-correct 18/20 pixel margins and move the enlarged box upward by
    // the extra height instead of allowing it to grow into the letterbox.
    const float xAutosave = psaveblot->x;
    const float yAutosave = psaveblot->y - (dyAutosave - psaveblot->dy) +
        yOffset * guiScale.y;

    CTextBox tbx;
    tbx.SetPos(xAutosave, yAutosave);
    tbx.SetSize(dxAutosave, dyAutosave);
    tbx.SetTextColor(&psaveblot->rgba);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    const char chPrompt = GetAnimatedPromptCharacter();
    const int percent = g_pgsCur != nullptr
        ? CalculatePercentCompletion(g_pgsCur)
        : 0;

    char achz[32];
    // Exact retail format: switch to the percentage font, apply its 0.7
    // rich-text scale and yellow color, then restore the base font.
    std::snprintf(achz, sizeof(achz), "%c&1^07~b2a83d%d%%&.", chPrompt, percent);

    psaveblot->pfont->PushScaling(psaveblot->rFontScale, psaveblot->rFontScale);

    CRichText richText(achz, psaveblot->pfont);
    richText.Draw(&tbx, nullptr, guiScale.x, guiScale.y);

    psaveblot->pfont->PopScaling();
}

void AutosaveCurrentGame(SAVEDATA* psaveData)
{
    if (!SaveCurrentGameToDisk(psaveData))
        return;

    g_autosave.fSaveComplete = 0;
    if (g_autosave.pvtblot != nullptr && g_autosave.pvtblot->pfnShowBlot != nullptr)
        g_autosave.pvtblot->pfnShowBlot(&g_autosave);
}

void StartupSaveData(SAVEDATA* psaveData)
{
    if (psaveData == nullptr)
        return;

    *psaveData = {};

    // Remember the built-in defaults for Options > Reset to Default.
    s_videoDefaults = CaptureVideoSettings();
    SetDefaultAudioSettings(s_videoDefaults);
    s_fHaveVideoDefaults = true;

    VIDEOSAVESETTINGS settings{};
    bool hasSystemSettings = LoadSystemSettings(&settings);

    for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
    {
        VIDEOSAVESETTINGS legacySettings{};
        bool hasLegacySettings = false;
        ReadSaveSlot(slot, &psaveData->saveData[slot], &legacySettings, &hasLegacySettings);

        // Migrate the first old per-slot video block when no global settings
        // file exists yet. New game saves never carry system options.
        if (!hasSystemSettings && hasLegacySettings)
        {
            settings = legacySettings;
            hasSystemSettings = true;
        }
    }

    if (hasSystemSettings)
    {
        ApplySavedVideoSettings(settings);
        SaveSystemSettings();
    }
	else
	{
		settings = CaptureVideoSettings();
		SetDefaultAudioSettings(settings);
		ApplySavedVideoSettings(settings);
		SaveSystemSettings();
	}

    if (s_lastSaveSlot >= 0 && s_lastSaveSlot < SAVE_SLOT_COUNT &&
        psaveData->saveData[s_lastSaveSlot].dt > 0.0f)
    {
        psaveData->pgsAttractSave = &psaveData->saveData[s_lastSaveSlot];
    }

    for (GS& save : psaveData->saveData)
    {
        if (psaveData->pgsAttractSave == nullptr && save.dt > 0.0f)
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

    if (!WriteSaveSlot(static_cast<int>(slot), snapshot))
        return false;

    psaveData->saveData[slot] = snapshot;
    psaveData->pgsCurrentSave = &psaveData->saveData[slot];
    psaveData->pgsAttractSave = psaveData->pgsCurrentSave;
    s_lastSaveSlot = static_cast<int>(slot);
    SaveSystemSettings();
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
    ApplyVibrationSetting(g_pgsCur);
    const ptrdiff_t slot = psaveData->pgsCurrentSave - psaveData->saveData;
    if (slot >= 0 && slot < SAVE_SLOT_COUNT)
    {
        psaveData->pgsAttractSave = psaveData->pgsCurrentSave;
        s_lastSaveSlot = static_cast<int>(slot);
        SaveSystemSettings();
    }
    return 1;
}

SAVEDATA g_saveData;
SAVEBLOT g_autosave;
VTSAVEBLOT g_vtsaveblot;

// -----------------------------------------------------------------------------
// Settings import (Options > Import / Export Settings)
// -----------------------------------------------------------------------------
namespace
{
std::vector<std::filesystem::path> s_settingsBackups;

std::filesystem::path ExeDirectoryForSettings()
{
    wchar_t executablePath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    return std::filesystem::path(executablePath).parent_path();
}
}

int ListSettingsBackups(char (*pachzNames)[64], int cMax)
{
    s_settingsBackups.clear();
    std::error_code ec;
    const std::filesystem::path dir = ExeDirectoryForSettings() / L"Settings Backups";
    if (!std::filesystem::is_directory(dir, ec))
        return 0;

    std::vector<std::filesystem::path> folders;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dir, ec))
        if (entry.is_directory(ec))
            folders.push_back(entry.path());
    // Folder names are date_time stamps: newest first.
    std::sort(folders.begin(), folders.end(),
        [](const std::filesystem::path& a, const std::filesystem::path& b) { return a.filename() > b.filename(); });

    const int count = (std::min)(cMax, static_cast<int>(folders.size()));
    for (int i = 0; i < count; ++i)
    {
        s_settingsBackups.push_back(folders[i]);
        std::string name = folders[i].filename().string();
        // "2026-10-10_02-56-12" -> "2026-10-10 02:56:12"
        if (name.size() == 19 && name[10] == '_' && name[13] == '-' && name[16] == '-')
        {
            name[10] = ' ';
            name[13] = ':';
            name[16] = ':';
        }
        std::snprintf(pachzNames[i], 64, "Import: %s", name.c_str());
    }
    return count;
}

bool ImportSettingsBackup(int index)
{
    if (index < 0 || index >= static_cast<int>(s_settingsBackups.size()))
        return false;

    std::error_code ec;
    const std::filesystem::path folder = s_settingsBackups[static_cast<size_t>(index)];
    const std::filesystem::path exeDir = ExeDirectoryForSettings();
    int copied = 0;

    const wchar_t* const apwchzIni[] = { L"DLSS.ini", L"FSR.ini", L"QOL.ini", L"Mods.ini", L"MenuStyle.ini", L"Mouse.ini" };
    for (const wchar_t* pwchz : apwchzIni)
    {
        const std::filesystem::path source = folder / pwchz;
        if (std::filesystem::exists(source, ec) &&
            std::filesystem::copy_file(source, exeDir / pwchz, std::filesystem::copy_options::overwrite_existing, ec))
            ++copied;
    }

    const std::filesystem::path systemTarget = SystemSettingsPath();
    const std::filesystem::path systemSource = folder / systemTarget.filename();
    bool fSystem = false;
    if (std::filesystem::exists(systemSource, ec))
    {
        std::filesystem::create_directories(systemTarget.parent_path(), ec);
        fSystem = std::filesystem::copy_file(systemSource, systemTarget, std::filesystem::copy_options::overwrite_existing, ec);
        if (fSystem)
            ++copied;
    }

    const std::wstring options = (folder / L"GameOptions.ini").wstring();
    if (std::filesystem::exists(options, ec))
    {
        const int flags = static_cast<int>(GetPrivateProfileIntW(L"Game", L"InvertFlags", -1, options.c_str()));
        if (flags >= 0 && g_pgsCur != nullptr)
            g_pgsCur->grfgs = (g_pgsCur->grfgs & ~0x1E20) | (flags & 0x1E20);
        const int vibration = static_cast<int>(GetPrivateProfileIntW(L"Game", L"Vibration", -1, options.c_str()));
        if (vibration >= 0 && vibration <= 3)
            vibrationSetting = vibration;
    }

    // Video / sound / bindings take effect right away.
    if (fSystem)
    {
        VIDEOSAVESETTINGS settings{};
        if (LoadSystemSettings(&settings))
        {
            settings.lastSaveSlot = s_lastSaveSlot;
            ApplySavedVideoSettings(settings);
            SaveSystemSettings();
        }
    }
    return copied > 0;
}

#include "save.h"
#include "clock.h"
#include "cm.h"
#include "gl.h"
#include "gui_layout.h"
#include "glob.h"
#include "debug.h"
#include "sound.h"
#include "shd.h"
#include "Input.h"
#include "content.h"
#ifdef __SWITCH__
#include "io_worker.h"
#endif
#include "../platform/platform.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{
constexpr char kSaveMagic[8] = { 'P', 'C', 'A', 'N', 'E', 'S', 'A', 'V' };
constexpr uint32_t kSaveVersion = 3;
constexpr char kSettingsMagic[8] = { 'P', 'C', 'A', 'N', 'E', 'S', 'Y', 'S' };
constexpr uint32_t kSettingsVersion = 15;
int s_lastSaveSlot = -1;

std::filesystem::path RecoverWrittenFile(const std::filesystem::path& path)
{
#ifdef __SWITCH__
    return CaneIo::RecoverFile(path);
#else
    return path;
#endif
}

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

struct VideoSaveSettingsV10
{
	VideoSaveSettingsV9 base;
	int cutsceneSkipControlsEnabled;
};

struct VideoSaveSettingsV11
{
	VideoSaveSettingsV10 base;
	float mouseCameraSensitivity;
};

struct VideoSaveSettingsV12
{
	VideoSaveSettingsV11 base;
	int autoIconify;
};

struct VideoSaveSettingsV13
{
	VideoSaveSettingsV11 base;
	int autoIconify;
	int anisotropicFiltering;
};

// Version 14 ended immediately before the stable monitor-name field.
struct VideoSaveSettingsV14
{
	std::byte bytes[offsetof(VIDEOSAVESETTINGS, monitorName)];
};
static_assert(sizeof(VideoSaveSettingsV14) == offsetof(VIDEOSAVESETTINGS, monitorName));

void SetDefaultExtendedSettings(VIDEOSAVESETTINGS& settings)
{
	settings.outputGamma = 1.0f;
	settings.textureFiltering = 1;
	settings.monitorIndex = 0;
	settings.resolutionScalePercent = 100;
	settings.controllerDeadzone = 0.35f;
	settings.audioOutputDeviceId[0] = '\0';
	settings.monitorName[0] = '\0';
}

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
	settings.autoIconify = 0;
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
	settings.autoIconify = 0;
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
	settings.autoIconify = 0;
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
	settings.autoIconify = 0;
	SetDefaultAudioSettings(settings);
	return settings;
}

VIDEOSAVESETTINGS UpgradeVideoSettings(const VideoSaveSettingsV5& oldSettings)
{
	VIDEOSAVESETTINGS settings{};
	std::memcpy(&settings, &oldSettings, sizeof(oldSettings));
	settings.disableControllerInputWhenUnfocused = 0;
	settings.lastSaveSlot = -1;
	settings.autoIconify = 0;
	SetDefaultAudioSettings(settings);
	return settings;
}

VIDEOSAVESETTINGS UpgradeVideoSettings(const VideoSaveSettingsV6& oldSettings)
{
	VIDEOSAVESETTINGS settings{};
	std::memcpy(&settings, &oldSettings, sizeof(oldSettings));
	settings.lastSaveSlot = -1;
	settings.autoIconify = 0;
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
	settings.mouseCameraSensitivity = g_mouseCameraSensitivity;
	settings.autoIconify = g_fAutoIconify ? 1 : 0;
	settings.anisotropicFiltering = g_anisotropicFiltering;
	settings.outputGamma = g_outputGamma;
	settings.textureFiltering = g_textureFiltering;
	settings.monitorIndex = g_monitorIndex;
	settings.resolutionScalePercent = g_resolutionScalePercent;
	settings.controllerDeadzone = g_controllerDeadzone;
	const std::string audioDeviceId = GetAudioOutputDeviceId();
	std::snprintf(settings.audioOutputDeviceId, sizeof(settings.audioOutputDeviceId), "%s",
		audioDeviceId.c_str());
	std::snprintf(settings.monitorName, sizeof(settings.monitorName), "%s",
		VideoMonitorName(g_monitorIndex));
    return settings;
}

GLFWmonitor* CurrentMonitor()
{
    if (g_gl.window != nullptr)
    {
        if (GLFWmonitor* monitor = glfwGetWindowMonitor(g_gl.window))
            return monitor;
    }

	int count = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	if (monitors != nullptr && count > 0)
		return monitors[std::clamp(g_monitorIndex, 0, count - 1)];
    return glfwGetPrimaryMonitor();
}

void ValidateVideoSettings(VIDEOSAVESETTINGS& settings)
{
	const bool legacyExtendedSettings = !std::isfinite(settings.outputGamma) ||
		settings.outputGamma <= 0.0f || settings.resolutionScalePercent == 0;
	if (legacyExtendedSettings)
	{
		settings.outputGamma = 1.0f;
		settings.textureFiltering = 1;
		settings.resolutionScalePercent = 100;
		settings.controllerDeadzone = 0.35f;
		settings.monitorIndex = 0;
		settings.audioOutputDeviceId[0] = '\0';
	}
    settings.fogType = std::clamp(settings.fogType, 0, 2);
    settings.msaaEnabled = settings.msaaEnabled != 0;
	if (settings.frameRate < 30 || settings.frameRate > 1000)
		settings.frameRate = 60;
	if (!std::isfinite(settings.drawDistance)) settings.drawDistance = 1.0f;
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
	settings.autoIconify = settings.autoIconify != 0;
	if (!std::isfinite(settings.outputGamma)) settings.outputGamma = 1.0f;
	settings.outputGamma = std::clamp(settings.outputGamma, 0.50f, 1.50f);
	settings.textureFiltering = settings.textureFiltering != 0;
	settings.monitorIndex = std::clamp(settings.monitorIndex, 0, VideoMonitorCount() - 1);
	settings.resolutionScalePercent = std::clamp(settings.resolutionScalePercent, 50, 200);
	if (!std::isfinite(settings.controllerDeadzone)) settings.controllerDeadzone = 0.35f;
	settings.controllerDeadzone = std::clamp(settings.controllerDeadzone, 0.05f, 0.50f);
	settings.audioOutputDeviceId[sizeof(settings.audioOutputDeviceId) - 1] = '\0';
	settings.monitorName[sizeof(settings.monitorName) - 1] = '\0';
	const int validAnisotropyLevels[] = { 1, 2, 4, 8, 16 };
	const int maximumAnisotropy = MaxSupportedAnisotropicFiltering();
	if (std::find(std::begin(validAnisotropyLevels), std::end(validAnisotropyLevels),
		settings.anisotropicFiltering) == std::end(validAnisotropyLevels) ||
		settings.anisotropicFiltering > maximumAnisotropy)
	{
		settings.anisotropicFiltering = 1;
	}
	if (!std::isfinite(settings.mouseCameraSensitivity) || settings.mouseCameraSensitivity <= 0.0f)
		settings.mouseCameraSensitivity = 1.0f;
	settings.mouseCameraSensitivity = std::clamp(settings.mouseCameraSensitivity, 0.25f, 2.0f);
    settings.lastSaveSlot = std::clamp(settings.lastSaveSlot, -1, SAVE_SLOT_COUNT - 1);
	settings.stereoEnabled = settings.stereoEnabled != 0;
	if (!std::isfinite(settings.musicVolume)) settings.musicVolume = 1.0f;
	if (!std::isfinite(settings.soundEffectsVolume)) settings.soundEffectsVolume = 1.0f;
	if (!std::isfinite(settings.dialogueVolume)) settings.dialogueVolume = 1.0f;
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
		if (!FMouseBinding(key) &&
			(key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST || key == GLFW_KEY_ESCAPE))
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

    const int validInternalResolutions[] = { 0, 360, 448, 480, 720, 1080, 1440, 2160 };
    if (std::find(std::begin(validInternalResolutions), std::end(validInternalResolutions),
        settings.internalResolutionHeight) == std::end(validInternalResolutions))
        settings.internalResolutionHeight = 0;

    GLint maxSamples = 1;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    const int validMsaaSamples[] = { 2, 4, 8, 16 };
    if (std::find(std::begin(validMsaaSamples), std::end(validMsaaSamples),
        settings.msaaSamples) == std::end(validMsaaSamples) ||
        settings.msaaSamples > maxSamples)
    {
        settings.msaaEnabled = 0;
        settings.msaaSamples = 1;
    }
}

void ApplySavedVideoSettings(VIDEOSAVESETTINGS settings)
{
    ValidateVideoSettings(settings);

	const int namedMonitor = VideoMonitorIndexByName(settings.monitorName);
	g_monitorIndex = namedMonitor >= 0 ? namedMonitor : settings.monitorIndex;
	if (!ApplyWindowModeSettings(static_cast<WindowMode>(settings.windowMode)))
		ApplyWindowModeSettings(WindowMode_Windowed);
	if (g_windowMode == WindowMode_Windowed)
		ApplyMonitorSettings(g_monitorIndex);
    g_targetFrameRate = settings.frameRate;
    g_internalResolutionHeight = settings.internalResolutionHeight;
	g_resolutionScalePercent = settings.resolutionScalePercent;
	if (g_internalResolutionHeight != 0) g_resolutionScalePercent = 100;
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
	g_mouseCameraSensitivity = settings.mouseCameraSensitivity;
	ApplyAutoIconifySettings(settings.autoIconify != 0);
	ApplyAnisotropicFilteringSettings(settings.anisotropicFiltering);
	ApplyTextureFilteringSettings(settings.textureFiltering);
	ApplyOutputGammaSettings(settings.outputGamma);
	g_controllerDeadzone = settings.controllerDeadzone;
	SetAudioOutputDeviceId(settings.audioOutputDeviceId);
    s_lastSaveSlot = settings.lastSaveSlot;
	SetUserStereoEnabled(settings.stereoEnabled != 0);
	SetUserMusicVolume(settings.musicVolume);
	SetUserSfxVolume(settings.soundEffectsVolume);
	SetUserDialogueVolume(settings.dialogueVolume);

	ApplyAspectRatioSettings(static_cast<AspectMode>(settings.aspectMode));
	if (!ApplyInternalResolutionSettings())
	{
		g_internalResolutionHeight = 0;
		g_resolutionScalePercent = 100;
		ApplyInternalResolutionSettings();
	}
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

    std::ifstream stream(RecoverWrittenFile(SaveSlotPath(slot)), std::ios::binary);
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
#ifdef __SWITCH__
    CaneIo::Drain();
#endif

    std::ifstream stream(RecoverWrittenFile(SystemSettingsPath()), std::ios::binary);
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
		settings->autoIconify = 0;
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
		settings->autoIconify = 0;
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
		settings->mouseCameraSensitivity = 1.0f;
		settings->autoIconify = 0;
		return true;
	}

	if (header.version == 10 && header.payloadSize == sizeof(VideoSaveSettingsV10))
	{
		VideoSaveSettingsV10 oldV10{};
		if (!stream.read(reinterpret_cast<char*>(&oldV10), sizeof(oldV10)) ||
			SaveChecksum(&oldV10, sizeof(oldV10)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		std::memcpy(settings, &oldV10, sizeof(oldV10));
		settings->mouseCameraSensitivity = 1.0f;
		settings->autoIconify = 0;
		return true;
	}

	if (header.version == 11 && header.payloadSize == sizeof(VideoSaveSettingsV11))
	{
		VideoSaveSettingsV11 oldV11{};
		if (!stream.read(reinterpret_cast<char*>(&oldV11), sizeof(oldV11)) ||
			SaveChecksum(&oldV11, sizeof(oldV11)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		std::memcpy(settings, &oldV11, sizeof(oldV11));
		settings->autoIconify = 0;
		return true;
	}

	if (header.version == 12 && header.payloadSize == sizeof(VideoSaveSettingsV12))
	{
		VideoSaveSettingsV12 oldV12{};
		if (!stream.read(reinterpret_cast<char*>(&oldV12), sizeof(oldV12)) ||
			SaveChecksum(&oldV12, sizeof(oldV12)) != header.checksum)
		{
			*settings = {};
			return false;
		}

		std::memcpy(settings, &oldV12, sizeof(oldV12));
		settings->anisotropicFiltering = 1;
		SetDefaultExtendedSettings(*settings);
		return true;
	}

	if (header.version == 13 && header.payloadSize == sizeof(VideoSaveSettingsV13))
	{
		VideoSaveSettingsV13 oldV13{};
		if (!stream.read(reinterpret_cast<char*>(&oldV13), sizeof(oldV13)) ||
			SaveChecksum(&oldV13, sizeof(oldV13)) != header.checksum)
		{
			*settings = {};
			return false;
		}
		std::memcpy(settings, &oldV13, sizeof(oldV13));
		SetDefaultExtendedSettings(*settings);
		return true;
	}

	if (header.version == 14 && header.payloadSize == sizeof(VideoSaveSettingsV14))
	{
		VideoSaveSettingsV14 oldV14{};
		if (!stream.read(reinterpret_cast<char*>(&oldV14), sizeof(oldV14)) ||
			SaveChecksum(&oldV14, sizeof(oldV14)) != header.checksum)
		{
			*settings = {};
			return false;
		}
		std::memcpy(settings, &oldV14, sizeof(oldV14));
		settings->monitorName[0] = '\0';
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

namespace
{
bool WriteSystemSettings(const std::filesystem::path& path,
    const SettingsFileHeader& header, const VIDEOSAVESETTINGS& settings)
{
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec)
        return false;

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
}

bool SaveSystemSettings()
{

    const VIDEOSAVESETTINGS settings = CaptureVideoSettings();
    const SettingsFileHeader header{ { 'P', 'C', 'A', 'N', 'E', 'S', 'Y', 'S' },
        kSettingsVersion, static_cast<uint32_t>(sizeof(settings)),
        SaveChecksum(&settings, sizeof(settings)) };
    const std::filesystem::path path = SystemSettingsPath();
#ifdef __SWITCH__
    return CaneIo::QueueSettings([path,header,settings] {
        return WriteSystemSettings(path,header,settings);
    });
#else
    return WriteSystemSettings(path,header,settings);
#endif
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
    // Exact retail format: switch to the percentage font, apply its authored
    // 0.7 rich-text scale and yellow color, then restore the base font.
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
		settings.frameRate = FPalVideoStandard() ? 50 : 60;
		settings.autoIconify = 0;
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

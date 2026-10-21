#include "prompt.h"
#include "ui.h"
#include "dialog.h"
#include "wm.h"
#include "sm.h"
#include "credit.h"
#include "coin.h"
#include "pzo.h"
#include "binoc.h"
#include "rog.h"
#include "ctr.h"
#include "screen.h"
#include "gl.h"
#include "shd.h"
#include "gui_layout.h"
#include "debug.h"
#include "Input.h"
#include "content.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <vector>

namespace
{
int s_iKeyboardBindingCapture = -1;
bool s_fKeyboardBindingCaptureArmed = false;
bool s_fControllerBindingCapture = false;

const char* PcText(const char* english, const char* french, const char* german,
	const char* spanish, const char* italian)
{
	if (FJapaneseContent())
	{
		struct JapanesePcText { const char* english; const char* japanese; };
		static constexpr JapanesePcText translations[] =
		{
			{"On", "オン"}, {"Off", "オフ"},
			{"Video", "ビデオ"}, {"Sound", "サウンド"}, {"Controls", "操作設定"},
			{"Keyboard Mapping", "キーボード設定"}, {"Controller Mapping", "コントローラー設定"},
			{"Frame Rate", "フレームレート"}, {"Resolution", "解像度"},
			{"Scaled", "スケーリング"}, {"Native", "ネイティブ"}, {"Fog", "フォグ"},
			{"Low", "低"}, {"Original", "オリジナル"}, {"High", "高"},
			{"Very High", "最高"}, {"Extreme", "極限"}, {"Draw Distance", "描画距離"},
			{"Custom", "カスタム"}, {"Windowed", "ウィンドウ"},
			{"Borderless", "ボーダーレス"}, {"Fullscreen", "フルスクリーン"},
			{"Window Mode", "表示モード"}, {"Minimize When Unfocused", "非アクティブ時に最小化"},
			{"Anisotropic Filtering", "異方性フィルタリング"},
			{"Brightness / Gamma", "明るさ / ガンマ"},
			{"Texture Filtering", "テクスチャフィルタリング"}, {"Monitor", "モニター"},
			{"Trilinear", "トライリニア"}, {"Bilinear", "バイリニア"},
			{"Resolution Scale", "解像度スケール"},
			{"Controller Deadzone", "コントローラーのデッドゾーン"},
			{"System Default", "システム既定"}, {"Audio Output", "オーディオ出力"},
			{"Fit to Screen", "画面に合わせる"}, {"Aspect Ratio", "アスペクト比"},
			{"GUI Scale", "GUIサイズ"}, {"GUI Style", "GUIスタイル"}, {"Modern", "モダン"},
			{"Disable Controller When Unfocused", "非アクティブ時にコントローラーを無効化"},
			{"Mouse Camera", "マウスカメラ"}, {"Mouse Sensitivity", "マウス感度"},
			{"Dialogue Skip Controls", "セリフスキップ操作"}, {"Camera Invert", "カメラ反転"},
			{"Press a button", "ボタンを押してください"},
			{"Press a key or mouse button", "キーまたはマウスボタンを押してください"},
			{"Speakers", "スピーカー"}, {"Stereo", "ステレオ"}, {"Mono", "モノラル"},
			{"Music Volume", "音楽音量"},
			{"Sound Effects", "効果音"}, {"Dialogue", "セリフ"},
			{"Video...", "ビデオ..."}, {"Sound...", "サウンド..."},
			{"Keyboard Mapping...", "キーボード設定..."},
			{"Controller Mapping...", "コントローラー設定..."},
			{"Reset Video Settings", "ビデオ設定をリセット"},
			{"Reset Mappings", "操作設定をリセット"},
			{"Exit to Desktop", "デスクトップに戻る"}, {"Are you sure?", "よろしいですか？"},
			{"New Game", "ニューゲーム"}, {"Empty", "データなし"}, {"Unknown", "不明"},
			{"Locked", "未解除"}
		};
		for (const JapanesePcText& translation : translations)
		{
			if (std::strcmp(english, translation.english) == 0)
				return translation.japanese;
		}
	}
	return ContentUiText(english, french, german, spanish, italian);
}

const char* PcOnOff(bool enabled)
{
	return enabled
		? PcText("On", "Activé", "Ein", "Activado", "Attivo")
		: PcText("Off", "Désactivé", "Aus", "Desactivado", "Disattivato");
}

constexpr const char* s_apchzKeyboardActions[BTN_MAX] =
{
    "Move Up", "Move Down", "Move Left", "Move Right",
    "Square", "Circle", "Cross", "Triangle",
    "Start", "Select", "L1", "R1", "L2", "R2", "L3", "R3"
};

const char* PcKeyboardAction(int button)
{
	static constexpr const char* french[BTN_MAX] =
	{
		"Avancer", "Reculer", "Aller à gauche", "Aller à droite",
		"Carré", "Cercle", "Croix", "Triangle", "Start", "Select",
		"L1", "R1", "L2", "R2", "L3", "R3"
	};
	static constexpr const char* german[BTN_MAX] =
	{
		"Nach oben", "Nach unten", "Nach links", "Nach rechts",
		"Quadrat", "Kreis", "Kreuz", "Dreieck", "Start", "Select",
		"L1", "R1", "L2", "R2", "L3", "R3"
	};
	static constexpr const char* spanish[BTN_MAX] =
	{
		"Mover arriba", "Mover abajo", "Mover izquierda", "Mover derecha",
		"Cuadrado", "Círculo", "Cruz", "Triángulo", "Start", "Select",
		"L1", "R1", "L2", "R2", "L3", "R3"
	};
	static constexpr const char* italian[BTN_MAX] =
	{
		"Muovi su", "Muovi giù", "Muovi a sinistra", "Muovi a destra",
		"Quadrato", "Cerchio", "Croce", "Triangolo", "Start", "Select",
		"L1", "R1", "L2", "R2", "L3", "R3"
	};
	static constexpr const char* japanese[BTN_MAX] =
	{
		"上へ移動", "下へ移動", "左へ移動", "右へ移動",
		"四角", "丸", "バツ", "三角", "START", "SELECT",
		"L1", "R1", "L2", "R2", "L3", "R3"
	};

	if (button < 0 || button >= BTN_MAX)
		return PcText("Unknown", "Inconnu", "Unbekannt", "Desconocido", "Sconosciuto");
	if (!FLocalizedContent())
		return s_apchzKeyboardActions[button];
	switch (ContentPackage().language)
	{
	case CONTENT_LANGUAGE::French: return french[button];
	case CONTENT_LANGUAGE::German: return german[button];
	case CONTENT_LANGUAGE::Spanish: return spanish[button];
	case CONTENT_LANGUAGE::Italian: return italian[button];
	case CONTENT_LANGUAGE::Japanese: return japanese[button];
	default: return s_apchzKeyboardActions[button];
	}
}

int KeyboardKeyPressed(GLFWwindow* window)
{
    for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; ++key)
        if (glfwGetKey(window, key) == GLFW_PRESS) return key;
    for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; ++key)
        if (glfwGetKey(window, key) == GLFW_PRESS) return key;
    for (int key = GLFW_KEY_F1; key <= GLFW_KEY_F25; ++key)
        if (glfwGetKey(window, key) == GLFW_PRESS) return key;
    for (int key = GLFW_KEY_KP_0; key <= GLFW_KEY_KP_EQUAL; ++key)
        if (glfwGetKey(window, key) == GLFW_PRESS) return key;

    constexpr int specialKeys[] =
    {
        GLFW_KEY_SPACE, GLFW_KEY_APOSTROPHE, GLFW_KEY_COMMA, GLFW_KEY_MINUS,
        GLFW_KEY_PERIOD, GLFW_KEY_SLASH, GLFW_KEY_SEMICOLON, GLFW_KEY_EQUAL,
        GLFW_KEY_LEFT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_RIGHT_BRACKET,
        GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_ENTER, GLFW_KEY_TAB, GLFW_KEY_BACKSPACE,
        GLFW_KEY_INSERT, GLFW_KEY_DELETE, GLFW_KEY_RIGHT, GLFW_KEY_LEFT,
        GLFW_KEY_DOWN, GLFW_KEY_UP, GLFW_KEY_PAGE_UP, GLFW_KEY_PAGE_DOWN,
        GLFW_KEY_HOME, GLFW_KEY_END, GLFW_KEY_CAPS_LOCK, GLFW_KEY_SCROLL_LOCK,
        GLFW_KEY_NUM_LOCK, GLFW_KEY_PRINT_SCREEN, GLFW_KEY_PAUSE,
        GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_ALT,
        GLFW_KEY_LEFT_SUPER, GLFW_KEY_RIGHT_SHIFT, GLFW_KEY_RIGHT_CONTROL,
        GLFW_KEY_RIGHT_ALT, GLFW_KEY_RIGHT_SUPER, GLFW_KEY_MENU
    };

    for (int key : specialKeys)
        if (glfwGetKey(window, key) == GLFW_PRESS) return key;
    return GLFW_KEY_UNKNOWN;
}

int GamepadBindingPressed()
{
    if (g_fDisableControllerInputWhenUnfocused &&
        (g_gl.window == nullptr ||
            glfwGetWindowAttrib(g_gl.window, GLFW_FOCUSED) != GLFW_TRUE))
    {
        return -1;
    }

    if (!g_joy.gamepadConnected || !g_joy.hasGamepadState)
        return -1;

    const GLFWgamepadstate& state = g_joy.gamepadState;

    for (int button = 0; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button)
        if (state.buttons[button] == GLFW_PRESS) return button;
    if (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.25f)
        return GAMEPAD_BINDING_LEFT_TRIGGER;
    if (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.25f)
        return GAMEPAD_BINDING_RIGHT_TRIGGER;
    return -1;
}

bool FGamepadAvailable()
{
    return g_joy.gamepadConnected || FControllerInputAvailable();
}

int KeyboardOrMouseBindingPressed(GLFWwindow* window)
{
    const int key = KeyboardKeyPressed(window);
    if (key != GLFW_KEY_UNKNOWN)
        return key;
    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button)
        if (MOUSE::Button(button))
            return MouseBindingFromButton(button);
    return GLFW_KEY_UNKNOWN;
}

bool FMappingPrompt(PRK prk)
{
	return prk == PRK_KeyboardMapping || prk == PRK_ControllerMapping;
}

PRD& PrdPrompt(PRK prk)
{
	// Keep PC-only mapping descriptors out of the release descriptor table.
	// The rest of the prompt system can still treat them exactly like a PRD,
	// without indexing beyond the original table storage in stale/incremental
	// builds.
	static PRD keyboardMapping =
	{
		"Keyboard Mapping", 0.8f, 0.55f, 1,
		static_cast<int>(std::size(s_arespkKeyboardMapping)),
		s_arespkKeyboardMapping
	};
	static PRD controllerMapping =
	{
		"Controller Mapping", 0.8f, 0.55f, 1,
		static_cast<int>(std::size(s_arespkKeyboardMapping)),
		s_arespkKeyboardMapping
	};
	static RESPK languageResponses[6]{};
	static PRD languageMenu =
	{
		"Language...", 1.0f, 1.0f, 1, 0, languageResponses
	};

	if (prk == PRK_KeyboardMapping)
		return keyboardMapping;
	if (prk == PRK_ControllerMapping)
		return controllerMapping;
	if (prk == PRK_LanguageMenu)
	{
		if (FJapaneseContent())
		{
			// Reuse the Italian response token as the Japanese voice choice;
			// response IDs are retail ABI values and must not be renumbered.
			languageResponses[0] = RESPK_LanguageItalian;
			languageResponses[1] = RESPK_LanguageEnglish;
			languageResponses[2] = RESPK_Back;
			languageMenu.crespk = 3;
			return languageMenu;
		}
		static constexpr RESPK responses[] =
		{
			RESPK_LanguageEnglish, RESPK_LanguageFrench, RESPK_LanguageGerman,
			RESPK_LanguageSpanish, RESPK_LanguageItalian
		};
		languageMenu.crespk = 0;
		for (int index = 0; index < 5; ++index)
		{
			if (FContentLanguageAvailable(static_cast<CONTENT_LANGUAGE>(index)))
				languageResponses[languageMenu.crespk++] = responses[index];
		}
		languageResponses[languageMenu.crespk++] = RESPK_Back;
		return languageMenu;
	}
	return s_mpprkprd[prk];
}

const char* PchzPromptTitle(PRK prk, const PRD& prd)
{
	if (!FLocalizedContent())
		return prd.pchz;
	if (prk >= PRK_PauseMenu && prk <= PRK_MtsFailedBestTime)
	{
		if (const char* localized = ContentLocalizedText(83 + static_cast<int>(prk)))
			return localized;
	}
	if (prk == PRK_LanguageMenu)
	{
		if (const char* localized = ContentLocalizedText(116))
			return localized;
	}
	if (prk == PRK_VideoMenu)
		return PcText("Video", "Vidéo", "Video", "Vídeo", "Video");
	if (prk == PRK_SoundMenu)
		return PcText("Sound", "Son", "Sound", "Sonido", "Audio");
	if (prk == PRK_ControlsMenu)
		return PcText("Controls", "Commandes", "Steuerung", "Controles", "Comandi");
	if (prk == PRK_KeyboardMapping)
		return PcText("Keyboard Mapping", "Configuration du clavier", "Tastaturbelegung", "Asignación del teclado", "Configurazione tastiera");
	if (prk == PRK_ControllerMapping)
		return PcText("Controller Mapping", "Configuration de la manette", "Controllerbelegung", "Asignación del mando", "Configurazione controller");
	if (prk == PRK_ExitToDesktopConfirm)
		return PcText("Are you sure?", "Êtes-vous sûr ?", "Sind Sie sicher?", "¿Está seguro?", "Sei sicuro?");
	return prd.pchz;
}

int PalResponseLocalizationIndex(RESPK respk, int variant)
{
	if (respk >= RESPK_Yes && respk <= RESPK_SaveGame)
		return 117 + static_cast<int>(respk);
	if (respk == RESPK_Quit)
		return 136 + variant;
	if (respk == RESPK_Vibration)
		return 138 + variant;
	if (respk == RESPK_CenterScreen)
		return 126;
	if (respk == RESPK_Music)
		return 140 + variant;
	if (respk == RESPK_Speakers)
		return 142 + variant;
	if (respk >= RESPK_Back && respk <= RESPK_Controls)
		return 127 + (static_cast<int>(respk) - static_cast<int>(RESPK_Back));
	if (respk >= RESPK_BinocInvert && respk <= RESPK_JetpackInvert)
		return 144 + (static_cast<int>(respk) - static_cast<int>(RESPK_BinocInvert)) * 2 + variant;
	// PAL stores these in locale-table order (English, French, Italian,
	// German, Spanish), while the retail language prompt displays/selects
	// English, French, German, Spanish, Italian.
	switch (respk)
	{
	case RESPK_LanguageEnglish: return 150;
	case RESPK_LanguageFrench: return 151;
	case RESPK_LanguageGerman: return 153;
	case RESPK_LanguageSpanish: return 154;
	case RESPK_LanguageItalian: return 152;
	default: break;
	}
	return -1;
}

const char* PalResponseText(RESPK respk, int variant)
{
	if (FJapaneseContent())
	{
		if (respk == RESPK_LanguageItalian)
			return ContentLocalizedText(155); // 日本語
		if (respk == RESPK_LanguageEnglish)
			return ContentLocalizedText(150);
	}
	const int index = PalResponseLocalizationIndex(respk, variant);
	return index >= 0 ? ContentLocalizedText(static_cast<std::size_t>(index)) : nullptr;
}

RESPK* ArespkPrompt(PRK prk, const PRD& prd)
{
	return FMappingPrompt(prk) ? s_arespkKeyboardMapping : prd.arespk;
}

int CrespkPrompt(PRK prk, const PRD& prd)
{
	return FMappingPrompt(prk)
		? static_cast<int>(std::size(s_arespkKeyboardMapping))
		: prd.crespk;
}

int VisiblePromptResponseCount(PRK prk, int responseCount)
{
	if (FMappingPrompt(prk))
		return std::min(responseCount, 9);
	if (prk == PRK_VideoMenu || prk == PRK_ControlsMenu)
		return std::min(responseCount, 10);
	return responseCount;
}

GLFWmonitor* ActiveMonitor()
{
    if (GLFWmonitor* monitor = glfwGetWindowMonitor(g_gl.window))
        return monitor;

    int windowX = 0;
    int windowY = 0;
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowPos(g_gl.window, &windowX, &windowY);
    glfwGetWindowSize(g_gl.window, &windowWidth, &windowHeight);

    const int centerX = windowX + windowWidth / 2;
    const int centerY = windowY + windowHeight / 2;
    int monitorCount = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

    for (int i = 0; i < monitorCount; ++i)
    {
        int monitorX = 0;
        int monitorY = 0;
        glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

        if (mode != nullptr && centerX >= monitorX && centerX < monitorX + mode->width &&
            centerY >= monitorY && centerY < monitorY + mode->height)
        {
            return monitors[i];
        }
    }

    return glfwGetPrimaryMonitor();
}

std::vector<int> SupportedFrameRates()
{
    // A frame cap is independent of the display modes exposed by GLFW.  Some
    // Linux compositors (including Steam Deck's game session) expose only the
    // current refresh rate, which previously left this option with one value
    // and made it appear broken.  Keep the universally useful caps available
    // and supplement them with the monitor's reported refresh rates.
    std::vector<int> frameRates{ 30, 60 };
    GLFWmonitor* monitor = ActiveMonitor();

    if (monitor != nullptr)
    {
        int modeCount = 0;
        const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);
		bool supports50Fps = false;

        for (int i = 0; i < modeCount; ++i)
        {
            if (modes[i].refreshRate >= 30)
                frameRates.push_back(modes[i].refreshRate);
			if (modes[i].refreshRate >= 50)
				supports50Fps = true;
        }
		if (supports50Fps)
			frameRates.push_back(50);
    }

    std::sort(frameRates.begin(), frameRates.end());
    frameRates.erase(std::unique(frameRates.begin(), frameRates.end()), frameRates.end());

    return frameRates;
}

std::vector<int> SupportedMsaaSamples()
{
	GLint maximum = 1;
	glGetIntegerv(GL_MAX_SAMPLES, &maximum);
	std::vector<int> samples{ 0 };
	for (int sampleCount : { 2, 4, 8, 16 })
	{
		if (sampleCount <= maximum)
			samples.push_back(sampleCount);
	}
	return samples;
}

void SelectSupportedFrameRate(int direction)
{
    const std::vector<int> frameRates = SupportedFrameRates();

    if (direction < 0)
    {
        auto previous = std::lower_bound(frameRates.begin(), frameRates.end(), g_targetFrameRate);
        if (previous == frameRates.begin())
            previous = frameRates.end();
        g_targetFrameRate = *--previous;
    }
    else
    {
        auto next = std::upper_bound(frameRates.begin(), frameRates.end(), g_targetFrameRate);
        if (next == frameRates.end())
            next = frameRates.begin();
        g_targetFrameRate = *next;
    }
}

void SelectInternalResolution(int direction)
{
    constexpr int resolutions[] = { 0, 360, 448, 480, 720, 1080, 1440, 2160 };
    auto current = std::find(std::begin(resolutions), std::end(resolutions),
        g_internalResolutionHeight);
    int index = current == std::end(resolutions)
        ? 0
        : static_cast<int>(current - std::begin(resolutions));
    constexpr int count = static_cast<int>(std::size(resolutions));
    index = (index + (direction < 0 ? -1 : 1) + count) % count;
	const int previousResolution = g_internalResolutionHeight;
	const int previousScale = g_resolutionScalePercent;
	g_internalResolutionHeight = resolutions[index];
	if (g_internalResolutionHeight != 0)
		g_resolutionScalePercent = 100;
	if (!ApplyInternalResolutionSettings())
	{
		g_internalResolutionHeight = previousResolution;
		g_resolutionScalePercent = previousScale;
		ApplyInternalResolutionSettings();
	}
}

void SelectGuiScale(int direction)
{
    constexpr float ps2Scales[] = { 0.75f, 1.0f };
    constexpr float modernScales[] = { 1.0f, 1.25f };
    const float* scales = g_guiStyle == GuiStyle_Modern ? modernScales : ps2Scales;
    constexpr int count = 2;
    int index = std::abs(g_guiScale - scales[1]) < 0.001f ? 1 : 0;
    index = (index + (direction < 0 ? -1 : 1) + count) % count;
    g_guiScale = scales[index];

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(g_gl.window, &width, &height);
    FrameBufferSizeCallBack(g_gl.window, width, height);
}

void SelectNextGuiStyle()
{
    g_guiStyle = g_guiStyle == GuiStyle_PS2 ? GuiStyle_Modern : GuiStyle_PS2;

    // 100% is the common scale between both styles. Normalize a value that
    // belongs only to the previous style when changing presentation modes.
    if ((g_guiStyle == GuiStyle_Modern && g_guiScale < 1.0f) ||
        (g_guiStyle == GuiStyle_PS2 && g_guiScale > 1.0f))
    {
        g_guiScale = 1.0f;
    }

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(g_gl.window, &width, &height);
    FrameBufferSizeCallBack(g_gl.window, width, height);
}

void SelectDrawDistance(int direction)
{
    if (g_pcm == nullptr)
        return;

    constexpr float multipliers[] = { 0.75f, 1.0f, 1.5f, 2.0f, 3.0f };
    auto current = std::lower_bound(std::begin(multipliers), std::end(multipliers),
        g_drawDistanceMultiplier - 0.001f);
    int index = current == std::end(multipliers)
        ? 0
        : static_cast<int>(current - std::begin(multipliers));
    constexpr int count = static_cast<int>(std::size(multipliers));
    index = (index + (direction < 0 ? -1 : 1) + count) % count;
    SetCmMrdRatio(g_pcm, multipliers[index]);
    g_drawDistanceMultiplier = multipliers[index];
}

void SelectWindowMode(int direction)
{
    const int next = (static_cast<int>(g_windowMode) + (direction < 0 ? -1 : 1) + 3) % 3;
    ApplyWindowModeSettings(static_cast<WindowMode>(next));
}
}

void StartupPrompt(PROMPT* pprompt)
{
    g_tePrompt.m_ch = 45;
    // Retail GS RGB is normalized around 0x80, not 0xff.
    g_tePrompt.m_rgba = { 0.0f, 75.0f / 128.0f, 125.0f / 128.0f, 1.0f };

    g_tePrompt.m_dxExtra = 15.0;
    g_tePrompt.m_ryScaling = 0.6;
    g_tePrompt.m_rxScaling = 0.6;

    pprompt->pvtprompt = &g_vtprompt;
}

void PostPromptLoad(PROMPT* pprompt)
{
    PostBlotLoad(pprompt);

    if (FFontLoaded(2))
    {
        pprompt->pte = &g_tePrompt; 
        pprompt->pte->m_pfont = PfontFromFont(2);
    }

    pprompt->prkSaveContext = PRK_Nil;
    pprompt->prk = PRK_Nil;

    for (int i = 0; i < 3; ++i)
        pprompt->mpprpprk[i] = PRK_Nil;

    pprompt->alpha = 1.0f;
}

void HandlePromptPrkTransition(PROMPT* pprompt, PRK prkOld, PRK prkNew)
{
    if (pprompt == nullptr || prkOld == prkNew)
        return;

    if (pprompt->fVagPlaying != 0)
    {
        StopVag();
        pprompt->fVagPlaying = 0;
    }

    switch (prkOld)
    {
        case PRK_MemcardChooseSaveSlot:
        case PRK_MemcardChooseLoadSlot:
        g_pnote.pvtblot->pfnSetBlotBlots(&g_pnote, BLOTS_Hidden);
        g_pnote.pvtblot->pfnSetBlotAchzDraw(&g_pnote, nullptr);
        break;

        default:
        break;
    }

    switch (prkNew)
    {
        case PRK_Nil:
        g_lifectr.pvtlifectr->pfnHideBlot(&g_lifectr);
        g_cluectr.pvtblot->pfnHideBlot(&g_cluectr);
        g_keyctr.pvtkeyctr->pfnHideBlot(&g_keyctr);
        g_coinctr.pvtcoinctr->pfnHideBlot(&g_coinctr);
        g_percentctr.pvtctr->pfnHideBlot(&g_percentctr);
        break;

        case PRK_MemcardChooseSaveSlot:
        case PRK_MemcardChooseLoadSlot:
        {
            const char* eraseText = FLocalizedContent() ? ContentLocalizedText(157) : nullptr;
            g_pnote.pvtblot->pfnSetBlotAchzDraw(&g_pnote,
                const_cast<char*>(eraseText != nullptr ? eraseText : "&2S&.: Erase Game"));
        }
        SetBlotDtVisible(&g_pnote, 0.0f);
        SetBlotFontScale(&g_pnote, 0.6f);
        g_pnote.pvtblot->pfnShowBlot(&g_pnote);
        break;

        default:
        break;
    }
}

void SetPrompt(PROMPT* pprompt, int prp, PRK prk)
{
    pprompt->mpprpprk[prp] = prk;

    // The last non-nil prompt slot has priority.
    PRK prkNew = PRK_Nil;

    for (int i = 0; i < 3; ++i)
    {
        if (pprompt->mpprpprk[i] != PRK_Nil)
            prkNew = static_cast<PRK>(pprompt->mpprpprk[i]);
    }

    const PRK prkCurrent =
        pprompt->fReshow
        ? pprompt->prkReshow
        : pprompt->prk;

    HandlePromptPrkTransition(pprompt, prkCurrent, prkNew);

    if (prkCurrent != prkNew)
    {
        if (pprompt->blots == BLOTS_Hidden)
            ChangePromptPrk(pprompt, prkNew);
        else if (pprompt->blots > BLOTS_Nil && pprompt->blots < BLOTS_Max)
        {
            if (prkNew == PRK_Nil)
            {
                pprompt->fReshow = 0;
                pprompt->prkReshow = PRK_Nil;
            }
            else
            {
                pprompt->fReshow = 1;
                pprompt->prkReshow = prkNew;

                if (pprompt->pvtblot->pfnSetBlotBlots != nullptr)
                    pprompt->pvtblot->pfnSetBlotBlots(pprompt, BLOTS_Disappearing);
            }
        }
    }

    if (prkNew == PRK_Nil)
    {
        if (pprompt->fActive != 0)
            PopUiActiveBlot(&g_ui);
    }
    else if (pprompt->fActive == 0)
        PushUiActiveBlot(&g_ui, pprompt);
}

SMA* PsmaFindSpecialPromptStateMachine()
{
    SM* psm = (SM*)PloFindSwObject(g_psw, 5, (OID)1068, nullptr);

    if (psm == nullptr)
        return nullptr;

    return psm->dlSma.psmaFirst;
}

void SetPromptPrk(PROMPT* pprompt)
{
    if (pprompt == nullptr || pprompt->pfont == nullptr)
        return;

    const PRK prk = pprompt->prk;

    if (prk < PRK_PauseMenu || prk >= PRK_Max)
        return;

    // -------------------------------------------------------------------------
    // Build the dynamic pause menu
    // -------------------------------------------------------------------------

    if (prk == PRK_PauseMenu)
    {
        PRD& prd = s_mpprkprd[PRK_PauseMenu];
        prd.arespk = s_arespkPauseMenu;
        prd.crespk = 0;

        for (RESPK response : s_arespkPauseMenuAll)
        {
            bool include = true;

            switch (response)
            {
                case RESPK_Map:
                include = g_wmc.pwmCurrent != nullptr;
                break;

                case RESPK_RestartRace:
                {
                    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);
                    include = false;

                    if (levelId == 516 || levelId == 1031)
                    {
                        SMA* psma = PsmaFindSpecialPromptStateMachine();

                        if (psma != nullptr)
                        {
                            int smaState[4]{};
                            GetSmaCur(psma, (OID*)smaState);
                            include = smaState[0] > 1070 && smaState[0] < 1073;
                        }
                    }

                    break;
                }

                case RESPK_RestartSprint:
                include = g_note.mtsState == MTSSTATE_Active;
                break;

                case RESPK_RestartLevel:
                {
                    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);
                    include = levelId == 772 || levelId == 775 || levelId == 1282;
                    break;
                }

                case RESPK_ExitLevel:
                if (g_pgsCur->gameWorldCur == GAMEWORLD_Intro)
                {
                    include = false;
                }
                else if (g_pgsCur->gameWorldCur == GAMEWORLD_Clockwerk ||
                    g_pgsCur->worldLevelCur == WORLDLEVEL_Hub ||
                    (g_pgsCur->worldLevelCur == WORLDLEVEL_Approach && (g_pwsCur->als[1].grfls & 1U) == 0))
                {
                    response = RESPK_ExitToHideout;
                }
                break;

                default:
                break;
            }

            if (include && prd.crespk < static_cast<int>(std::size(s_arespkPauseMenu)))
                prd.arespk[prd.crespk++] = response;
        }
    }

    // -------------------------------------------------------------------------
    // Build the dynamic options menu
    // -------------------------------------------------------------------------

    else if (prk == PRK_OptionsMenu)
    {
        PRD& prd = s_mpprkprd[PRK_OptionsMenu];
        prd.arespk = s_arespkOptionsMenu;
        prd.crespk = 0;

        for (RESPK response : s_arespkOptionsMenuAll)
        {
            bool include = true;

            if (response == RESPK_NewGame)
                include = g_ui.uisPlaying == UIS_Attract;
            else if (response == RESPK_SaveGame)
                include = g_ui.uisPlaying != UIS_Attract;

            if (include &&prd.crespk < static_cast<int>(std::size(s_arespkOptionsMenu)))
                prd.arespk[prd.crespk++] = response;
        }
    }

	else if (prk == PRK_ControlsMenu)
	{
		PRD& prd = s_mpprkprd[PRK_ControlsMenu];
		prd.arespk = s_arespkControlsMenu;
		prd.crespk = 0;
		const bool gamepadAvailable = FGamepadAvailable();

		for (RESPK response : s_arespkControlsMenuAll)
		{
			if (response == RESPK_ControllerMapping && !gamepadAvailable)
				continue;
			prd.arespk[prd.crespk++] = response;
		}
	}
    // -------------------------------------------------------------------------
    // Rebuild dynamic response strings
    // -------------------------------------------------------------------------

    std::snprintf(g_achzRespk9, sizeof(g_achzRespk9), "%s", s_apchzRespk9[g_saveData.pgsCurrentSave != nullptr ? 1 : 0]);

    char* const slotStrings[SAVE_SLOT_COUNT] =
    {
        g_achzRespk10,
        g_achzRespk11,
        g_achzRespk12,
        g_achzSaveSlot4,
        g_achzSaveSlot5,
        g_achzSaveSlot6
    };

    for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
    {
        GS* pgs = &g_saveData.saveData[slot];

        if (pgs->dt != 0.0f)
            BuildSaveSlotText(pgs, slotStrings[slot]);
        else
        {
            const bool choosingSlot = prk == PRK_MemcardChooseNewSlot || prk == PRK_MemcardChooseLoadSlot;
            std::snprintf(slotStrings[slot], 64, "%s", choosingSlot
				? PcText("New Game", "Nouvelle partie", "Neues Spiel", "Nueva partida", "Nuova partita")
				: PcText("Empty", "Vide", "Leer", "Vacío", "Vuoto"));
        }
    }

    std::snprintf(g_achzRespk13, sizeof(g_achzRespk13), "%s", s_apchzRespk13[FVibrationEnabled(g_pgsCur) ? 0 : 1]);
    std::snprintf(g_achzRespk16, sizeof(g_achzRespk16), "%s",
		s_apchzRespk16[FUserStereoEnabled() ? 0 : 1]);
    std::snprintf(g_achzRespk24, sizeof(g_achzRespk24), "%s", s_apchzRespk24[(g_pgsCur->grfgs & 0x200U) != 0 ? 1 : 0]);
    std::snprintf(g_achzRespk25, sizeof(g_achzRespk25), "%s", s_apchzRespk25[(g_pgsCur->grfgs & 0x400U) != 0 ? 1 : 0]);
    std::snprintf(g_achzRespk26, sizeof(g_achzRespk26), "%s", s_apchzRespk26[(g_pgsCur->grfgs & 0x800U) != 0 ? 1 : 0]);
	std::snprintf(g_achzRespk41, sizeof(g_achzRespk41), "%s", s_apchzRespk41[(g_pgsCur->grfgs & 0x1000U) != 0 ? 1 : 0]);
	for (int button = 0; button < BTN_MAX; ++button)
	{
		const char* bindingName = prk == PRK_ControllerMapping
			? PchzGamepadBindingName(g_gamepadBindings[button])
			: PchzKeyboardKeyName(g_keyboardBindings[button]);
		std::snprintf(g_aachzKeyboardBindings[button], sizeof(g_aachzKeyboardBindings[button]),
			"%s: %s", PcKeyboardAction(button), bindingName);
	}
    if (g_fMsaa)
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: %dx", g_msaaSamples);
    else
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: %s", PcOnOff(false));
    std::snprintf(g_achzRespk28, sizeof(g_achzRespk28),
		"%s: %d FPS", PcText("Frame Rate", "Fréquence d'images", "Bildrate", "Frecuencia de imagen", "Frequenza fotogrammi"), g_targetFrameRate);
    if (g_internalResolutionHeight == 0 && g_resolutionScalePercent != 100)
	{
		std::snprintf(g_achzRespk30, sizeof(g_achzRespk30),
			"%s: %s %d%% (%d x %d)", PcText("Resolution", "Résolution", "Auflösung", "Resolución", "Risoluzione"),
			PcText("Scaled", "Échelle", "Skaliert", "Escalada", "Scalata"), g_resolutionScalePercent,
			g_gl.renderWidth, g_gl.renderHeight);
	}
    else if (g_internalResolutionHeight == 0)
    {
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30),
            "%s: %s (%d x %d)", PcText("Resolution", "Résolution", "Auflösung", "Resolución", "Risoluzione"),
			PcText("Native", "Native", "Nativ", "Nativa", "Nativa"), g_gl.renderWidth, g_gl.renderHeight);
    }
    else
    {
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30),
            "%s: %d x %d", PcText("Resolution", "Résolution", "Auflösung", "Resolución", "Risoluzione"), g_gl.renderWidth, g_gl.renderHeight);
    }

    const char* fogName = g_fogType == 2 ? "PS3" : "PS2";
    std::snprintf(g_achzRespk31, sizeof(g_achzRespk31), "%s: %s", PcText("Fog", "Brouillard", "Nebel", "Niebla", "Nebbia"), fogName);

    const float drawDistance = g_drawDistanceMultiplier;
    const char* drawDistanceName = nullptr;
    if (drawDistance > 0.749f && drawDistance < 0.751f)
        drawDistanceName = PcText("Low", "Faible", "Niedrig", "Baja", "Bassa");
    else if (drawDistance > 0.999f && drawDistance < 1.001f)
        drawDistanceName = PcText("Original", "Originale", "Original", "Original", "Originale");
    else if (drawDistance > 1.499f && drawDistance < 1.501f)
        drawDistanceName = PcText("High", "Élevée", "Hoch", "Alta", "Alta");
    else if (drawDistance > 1.999f && drawDistance < 2.001f)
        drawDistanceName = PcText("Very High", "Très élevée", "Sehr hoch", "Muy alta", "Molto alta");
    else if (drawDistance > 2.999f && drawDistance < 3.001f)
        drawDistanceName = PcText("Extreme", "Extrême", "Extrem", "Extrema", "Estrema");

    if (drawDistanceName != nullptr)
        std::snprintf(g_achzRespk32, sizeof(g_achzRespk32), "%s: %s", PcText("Draw Distance", "Distance d'affichage", "Sichtweite", "Distancia de dibujado", "Distanza di visualizzazione"), drawDistanceName);
    else
        std::snprintf(g_achzRespk32, sizeof(g_achzRespk32), "%s: %s (%.2fx)", PcText("Draw Distance", "Distance d'affichage", "Sichtweite", "Distancia de dibujado", "Distanza di visualizzazione"), PcText("Custom", "Personnalisée", "Benutzerdefiniert", "Personalizada", "Personalizzata"), drawDistance);

    const int windowModeIndex = (std::clamp)(static_cast<int>(g_windowMode), 0, 2);
    const char* windowModeNames[] = {
		PcText("Windowed", "Fenêtré", "Fenster", "Ventana", "Finestra"),
		PcText("Borderless", "Sans bordure", "Randlos", "Sin bordes", "Senza bordi"),
		PcText("Fullscreen", "Plein écran", "Vollbild", "Pantalla completa", "Schermo intero") };
    std::snprintf(g_achzRespk33, sizeof(g_achzRespk33), "%s: %s",
		PcText("Window Mode", "Mode d'affichage", "Fenstermodus", "Modo de ventana", "Modalità finestra"), windowModeNames[windowModeIndex]);
    std::snprintf(g_achzRespk34, sizeof(g_achzRespk34), "VSync: %s",
		PcOnOff(g_fVsync));
	std::snprintf(g_achzRespk69, sizeof(g_achzRespk69), "%s: %s",
		PcText("Minimize When Unfocused", "Réduire en arrière-plan", "Bei Fokusverlust minimieren", "Minimizar en segundo plano", "Riduci in secondo piano"), PcOnOff(g_fAutoIconify));
	if (g_anisotropicFiltering <= 1)
		std::snprintf(g_achzRespk70, sizeof(g_achzRespk70), "%s: %s", PcText("Anisotropic Filtering", "Filtrage anisotrope", "Anisotrope Filterung", "Filtrado anisotrópico", "Filtro anisotropico"), PcOnOff(false));
	else
		std::snprintf(g_achzRespk70, sizeof(g_achzRespk70), "%s: %dx", PcText("Anisotropic Filtering", "Filtrage anisotrope", "Anisotrope Filterung", "Filtrado anisotrópico", "Filtro anisotropico"), g_anisotropicFiltering);
	std::snprintf(g_achzRespk71, sizeof(g_achzRespk71), "%s: %d%%", PcText("Brightness / Gamma", "Luminosité / Gamma", "Helligkeit / Gamma", "Brillo / Gamma", "Luminosità / Gamma"),
		static_cast<int>(std::lround(g_outputGamma * 100.0f)));
	std::snprintf(g_achzRespk72, sizeof(g_achzRespk72), "%s: %s", PcText("Texture Filtering", "Filtrage des textures", "Texturfilterung", "Filtrado de texturas", "Filtro texture"),
		g_textureFiltering != 0
			? PcText("Trilinear", "Trilinéaire", "Trilinear", "Trilineal", "Trilineare")
			: PcText("Bilinear", "Bilinéaire", "Bilinear", "Bilineal", "Bilineare"));
	std::snprintf(g_achzRespk73, sizeof(g_achzRespk73), "%s: %s", PcText("Monitor", "Moniteur", "Monitor", "Monitor", "Monitor"),
		VideoMonitorName(g_monitorIndex));
	std::snprintf(g_achzRespk75, sizeof(g_achzRespk75), "%s: %d%%", PcText("Resolution Scale", "Échelle de résolution", "Auflösungsskalierung", "Escala de resolución", "Scala risoluzione"),
		g_resolutionScalePercent);
	std::snprintf(g_achzRespk76, sizeof(g_achzRespk76), "%s: %d%%", PcText("Controller Deadzone", "Zone morte de la manette", "Controller-Totzone", "Zona muerta del mando", "Zona morta controller"),
		static_cast<int>(std::lround(g_controllerDeadzone * 100.0f)));
	const std::string audioDeviceName = GetAudioOutputDeviceName();
	const char* audioDeviceDisplay = audioDeviceName == "System Default"
		? PcText("System Default", "Valeur système", "Systemstandard", "Predeterminado del sistema", "Predefinito di sistema")
		: audioDeviceName.c_str();
	std::snprintf(g_achzRespk77, sizeof(g_achzRespk77), "%s: %s", PcText("Audio Output", "Sortie audio", "Audioausgabe", "Salida de audio", "Uscita audio"),
		audioDeviceDisplay);
    const char* aspectRatioName = PcText("Fit to Screen", "Adapter à l'écran", "An Bildschirm anpassen", "Ajustar a pantalla", "Adatta allo schermo");
    if (g_gl.aspectMode == Fixed_16_9)
        aspectRatioName = "16:9";
    else if (g_gl.aspectMode == Fixed_16_10)
        aspectRatioName = "16:10";
    else if (g_gl.aspectMode == Fixed_4_3)
        aspectRatioName = "4:3";
    else if (g_gl.aspectMode == PS2_4_3)
        aspectRatioName = "4:3 (PS2)";
    else if (g_gl.aspectMode == PS2_16_9)
        aspectRatioName = "16:9 (PS2)";
    std::snprintf(g_achzRespk35, sizeof(g_achzRespk35), "%s: %s", PcText("Aspect Ratio", "Format d'image", "Seitenverhältnis", "Relación de aspecto", "Proporzioni"), aspectRatioName);
    std::snprintf(g_achzRespk40, sizeof(g_achzRespk40), "%s: %d%%", PcText("GUI Scale", "Échelle de l'interface", "UI-Skalierung", "Escala de interfaz", "Scala interfaccia"),
        static_cast<int>(std::lround(g_guiScale * 100.0f)));
    std::snprintf(g_achzRespk60, sizeof(g_achzRespk60), "%s: %s", PcText("GUI Style", "Style d'interface", "UI-Stil", "Estilo de interfaz", "Stile interfaccia"),
        g_guiStyle == GuiStyle_Modern ? PcText("Modern", "Moderne", "Modern", "Moderno", "Moderno") : "PS2");
	std::snprintf(g_achzRespk61, sizeof(g_achzRespk61), "%s: %s",
        PcText("Disable Controller When Unfocused", "Désactiver la manette en arrière-plan", "Controller im Hintergrund deaktivieren", "Desactivar mando en segundo plano", "Disattiva controller in secondo piano"),
        PcOnOff(g_fDisableControllerInputWhenUnfocused));
	std::snprintf(g_achzRespk66, sizeof(g_achzRespk66), "%s: %s", PcText("Mouse Camera", "Caméra à la souris", "Mauskamera", "Cámara con ratón", "Telecamera con mouse"),
		PcOnOff(g_fMouseCamera));
	std::snprintf(g_achzRespk68, sizeof(g_achzRespk68), "%s: %d%%", PcText("Mouse Sensitivity", "Sensibilité de la souris", "Mausempfindlichkeit", "Sensibilidad del ratón", "Sensibilità mouse"),
		static_cast<int>(std::lround(g_mouseCameraSensitivity * 100.0f)));
	std::snprintf(g_achzRespk67, sizeof(g_achzRespk67), "%s: %s",
		PcText("Dialogue Skip Controls", "Commandes pour passer les dialogues", "Dialog-Überspringen", "Controles para omitir diálogos", "Comandi per saltare i dialoghi"),
		PcOnOff(g_fSkipBinocDialogs));
	std::snprintf(g_achzRespk41, sizeof(g_achzRespk41), "%s: %s",
		PcText("Camera Invert", "Inversion de la caméra", "Kamera invertieren", "Invertir cámara", "Inverti telecamera"),
		PcOnOff((g_pgsCur->grfgs & 0x1000U) != 0));

    // -------------------------------------------------------------------------
    // Choose initial response
    // -------------------------------------------------------------------------

    pprompt->irespk = 0;

    switch (prk)
    {
        case PRK_QuitConfirm:
		case PRK_ExitToDesktopConfirm:
        case PRK_MemcardFormatConfirm:
        case PRK_MemcardCreateConfirm:
        case PRK_MemcardOverwriteConfirm:
        case PRK_MemcardEraseConfirm:
        pprompt->irespk = 1;
        break;

        case PRK_MemcardChooseSaveSlot:
        case PRK_MemcardChooseLoadSlot:
        for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
        {
            if (g_saveData.pgsCurrentSave == &g_saveData.saveData[slot])
            {
                pprompt->irespk = slot;
                break;
            }
        }
        break;

        case PRK_MemcardChooseNewSlot:
        while (pprompt->irespk < SAVE_SLOT_COUNT &&
            g_saveData.saveData[pprompt->irespk].dt != 0.0f)
            ++pprompt->irespk;
        break;

        case PRK_MemcardAutoNotify:
        if (!FVagPlaying())
        {
            // Extracted equivalent of the original VAG descriptor at 0x0026A118.
            PreloadVag("PromptPrk18.wav");
            ContinueVag();
            pprompt->fVagPlaying = FVagPlaying();
        }
        break;

		case PRK_LanguageMenu:
		if (FJapaneseContent())
			pprompt->irespk = ContentVoiceLanguage() == CONTENT_LANGUAGE::English ? 1 : 0;
		else
		{
			pprompt->irespk = 0;
			for (int index = 0; index < static_cast<int>(ContentPackage().language); ++index)
			{
				if (FContentLanguageAvailable(static_cast<CONTENT_LANGUAGE>(index)))
					++pprompt->irespk;
			}
		}
		break;

        default:
        break;
    }

    // -------------------------------------------------------------------------
    // Measure prompt
    // -------------------------------------------------------------------------

    PRD& prd = PrdPrompt(prk);
	RESPK* const promptResponses = ArespkPrompt(prk, prd);
	const int promptResponseCount = CrespkPrompt(prk, prd);

    float titleWidth = 0.0f;
    float titleHeight = 0.0f;
    float responseWidth = 0.0f;

    const char* const promptTitle = PchzPromptTitle(prk, prd);
    if (promptTitle != nullptr)
    {
        pprompt->pfont->PushScaling(prd.rScaleTitle, prd.rScaleTitle);
		CRichText titleText(promptTitle, pprompt->pfont);
		titleText.GetExtents(&titleWidth, &titleHeight, 500.0f);
        pprompt->pfont->PopScaling();

        titleWidth = std::max(titleWidth, 0.0f);
    }

    pprompt->pfont->PushScaling(prd.rScaleRespk, prd.rScaleRespk);

    const float lineHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    const int visibleResponseCount = VisiblePromptResponseCount(prk, promptResponseCount);
    float responseHeight = prd.fVertical != 0 ? lineHeight * static_cast<float>(visibleResponseCount) : lineHeight;

    if (promptResponseCount == 0)
        responseHeight = prd.fVertical != 0 ? 0.0f : lineHeight;

    for (int i = 0; i < promptResponseCount; ++i)
    {
        const RESPK response = promptResponses[i];

        if (response == RESPK_Nil)
            break;

        if (response < RESPK_Yes || response >= RESPK_Max)
            continue;

        const RESPD& responseData = s_arespd[response];
        float itemWidth = 0.0f;

        if (responseData.cpchz <= 0 || responseData.apchz == nullptr)
        {
            CRichText text(const_cast<char*>(AchzFromRespk(response)), pprompt->pfont);
            itemWidth = text.Dx();
        }
        else
        {
            for (int variant = 0; variant < responseData.cpchz; ++variant)
            {
                const char* variantText = FLocalizedContent()
					? PalResponseText(response, variant) : nullptr;
				if (variantText == nullptr)
					variantText = responseData.apchz[variant];
                if (variantText == nullptr)
                    continue;

                CRichText text(const_cast<char*>(variantText), pprompt->pfont);
                itemWidth = std::max(itemWidth, text.Dx());
            }
        }

        if (prd.fVertical != 0)
            responseWidth = std::max(responseWidth, itemWidth);
        else
        {
            if (i != 0)
            {
                const float spaceWidth = static_cast<float>(pprompt->pfont->m_dxSpaceUnscaled) * pprompt->pfont->m_rxScale;
                responseWidth += spaceWidth * 2.0f;
            }

            responseWidth += itemWidth;
        }
    }

    pprompt->pfont->PopScaling();

    // Japanese retail DrawPrompt advances eight authored screen units before
    // its response rows. This is a layout gap, not a global prompt scale.
    const float responseGap = FJapaneseContent() && promptResponseCount > 0 ? 8.0f : 0.0f;
    ResizeBlot(pprompt, std::max(titleWidth, responseWidth) + 10.0f,
        titleHeight + responseGap + responseHeight);

}

void ChangePromptPrk(PROMPT* pprompt, PRK prk)
{
    if ((pprompt->prk != prk) && (pprompt->prk = prk, prk != PRK_Nil))
        SetPromptPrk(pprompt);
}

void OpenMappingPrompt(PROMPT* pprompt, PRK prk)
{
    if (pprompt == nullptr)
        return;

    const PRK oldPrk = pprompt->prk;
    pprompt->mpprpprk[PRP_Basic] = prk;
    pprompt->fReshow = 0;
    pprompt->prkReshow = PRK_Nil;
    HandlePromptPrkTransition(pprompt, oldPrk, prk);
    ChangePromptPrk(pprompt, prk);

    // Mapping pages are PC-only menus and do not need retail's full
    // hide/reshow transition. Opening them directly also prevents a held
    // confirm input from leaving the prompt at its collapsed animation size.
    SetBlotBlots(pprompt, BLOTS_Visible);
}

void ExecutePrompt(PROMPT* pprompt)
{
    ExecutePrompt(pprompt, 1);
}

void ExecutePrompt(PROMPT* pprompt, int direction)
{
    if (pprompt == nullptr || pprompt->prk < PRK_PauseMenu || pprompt->prk >= PRK_Max)
        return;

    const PRD& promptData = PrdPrompt(pprompt->prk);
	RESPK* const promptResponses = ArespkPrompt(pprompt->prk, promptData);
	const int promptResponseCount = CrespkPrompt(pprompt->prk, promptData);

    RESPK response = RESPK_Nil;

    if (promptResponses != nullptr && pprompt->irespk >= 0 && pprompt->irespk < promptResponseCount)
        response = promptResponses[pprompt->irespk];

    if (response == RESPK_Back)
    {
        CancelPrompt(pprompt);
        return;
    }

    const auto openOptionsMenu = [&]()
    {
        SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
    };

    const auto playInvalidSelectionSound = [&]()
    {
        StartSound(static_cast<SFXID>(123), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
    };

    const auto writeSelectedSave = [&]()
    {
        const bool fQuitToTitle = pprompt->prkSaveContext == PRK_QuitConfirm;

        if (!SaveCurrentGameToDisk(&g_saveData))
        {
            SetPrompt(pprompt, PRP_Memcard, PRK_Unknown15);
            return;
        }

        // Retail clears MemcardSlotSaving as soon as the memory-card
        // operation completes successfully.  The PC save is synchronous, so
        // complete the same transition here instead of introducing a second
        // SlotSaved/Continue prompt.
        SetPrompt(pprompt, PRP_Memcard, PRK_Nil);

        if (fQuitToTitle)
        {
            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            WipeToTitleScreen();
        }
    };

    const auto getSaveSlotIndex = [](RESPK slotResponse) -> int
    {
        for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
        {
            if (s_arespkSlots[slot] == slotResponse)
                return slot;
        }

        return -1;
    };

    const auto closeSaveManagerPrompt = [&]()
    {
        SetPrompt(pprompt, PRP_Memcard, PRK_Nil);
        SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Closing);

        const PRK basicPrompt = pprompt->mpprpprk[PRP_Basic];

        if (basicPrompt == PRK_MemcardChooseSaveSlot ||
            basicPrompt == PRK_MemcardChooseNewSlot ||
            basicPrompt == PRK_MemcardChooseLoadSlot)
        {
            openOptionsMenu();
        }
    };

    const auto rebuildPromptKeepingSelection = [&]()
    {
        const int selectedResponse = pprompt->irespk;
        SetPromptPrk(pprompt);
        pprompt->irespk = selectedResponse;
    };

    PRP promptGroupToClear = PRP_Basic;

    switch (pprompt->prk)
    {
        case PRK_PauseMenu:
        {
            switch (response)
            {
                case RESPK_ReturnToGame:
                break;

                case RESPK_Map:
                if (g_wmc.pwmCurrent == nullptr)
                {
                    playInvalidSelectionSound();
                    return;
                }

                SetPrompt(pprompt, PRP_Basic, PRK_Nil);
                PushUiActiveBlot(&g_ui, &g_wmc);
                return;

                case RESPK_Options:
                openOptionsMenu();
                return;

                case RESPK_Quit:
                SetPrompt(pprompt, PRP_Basic, PRK_QuitConfirm);
                return;

                case RESPK_ExitToDesktop:
				SetPrompt(pprompt, PRP_Basic, PRK_ExitToDesktopConfirm);
                return;

                case RESPK_RestartRace:
                {
                    SMA* psma = PsmaFindSpecialPromptStateMachine();
                    PO* ppo = PpoCur();

                    if (psma != nullptr && ppo != nullptr && FIsBasicDerivedFrom(ppo, CID_SUV))
                    {
                        int state[4]{};
                        GetSmaCur(psma, (OID*)state);

                        if (state[0] > 1070 && state[0] < 1073)
                        {
                            HandleSuvRaceLoss(reinterpret_cast<SUV*>(ppo));
                            SeekSma(psma, (OID)1070);
                        }
                    }

                    break;
                }

                case RESPK_RestartSprint:
                case RESPK_RestartLevel: 
                ExitOrRestartLevelFromPrompt(pprompt, true, WIPEK_Keyhole);
                return;

                case RESPK_ExitLevel:
                SetPrompt(pprompt, PRP_Basic, PRK_Nil);
                TriggerDefaultExit(true, WIPEK_WorldMap);
                return;

                case RESPK_ExitToHideout:
                {
                    LEVELINFO* plevelHideout = PlevelinfoFromSearchKey(9);

                    if (plevelHideout == nullptr)
                    {
                        std::printf("PROMPT ERROR: Hideout search key 9 not found\n");
                        playInvalidSelectionSound();
                        return;
                    }

                    SetPrompt(pprompt, PRP_Basic, PRK_Nil);
                    WipeToWorldWarp(plevelHideout, OID_Nil, WIPEK_WorldMap);
                    return;
                }

                case RESPK_Controls:
                SetPrompt(pprompt, PRP_Basic, PRK_ControlsMenu);
                return;

                default:
                return;
            }

            break;
        }

        case PRK_VideoMenu:
        switch (response)
        {
            case RESPK_Msaa:
            {
				const std::vector<int> samples = SupportedMsaaSamples();
                const int currentSamples = g_fMsaa ? g_msaaSamples : 0;
				auto current = std::find(samples.begin(), samples.end(), currentSamples);
				int index = current == samples.end()
                    ? 0
					: static_cast<int>(current - samples.begin());
				const int count = static_cast<int>(samples.size());
                index = (index + (direction < 0 ? -1 : 1) + count) % count;
                g_fMsaa = samples[index] != 0;
                if (g_fMsaa)
                    g_msaaSamples = samples[index];
            }

            ApplyMsaaSettings();
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_FrameRate:
            SelectSupportedFrameRate(direction);
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_InternalResolution:
            SelectInternalResolution(direction);
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_GuiScale:
            SelectGuiScale(direction);
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_GuiStyle:
            SelectNextGuiStyle();
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_DrawDistance:
            SelectDrawDistance(direction);
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_WindowMode:
            SelectWindowMode(direction);
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

			case RESPK_ResolutionScale:
			{
				constexpr int scales[] = { 50, 75, 100, 125, 150, 175, 200 };
				auto current = std::find(std::begin(scales), std::end(scales), g_resolutionScalePercent);
				int index = current == std::end(scales) ? 2 : static_cast<int>(current - std::begin(scales));
				index = (index + (direction < 0 ? -1 : 1) + static_cast<int>(std::size(scales))) %
					static_cast<int>(std::size(scales));
				const int oldScale = g_resolutionScalePercent;
				const int oldHeight = g_internalResolutionHeight;
				g_resolutionScalePercent = scales[index];
				g_internalResolutionHeight = 0;
				if (!ApplyInternalResolutionSettings())
				{
					g_resolutionScalePercent = oldScale;
					g_internalResolutionHeight = oldHeight;
					ApplyInternalResolutionSettings();
				}
			}
			SaveSystemSettings();
			rebuildPromptKeepingSelection();
			return;

			case RESPK_OutputGamma:
			ApplyOutputGammaSettings(g_outputGamma + (direction < 0 ? -0.05f : 0.05f));
			SaveSystemSettings();
			rebuildPromptKeepingSelection();
			return;

			case RESPK_TextureFiltering:
			ApplyTextureFilteringSettings(g_textureFiltering == 0 ? 1 : 0);
			SaveSystemSettings();
			rebuildPromptKeepingSelection();
			return;

			case RESPK_Monitor:
			ApplyMonitorSettings((g_monitorIndex + (direction < 0 ? -1 : 1) + VideoMonitorCount()) % VideoMonitorCount());
			SaveSystemSettings();
			rebuildPromptKeepingSelection();
			return;

			case RESPK_ResetVideo:
			g_fMsaa = false;
			g_msaaSamples = 1;
			ApplyAnisotropicFilteringSettings(1);
			ApplyTextureFilteringSettings(1);
			ApplyOutputGammaSettings(1.0f);
			g_targetFrameRate = FPalVideoStandard() ? 50 : 60;
			g_internalResolutionHeight = 0;
			g_resolutionScalePercent = 100;
			g_drawDistanceMultiplier = 1.0f;
			if (g_pcm != nullptr)
				SetCmMrdRatio(g_pcm, g_drawDistanceMultiplier);
			g_fVsync = true;
			g_fogType = 1;
			g_guiScale = 1.0f;
			g_guiStyle = GuiStyle_PS2;
			ApplyAutoIconifySettings(false);
			ApplyMonitorSettings(0);
			ApplyWindowModeSettings(WindowMode_Windowed);
			ApplyAspectRatioSettings(FitToScreen);
			ApplyMsaaSettings();
			glfwSwapInterval(1);
			glGlobShader.Use();
			glUniform1i(glslFogType, g_fogType);
			SaveSystemSettings();
			rebuildPromptKeepingSelection();
			return;

			case RESPK_AnisotropicFiltering:
			{
				constexpr int levels[] = { 1, 2, 4, 8, 16 };
				const int maximum = MaxSupportedAnisotropicFiltering();
				std::vector<int> supported;
				for (int level : levels)
					if (level <= maximum)
						supported.push_back(level);
				auto current = std::find(supported.begin(), supported.end(),
					g_anisotropicFiltering);
				int index = current == supported.end()
					? 0 : static_cast<int>(current - supported.begin());
				index = (index + (direction < 0 ? -1 : 1) +
					static_cast<int>(supported.size())) % static_cast<int>(supported.size());
				ApplyAnisotropicFilteringSettings(supported[index]);
			}
			SaveSystemSettings();
			rebuildPromptKeepingSelection();
			return;

            case RESPK_Vsync:
            g_fVsync = !g_fVsync;
            glfwSwapInterval(g_fVsync ? 1 : 0);
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_AspectRatio:
            {
                constexpr AspectMode modes[] =
                {
                    FitToScreen, Fixed_16_9, Fixed_16_10,
                    Fixed_4_3, PS2_4_3, PS2_16_9
                };
                auto current = std::find(std::begin(modes), std::end(modes), g_gl.aspectMode);
                int index = current == std::end(modes)
                    ? 0
                    : static_cast<int>(current - std::begin(modes));
                constexpr int count = static_cast<int>(std::size(modes));
                index = (index + (direction < 0 ? -1 : 1) + count) % count;
                ApplyAspectRatioSettings(modes[index]);
            }
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_Fog:
            g_fogType = g_fogType == 1 ? 2 : 1;
            glGlobShader.Use();
            glUniform1i(glslFogType, g_fogType);
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_Back:
            SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
            return;

            default:
            return;
        }

		case PRK_SoundMenu:
		switch (response)
		{
			case RESPK_AudioOutputDevice:
			SelectAudioOutputDevice(direction);
			break;
			case RESPK_Speakers:
			SetUserStereoEnabled(!FUserStereoEnabled());
			break;
			case RESPK_MusicVolume:
			SetUserMusicVolume(direction < 0
				? (GetUserMusicVolume() <= 0.001f ? 1.0f : GetUserMusicVolume() - 0.1f)
				: (GetUserMusicVolume() >= 0.999f ? 0.0f : GetUserMusicVolume() + 0.1f));
			break;
			case RESPK_SfxVolume:
			SetUserSfxVolume(direction < 0
				? (GetUserSfxVolume() <= 0.001f ? 1.0f : GetUserSfxVolume() - 0.1f)
				: (GetUserSfxVolume() >= 0.999f ? 0.0f : GetUserSfxVolume() + 0.1f));
			break;
			case RESPK_DialogueVolume:
			SetUserDialogueVolume(direction < 0
				? (GetUserDialogueVolume() <= 0.001f ? 1.0f : GetUserDialogueVolume() - 0.1f)
				: (GetUserDialogueVolume() >= 0.999f ? 0.0f : GetUserDialogueVolume() + 0.1f));
			break;
			case RESPK_Back:
			SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
			return;
			default:
			return;
		}
		SaveSystemSettings();
		rebuildPromptKeepingSelection();
		return;

        case PRK_GameOver:
        case PRK_TryAgain:
        case PRK_MtsExpired:
        case PRK_MtsBestTime:
        case PRK_MtsFailedBestTime:
        ExitOrRestartLevelFromPrompt(pprompt, response == RESPK_Yes, WIPEK_Keyhole);
        return;

        case PRK_QuitConfirm:
        {
            if (response == RESPK_Yes)
            {
                if (g_saveData.pgsCurrentSave == nullptr)
                {
                    WipeToTitleScreen();
                    return;
                }

                pprompt->prkSaveContext = PRK_QuitConfirm;

                if ((g_pgsCur->grfgs & 0x100) == 0)
                    SetPrompt(pprompt, PRP_Memcard, PRK_MemcardAutoNotify);
                else
                {
                    writeSelectedSave();
                    return;
                }
            }

            break;
        }

		case PRK_ExitToDesktopConfirm:
		if (response == RESPK_Yes)
			fQuitGame = true;
		else
			SetPrompt(pprompt, PRP_Basic, PRK_PauseMenu);
		return;

        case PRK_MemcardMissing:
        case PRK_MemcardFormatError:
        case PRK_MemcardCreateError:
        case PRK_MemcardCardFull:
        case PRK_Unknown21:
        case PRK_Unknown32:
        closeSaveManagerPrompt();
        return;

        case PRK_MemcardFormatConfirm:
        if (response == RESPK_Yes)
        {
            //StartMemoryCardOperation(2529680, 1622440, 2509376);
            SetPrompt(pprompt, PRP_Memcard, PRK_MemcardFormatting);
            return;
        }

        closeSaveManagerPrompt();
        return;

        case PRK_MemcardCreateConfirm:
        if (response == RESPK_Yes)
        {
            //StartMemoryCardOperation(2529680, 1622608, 2509376);
            SetPrompt(pprompt, PRP_Memcard, PRK_MemcardCreateError);
            return;
        }

        closeSaveManagerPrompt();
        return;

        case PRK_MemcardChooseSaveSlot:
        {
            const int slot = getSaveSlotIndex(response);

            if (slot < 0)
                return;

            GS* selectedSave = &g_saveData.saveData[slot];
            g_saveData.pgsSelectedSave = selectedSave;

            if (selectedSave->dt != 0.0f)
            {
                SetPrompt(pprompt, PRP_Memcard, PRK_MemcardOverwriteConfirm);
                return;
            }

            g_saveData.pgsCurrentSave = selectedSave;
            pprompt->prkSaveContext = PRK_MemcardChooseSaveSlot;

            if ((g_pgsCur->grfgs & 0x100) == 0)
                SetPrompt(pprompt, PRP_Memcard, PRK_MemcardAutoNotify);
            else
            {
                writeSelectedSave();
            }

            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            return;
        }

        case PRK_MemcardSlotSaved:
        promptGroupToClear = PRP_Memcard;
        break;

        case PRK_MemcardOverwriteConfirm:
        if (response != RESPK_Yes)
        {
            promptGroupToClear = PRP_Memcard;
            break;
        }

        g_saveData.pgsCurrentSave = g_saveData.pgsSelectedSave;
        pprompt->prkSaveContext = PRK_MemcardChooseSaveSlot;

        if ((g_pgsCur->grfgs & 0x100) == 0)
            SetPrompt(pprompt, PRP_Memcard, PRK_MemcardAutoNotify);
        else
        {
            writeSelectedSave();
        }

        SetPrompt(pprompt, PRP_Basic, PRK_Nil);
        return;

        case PRK_MemcardChooseNewSlot:
        {
            const int slot = getSaveSlotIndex(response);

            if (slot < 0)
                return;

            if (g_saveData.saveData[slot].dt != 0.0f)
            {
                playInvalidSelectionSound();
                return;
            }

            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            g_saveData.pgsCurrentSave = &g_saveData.saveData[slot];
            InitGameState(g_pgsCur);
            if (!SaveCurrentGameToDisk(&g_saveData))
            {
                SetPrompt(pprompt, PRP_Memcard, PRK_Unknown15);
                return;
            }
            ReloadCurrentLevel();
            return;
        }

        case PRK_MemcardChooseLoadSlot:
        {
            const int slot = getSaveSlotIndex(response);

            if (slot < 0)
                return;

            g_saveData.pgsCurrentSave = &g_saveData.saveData[slot];

            // Retail sends both occupied saves and the "New Game" entries in
            // the Load Game menu through this selection path. An empty slot
            // starts a fresh game instead of being rejected as a failed load.
            if (g_saveData.saveData[slot].dt == 0.0f)
            {
                SetPrompt(pprompt, PRP_Basic, PRK_Nil);
                InitGameState(g_pgsCur);
                if (!SaveCurrentGameToDisk(&g_saveData))
                {
                    SetPrompt(pprompt, PRP_Memcard, PRK_Unknown15);
                    return;
                }
                ReloadCurrentLevel();
                return;
            }

            if (!LoadCurrentSave(&g_saveData))
            {
                playInvalidSelectionSound();
                return;
            }

            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            ReloadCurrentLevel();
            return;
        }

        case PRK_MemcardEraseConfirm:
        if (response == RESPK_Yes)
        {
            int slot = -1;
            if (g_saveData.pgsSelectedSave != nullptr)
                slot = static_cast<int>(g_saveData.pgsSelectedSave - g_saveData.saveData);

            if (!DeleteSaveSlotFromDisk(&g_saveData, slot))
            {
                playInvalidSelectionSound();
                return;
            }

            SetPrompt(pprompt, PRP_Memcard, PRK_Nil);
            return;
        }

        promptGroupToClear = PRP_Memcard;
        break;

        case PRK_MemcardAutoNotify:
        {
            g_pgsCur->grfgs |= 0x100;

            switch (pprompt->prkSaveContext)
            {
                case PRK_Nil:
                SetPrompt(pprompt, PRP_Memcard, PRK_Nil);
                if (g_autosave.pvtblot != nullptr &&
                    g_autosave.pvtblot->pfnShowBlot != nullptr)
                {
                    g_autosave.pvtblot->pfnShowBlot(&g_autosave);
                }
                break;

                case PRK_QuitConfirm:
                case PRK_MemcardChooseSaveSlot:
                writeSelectedSave();
                break;

                default:
                break;
            }
            return;
        }

        case PRK_OptionsMenu:
        {
            switch (response)
            {
                case RESPK_NewGame:
                SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Selecting);
                SetPrompt(pprompt, PRP_Basic, PRK_MemcardChooseNewSlot);
                return;

                case RESPK_LoadGame:
                SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Selecting);
                SetPrompt(pprompt, PRP_Basic, PRK_MemcardChooseLoadSlot);
                return;

                case RESPK_SaveGame:
                SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Selecting);
                SetPrompt(pprompt, PRP_Basic, PRK_MemcardChooseSaveSlot);
                return;

                case RESPK_Vibration:
                ToggleVibration(g_pgsCur);
                rebuildPromptKeepingSelection();
                return;

				case RESPK_CutsceneSkipControls:
				g_fSkipBinocDialogs = !g_fSkipBinocDialogs;
				SaveSystemSettings();
				rebuildPromptKeepingSelection();
				return;

				case RESPK_Sound:
				SetPrompt(pprompt, PRP_Basic, PRK_SoundMenu);
				return;

                case RESPK_Controls:
                SetPrompt(pprompt, PRP_Basic, PRK_ControlsMenu);
                return;

                case RESPK_Video:
                SetPrompt(pprompt, PRP_Basic, PRK_VideoMenu);
                return;

				case RESPK_AutoIconify:
				ApplyAutoIconifySettings(!g_fAutoIconify);
				SaveSystemSettings();
				rebuildPromptKeepingSelection();
				return;

                default:
                return;
            }
        }

		case PRK_LanguageMenu:
		{
			CONTENT_LANGUAGE language;
			switch (response)
			{
				case RESPK_LanguageEnglish: language = CONTENT_LANGUAGE::English; break;
				case RESPK_LanguageFrench: language = CONTENT_LANGUAGE::French; break;
				case RESPK_LanguageGerman: language = CONTENT_LANGUAGE::German; break;
				case RESPK_LanguageSpanish: language = CONTENT_LANGUAGE::Spanish; break;
				case RESPK_LanguageItalian: language = FJapaneseContent()
					? CONTENT_LANGUAGE::Japanese : CONTENT_LANGUAGE::Italian; break;
				default: return;
			}

			BeginContentLanguageChange(language);
			SetPrompt(pprompt, PRP_Basic, PRK_Nil);
			if (g_transition.m_plevelCurrent != nullptr)
			{
				// Retail PAL's PRK 0x21 handler reloads language-specific level
				// ID 3 at OID 446 with wipe kind 1. OID 446 is Paris's attract
				// entry point; OID_Nil resumes the ordinary playable spawn instead.
				WipeToWorldWarp(g_transition.m_plevelCurrent,
					static_cast<OID>(446), WIPEK_Keyhole);
			}
			else
			{
				FinishContentLanguageChange(false);
			}
			return;
		}

        case PRK_SpecialRetry:
        {
            SMA* psma = pprompt->psma;
            pprompt->psma = nullptr;

            if (psma != nullptr)
            {
                const OID oidGoal = response == RESPK_Yes ? static_cast<OID>(1076) : static_cast<OID>(1077);
                SetSmaGoal(psma, oidGoal);
            }

            break;
        }

        case PRK_MtsComplete:
        if ((GetGameProgress() & 0x08) == 0)
            ExitOrRestartLevelFromPrompt(pprompt, false, WIPEK_WorldMap);
        else
            PlayEndingFromCompletionFlag(8);

        return;

		case PRK_ControlsMenu:
        switch (response)
        {
			case RESPK_Vibration:
			ToggleVibration(g_pgsCur);
			break;

			case RESPK_KeyboardMapping:
			s_fControllerBindingCapture = false;
			OpenMappingPrompt(pprompt, PRK_KeyboardMapping);
			return;

			case RESPK_ControllerMapping:
			s_fControllerBindingCapture = true;
			OpenMappingPrompt(pprompt, PRK_ControllerMapping);
			return;

			case RESPK_DisableControllerWhenUnfocused:
			g_fDisableControllerInputWhenUnfocused =
				!g_fDisableControllerInputWhenUnfocused;
			SaveSystemSettings();
			break;

			case RESPK_CameraInvert:
			g_pgsCur->grfgs ^= 0x1000;
			break;

			case RESPK_MouseCamera:
			g_fMouseCamera = !g_fMouseCamera;
			SaveSystemSettings();
			break;

			case RESPK_MouseSensitivity:
			g_mouseCameraSensitivity = std::clamp(
				g_mouseCameraSensitivity + (direction < 0 ? -0.25f : 0.25f),
				0.25f, 2.0f);
			SaveSystemSettings();
			break;

			case RESPK_ControllerDeadzone:
			g_controllerDeadzone = std::clamp(
				g_controllerDeadzone + (direction < 0 ? -0.05f : 0.05f), 0.05f, 0.50f);
			SaveSystemSettings();
			break;

            case RESPK_BinocInvert:
            g_pgsCur->grfgs ^= 0x200;
            break;

            case RESPK_TurretInvert:
            g_pgsCur->grfgs ^= 0x400;
            break;

            case RESPK_JetpackInvert:
            g_pgsCur->grfgs ^= 0x800;
            break;

            default:
            return;
        }

        rebuildPromptKeepingSelection();
        return;

		case PRK_KeyboardMapping:
		case PRK_ControllerMapping:
		if (response == RESPK_Back)
		{
			s_iKeyboardBindingCapture = -1;
			SetPrompt(pprompt, PRP_Basic, PRK_ControlsMenu);
			return;
		}

		if (response == RESPK_ResetMappings)
		{
			if (pprompt->prk == PRK_ControllerMapping)
				ResetControllerBindings();
			else
				ResetKeyboardMouseBindings();
			SaveSystemSettings();
			rebuildPromptKeepingSelection();
			return;
		}

		if (response >= RESPK_KeyUp && response <= RESPK_KeyR3)
		{
			s_iKeyboardBindingCapture = static_cast<int>(response) - static_cast<int>(RESPK_KeyUp);
			s_fKeyboardBindingCaptureArmed = false;
			std::snprintf(g_aachzKeyboardBindings[s_iKeyboardBindingCapture],
				sizeof(g_aachzKeyboardBindings[s_iKeyboardBindingCapture]),
				"%s: %s", PcKeyboardAction(s_iKeyboardBindingCapture),
				s_fControllerBindingCapture
					? PcText("Press a button", "Appuyez sur un bouton", "Taste drücken", "Pulsa un botón", "Premi un pulsante")
					: PcText("Press a key or mouse button", "Appuyez sur une touche ou un bouton de souris",
						"Taste oder Maustaste drücken", "Pulsa una tecla o botón del ratón",
						"Premi un tasto o pulsante del mouse"));
		}
		return;

        default:
        return;
    }

    SetPrompt(pprompt, promptGroupToClear, PRK_Nil);
}

void SetPromptBlots(PROMPT* pprompt, BLOTS blots)
{
    if (pprompt->blots != blots) {
        if ((pprompt->blots == BLOTS_Hidden) && (pprompt->prk == PRK_PauseMenu)) {
            SetPromptPrk(pprompt);
        }
        if (blots == BLOTS_Hidden) {
            if (pprompt->fReshow == 0) {
                if (pprompt->prk == PRK_PauseMenu) {
                    SetPromptPrk(pprompt);
                }
            }
            else {
                blots = BLOTS_Appearing;
                ChangePromptPrk(pprompt, pprompt->prkReshow);
                pprompt->fReshow = 0;
                pprompt->prkReshow = PRK_Nil;
            }
        }
        SetBlotBlots(pprompt, blots);
    }
}

void ShowPrompt(PROMPT* pprompt)
{
    if ((pprompt->blots != BLOTS_Disappearing) || (pprompt->fReshow == 0))
        ShowBlot(pprompt);
}

void HidePrompt(PROMPT* pprompt)
{
    if ((pprompt->blots == BLOTS_Disappearing) && (pprompt->fReshow != 0)) {
        pprompt->fReshow = 0;
        pprompt->prkReshow = (PRK)-1;
    }
    else
        HideBlot(pprompt);
}

const char* AchzFromRespk(RESPK respk)
{
	if (FLocalizedContent())
	{
		int variant = 0;
		switch (respk)
		{
			case RESPK_Quit: variant = g_saveData.pgsCurrentSave != nullptr; break;
			case RESPK_Vibration: variant = FVibrationEnabled(g_pgsCur) ? 0 : 1; break;
			case RESPK_Music: variant = (g_pgsCur->grfgs & 0x80U) != 0; break;
			case RESPK_Speakers: variant = (g_pgsCur->grfgs & 0x40U) != 0; break;
			case RESPK_BinocInvert: variant = (g_pgsCur->grfgs & 0x200U) != 0; break;
			case RESPK_TurretInvert: variant = (g_pgsCur->grfgs & 0x400U) != 0; break;
			case RESPK_JetpackInvert: variant = (g_pgsCur->grfgs & 0x800U) != 0; break;
			default: break;
		}
		if (const char* localized = PalResponseText(respk, variant))
			return localized;
	}

	if (respk == RESPK_Speakers)
		std::snprintf(g_achzRespk16, sizeof(g_achzRespk16), "%s: %s",
			PcText("Speakers", "Haut-parleurs", "Lautsprecher", "Altavoces", "Altoparlanti"),
			FUserStereoEnabled()
				? PcText("Stereo", "Stéréo", "Stereo", "Estéreo", "Stereo")
				: PcText("Mono", "Mono", "Mono", "Mono", "Mono"));
	else if (respk == RESPK_MusicVolume)
		std::snprintf(g_achzRespk63, sizeof(g_achzRespk63), "%s: %d%%", PcText("Music Volume", "Volume de la musique", "Musiklautstärke", "Volumen de música", "Volume musica"), static_cast<int>(GetUserMusicVolume() * 100.0f + 0.5f));
	else if (respk == RESPK_SfxVolume)
		std::snprintf(g_achzRespk64, sizeof(g_achzRespk64), "%s: %d%%", PcText("Sound Effects", "Effets sonores", "Soundeffekte", "Efectos de sonido", "Effetti sonori"), static_cast<int>(GetUserSfxVolume() * 100.0f + 0.5f));
	else if (respk == RESPK_DialogueVolume)
		std::snprintf(g_achzRespk65, sizeof(g_achzRespk65), "%s: %d%%", PcText("Dialogue", "Dialogues", "Dialog", "Diálogo", "Dialoghi"), static_cast<int>(GetUserDialogueVolume() * 100.0f + 0.5f));

	if (FLocalizedContent())
	{
		switch (respk)
		{
		case RESPK_Video: return PcText("Video...", "Vidéo...", "Video...", "Vídeo...", "Video...");
		case RESPK_Sound: return PcText("Sound...", "Son...", "Sound...", "Sonido...", "Audio...");
		case RESPK_KeyboardMapping: return PcText("Keyboard Mapping...", "Configuration du clavier...", "Tastaturbelegung...", "Asignación del teclado...", "Configurazione tastiera...");
		case RESPK_ControllerMapping: return PcText("Controller Mapping...", "Configuration de la manette...", "Controllerbelegung...", "Asignación del mando...", "Configurazione controller...");
		case RESPK_ResetVideo: return PcText("Reset Video Settings", "Réinitialiser les paramètres vidéo", "Videoeinstellungen zurücksetzen", "Restablecer ajustes de vídeo", "Ripristina impostazioni video");
		case RESPK_ResetMappings: return PcText("Reset Mappings", "Réinitialiser les commandes", "Belegung zurücksetzen", "Restablecer asignaciones", "Ripristina configurazione");
		case RESPK_ExitToDesktop: return PcText("Exit to Desktop", "Quitter vers le bureau", "Zum Desktop beenden", "Salir al escritorio", "Esci al desktop");
		default: break;
		}
	}
    return s_mprespkachz[respk];
}

void BuildSaveSlotText(GS* pgs, char* pchzDst)
{
    int percentComplete = CalculatePercentCompletion(pgs);
    const int levelId = (pgs->gameWorldCur << 8) | pgs->worldLevelCur;
    const LEVELINFO* levelInfo = PlevelInfoFromId(levelId);
    const char* pchzLevel = PchzFriendlyFromLevelId(levelId);
    if (FLocalizedContent() && levelInfo != nullptr)
    {
        const std::ptrdiff_t levelIndexInTable = levelInfo - g_levelTable;
        const char* localized = levelIndexInTable >= 0 && levelIndexInTable < 46
            ? ContentLocalizedText(25 + static_cast<std::size_t>(levelIndexInTable)) : nullptr;
        if (localized != nullptr)
            pchzLevel = localized;
    }
    int totalSeconds = std::max(static_cast<int>(pgs->dt / 60.0f), 1);

    std::snprintf(pchzDst, 64, "%d%% - %s - %d:%02d", percentComplete, pchzLevel, totalSeconds / 60, totalSeconds % 60);
}

void OnPromptActive(PROMPT* pprompt, int fActive)
{
    if (pprompt->fActive == fActive)
        return;

    const int levelId = (g_pgsCur->gameWorldCur << 8) | g_pgsCur->worldLevelCur;

    if (fActive != 0 && g_fMouseCamera && g_gl.window != nullptr)
    {
        glfwSetInputMode(g_gl.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        MOUSE::ResetDelta();
    }

    if (fActive == 0)
    {
        if (levelId == 0x0308)
        {
            ResumeVag();
            ContinueMusicSequencer(false);
        }

        if (levelId != 0x0308 &&
            g_binoc.pdialogPlaying == nullptr &&
            pprompt->prk == PRK_PauseMenu)
        {
            SetUiUis(&g_ui, UIS_Unpausing);
        }
        else
            SetUiUis(&g_ui, UIS_Playing);

        HandlePromptPrkTransition(pprompt, pprompt->prk, PRK_Nil);
        pprompt->pvtblot->pfnHideBlot(pprompt);

        RemoveGrfusr(1);

        pprompt->fActive = 0;
        return;
    }

    if (levelId == 0x0308)
    {
        PauseVag();
        // The PS2 pauses the Deadly Dance audio through s_sfxhVag.  The PC
        // reconstruction splits that presentation between the VAG stream and
        // the music sequencer, so both owners must stop to reproduce it.
        PauseMusicSequencer();
        SetUiUis(&g_ui, UIS_Paused);
    }
    else if (g_binoc.pdialogPlaying == nullptr)
    {
        if (g_ui.uis == UIS_Playing)
            SetUiUis(&g_ui, UIS_Pausing);
        else
            SetUiUis(&g_ui, UIS_Paused);
    }
    else
        SetUiUis(&g_ui, UIS_Paused);

    HandlePromptPrkTransition(pprompt, PRK_Nil, pprompt->prk);
    pprompt->pvtblot->pfnShowBlot(pprompt);

    AddGrfusr(1);
    g_joy.StartJoySelection();

    pprompt->fActive = 1;
}

void ExitOrRestartLevelFromPrompt(PROMPT* pprompt, int fRestartLevel, WIPEK wipek)
{
    if (pprompt == nullptr || g_pgsCur == nullptr)
        return;

    SetPrompt(pprompt, PRP_Basic, PRK_Nil);

    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);

    if (levelId == 0x0106)
    {
        if (fRestartLevel == 0)
        {
            ROB* prob = static_cast<ROB*>(PloFindSwObjectByClass(g_psw, 5, CID_ROB, nullptr));

            if (prob != nullptr)
                SetRobRobs(prob, ROBS_Max);
        }
        else
        {
            LEVELINFO* plevelCurrent = g_transition.m_plevelCurrent;

            if (plevelCurrent == nullptr && !g_transition.m_worldCurrent.empty())
                plevelCurrent = PlevelinfoFromLevelName(g_transition.m_worldCurrent);

            if (plevelCurrent == nullptr)
            {
                std::printf("RESTART LEVEL ERROR: no current level\n");
                return;
            }

            TRANS trans{};
            trans.fSet = 1;
            trans.plevel = plevelCurrent;
            trans.oidWarp = OID_Nil;
            trans.oidWarpContext = OID_Nil;
            trans.grftrans = static_cast<GRFTRANS>(0);

            ActivateWipe(&g_wipe, &trans, wipek);
        }

        return;
    }

    if (fRestartLevel == 0)
    {
        TriggerDefaultExit(0, wipek);
        return;
    }

    LEVELINFO* plevelCurrent = g_transition.m_plevelCurrent;

    if (plevelCurrent == nullptr && !g_transition.m_worldCurrent.empty())
        plevelCurrent = PlevelinfoFromLevelName(g_transition.m_worldCurrent);

    if (plevelCurrent == nullptr)
    {
        std::printf("RESTART LEVEL ERROR: no current level\n");
        return;
    }

    TRANS trans{};
    trans.fSet = 1;
    trans.plevel = plevelCurrent;
    trans.oidWarp = OID_Nil;
    trans.oidWarpContext = OID_Nil;
    trans.grftrans = static_cast<GRFTRANS>(0x10);

    ActivateWipe(&g_wipe, &trans, wipek);
}

void WipeToTitleScreen()
{
    LEVELINFO* plevelSplash = PlevelinfoFromSearchKey(21);

    if (plevelSplash == nullptr)
    {
        std::printf("TITLE SCREEN ERROR: Splash level not found\n");
        return;
    }

    TRANS trans{};
    trans.fSet = 1;
    trans.plevel = plevelSplash;
    trans.oidWarp = OID_Nil;
    trans.oidWarpContext = OID_Nil;
    trans.grftrans = static_cast<GRFTRANS>(0x04);

    ActivateWipe(&g_wipe, &trans, WIPEK_Fade);
}

void UpdatePromptActive(PROMPT* pprompt, JOY* pjoy)
{
    constexpr uint16_t kBtnConfirm = 0x0040;
    constexpr uint16_t kBtnErase = 0x0080;
    constexpr uint16_t kBtnCancel = 0x0810;

    if (pprompt == nullptr || pjoy == nullptr || g_pgsCur == nullptr)
        return;

    const PRK prk = pprompt->prk;

    if (prk < PRK_PauseMenu || prk >= PRK_Max)
        return;

	if ((prk == PRK_KeyboardMapping || prk == PRK_ControllerMapping) &&
		s_iKeyboardBindingCapture >= 0 && g_gl.window != nullptr)
	{
		// Escape is reserved for leaving menus and is never assignable.
		if (glfwGetKey(g_gl.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			const int selection = pprompt->irespk;
			s_iKeyboardBindingCapture = -1;
			s_fKeyboardBindingCaptureArmed = false;
			SetPromptPrk(pprompt);
			pprompt->irespk = selection;
			return;
		}

		const int key = s_fControllerBindingCapture
			? GamepadBindingPressed()
			: KeyboardOrMouseBindingPressed(g_gl.window);
		const int noBinding = s_fControllerBindingCapture ? -1 : GLFW_KEY_UNKNOWN;
		if (!s_fKeyboardBindingCaptureArmed)
		{
			if (key == noBinding)
				s_fKeyboardBindingCaptureArmed = true;
			return;
		}

		if (key != noBinding)
		{
			const int selection = pprompt->irespk;
			if (s_fControllerBindingCapture)
				g_gamepadBindings[s_iKeyboardBindingCapture] = key;
			else
				g_keyboardBindings[s_iKeyboardBindingCapture] = key;
			SaveSystemSettings();
			s_iKeyboardBindingCapture = -1;
			s_fKeyboardBindingCaptureArmed = false;
			SetPromptPrk(pprompt);
			pprompt->irespk = selection;
		}
		return;
	}

    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);
    const PRD& promptData = PrdPrompt(prk);
	RESPK* const promptResponses = ArespkPrompt(prk, promptData);
	const int promptResponseCount = CrespkPrompt(prk, promptData);

    const bool isHudExcludedLevel =
        levelId == 262 ||
        levelId == 516 ||
        levelId == 772 ||
        levelId == 775 ||
        levelId == 1031 ||
        levelId == 1282 ||
        levelId == 1284;

    const bool showGameplayCounters = prk == PRK_PauseMenu && !isHudExcludedLevel;

    if (showGameplayCounters)
    {
        g_lifectr.pvtlifectr->pfnShowBlot(&g_lifectr);
        g_cluectr.pvtblot->pfnShowBlot(&g_cluectr);
        g_keyctr.pvtkeyctr->pfnShowBlot(&g_keyctr);
        g_coinctr.pvtcoinctr->pfnShowBlot(&g_coinctr);
    }
    else
    {
        g_lifectr.pvtlifectr->pfnHideBlot(&g_lifectr);
        g_cluectr.pvtblot->pfnHideBlot(&g_cluectr);
        g_keyctr.pvtkeyctr->pfnHideBlot(&g_keyctr);
        g_coinctr.pvtcoinctr->pfnHideBlot(&g_coinctr);
    }

    const bool showPercentCompletion =
        prk == PRK_PauseMenu ||
        prk == PRK_MemcardChooseSaveSlot ||
        prk == PRK_MemcardChooseLoadSlot;

    if (showPercentCompletion)
    {
        g_percentCompletion = CalculatePercentCompletion(g_pgsCur);
        g_percentctr.pnActual = &g_percentCompletion;
        g_percentctr.pvtctr->pfnShowBlot(&g_percentctr);
    }
    else
        g_percentctr.pvtctr->pfnHideBlot(&g_percentctr);

    if (pprompt->fVagPlaying != 0)
        pprompt->fVagPlaying = FVagPlaying();

    const bool canAcceptInput =
        (pprompt->fVagPlaying == 0 || prk == PRK_MemcardAutoNotify) &&
        pprompt->fReshow == 0 &&
        pprompt->blots == BLOTS_Visible &&
        g_clock.tReal - pprompt->tBlots > 0.1f;

    if (promptResponseCount > 0)
    {
        const int selectionDelta = promptData.fVertical != 0 ? pjoy->DySelectionJoy(g_clock.tReal) : pjoy->DxSelectionJoy(g_clock.tReal);

        if (selectionDelta > 0)
        {
            pprompt->irespk = (pprompt->irespk + 1) % promptResponseCount;
            StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            return;
        }

        if (selectionDelta < 0)
        {
            pprompt->irespk = (pprompt->irespk + promptResponseCount - 1) % promptResponseCount;
            StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            return;
        }
    }

    if (!canAcceptInput)
        return;

    if (promptResponses != nullptr && pprompt->irespk >= 0 &&
        pprompt->irespk < promptResponseCount)
    {
        const RESPK selectedResponse = promptResponses[pprompt->irespk];
        bool adjustable = prk == PRK_VideoMenu || prk == PRK_SoundMenu;

        if (prk == PRK_ControlsMenu)
        {
            adjustable = selectedResponse == RESPK_CameraInvert ||
                selectedResponse == RESPK_Vibration ||
                selectedResponse == RESPK_MouseCamera ||
                selectedResponse == RESPK_MouseSensitivity ||
				selectedResponse == RESPK_ControllerDeadzone ||
                selectedResponse == RESPK_BinocInvert ||
                selectedResponse == RESPK_TurretInvert ||
                selectedResponse == RESPK_JetpackInvert ||
                selectedResponse == RESPK_DisableControllerWhenUnfocused;
        }

        adjustable = adjustable && selectedResponse != RESPK_Back;

        if (adjustable)
        {
            const int optionDelta = pjoy->DxSelectionJoy(g_clock.tReal);

            if (optionDelta != 0)
            {
                ExecutePrompt(pprompt, optionDelta < 0 ? -1 : 1);
                StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr,
                    3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                return;
            }
        }
    }

    const JOY_BUTTON confirmButton = FJapaneseContent() ? BTN_CIRCLE : BTN_CROSS;
    const JOY_BUTTON cancelButton = FJapaneseContent() ? BTN_CROSS : BTN_TRIANGLE;

    if (pjoy->IsPressed(BTN_START) || pjoy->IsPressed(cancelButton))
    {
        pjoy->SetHandled(BTN_START);
        pjoy->SetHandled(cancelButton);
        CancelPrompt(pprompt);
        return;
    }

    if (pjoy->IsPressed(BTN_SQUARE))
    {
        pjoy->SetHandled(BTN_SQUARE);

        if ((prk == PRK_MemcardChooseSaveSlot ||
             prk == PRK_MemcardChooseLoadSlot) &&
            promptResponses != nullptr &&
            pprompt->irespk >= 0 && pprompt->irespk < SAVE_SLOT_COUNT)
        {
            const RESPK response = promptResponses[pprompt->irespk];
            int slot = -1;

            for (int i = 0; i < SAVE_SLOT_COUNT; ++i)
            {
                if (s_arespkSlots[i] == response)
                {
                    slot = i;
                    break;
                }
            }

            if (slot >= 0 && g_saveData.saveData[slot].dt != 0.0f)
            {
                g_saveData.pgsSelectedSave = &g_saveData.saveData[slot];
                SetPrompt(pprompt, PRP_Memcard, PRK_MemcardEraseConfirm);
            }
        }

        return;
    }

    if (pjoy->IsPressed(confirmButton))
    {
        // Consume the confirmation before ExecutePrompt can close the menu or
        // reload a level. Otherwise JT sees this same press as a jump.
        pjoy->SetHandled(confirmButton);
        ExecutePrompt(pprompt);
        return;
    }
}

void CancelPrompt(PROMPT* pprompt)
{
    if (pprompt == nullptr)
        return;

    switch (pprompt->prk)
    {
        case PRK_PauseMenu:
        SetPrompt(pprompt, PRP_Basic, PRK_Nil);
        break;

        case PRK_QuitConfirm:
		case PRK_ExitToDesktopConfirm:
        SetPrompt(pprompt, PRP_Basic, PRK_PauseMenu);
        break;

        case PRK_MemcardChooseSaveSlot:
        case PRK_MemcardChooseNewSlot:
        case PRK_MemcardChooseLoadSlot:
        case PRK_ControlsMenu:
        case PRK_VideoMenu:
		case PRK_SoundMenu:
        SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
        break;

		case PRK_LanguageMenu:
		SetPrompt(pprompt, PRP_Basic, PRK_Nil);
		break;

		case PRK_KeyboardMapping:
		case PRK_ControllerMapping:
		s_iKeyboardBindingCapture = -1;
		s_fKeyboardBindingCaptureArmed = false;
		SetPrompt(pprompt, PRP_Basic, PRK_ControlsMenu);
		break;

        case PRK_MemcardSlotSaved:
        case PRK_MemcardOverwriteConfirm:
        case PRK_MemcardEraseConfirm:
        SetPrompt(pprompt, PRP_Memcard, PRK_Nil);
        break;

        case PRK_OptionsMenu:
        {
            const unsigned playingState = static_cast<unsigned>(static_cast<int>(g_ui.uisPlaying) + static_cast<int>(UIS_Nil));

            if (playingState > 1U)
                SetPrompt(pprompt, PRP_Basic, PRK_PauseMenu);
            else
                SetPrompt(pprompt, PRP_Basic, PRK_Nil);

            break;
        }

        default:
        break;
    }
}

void DrawPrompt(PROMPT* pprompt)
{
    if (pprompt == nullptr || pprompt->pfont == nullptr || pprompt->blots == BLOTS_Hidden || pprompt->uOn <= 0.0f)
        return;

    if (pprompt->prk < PRK_PauseMenu || pprompt->prk >= PRK_Max)
        return;

    PRD& prd = PrdPrompt(pprompt->prk);
	RESPK* const promptResponses = ArespkPrompt(pprompt->prk, prd);
	const int promptResponseCount = CrespkPrompt(pprompt->prk, prd);

    const float uOn = pprompt->uOn;
    const float dxUnscaled = pprompt->dx * uOn;
    const float dyUnscaled = pprompt->dy * uOn;

    // Retail draws prompts in the PS2's 640 x 492.8 screen space.  The GS
    // presentation then expands that canvas to the output dimensions.  Our
    // blot projection is expressed directly in output pixels, so perform the
    // same conversion here instead of treating retail coordinates as pixels.
    const GuiScale promptScale = GetGuiScale();
    const float promptScaleX = promptScale.x;
    const float promptScaleY = promptScale.y;

    const float centerX = pprompt->xOn + pprompt->dx * 0.5f;
    const float centerY = pprompt->yOn + pprompt->dy * 0.5f;

    const float xUnscaled = pprompt->xOn * uOn + centerX * (1.0f - uOn);
    const float yUnscaled = pprompt->yOn * uOn + centerY * (1.0f - uOn);
    const float dx = dxUnscaled * promptScaleX;
    const float dy = dyUnscaled * promptScaleY;
    const float x = centerX + (xUnscaled - centerX) * promptScaleX;
    float y = centerY + (yUnscaled - centerY) * promptScaleY;

    const float pulse = std::sin(g_clock.tReal * 10.0f) * 0.5f + 0.5f;
    const float pulseSquared = pulse * pulse;

    float targetAlpha = 1.0f;

    if (pprompt->prk == PRK_PauseMenu && g_ui.cpblotActive == 2)
        targetAlpha = 1.0f - std::clamp(g_joy.uDeflect2, 0.0f, 1.0f);

    const float alphaBlend = 1.0f - std::exp(-12.0f * g_clock.dtReal);
    pprompt->alpha = glm::mix(pprompt->alpha, targetAlpha, std::clamp(alphaBlend, 0.0f, 1.0f));

    const float alpha = std::clamp(pprompt->alpha, 0.0f, 1.0f);

    // These are PS2 GS vertex-color values. RGB uses 0x80 as full intensity,
    // while alpha in this path is still authored over the full byte range.
    // Dividing the RGB channels by 255 makes the entire prompt about half as
    // bright as retail.
    constexpr float kGsColorScale = 1.0f / 128.0f;
    glm::vec4 edgeBoxColor(128.0f * kGsColorScale, 128.0f * kGsColorScale, 128.0f * kGsColorScale, alpha);
    glm::vec4 edgePulseTarget(111.0f * kGsColorScale, 111.0f * kGsColorScale, 111.0f * kGsColorScale, alpha);
    glm::vec4 titleColor(127.0f * kGsColorScale, 127.0f * kGsColorScale, 127.0f * kGsColorScale, alpha);
    glm::vec4 responseColor(95.0f * kGsColorScale, 95.0f * kGsColorScale, 95.0f * kGsColorScale, alpha);

    const auto GetSelectedColor = [&](float pulseValue)
        {
            const float blue = (47.0f + pulseValue * 64.0f) * kGsColorScale;
            return glm::vec4(111.0f * kGsColorScale, 111.0f * kGsColorScale, blue, alpha);
        };

    CTextBox textBox;

    // -------------------------------------------------------------------------
    // Background/edge rectangle
    // -------------------------------------------------------------------------

    if (pprompt->pte != nullptr)
    {
        textBox.SetPos(x, y);
        textBox.SetSize(dx, dy);
        textBox.SetTextColor(&edgeBoxColor);
        textBox.SetHorizontalJust(JH_Left);
        textBox.SetVerticalJust(JV_Top);

        const glm::vec4 savedEdgeColor = pprompt->pte->m_rgba;

        glm::vec4 edgeColor = savedEdgeColor;
        edgeColor.a = alpha;

        if (promptResponseCount == 0)
            edgeColor = glm::mix(edgeColor, edgePulseTarget, pulseSquared);

        pprompt->pte->m_rgba = edgeColor;

        if (pprompt->pte->m_pfont != nullptr)
        {
            const float savedEdgeScaleX = pprompt->pte->m_rxScaling;
            const float savedEdgeScaleY = pprompt->pte->m_ryScaling;
            pprompt->pte->m_rxScaling *= promptScaleX;
            pprompt->pte->m_ryScaling *= promptScaleY;
            pprompt->pte->m_pfont->EdgeRect(pprompt->pte, &textBox);
            pprompt->pte->m_rxScaling = savedEdgeScaleX;
            pprompt->pte->m_ryScaling = savedEdgeScaleY;
        }

        pprompt->pte->m_rgba = savedEdgeColor;
    }

    // -------------------------------------------------------------------------
    // Prompt title
    // -------------------------------------------------------------------------

    const char* const promptTitle = PchzPromptTitle(pprompt->prk, prd);
    if (promptTitle != nullptr)
    {
        const float titleScaleX = prd.rScaleTitle * uOn * promptScaleX;
        const float titleScaleY = prd.rScaleTitle * uOn * promptScaleY;

        pprompt->pfont->PushScaling(titleScaleX, titleScaleY);

        textBox.SetPos(x, y);
        textBox.SetSize(dx, dy);
        textBox.SetTextColor(&titleColor);
        textBox.SetHorizontalJust(JH_Center);
        textBox.SetVerticalJust(JV_Top);

        CRichText titleText(const_cast<char*>(promptTitle), pprompt->pfont);

        titleText.Draw(&textBox, nullptr);
        y += titleText.DyWrap(500.0f * promptScaleX);

        pprompt->pfont->PopScaling();
    }

    // NTSC/PAL place the divider one unit above the response row. Japanese
    // places it at the title baseline and starts responses eight units later.
    const bool japanesePromptLayout = FJapaneseContent() && promptResponseCount > 0;
    const float dividerY = japanesePromptLayout ? y : y - promptScaleY;
    if (japanesePromptLayout)
        y += 8.0f * promptScaleY;

    // -------------------------------------------------------------------------
    // Responses
    // -------------------------------------------------------------------------

    const float responseScaleX = prd.rScaleRespk * uOn * promptScaleX;
    const float responseScaleY = prd.rScaleRespk * uOn * promptScaleY;

    pprompt->pfont->PushScaling(responseScaleX, responseScaleY);

    const float baseLineHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    const float spaceWidth = static_cast<float>(pprompt->pfont->m_dxSpaceUnscaled) * pprompt->pfont->m_rxScale;

    float cursorX = x;
    float cursorY = y;

    // The original prompt was designed for short menus. Keep long PC settings
    // and mapping menus within that layout and scroll around the selection.
    int firstResponse = 0;
    int lastResponse = promptResponseCount;
    const int visibleResponses = VisiblePromptResponseCount(pprompt->prk, promptResponseCount);
    if (visibleResponses < promptResponseCount)
    {
        firstResponse = std::clamp(pprompt->irespk - visibleResponses / 2,
            0, std::max(0, promptResponseCount - visibleResponses));
        lastResponse = std::min(promptResponseCount, firstResponse + visibleResponses);
    }

    // Measure the complete horizontal response row.
    if (prd.fVertical == 0)
    {
        float totalWidth = 0.0f;

        for (int i = firstResponse; i < lastResponse; ++i)
        {
            if (i > 0)
                totalWidth += spaceWidth * 2.0f;

            const RESPK response = promptResponses[i];

            if (response == RESPK_Nil)
                continue;

            const char* responseText = AchzFromRespk(response);

            if (responseText == nullptr)
                continue;

            CRichText richText(const_cast<char*>(responseText), pprompt->pfont);

            totalWidth += richText.Dx();
        }

        cursorX = x + (dx - totalWidth) * 0.5f;
    }

    for (int i = firstResponse; i < lastResponse; ++i)
    {
        const RESPK response = promptResponses[i];

        if (response == RESPK_Nil)
            continue;

        const char* responseText = AchzFromRespk(response);

        if (responseText == nullptr)
            continue;

        // Measure the item at its normal response scale.
        CRichText unselectedText(const_cast<char*>(responseText), pprompt->pfont);

        const float unselectedWidth = unselectedText.Dx();
        const bool selected = i == pprompt->irespk;

        glm::vec4 color = selected ? GetSelectedColor(pulse) : responseColor;
        const float selectionScale = selected ? 1.0f + (1.0f - pulse) * 0.1f : 1.0f;
        const float itemScaleX = responseScaleX * selectionScale;
        const float itemScaleY = responseScaleY * selectionScale;

        pprompt->pfont->PushScaling(itemScaleX, itemScaleY);

        // Reconstruct after changing the scale so switched fonts inherit it.
        CRichText selectedText(const_cast<char*>(responseText), pprompt->pfont);

        const float selectedWidth = selectedText.Dx();
        const float selectedHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;

        float drawX = cursorX + (unselectedWidth - selectedWidth) * 0.5f;
        const float drawY = cursorY + (baseLineHeight - selectedHeight) * 0.5f;

        if (prd.fVertical != 0)
            drawX = x + (dx - unselectedWidth) * 0.5f + (unselectedWidth - selectedWidth) * 0.5f;

        textBox.SetPos(drawX, drawY);
        textBox.SetSize(dx, selectedHeight);
        textBox.SetTextColor(&color);
        textBox.SetHorizontalJust(JH_Left);
        textBox.SetVerticalJust(JV_Top);

        selectedText.Draw(&textBox, nullptr);

        pprompt->pfont->PopScaling();

        if (prd.fVertical == 0)
            cursorX += unselectedWidth + spaceWidth * 2.0f;
        else
            cursorY += baseLineHeight;
    }

    pprompt->pfont->PopScaling();

    // -------------------------------------------------------------------------
    // Title/response divider
    // -------------------------------------------------------------------------

    if (promptTitle != nullptr && promptResponseCount > 0)
    {
        const glm::vec4 dividerColor(95.0f * kGsColorScale, 95.0f * kGsColorScale, 95.0f * kGsColorScale, alpha);

        DrawLineScreen(x, dividerY, 0.0f, x + dx, dividerY, 0.0f, dividerColor, false);
    }
}

char GetAnimatedPromptCharacter()
{
    constexpr int frameCount = static_cast<int>(sizeof(s_achzAnimatedPromptFrames) - 1);

    const int frame =
        static_cast<int>(g_clock.tReal * 5.0f) % frameCount;

    return s_achzAnimatedPromptFrames[frame];
}

PROMPT g_prompt;
CTextEdge g_tePrompt;
float g_promptFade;

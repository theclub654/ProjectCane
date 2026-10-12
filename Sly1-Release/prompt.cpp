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
#include "gui_layout.h"
#include "render.h"
#include "font.h"
#include "menustyle.h"
#include "mouseinput.h"
#include "menuhelp.h"
#include "menufont.h"
#include <cctype>
#include "dlss.h"
#include "fsr.h"
#include <cstdlib>
#include <string>
#include "debug.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <vector>

namespace
{
int s_iKeyboardBindingCapture = -1;
bool s_fKeyboardBindingCaptureArmed = false;
bool s_fControllerBindingCapture = false;

constexpr const char* s_apchzKeyboardActions[BTN_MAX] =
{
    "Move Up", "Move Down", "Move Left", "Move Right",
    "Square", "Circle", "Cross", "Triangle",
    "Start", "Select", "L1", "R1", "L2", "R2", "L3", "R3"
};

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
    // Any mouse button (1-8) or the wheel can be bound like a key.
    const int mouse = MouseBindingPressed(window);
    return mouse >= 0 ? mouse : GLFW_KEY_UNKNOWN;
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
    if (g_joy.gamepadConnected)
        return true;
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid)
    {
        if (glfwJoystickPresent(jid))
            return true;
    }
    return false;
}

bool FMappingPrompt(PRK prk)
{
	return prk == PRK_KeyboardMapping || prk == PRK_ControllerMapping;
}

// PC-only hotkeys listed after the 16 game buttons in Mouse & Keyboard
// Mapping. Capture index BTN_MAX + k.
constexpr int kExtraBindings = 3;
const char* const s_apchzExtraActions[kExtraBindings] =
{
	"Fast Forward (Hold)", "Fast Forward (Toggle)", "Screenshot"
};

int ExtraBindingKey(int k)
{
	switch (k)
	{
	case 0: return ModActionKey(MODACT_FastForwardHold);
	case 1: return ModActionKey(MODACT_FastForwardToggle);
	default: return QolScreenshotKey();
	}
}

void SetExtraBindingKey(int k, int key)
{
	switch (k)
	{
	case 0: SetModActionKey(MODACT_FastForwardHold, key); break;
	case 1: SetModActionKey(MODACT_FastForwardToggle, key); break;
	default: SetQolScreenshotKey(key); break;
	}
}

char* BindingLabel(int index)
{
	return index < BTN_MAX ? g_aachzKeyboardBindings[index] : g_aachzExtraBindings[index - BTN_MAX];
}

const char* BindingActionName(int index)
{
	return index < BTN_MAX ? s_apchzKeyboardActions[index] : s_apchzExtraActions[index - BTN_MAX];
}

PRD& PrdPrompt(PRK prk)
{
	// Keep PC-only mapping descriptors out of the release descriptor table.
	// The rest of the prompt system can still treat them exactly like a PRD,
	// without indexing beyond the original table storage in stale/incremental
	// builds.
	static PRD keyboardMapping =
	{
		"Mouse && Keyboard Mapping", 0.8f, 0.55f, 1,
		static_cast<int>(std::size(s_arespkKbMouseMapping)),
		s_arespkKbMouseMapping
	};
	static PRD controllerMapping =
	{
		"Controller Mapping", 0.8f, 0.55f, 1,
		static_cast<int>(std::size(s_arespkKeyboardMapping)),
		s_arespkKeyboardMapping
	};

	if (prk == PRK_KeyboardMapping)
		return keyboardMapping;
	if (prk == PRK_ControllerMapping)
		return controllerMapping;
	return s_mpprkprd[prk];
}

RESPK* ArespkPrompt(PRK prk, const PRD& prd)
{
	if (prk == PRK_KeyboardMapping)
		return s_arespkKbMouseMapping;
	return FMappingPrompt(prk) ? s_arespkKeyboardMapping : prd.arespk;
}

int CrespkPrompt(PRK prk, const PRD& prd)
{
	int crespk = prd.crespk;
	if (prk == PRK_KeyboardMapping)
		crespk = static_cast<int>(std::size(s_arespkKbMouseMapping));
	else if (FMappingPrompt(prk))
		crespk = static_cast<int>(std::size(s_arespkKeyboardMapping));

	// Menus have no "Back" row: Circle / Esc / right click go back (see the
	// button prompt bar).
	const RESPK* arespk = ArespkPrompt(prk, prd);
	if (prd.fVertical != 0 && arespk != nullptr && crespk > 1 && arespk[crespk - 1] == RESPK_Back)
		--crespk;
	return crespk;
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

        for (int i = 0; i < modeCount; ++i)
        {
            if (modes[i].refreshRate >= 30)
                frameRates.push_back(modes[i].refreshRate);
        }
    }

    std::sort(frameRates.begin(), frameRates.end());
    frameRates.erase(std::unique(frameRates.begin(), frameRates.end()), frameRates.end());

    return frameRates;
}

void SelectSupportedFrameRate(int direction)
{
    const std::vector<int> frameRates = SupportedFrameRates();

    // Cycles the monitor's refresh rates plus "Uncapped" (0), in either direction.
    std::vector<int> choices = frameRates;
    choices.push_back(0);
    const int count = static_cast<int>(choices.size());
    auto current = std::find(choices.begin(), choices.end(), g_targetFrameRate);
    int index = current == choices.end()
        ? (direction < 0 ? count : -1)          // unknown rate: start from either end
        : static_cast<int>(current - choices.begin());
    index = ((index + (direction < 0 ? -1 : 1)) % count + count) % count;
    g_targetFrameRate = choices[index];
}

void SelectInternalResolution(int direction)
{
    constexpr int resolutions[] = { 0, 360, 480, 720, 1080, 1440, 2160 };
    auto current = std::find(std::begin(resolutions), std::end(resolutions),
        g_internalResolutionHeight);
    int index = current == std::end(resolutions)
        ? 0
        : static_cast<int>(current - std::begin(resolutions));
    constexpr int count = static_cast<int>(std::size(resolutions));
    index = (index + (direction < 0 ? -1 : 1) + count) % count;
	const int previousResolution = g_internalResolutionHeight;
	g_internalResolutionHeight = resolutions[index];
	if (!ApplyInternalResolutionSettings())
		g_internalResolutionHeight = previousResolution;
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
        // With the button prompt bar on, "Erase Game" is shown there with the
        // right icon for the controller / keyboard / mouse.
        if (MenuStyleValue(MSOPT_PromptBar) == 0)
        {
            g_pnote.pvtblot->pfnSetBlotAchzDraw(&g_pnote, (char*)"&2S&.: Erase Game");
            SetBlotDtVisible(&g_pnote, 0.0f);
            SetBlotFontScale(&g_pnote, 0.6f);
            g_pnote.pvtblot->pfnShowBlot(&g_pnote);
        }
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

// -----------------------------------------------------------------------------
// Two-column layout for long vertical menus (Video, Mods, ...): items fill the
// left column top to bottom, then the right column; a trailing "Back" gets its
// own centered row underneath. Up/Down move within a column, Left/Right switch
// columns. The text scale shrinks automatically if the box would not fit.
// -----------------------------------------------------------------------------
namespace
{
constexpr int kTwoColumnMinResponses = 12;
constexpr float kPromptMaxWidth = 600.0f;        // PS2 canvas is 640 wide
float s_arScaleRespkFit[PRK_Max] = {};           // 0 = use the PRD scale

// Mouse hit-testing: DrawPrompt records where each response was drawn
// (menu/output pixels); UpdatePromptActive tests the cursor against it.
struct PROMPTHIT
{
	float x0, y0, x1, y1;
	int irespk;
};
constexpr int kMaxPromptHits = 64;
PROMPTHIT s_ahitPrompt[kMaxPromptHits] = {};
int s_chitPrompt = 0;
PRK s_prkHit = PRK_Nil;
float s_tHit = -1.0f;

int IrespkFromMouse(PRK prk)
{
	float mx = 0.0f;
	float my = 0.0f;
	if (prk != s_prkHit || g_clock.tReal - s_tHit > 0.25f || !MouseMenuCursor(&mx, &my))
		return -1;
	for (int i = 0; i < s_chitPrompt; ++i)
	{
		const PROMPTHIT& hit = s_ahitPrompt[i];
		if (mx >= hit.x0 && mx < hit.x1 && my >= hit.y0 && my < hit.y1)
			return hit.irespk;
	}
	return -1;
}

void ConsumeAllButtons(JOY* pjoy)
{
	for (int button = 0; button < BTN_MAX; ++button)
		pjoy->SetHandled(static_cast<JOY_BUTTON>(button));
}

// Escape edge detection for "Back" (shared with the value editor so closing
// the editor with Escape does not also leave the menu).
bool s_fPromptEscapeWasDown = true;

// Option name without its value: "Text Size: 100%" -> "Text Size". Empty
// when the label uses controller-glyph markup.
std::string SettingName(RESPK respk)
{
	const char* pchz = AchzFromRespk(respk);
	std::string name = pchz != nullptr ? pchz : "";
	const size_t colon = name.find(':');
	if (colon != std::string::npos)
		name.resize(colon);
	for (size_t ich = 0; ich < name.size(); ++ich)
	{
		if (name[ich] != '&')
			continue;
		if (ich + 1 < name.size() && name[ich + 1] == '&')
		{
			++ich;
			continue;
		}
		return std::string();
	}
	return name;
}

// -----------------------------------------------------------------------------
// Numeric settings (percentages, colors, sizes, volumes)
// -----------------------------------------------------------------------------
struct NUMROW
{
	int value = 0;
	int min = 0;
	int max = 100;
	int step = 1;
};

int PercentFromVolume(float volume)
{
	return static_cast<int>(std::lround(volume * 100.0f));
}

bool FNumericRow(RESPK respk, NUMROW* pnum)
{
	NUMROW num;
	if (respk >= RESPK_MsOpt0 && respk < RESPK_MsOpt0 + MSOPT_Max)
	{
		const MSOPT opt = static_cast<MSOPT>(respk - RESPK_MsOpt0);
		if (!MenuStyleNumericRange(opt, &num.min, &num.max, &num.step))
			return false;
		num.value = MenuStyleValue(opt);
	}
	else if (respk >= RESPK_MouseOpt0 && respk < RESPK_MouseOpt0 + MOUSEOPT_Max)
	{
		const MOUSEOPT opt = static_cast<MOUSEOPT>(respk - RESPK_MouseOpt0);
		if (!MouseOptNumericRange(opt, &num.min, &num.max, &num.step))
			return false;
		num.value = MouseOptValue(opt);
	}
	else if (respk == RESPK_CameraSensitivity)
		num = { QolCameraSensitivityPercent(), 10, 400, 5 };
	else if (respk == RESPK_Fov)
		num = { QolFovPercent(), 0, 60, 5 };
	else if (respk == RESPK_MusicVolume)
		num = { PercentFromVolume(GetUserMusicVolume()), 0, 100, 5 };
	else if (respk == RESPK_SfxVolume)
		num = { PercentFromVolume(GetUserSfxVolume()), 0, 100, 5 };
	else if (respk == RESPK_DialogueVolume)
		num = { PercentFromVolume(GetUserDialogueVolume()), 0, 100, 5 };
	else
		return false;
	if (pnum != nullptr)
		*pnum = num;
	return true;
}

struct VALUEEDIT
{
	bool fActive = false;
	RESPK respk = RESPK_Nil;
	PRK prk = PRK_Nil;
	NUMROW num;
	int value = 0;
	int orig = 0;
	int origTheme = 0;
	char achzTyped[8] = {};
	bool fTyping = false;
	bool fDragging = false;
	bool fRects = false;
	float xBar0 = 0.0f, yBar0 = 0.0f, xBar1 = 0.0f, yBar1 = 0.0f;
	float xPanel0 = 0.0f, yPanel0 = 0.0f, xPanel1 = 0.0f, yPanel1 = 0.0f;
	bool afKeyPrev[32] = {};
};
VALUEEDIT s_edit;

const int s_aeditKey[] =
{
	GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4,
	GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9,
	GLFW_KEY_KP_0, GLFW_KEY_KP_1, GLFW_KEY_KP_2, GLFW_KEY_KP_3, GLFW_KEY_KP_4,
	GLFW_KEY_KP_5, GLFW_KEY_KP_6, GLFW_KEY_KP_7, GLFW_KEY_KP_8, GLFW_KEY_KP_9,
	GLFW_KEY_BACKSPACE, GLFW_KEY_DELETE, GLFW_KEY_ENTER, GLFW_KEY_KP_ENTER, GLFW_KEY_ESCAPE
};
constexpr int kEditKeyCount = static_cast<int>(std::size(s_aeditKey));
constexpr int kEditKeyBackspace = 20;
constexpr int kEditKeyDelete = 21;
constexpr int kEditKeyEnter = 22;
constexpr int kEditKeyKpEnter = 23;
constexpr int kEditKeyEscape = 24;
static_assert(kEditKeyCount <= 32, "afKeyPrev too small");

void SetNumericRow(RESPK respk, int value, bool fRaw)
{
	NUMROW num;
	if (!FNumericRow(respk, &num))
		return;
	value = std::clamp(value, num.min, num.max);
	if (respk >= RESPK_MsOpt0 && respk < RESPK_MsOpt0 + MSOPT_Max)
		SetMenuStyleValue(static_cast<MSOPT>(respk - RESPK_MsOpt0), value, fRaw);
	else if (respk >= RESPK_MouseOpt0 && respk < RESPK_MouseOpt0 + MOUSEOPT_Max)
		SetMouseOpt(static_cast<MOUSEOPT>(respk - RESPK_MouseOpt0), value);
	else if (respk == RESPK_CameraSensitivity)
		SetQolCameraSensitivityPercent(value);
	else if (respk == RESPK_Fov)
		SetQolFovPercent(value);
	else
	{
		if (respk == RESPK_MusicVolume)
			SetUserMusicVolume(static_cast<float>(value) / 100.0f);
		else if (respk == RESPK_SfxVolume)
			SetUserSfxVolume(static_cast<float>(value) / 100.0f);
		else if (respk == RESPK_DialogueVolume)
			SetUserDialogueVolume(static_cast<float>(value) / 100.0f);
		if (!s_edit.fActive)   // the editor saves once when it closes
			SaveSystemSettings();
	}
}

void RebuildKeepingSelection(PROMPT* pprompt)
{
	const int selection = pprompt->irespk;
	SetPromptPrk(pprompt);
	pprompt->irespk = selection;
}

// Opens the value editor for a numeric row. Returns false for other rows.
bool OpenValueEditor(PROMPT* pprompt, RESPK respk)
{
	NUMROW num;
	if (pprompt == nullptr || !FNumericRow(respk, &num))
		return false;
	s_edit = VALUEEDIT{};
	s_edit.fActive = true;
	s_edit.respk = respk;
	s_edit.prk = pprompt->prk;
	s_edit.num = num;
	s_edit.value = num.value;
	s_edit.orig = num.value;
	s_edit.origTheme = MenuStyleValue(MSOPT_Theme);
	for (int i = 0; i < kEditKeyCount; ++i)
		s_edit.afKeyPrev[i] = g_gl.window != nullptr && glfwGetKey(g_gl.window, s_aeditKey[i]) == GLFW_PRESS;
	return true;
}

void ApplyEditValue(PROMPT* pprompt, int value)
{
	value = std::clamp(value, s_edit.num.min, s_edit.num.max);
	if (value == s_edit.value)
		return;
	s_edit.value = value;
	SetNumericRow(s_edit.respk, value, false);
	RebuildKeepingSelection(pprompt);
}

void CloseValueEditor(PROMPT* pprompt, bool fCommit)
{
	if (fCommit && s_edit.fTyping && s_edit.achzTyped[0] != '\0')
		ApplyEditValue(pprompt, std::atoi(s_edit.achzTyped));
	if (!fCommit)
	{
		// Restore exactly (including the Customization theme).
		SetNumericRow(s_edit.respk, s_edit.orig, true);
		if (s_edit.respk >= RESPK_MsOpt0 && s_edit.respk < RESPK_MsOpt0 + MSOPT_Max)
			SetMenuStyleValue(MSOPT_Theme, s_edit.origTheme, true);
	}
	s_edit.fActive = false;
	if (s_edit.respk == RESPK_MusicVolume || s_edit.respk == RESPK_SfxVolume || s_edit.respk == RESPK_DialogueVolume)
		SaveSystemSettings();
	s_fPromptEscapeWasDown = true;
	if (FMenuStyleSounds())
		StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
	RebuildKeepingSelection(pprompt);
}

int ValueFromBarX(float x)
{
	const float width = s_edit.xBar1 - s_edit.xBar0;
	const float u = width > 1.0f ? std::clamp((x - s_edit.xBar0) / width, 0.0f, 1.0f) : 0.0f;
	const float raw = static_cast<float>(s_edit.num.min) + u * static_cast<float>(s_edit.num.max - s_edit.num.min);
	return static_cast<int>(std::lround(raw));
}

// While the editor is open it owns all menu input.
void UpdateValueEditor(PROMPT* pprompt, JOY* pjoy)
{
	if (pprompt->prk != s_edit.prk || pprompt->blots == BLOTS_Hidden)
	{
		s_edit.fActive = false;
		return;
	}

	bool afPressed[kEditKeyCount] = {};
	for (int i = 0; i < kEditKeyCount; ++i)
	{
		const bool fDown = g_gl.window != nullptr && glfwGetKey(g_gl.window, s_aeditKey[i]) == GLFW_PRESS;
		afPressed[i] = fDown && !s_edit.afKeyPrev[i];
		s_edit.afKeyPrev[i] = fDown;
	}

	const int dx = pjoy->DxSelectionJoy(g_clock.tReal);
	const int dy = pjoy->DySelectionJoy(g_clock.tReal);
	const bool fAccept = afPressed[kEditKeyEnter] || afPressed[kEditKeyKpEnter] || pjoy->IsPressed(BTN_CROSS);
	const bool fCancel = afPressed[kEditKeyEscape] || pjoy->IsPressed(BTN_CIRCLE) ||
		pjoy->IsPressed(BTN_TRIANGLE) || FMouseRightPressed();
	ConsumeAllButtons(pjoy);

	if (fCancel)
	{
		CloseValueEditor(pprompt, false);
		return;
	}
	if (fAccept)
	{
		CloseValueEditor(pprompt, true);
		return;
	}

	// Typing digits.
	for (int i = 0; i < 20; ++i)
	{
		if (!afPressed[i])
			continue;
		if (!s_edit.fTyping)
		{
			s_edit.fTyping = true;
			s_edit.achzTyped[0] = '\0';
		}
		const size_t len = std::strlen(s_edit.achzTyped);
		if (len < 5)
		{
			s_edit.achzTyped[len] = static_cast<char>('0' + (i % 10));
			s_edit.achzTyped[len + 1] = '\0';
		}
		ApplyEditValue(pprompt, std::atoi(s_edit.achzTyped));
	}
	if (afPressed[kEditKeyBackspace] || afPressed[kEditKeyDelete])
	{
		if (!s_edit.fTyping)
		{
			s_edit.fTyping = true;
			std::snprintf(s_edit.achzTyped, sizeof(s_edit.achzTyped), "%d", s_edit.value);
		}
		const size_t len = std::strlen(s_edit.achzTyped);
		if (len > 0)
			s_edit.achzTyped[len - 1] = '\0';
		if (s_edit.achzTyped[0] != '\0')
			ApplyEditValue(pprompt, std::atoi(s_edit.achzTyped));
	}

	// Left/Right = one step, Up/Down = fine (1), wheel = one step per notch.
	int delta = dx * s_edit.num.step - dy;
	delta += MouseWheelNotches() * s_edit.num.step;
	if (delta != 0)
	{
		s_edit.fTyping = false;
		ApplyEditValue(pprompt, s_edit.value + delta);
	}

	// Mouse: drag the slider; clicking outside the panel accepts.
	float mx = 0.0f;
	float my = 0.0f;
	const bool fCursor = MouseMenuCursor(&mx, &my) && s_edit.fRects;
	if (fCursor && FMouseLeftPressed())
	{
		const float slop = (s_edit.yBar1 - s_edit.yBar0) * 1.5f;
		if (mx >= s_edit.xBar0 - slop && mx <= s_edit.xBar1 + slop &&
			my >= s_edit.yBar0 - slop && my <= s_edit.yBar1 + slop)
			s_edit.fDragging = true;
		else if (mx < s_edit.xPanel0 || mx > s_edit.xPanel1 || my < s_edit.yPanel0 || my > s_edit.yPanel1)
		{
			CloseValueEditor(pprompt, true);
			return;
		}
	}
	if (s_edit.fDragging)
	{
		if (!FMouseLeftHeld())
			s_edit.fDragging = false;
		else if (fCursor)
		{
			s_edit.fTyping = false;
			ApplyEditValue(pprompt, ValueFromBarX(mx));
		}
	}
}

int s_iImportBackup = -1;   // backup picked in Import / Export Settings

// After an import: re-read every settings file and apply it.
void ReloadAllSettings()
{
	ReloadDlssUserSettings();
	ReloadFsrSettings();
	ReloadQolSettings();
	ReloadModsSettings();
	StartupMenuStyle();
	StartupMouseInput();
	RefreshMenuStyleStrings();
	RefreshMouseStrings();
	RefreshModsMenuStrings();
}

// Options > Reset to Default.
void ResetAllSettings()
{
	ResetSystemSettingsToDefault();   // video, sound, key / pad bindings
	ResetDlssUserSettings();
	ResetFsrSettings();
	ResetQolSettings();
	ResetModsSettings();
	ResetMenuStyle();
	ResetMouseInput();
	if (g_pgsCur != nullptr)
	{
		// Invert options (camera, binoc-u-com, turrets, jetpack) and vibration.
		g_pgsCur->grfgs &= ~static_cast<GRFGS>(0x1000 | 0x800 | 0x400 | 0x200 | 0x20);
	}
	vibrationSetting = 1;
	RefreshMenuStyleStrings();
	RefreshMouseStrings();
	RefreshModsMenuStrings();
}

float LineHeightFont(CFontBrx* pfont)
{
	return static_cast<float>(pfont->m_dyUnscaled) * pfont->m_ryScale;
}

// Width of a menu string with the current font scaling, using the TrueType
// font from Customization > Text > Font when one is selected.
float DxPromptText(const char* pchz, CFontBrx* pfont)
{
	const int ttf = MenuStyleTtfFont();
	if (FMenuFontUsable(ttf, pchz))
		return DxMenuFontText(ttf, pchz, LineHeightFont(pfont));
	CRichText text(pchz, pfont);
	return text.Dx();
}

// Word-wraps plain text to dxMax using the current font scaling.
std::string WrapText(const char* pchz, CFontBrx* pfont, float dxMax, int* pcLines)
{
	std::string out;
	std::string line;
	int cLines = 1;
	const char* pch = pchz;
	while (*pch != '\0')
	{
		while (*pch == ' ')
			++pch;
		const char* pchEnd = pch;
		while (*pchEnd != '\0' && *pchEnd != ' ')
			++pchEnd;
		if (pchEnd == pch)
			break;
		const std::string word(pch, pchEnd);
		const std::string candidate = line.empty() ? word : line + " " + word;
		if (!line.empty() && DxPromptText(candidate.c_str(), pfont) > dxMax)
		{
			out += line;
			out += '\n';
			++cLines;
			line = word;
		}
		else
			line = candidate;
		pch = pchEnd;
	}
	out += line;
	if (pcLines != nullptr)
		*pcLines = cLines;
	return out;
}

// Multi-column menus: more than 5 options (not counting Back) adds a column,
// up to 4 columns. Columns are filled top to bottom, left to right, and kept
// balanced; Back stays centered on its own row underneath.
constexpr int kMaxRowsPerColumn = 5;
constexpr int kMaxColumns = 4;
constexpr int kMaxLayoutItems = 64;

// Section headers ("Game", "Camera", ...) are rows that can't be selected.
bool FHeaderRespk(RESPK respk)
{
	return respk >= RESPK_HdrFirst && respk <= RESPK_HdrLast;
}

struct PROMPTLAYOUT
{
	bool fTwoColumn = false;   // true for any multi-column / sectioned layout
	bool fBack = false;
	int cItems = 0;            // responses excluding the trailing Back
	int cRows = 0;             // rows per column
	int cCols = 1;
	signed char acol[kMaxLayoutItems] = {};
	signed char arow[kMaxLayoutItems] = {};
};

// Vertical menus: with section headers, every section gets its own column
// (up to 4). Otherwise more than 5 options (not counting Back) adds a column,
// up to 4, with balanced rows. Back stays centered on its own row underneath.
PROMPTLAYOUT LayoutPrompt(PRK prk, const PRD& prd, const RESPK* arespk, int crespk)
{
	(void)prk;
	PROMPTLAYOUT layout;
	if (prd.fVertical == 0 || arespk == nullptr || crespk <= 0 || crespk > kMaxLayoutItems)
		return layout;
	const bool fBack = arespk[crespk - 1] == RESPK_Back;
	const int cItems = crespk - (fBack ? 1 : 0);

	bool fHeaders = false;
	for (int i = 0; i < cItems; ++i)
		fHeaders = fHeaders || FHeaderRespk(arespk[i]);

	if (fHeaders)
	{
		int col = 0;
		int row = 0;
		int maxRows = 0;
		for (int i = 0; i < cItems; ++i)
		{
			if (FHeaderRespk(arespk[i]) && i > 0 && col < kMaxColumns - 1)
			{
				++col;
				row = 0;
			}
			layout.acol[i] = static_cast<signed char>(col);
			layout.arow[i] = static_cast<signed char>(row);
			++row;
			maxRows = (std::max)(maxRows, row);
		}
		layout.cCols = col + 1;
		layout.cRows = maxRows;
	}
	else
	{
		if (cItems <= kMaxRowsPerColumn)
		{
			// Classic single column (Back included as an ordinary row).
			for (int i = 0; i < crespk; ++i)
			{
				layout.acol[i] = 0;
				layout.arow[i] = static_cast<signed char>(i);
			}
			layout.cRows = crespk;
			return layout;
		}
		layout.cCols = (std::min)(kMaxColumns, (cItems + kMaxRowsPerColumn - 1) / kMaxRowsPerColumn);
		layout.cRows = (cItems + layout.cCols - 1) / layout.cCols;
		for (int i = 0; i < cItems; ++i)
		{
			layout.acol[i] = static_cast<signed char>(i / layout.cRows);
			layout.arow[i] = static_cast<signed char>(i % layout.cRows);
		}
	}

	layout.fTwoColumn = true;
	layout.fBack = fBack;
	layout.cItems = cItems;
	if (fBack)
	{
		layout.acol[cItems] = 0;
		layout.arow[cItems] = static_cast<signed char>(layout.cRows);
	}
	return layout;
}

int ColumnOf(const PROMPTLAYOUT& layout, int i) { return (i >= 0 && i < kMaxLayoutItems) ? layout.acol[i] : 0; }
int RowOf(const PROMPTLAYOUT& layout, int i) { return (i >= 0 && i < kMaxLayoutItems) ? layout.arow[i] : 0; }

// Widest box (PS2 canvas units) that still fits the screen.
float PromptMaxWidth()
{
	const GuiScale scale = GetGuiScale();
	const float canvasWidth = scale.x > 0.0f ? g_gl.width / scale.x : 640.0f;
	return (std::max)(kPromptMaxWidth, canvasWidth * 0.88f);
}

// Customization > Text Size / Title Size scale every prompt.
float RespkBaseScale(const PRD& prd)
{
	return prd.rScaleRespk * MenuStyleTextScale();
}

float TitleBaseScale(const PRD& prd)
{
	return prd.rScaleTitle * MenuStyleTitleScale();
}

float ScaleRespkPrompt(PRK prk, const PRD& prd)
{
	const float fit = (prk >= 0 && prk < PRK_Max) ? s_arScaleRespkFit[prk] : 0.0f;
	return fit > 0.0f ? fit : RespkBaseScale(prd);
}

// Customization > Font: the prompt normally uses the game's main font.
void ApplyMenuStyleFont(PROMPT* pprompt)
{
	if (pprompt != nullptr && g_pfont != nullptr)
		pprompt->pfont = static_cast<CFontBrx*>(MenuStyleFont(g_pfont));
}

bool FMenuStylePrompt(PRK prk)
{
	return prk >= PRK_Custom && prk <= PRK_CustEffects;
}

// Up/Down/Left/Right in a multi-column (or sectioned) menu. Headers are
// skipped; Down past the end of a column continues in the next one.
int MoveTwoColumnSelection(const PROMPTLAYOUT& layout, const RESPK* arespk, int i, int dx, int dy)
{
	const int back = layout.fBack ? layout.cItems : -1;
	const auto Selectable = [&](int j) { return j >= 0 && j < layout.cItems && !FHeaderRespk(arespk[j]); };
	const auto Find = [&](int col, int row)
		{
			for (int j = 0; j < layout.cItems; ++j)
				if (layout.acol[j] == col && layout.arow[j] == row)
					return j;
			return -1;
		};
	const auto FirstInCol = [&](int col)
		{
			for (int r = 0; r < layout.cRows; ++r)
			{
				const int j = Find(col, r);
				if (Selectable(j))
					return j;
			}
			return -1;
		};
	const auto LastInCol = [&](int col)
		{
			for (int r = layout.cRows - 1; r >= 0; --r)
			{
				const int j = Find(col, r);
				if (Selectable(j))
					return j;
			}
			return -1;
		};
	const auto FirstAny = [&]()
		{
			for (int c = 0; c < layout.cCols; ++c)
			{
				const int j = FirstInCol(c);
				if (j >= 0)
					return j;
			}
			return back >= 0 ? back : i;
		};
	const auto LastAny = [&]()
		{
			for (int c = layout.cCols - 1; c >= 0; --c)
			{
				const int j = LastInCol(c);
				if (j >= 0)
					return j;
			}
			return back >= 0 ? back : i;
		};

	if (i == back || !Selectable(i))
	{
		if (dy > 0)
			return FirstAny();
		if (dy < 0)
		{
			const int j = LastInCol(0);
			return j >= 0 ? j : LastAny();
		}
		return i;
	}

	const int col = layout.acol[i];
	const int row = layout.arow[i];

	if (dy > 0)
	{
		for (int r = row + 1; r < layout.cRows; ++r)
		{
			const int j = Find(col, r);
			if (Selectable(j))
				return j;
		}
		for (int c = col + 1; c < layout.cCols; ++c)
		{
			const int j = FirstInCol(c);
			if (j >= 0)
				return j;
		}
		return back >= 0 ? back : FirstAny();
	}
	if (dy < 0)
	{
		for (int r = row - 1; r >= 0; --r)
		{
			const int j = Find(col, r);
			if (Selectable(j))
				return j;
		}
		for (int c = col - 1; c >= 0; --c)
		{
			const int j = LastInCol(c);
			if (j >= 0)
				return j;
		}
		return back >= 0 ? back : LastAny();
	}
	if (dx != 0)
	{
		const int step = dx > 0 ? 1 : -1;
		for (int c = col + step; c >= 0 && c < layout.cCols; c += step)
		{
			int best = -1;
			int bestDistance = 1000;
			for (int r = 0; r < layout.cRows; ++r)
			{
				const int j = Find(c, r);
				if (!Selectable(j))
					continue;
				const int distance = std::abs(r - row);
				if (distance < bestDistance)
				{
					best = j;
					bestDistance = distance;
				}
			}
			if (best >= 0)
				return best;
		}
	}
	return i;
}

// Keeps the selection on a selectable row (never on a section header).
void NormalizeSelection(PROMPT* pprompt, const RESPK* arespk, int crespk)
{
	if (pprompt == nullptr || arespk == nullptr || crespk <= 0)
		return;
	if (pprompt->irespk < 0 || pprompt->irespk >= crespk)
		pprompt->irespk = std::clamp(pprompt->irespk, 0, crespk - 1);
	if (!FHeaderRespk(arespk[pprompt->irespk]))
		return;
	for (int j = pprompt->irespk + 1; j < crespk; ++j)
		if (!FHeaderRespk(arespk[j]))
		{
			pprompt->irespk = j;
			return;
		}
	for (int j = pprompt->irespk - 1; j >= 0; --j)
		if (!FHeaderRespk(arespk[j]))
		{
			pprompt->irespk = j;
			return;
		}
}

// -----------------------------------------------------------------------------
// Tabs: pages of one menu, switched with L1 / R1 (Q / E) or a click.
// -----------------------------------------------------------------------------
struct TABGROUP
{
	const PRK* aprk;
	const char* const* apchz;
	int c;
};

const PRK s_aprkTabsVideo[] = { PRK_VideoMenu, PRK_VideoGraphics, PRK_VideoUpscaling };
const char* const s_apchzTabsVideo[] = { "Display", "Graphics", "Upscaling" };
const PRK s_aprkTabsCustom[] = { PRK_Custom, PRK_CustText, PRK_CustBox, PRK_CustBorder, PRK_CustHighlight, PRK_CustEffects };
const char* const s_apchzTabsCustom[] = { "General", "Text", "Background", "Border", "Highlight", "Effects" };
const PRK s_aprkTabsMods[] = { PRK_ModsMenu, PRK_ModsStates, PRK_ModsTimer, PRK_ModsCombos };
const char* const s_apchzTabsMods[] = { "General", "Save States", "Timer", "Combos" };

const TABGROUP s_atabgroup[] =
{
	{ s_aprkTabsVideo, s_apchzTabsVideo, static_cast<int>(std::size(s_aprkTabsVideo)) },
	{ s_aprkTabsCustom, s_apchzTabsCustom, static_cast<int>(std::size(s_aprkTabsCustom)) },
	{ s_aprkTabsMods, s_apchzTabsMods, static_cast<int>(std::size(s_aprkTabsMods)) },
};

const TABGROUP* PtabgroupFromPrk(PRK prk, int* pitab)
{
	for (const TABGROUP& group : s_atabgroup)
		for (int i = 0; i < group.c; ++i)
			if (group.aprk[i] == prk)
			{
				if (pitab != nullptr)
					*pitab = i;
				return &group;
			}
	return nullptr;
}

constexpr float kTabScale = 0.85f;      // tab labels relative to the response scale
constexpr float kTabLines = 1.45f;      // strip height in tab-label lines

PROMPTHIT s_ahitTab[8] = {};
int s_chitTab = 0;

int ItabFromMouse(PRK prk)
{
	float mx = 0.0f;
	float my = 0.0f;
	if (prk != s_prkHit || g_clock.tReal - s_tHit > 0.25f || !MouseMenuCursor(&mx, &my))
		return -1;
	for (int i = 0; i < s_chitTab; ++i)
	{
		const PROMPTHIT& hit = s_ahitTab[i];
		if (mx >= hit.x0 && mx < hit.x1 && my >= hit.y0 && my < hit.y1)
			return hit.irespk;
	}
	return -1;
}

// Slide animation when switching tabs (and the gliding highlight bar).
int s_dirSlide = 0;
float s_tSlide = -10.0f;
PRK s_prkGlide = PRK_Nil;
float s_aglide[4] = {};       // current highlight rect x0, y0, x1, y1
bool s_fGlideValid = false;

// -----------------------------------------------------------------------------
// Modern rows: "Name: Value" is drawn as name on the left, value on the right.
// -----------------------------------------------------------------------------
bool FSplitLabel(const char* pchz, std::string* pname, std::string* pvalue)
{
	if (pchz == nullptr)
		return false;
	const char* colon = std::strstr(pchz, ": ");
	if (colon == nullptr || colon == pchz)
		return false;
	// The name must be plain text (markup is allowed only in the value).
	for (const char* pch = pchz; pch < colon; ++pch)
		if (*pch == '&' && pch[1] != '&')
			return false;
	if (pname != nullptr)
		pname->assign(pchz, colon);
	if (pvalue != nullptr)
		pvalue->assign(colon + 2);
	return true;
}

// Rows whose value can also be changed with Left/Right.
bool FArrowRow(RESPK respk)
{
	NUMROW num;
	if (FNumericRow(respk, &num))
		return true;
	return (respk >= RESPK_MsOpt0 && respk < RESPK_MsOpt0 + MSOPT_Max) ||
		(respk >= RESPK_MouseOpt0 && respk < RESPK_MouseOpt0 + MOUSEOPT_Max);
}

bool FToggleValue(const std::string& value)
{
	return value == "On" || value == "Off";
}

// Width of a modern row's value widget (toggle pill / slider / text).
float DxModernValue(RESPK respk, const std::string& value, CFontBrx* pfont)
{
	const float lineH = LineHeightFont(pfont);
	const float space = static_cast<float>(pfont->m_dxSpaceUnscaled) * pfont->m_rxScale;
	if (FToggleValue(value))
		return lineH * 1.6f;
	NUMROW num;
	if (FNumericRow(respk, &num))
		return lineH * 2.6f + space + DxPromptText(value.c_str(), pfont);
	float dx = DxPromptText(value.c_str(), pfont);
	if (FArrowRow(respk))
		dx += DxPromptText("<  >", pfont) + space;
	return dx;
}

// Width a row needs in the modern layout (name + gap + value).
float DxModernRow(RESPK respk, const char* pchz, CFontBrx* pfont)
{
	std::string name;
	std::string value;
	const float space = static_cast<float>(pfont->m_dxSpaceUnscaled) * pfont->m_rxScale;
	if (!FSplitLabel(pchz, &name, &value))
		return DxPromptText(pchz, pfont) + space * 4.0f;
	return DxPromptText(name.c_str(), pfont) + space * 6.0f + DxModernValue(respk, value, pfont);
}

// -----------------------------------------------------------------------------
// Changed-setting markers and single-setting reset (Square / Delete / middle click).
// -----------------------------------------------------------------------------
SYSOPT SysoptFromRespk(RESPK respk)
{
	switch (respk)
	{
	case RESPK_FrameRate: return SYSOPT_FrameRate;
	case RESPK_InternalResolution: return SYSOPT_InternalResolution;
	case RESPK_DrawDistance: return SYSOPT_DrawDistance;
	case RESPK_WindowMode: return SYSOPT_WindowMode;
	case RESPK_Vsync: return SYSOPT_Vsync;
	case RESPK_AspectRatio: return SYSOPT_AspectRatio;
	case RESPK_Msaa: return SYSOPT_Msaa;
	case RESPK_Fog: return SYSOPT_Fog;
	case RESPK_GuiScale: return SYSOPT_GuiScale;
	case RESPK_GuiStyle: return SYSOPT_GuiStyle;
	case RESPK_Speakers: return SYSOPT_Stereo;
	case RESPK_DisableControllerWhenUnfocused: return SYSOPT_DisableControllerUnfocused;
	default: return SYSOPT_Max;
	}
}

int ExtraBindingDefault(int k)
{
	return k == 0 ? GLFW_KEY_TAB : (k == 1 ? GLFW_KEY_F9 : GLFW_KEY_F12);
}

// Returns true if the row is a setting with a known default; *pfDefault says
// whether it currently has that default.
bool FRowDefaultInfo(PRK prk, RESPK respk, bool* pfDefault)
{
	bool fDefault = true;
	bool fKnown = true;
	NUMROW num;

	if (respk >= RESPK_MsOpt0 && respk < RESPK_MsOpt0 + MSOPT_Max)
	{
		const MSOPT opt = static_cast<MSOPT>(respk - RESPK_MsOpt0);
		if (opt == MSOPT_Theme)
			fKnown = false;
		else
			fDefault = MenuStyleValue(opt) == MenuStyleDefault(opt);
	}
	else if (respk >= RESPK_MouseOpt0 && respk < RESPK_MouseOpt0 + MOUSEOPT_Max)
	{
		const MOUSEOPT opt = static_cast<MOUSEOPT>(respk - RESPK_MouseOpt0);
		fDefault = MouseOptValue(opt) == MouseOptDefault(opt);
	}
	else if (respk >= RESPK_KeyUp && respk <= RESPK_KeyR3 && (prk == PRK_KeyboardMapping || prk == PRK_ControllerMapping))
	{
		const int button = respk - RESPK_KeyUp;
		fDefault = prk == PRK_ControllerMapping
			? g_gamepadBindings[button] == DefaultGamepadBinding(button)
			: g_keyboardBindings[button] == DefaultKeyboardBinding(button);
	}
	else if (respk >= RESPK_KeyFfHold && respk <= RESPK_KeyScreenshot)
	{
		const int k = respk - RESPK_KeyFfHold;
		fDefault = ExtraBindingKey(k) == ExtraBindingDefault(k);
	}
	else if (SysoptFromRespk(respk) != SYSOPT_Max)
		fDefault = FSystemOptionDefault(SysoptFromRespk(respk));
	else
	{
		switch (respk)
		{
		case RESPK_CameraSensitivity: fDefault = QolCameraSensitivityPercent() == 100; break;
		case RESPK_Fov: fDefault = QolFovPercent() == 0; break;
		case RESPK_Anisotropy: fDefault = FQolAnisotropyDefault(); break;
		case RESPK_PauseOnFocusLoss: fDefault = FQolPauseOnFocusLoss(); break;
		case RESPK_ButtonIcons: fDefault = QolButtonIcons() == 0; break;
		case RESPK_RetailTiming: fDefault = FRetailTiming(); break;
		case RESPK_CutsceneSkipControls: fDefault = !g_fSkipBinocDialogs; break;
		case RESPK_MusicVolume:
		case RESPK_SfxVolume:
		case RESPK_DialogueVolume:
			fKnown = FNumericRow(respk, &num);
			fDefault = num.value == 100;
			break;
		case RESPK_Dlss: fKnown = FDlssSupported(); fDefault = FDlssOptionDefault(0); break;
		case RESPK_DlssPreset: fKnown = FDlssSupported(); fDefault = FDlssOptionDefault(1); break;
		case RESPK_FrameGen: fKnown = FDlssFrameGenAvailable(); fDefault = FDlssOptionDefault(2); break;
		case RESPK_FpsCounter: fDefault = FDlssOptionDefault(3); break;
		case RESPK_Fsr: fDefault = FFsrOptionDefault(0); break;
		case RESPK_FsrSharpness: fDefault = FFsrOptionDefault(1); break;
		case RESPK_Vibration: fDefault = g_pgsCur == nullptr || FVibrationEnabled(g_pgsCur); break;
		case RESPK_CameraInvert: fDefault = g_pgsCur == nullptr || (g_pgsCur->grfgs & 0x1000) == 0; break;
		case RESPK_BinocInvert: fDefault = g_pgsCur == nullptr || (g_pgsCur->grfgs & 0x200) == 0; break;
		case RESPK_TurretInvert: fDefault = g_pgsCur == nullptr || (g_pgsCur->grfgs & 0x400) == 0; break;
		case RESPK_JetpackInvert: fDefault = g_pgsCur == nullptr || (g_pgsCur->grfgs & 0x800) == 0; break;
		case RESPK_ModEnable: fDefault = FModOptionDefault(0); break;
		case RESPK_ModFfSpeed: fDefault = FModOptionDefault(1); break;
		default:
			if (respk >= RESPK_TimerEnable && respk < RESPK_TimerReset)
				fDefault = FModOptionDefault(2 + (respk - RESPK_TimerEnable));
			else
				fKnown = false;
			break;
		}
	}

	if (pfDefault != nullptr)
		*pfDefault = fDefault;
	return fKnown;
}

// Resets one row's setting. Returns false if it has no default.
bool ResetRowToDefault(PRK prk, RESPK respk)
{
	bool fDefault = true;
	if (!FRowDefaultInfo(prk, respk, &fDefault) || fDefault)
		return false;

	if (respk >= RESPK_MsOpt0 && respk < RESPK_MsOpt0 + MSOPT_Max)
	{
		const MSOPT opt = static_cast<MSOPT>(respk - RESPK_MsOpt0);
		SetMenuStyleValue(opt, MenuStyleDefault(opt), false);
	}
	else if (respk >= RESPK_MouseOpt0 && respk < RESPK_MouseOpt0 + MOUSEOPT_Max)
	{
		const MOUSEOPT opt = static_cast<MOUSEOPT>(respk - RESPK_MouseOpt0);
		SetMouseOpt(opt, MouseOptDefault(opt));
	}
	else if (respk >= RESPK_KeyUp && respk <= RESPK_KeyR3)
	{
		const int button = respk - RESPK_KeyUp;
		if (prk == PRK_ControllerMapping)
			g_gamepadBindings[button] = DefaultGamepadBinding(button);
		else
			g_keyboardBindings[button] = DefaultKeyboardBinding(button);
		SaveSystemSettings();
	}
	else if (respk >= RESPK_KeyFfHold && respk <= RESPK_KeyScreenshot)
	{
		const int k = respk - RESPK_KeyFfHold;
		SetExtraBindingKey(k, ExtraBindingDefault(k));
	}
	else if (SysoptFromRespk(respk) != SYSOPT_Max)
		ResetSystemOption(SysoptFromRespk(respk));
	else
	{
		switch (respk)
		{
		case RESPK_CameraSensitivity: SetQolCameraSensitivityPercent(100); break;
		case RESPK_Fov: SetQolFovPercent(0); break;
		case RESPK_Anisotropy: ResetQolAnisotropy(); break;
		case RESPK_PauseOnFocusLoss: if (!FQolPauseOnFocusLoss()) ToggleQolPauseOnFocusLoss(); break;
		case RESPK_ButtonIcons: SetQolButtonIcons(0); break;
		case RESPK_RetailTiming: if (!FRetailTiming()) SetRetailTiming(true); break;
		case RESPK_CutsceneSkipControls: g_fSkipBinocDialogs = false; SaveSystemSettings(); break;
		case RESPK_MusicVolume:
		case RESPK_SfxVolume:
		case RESPK_DialogueVolume: SetNumericRow(respk, 100, false); break;
		case RESPK_Dlss: ResetDlssOption(0); break;
		case RESPK_DlssPreset: ResetDlssOption(1); break;
		case RESPK_FrameGen: ResetDlssOption(2); break;
		case RESPK_FpsCounter: ResetDlssOption(3); break;
		case RESPK_Fsr: ResetFsrOption(0); break;
		case RESPK_FsrSharpness: ResetFsrOption(1); break;
		case RESPK_Vibration: if (g_pgsCur != nullptr) ToggleVibration(g_pgsCur); break;
		case RESPK_CameraInvert: if (g_pgsCur != nullptr) g_pgsCur->grfgs &= ~0x1000; break;
		case RESPK_BinocInvert: if (g_pgsCur != nullptr) g_pgsCur->grfgs &= ~0x200; break;
		case RESPK_TurretInvert: if (g_pgsCur != nullptr) g_pgsCur->grfgs &= ~0x400; break;
		case RESPK_JetpackInvert: if (g_pgsCur != nullptr) g_pgsCur->grfgs &= ~0x800; break;
		case RESPK_ModEnable: ResetModOption(0); break;
		case RESPK_ModFfSpeed: ResetModOption(1); break;
		default:
			if (respk >= RESPK_TimerEnable && respk < RESPK_TimerReset)
				ResetModOption(2 + (respk - RESPK_TimerEnable));
			else
				return false;
			break;
		}
	}
	return true;
}

// -----------------------------------------------------------------------------
// "Keep these display settings?" (window mode / aspect ratio), auto-revert.
// -----------------------------------------------------------------------------
constexpr float kDisplayConfirmSeconds = 10.0f;
struct DISPLAYCONFIRM
{
	bool fPending = false;
	VIDEOSAVESETTINGS before{};
	float tStart = 0.0f;
};
DISPLAYCONFIRM s_displayconfirm;

// A choice row (Theme, Font, Box Style, ...) is changed with Left/Right only
// after it has been selected with Confirm; Confirm / Back leave it again.
struct ROWFOCUS
{
	bool fActive = false;
	PRK prk = PRK_Nil;
	int irespk = -1;
};
ROWFOCUS s_focus;

bool FFocusRow(PRK prk, int irespk)
{
	return s_focus.fActive && s_focus.prk == prk && s_focus.irespk == irespk;
}

bool FFocusableRow(RESPK respk)
{
	if (respk < RESPK_MsOpt0 || respk >= RESPK_MsOpt0 + MSOPT_Max)
		return false;
	NUMROW num;
	const MSOPT opt = static_cast<MSOPT>(respk - RESPK_MsOpt0);
	return !FNumericRow(respk, &num) && !FMenuStyleToggle(opt);
}

void UpdateDisplayConfirmTitle()
{
	const float remaining = kDisplayConfirmSeconds - (g_clock.tReal - s_displayconfirm.tStart);
	std::snprintf(g_achzDisplayConfirm, sizeof(g_achzDisplayConfirm),
		"Keep these display settings? Reverting in %d", (std::max)(0, static_cast<int>(std::ceil(remaining))));
}

void BeginDisplayConfirm(PROMPT* pprompt, const VIDEOSAVESETTINGS& before)
{
	s_displayconfirm.fPending = true;
	s_displayconfirm.before = before;
	s_displayconfirm.tStart = g_clock.tReal;
	UpdateDisplayConfirmTitle();
	SetPrompt(pprompt, PRP_Basic, PRK_DisplayConfirm);
}

void FinishDisplayConfirm(PROMPT* pprompt, bool fKeep)
{
	if (s_displayconfirm.fPending && !fKeep)
		RestoreVideoSettings(s_displayconfirm.before);
	s_displayconfirm.fPending = false;
	SetPrompt(pprompt, PRP_Basic, PRK_VideoMenu);
}

// -----------------------------------------------------------------------------
// Button prompt bar text (follows the last input device).
// -----------------------------------------------------------------------------
std::string KeyLabel(int key)
{
	std::string name = PchzKeyboardKeyName(key);
	if (name.size() == 1)
		name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
	return name;
}

std::string PromptBarText(bool fTabs, bool fAdjust, bool fReset, bool fBack, bool fEdit)
{
	const char* const pchzSelect = fEdit ? "Edit" : "Select";
	std::string text;
	const auto Add = [&](const std::string& item)
		{
			if (!text.empty())
				text += "     ";
			text += item;
		};
	switch (LastInputDevice())
	{
	case INPUTDEV_Gamepad:
		Add(std::string("&2X&. ") + pchzSelect);
		if (fBack)
			Add("&2T&. Back");
		if (fTabs)
			Add("L1/R1 Tabs");
		if (fAdjust)
			Add("&2+&. Adjust");
		if (fReset)
			Add("&2S&. Reset");
		break;
	case INPUTDEV_Mouse:
		Add(std::string("Left Click ") + pchzSelect);
		if (fBack)
			Add("Right Click Back");
		if (fTabs)
			Add("Click a Tab");
		if (fAdjust)
			Add("Wheel Adjust");
		if (fReset)
			Add("Middle Click Reset");
		break;
	default:
		Add(KeyLabel(g_keyboardBindings[BTN_CROSS]) + " " + pchzSelect);
		if (fBack)
			Add("Esc Back");
		if (fTabs)
			Add(KeyLabel(g_keyboardBindings[BTN_L1]) + "/" + KeyLabel(g_keyboardBindings[BTN_R1]) + " Tabs");
		if (fAdjust)
			Add(KeyLabel(g_keyboardBindings[BTN_LEFT]) + "/" + KeyLabel(g_keyboardBindings[BTN_RIGHT]) + " Adjust");
		if (fReset)
			Add("Del Reset");
		break;
	}
	return text;
}

// -----------------------------------------------------------------------------
// Button prompt bar with drawn icons: Xbox / PlayStation / Nintendo face
// buttons (by position), shoulders, D-pad, keyboard keys and mouse buttons.
// Everything is drawn on the ImGui overlay in menu (output) pixels.
// -----------------------------------------------------------------------------
enum BARICON
{
	BARICON_FaceBottom,     // Confirm (Cross / A / B on Nintendo)
	BARICON_FaceRight,      // Back (Circle / B / A on Nintendo)
	BARICON_FaceLeft,       // Reset (Square / X / Y on Nintendo)
	BARICON_ShoulderL,
	BARICON_ShoulderR,
	BARICON_Dpad,
	BARICON_Key,            // keyboard key cap with a label
	BARICON_MouseLeft,
	BARICON_MouseRight,
	BARICON_MouseMiddle,
	BARICON_MouseWheel,
};

struct BARITEM
{
	BARICON aicon[2];
	std::string akey[2];
	int cicon = 1;
	std::string label;
};

int OverlayFontIndex()
{
	const int ttf = MenuStyleTtfFont();
	return ttf >= 0 ? ttf : (CMenuFonts() > 0 ? 0 : -1);
}

float DxBarIcon(BARICON icon, const std::string& key, float size)
{
	switch (icon)
	{
	case BARICON_ShoulderL:
	case BARICON_ShoulderR:
		return size * 1.45f;
	case BARICON_Key:
		return (std::max)(size, DxOverlayText(OverlayFontIndex(), key.c_str(), size * 0.8f) + size * 0.55f);
	case BARICON_MouseLeft:
	case BARICON_MouseRight:
	case BARICON_MouseMiddle:
	case BARICON_MouseWheel:
		return size * 0.72f;
	default:
		return size;
	}
}

void DrawCenteredOverlayText(const char* pchz, float cx, float cy, float size, const glm::vec4& color)
{
	const int ifont = OverlayFontIndex();
	const float w = DxOverlayText(ifont, pchz, size);
	DrawOverlayText(ifont, pchz, cx - w * 0.5f, cy - size * 0.5f, size, color);
}

void DrawBarIcon(BARICON icon, const std::string& key, float x, float y, float size, float alpha, const glm::vec4& accent)
{
	const glm::vec4 body(0.12f, 0.13f, 0.16f, 0.95f * alpha);
	const glm::vec4 edge(0.55f, 0.58f, 0.64f, 0.9f * alpha);
	const glm::vec4 white(1.0f, 1.0f, 1.0f, alpha);
	const float line = (std::max)(1.0f, size * 0.07f);
	const float r = size * 0.5f;
	const float cx = x + r;
	const float cy = y + r;
	const GAMEPADTYPE type = ActiveGamepadType();

	switch (icon)
	{
	case BARICON_FaceBottom:
	case BARICON_FaceRight:
	case BARICON_FaceLeft:
	{
		OverlayCircle(cx, cy, r, body, 0.0f);
		OverlayCircle(cx, cy, r - line * 0.5f, edge, line);
		const int position = icon == BARICON_FaceBottom ? 0 : (icon == BARICON_FaceRight ? 1 : 2);
		if (type == GAMEPADTYPE_PlayStation)
		{
			const float s = r * 0.42f;
			const float w = (std::max)(1.5f, size * 0.09f);
			if (position == 0)          // Cross
			{
				const glm::vec4 c(0.50f, 0.66f, 1.0f, alpha);
				OverlayLine(cx - s, cy - s, cx + s, cy + s, c, w);
				OverlayLine(cx - s, cy + s, cx + s, cy - s, c, w);
			}
			else if (position == 1)     // Circle
				OverlayCircle(cx, cy, s * 1.05f, glm::vec4(1.0f, 0.40f, 0.47f, alpha), w);
			else                        // Square
				OverlayRect(cx - s, cy - s, cx + s, cy + s, glm::vec4(0.96f, 0.58f, 0.86f, alpha), 0.0f, w);
		}
		else
		{
			const char* letter = nullptr;
			glm::vec4 c = white;
			if (type == GAMEPADTYPE_Nintendo)
				letter = position == 0 ? "B" : (position == 1 ? "A" : "Y");
			else
			{
				letter = position == 0 ? "A" : (position == 1 ? "B" : "X");
				c = position == 0 ? glm::vec4(0.45f, 0.82f, 0.28f, alpha)
					: (position == 1 ? glm::vec4(0.95f, 0.33f, 0.30f, alpha) : glm::vec4(0.30f, 0.58f, 1.0f, alpha));
			}
			DrawCenteredOverlayText(letter, cx, cy, size * 0.95f, c);
		}
		break;
	}
	case BARICON_ShoulderL:
	case BARICON_ShoulderR:
	{
		const float w = DxBarIcon(icon, key, size);
		OverlayRect(x, y + size * 0.12f, x + w, y + size * 0.88f, body, size * 0.22f, 0.0f);
		OverlayRect(x, y + size * 0.12f, x + w, y + size * 0.88f, edge, size * 0.22f, line);
		const bool fLeft = icon == BARICON_ShoulderL;
		const char* text = type == GAMEPADTYPE_PlayStation ? (fLeft ? "L1" : "R1")
			: (type == GAMEPADTYPE_Nintendo ? (fLeft ? "L" : "R") : (fLeft ? "LB" : "RB"));
		DrawCenteredOverlayText(text, x + w * 0.5f, cy, size * 0.72f, white);
		break;
	}
	case BARICON_Dpad:
	{
		const float arm = size * 0.30f;
		const glm::vec4 c(0.85f, 0.87f, 0.92f, alpha);
		OverlayRect(cx - arm * 0.5f, y + size * 0.08f, cx + arm * 0.5f, y + size * 0.92f, c, size * 0.06f, 0.0f);
		OverlayRect(x + size * 0.08f, cy - arm * 0.5f, x + size * 0.92f, cy + arm * 0.5f, c, size * 0.06f, 0.0f);
		break;
	}
	case BARICON_Key:
	{
		const float w = DxBarIcon(icon, key, size);
		OverlayRect(x, y + size * 0.04f, x + w, y + size * 0.96f, body, size * 0.16f, 0.0f);
		OverlayRect(x, y + size * 0.04f, x + w, y + size * 0.96f, edge, size * 0.16f, line);
		DrawCenteredOverlayText(key.c_str(), x + w * 0.5f, cy, size * 0.8f, white);
		break;
	}
	case BARICON_MouseLeft:
	case BARICON_MouseRight:
	case BARICON_MouseMiddle:
	case BARICON_MouseWheel:
	{
		const float w = DxBarIcon(icon, key, size);
		const float x1 = x + w;
		const float splitY = y + size * 0.42f;
		const float midX = x + w * 0.5f;
		if (icon == BARICON_MouseLeft)
			OverlayRect(x + line, y + line, midX, splitY, accent, size * 0.25f, 0.0f);
		else if (icon == BARICON_MouseRight)
			OverlayRect(midX, y + line, x1 - line, splitY, accent, size * 0.25f, 0.0f);
		OverlayRect(x, y, x1, y + size, edge, w * 0.48f, line);
		OverlayLine(x, splitY, x1, splitY, edge, line);
		OverlayLine(midX, y, midX, splitY, edge, line);
		if (icon == BARICON_MouseMiddle || icon == BARICON_MouseWheel)
			OverlayRect(midX - w * 0.1f, y + size * 0.1f, midX + w * 0.1f, y + size * 0.34f, accent, w * 0.08f, 0.0f);
		break;
	}
	}
}

// What can be pressed right now, for the device used last.
std::vector<BARITEM> BuildPromptBarItems(bool fTabs, bool fFocused, bool fReset, bool fEdit, bool fErase)
{
	std::vector<BARITEM> items;
	const auto Add = [&](BARICON icon, const std::string& key, const char* label)
		{
			BARITEM item;
			item.aicon[0] = icon;
			item.akey[0] = key;
			item.label = label;
			items.push_back(item);
		};
	const auto Add2 = [&](BARICON icon0, const std::string& key0, BARICON icon1, const std::string& key1, const char* label)
		{
			BARITEM item;
			item.aicon[0] = icon0;
			item.akey[0] = key0;
			item.aicon[1] = icon1;
			item.akey[1] = key1;
			item.cicon = 2;
			item.label = label;
			items.push_back(item);
		};
	const char* const pchzSelect = fFocused ? "Done" : (fEdit ? "Edit" : "Select");

	// Keyboard / mouse: show whatever the player bound to each action
	// (a key name, or a mouse icon when it is bound to a mouse button).
	const auto IconForBinding = [](int binding, BARICON& icon, std::string& key)
		{
			key.clear();
			if (binding == MOUSE_BINDING_BASE + GLFW_MOUSE_BUTTON_LEFT)
				icon = BARICON_MouseLeft;
			else if (binding == MOUSE_BINDING_BASE + GLFW_MOUSE_BUTTON_RIGHT)
				icon = BARICON_MouseRight;
			else if (binding == MOUSE_BINDING_BASE + GLFW_MOUSE_BUTTON_MIDDLE)
				icon = BARICON_MouseMiddle;
			else
			{
				icon = BARICON_Key;
				key = KeyLabel(binding);
			}
		};
	const auto FBound = [](int binding) { return binding != GLFW_KEY_UNKNOWN; };
	const auto AddBinding = [&](int binding, const char* fallbackKey, const char* label)
		{
			BARICON icon = BARICON_Key;
			std::string key = fallbackKey != nullptr ? fallbackKey : "";
			if (FBound(binding))
				IconForBinding(binding, icon, key);
			else if (fallbackKey == nullptr)
				return;
			Add(icon, key, label);
		};
	const auto AddBinding2 = [&](int binding0, int binding1, const char* label)
		{
			if (!FBound(binding0) || !FBound(binding1))
				return;
			BARICON icon0 = BARICON_Key, icon1 = BARICON_Key;
			std::string key0, key1;
			IconForBinding(binding0, icon0, key0);
			IconForBinding(binding1, icon1, key1);
			Add2(icon0, key0, icon1, key1, label);
		};

	if (LastInputDevice() == INPUTDEV_Gamepad)
	{
		if (fFocused)
			Add(BARICON_Dpad, "", "Change");
		Add(BARICON_FaceBottom, "", pchzSelect);
		Add(BARICON_FaceRight, "", "Back");
		if (fTabs && !fFocused)
			Add2(BARICON_ShoulderL, "", BARICON_ShoulderR, "", "Tabs");
		if (fReset && !fFocused)
			Add(BARICON_FaceLeft, "", "Reset");
		if (fErase)
			Add(BARICON_FaceLeft, "", "Erase Game");
	}
	else
	{
		const bool fMouse = LastInputDevice() == INPUTDEV_Mouse;
		if (fFocused)
		{
			if (fMouse)
				Add(BARICON_MouseWheel, "", "Change");
			else
				AddBinding2(g_keyboardBindings[BTN_LEFT], g_keyboardBindings[BTN_RIGHT], "Change");
		}
		AddBinding(g_keyboardBindings[BTN_CROSS], nullptr, pchzSelect);
		// Escape always goes back too, so it stands in when Circle is unbound.
		AddBinding(g_keyboardBindings[BTN_CIRCLE], "Esc", "Back");
		if (fTabs && !fFocused)
			AddBinding2(g_keyboardBindings[BTN_L1], g_keyboardBindings[BTN_R1], "Tabs");
		if (fReset && !fFocused)
		{
			if (fMouse)
				Add(BARICON_MouseMiddle, "", "Reset");
			else
				Add(BARICON_Key, "Del", "Reset");
		}
		if (fErase)
		{
			if (fMouse)
				Add(BARICON_MouseMiddle, "", "Erase Game");
			else
				Add(BARICON_Key, "Del", "Erase Game");
		}
	}
	return items;
}

// Draws the bar centered at the bottom of the screen.
void DrawPromptBar(const std::vector<BARITEM>& items, float size, float alpha, const glm::vec4& textColor, const glm::vec4& accent)
{
	if (items.empty())
		return;
	const int ifont = OverlayFontIndex();
	const float gapIcon = size * 0.22f;
	const float gapLabel = size * 0.35f;
	const float gapItem = size * 1.1f;
	const float textSize = size * 0.95f;

	float total = 0.0f;
	for (size_t i = 0; i < items.size(); ++i)
	{
		const BARITEM& item = items[i];
		for (int k = 0; k < item.cicon; ++k)
			total += DxBarIcon(item.aicon[k], item.akey[k], size) + (k > 0 ? gapIcon : 0.0f);
		total += gapLabel + DxOverlayText(ifont, item.label.c_str(), textSize);
		if (i + 1 < items.size())
			total += gapItem;
	}

	const float screenW = static_cast<float>(g_gl.width);
	const float screenH = static_cast<float>(g_gl.height);
	const float padX = size * 0.8f;
	const float padY = size * 0.35f;
	const float y1 = screenH - size * 0.7f;
	const float y0 = y1 - size - padY * 2.0f;
	float x = (screenW - total) * 0.5f;
	OverlayRect(x - padX, y0, x + total + padX, y1, glm::vec4(0.0f, 0.0f, 0.0f, 0.6f * alpha), size * 0.5f, 0.0f);

	const float iy = y0 + padY;
	for (size_t i = 0; i < items.size(); ++i)
	{
		const BARITEM& item = items[i];
		for (int k = 0; k < item.cicon; ++k)
		{
			if (k > 0)
				x += gapIcon;
			DrawBarIcon(item.aicon[k], item.akey[k], x, iy, size, alpha, accent);
			x += DxBarIcon(item.aicon[k], item.akey[k], size);
		}
		x += gapLabel;
		DrawOverlayText(ifont, item.label.c_str(), x, iy + (size - textSize) * 0.5f, textSize, textColor);
		x += DxOverlayText(ifont, item.label.c_str(), textSize) + gapItem;
	}
}

// Delete key (edge) for "reset this setting".
bool FDeleteKeyPressed()
{
	static bool s_fWasDown = true;
	const bool fDown = g_gl.window != nullptr && glfwGetKey(g_gl.window, GLFW_KEY_DELETE) == GLFW_PRESS;
	const bool fPressed = fDown && !s_fWasDown;
	s_fWasDown = fDown;
	return fPressed;
}

bool FMiddleClickPressed()
{
	static bool s_fWasDown = true;
	const bool fDown = FMouseMenuEnabled() && FMouseBindingHeld(MOUSE_BINDING_BASE + GLFW_MOUSE_BUTTON_MIDDLE);
	const bool fPressed = fDown && !s_fWasDown;
	s_fWasDown = fDown;
	return fPressed;
}

} // namespace

void SetPromptPrk(PROMPT* pprompt)
{
    ApplyMenuStyleFont(pprompt);
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

	// Video tabs: leave out rows that can't apply right now.
	else if (prk == PRK_VideoMenu || prk == PRK_VideoGraphics || prk == PRK_VideoUpscaling)
	{
		const bool fUpscaler = FDlssActive() || FFsrActive();
		const auto Include = [&](RESPK response)
			{
				switch (response)
				{
				case RESPK_InternalResolution: return !fUpscaler;                          // DLSS / FSR pick the render size
				case RESPK_Msaa: return !FDlssActive() && !FFsrTemporal();                  // replaced by DLSS / FSR anti-aliasing
				case RESPK_Dlss: return FDlssSupported();                                   // NVIDIA RTX only
				case RESPK_DlssPreset: return FDlssSupported() && DlssQuality() != DLSSQ_Off;
				case RESPK_FrameGen: return FDlssFrameGenAvailable();
				case RESPK_FsrSharpness: return FFsrActive();
				default: return true;
				}
			};
		const auto Build = [&](PRK prkTab, const RESPK* arespkAll, int cAll, RESPK* arespk)
			{
				PRD& prd = s_mpprkprd[prkTab];
				prd.arespk = arespk;
				prd.crespk = 0;
				for (int i = 0; i < cAll; ++i)
					if (Include(arespkAll[i]))
						arespk[prd.crespk++] = arespkAll[i];
			};
		if (prk == PRK_VideoMenu)
			Build(PRK_VideoMenu, s_arespkVideoDisplayAll, static_cast<int>(std::size(s_arespkVideoDisplayAll)), s_arespkVideoDisplay);
		else if (prk == PRK_VideoGraphics)
			Build(PRK_VideoGraphics, s_arespkVideoGraphicsAll, static_cast<int>(std::size(s_arespkVideoGraphicsAll)), s_arespkVideoGraphics);
		else
			Build(PRK_VideoUpscaling, s_arespkVideoUpscalingAll, static_cast<int>(std::size(s_arespkVideoUpscalingAll)), s_arespkVideoUpscaling);
	}

	else if (prk == PRK_Backups)
	{
		PRD& prd = s_mpprkprd[PRK_Backups];
		prd.arespk = s_arespkBackupsMenu;
		prd.crespk = 0;
		prd.arespk[prd.crespk++] = RESPK_ExportNow;
		const int cBackups = ListSettingsBackups(g_achzBackup, 8);
		for (int i = 0; i < cBackups; ++i)
			prd.arespk[prd.crespk++] = static_cast<RESPK>(RESPK_Backup0 + i);
		prd.arespk[prd.crespk++] = RESPK_Back;
	}

	else if (prk == PRK_ControllerSettings)
	{
		PRD& prd = s_mpprkprd[PRK_ControllerSettings];
		prd.arespk = s_arespkControllerMenu;
		prd.crespk = 0;
		const bool gamepadAvailable = FGamepadAvailable();
		for (RESPK response : s_arespkControllerMenuAll)
		{
			if (response == RESPK_ControllerMapping && !gamepadAvailable)
				continue;
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
            std::snprintf(slotStrings[slot], 64, "%s", choosingSlot ? "New Game" : "Empty");
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
			"%s: %s", s_apchzKeyboardActions[button], bindingName);
	}
	for (int k = 0; k < kExtraBindings; ++k)
	{
		std::snprintf(g_aachzExtraBindings[k], sizeof(g_aachzExtraBindings[k]),
			"%s: %s", s_apchzExtraActions[k], PchzKeyboardKeyName(ExtraBindingKey(k)));
	}
    if (FDlssSupported())
        std::snprintf(g_achzRespk66, sizeof(g_achzRespk66), "NVIDIA DLSS: %s", PchzDlssQuality(DlssQuality()));
    else
        std::snprintf(g_achzRespk66, sizeof(g_achzRespk66), "NVIDIA DLSS: Unavailable");
    RefreshModsMenuStrings();
    RefreshMenuStyleStrings();
    RefreshMouseStrings();
    std::snprintf(g_achzRespk80, sizeof(g_achzRespk80), "Frame Generation: %s", PchzDlssFrameGen());
    std::snprintf(g_achzRespk81, sizeof(g_achzRespk81), "FPS Counter: %s", PchzDlssFpsCounter());
    std::snprintf(g_achzRespk82, sizeof(g_achzRespk82), "AMD %s: %s", PchzFsrVersion(), PchzFsrQuality());
    std::snprintf(g_achzRespk83, sizeof(g_achzRespk83), "FSR Sharpness: %s", PchzFsrSharpness());
    std::snprintf(g_achzRespk84, sizeof(g_achzRespk84), "Field of View: %s", PchzQolFov());
    std::snprintf(g_achzRespk85, sizeof(g_achzRespk85), "Anisotropic Filtering: %s", PchzQolAnisotropy());
    std::snprintf(g_achzRespk86, sizeof(g_achzRespk86), "Camera Sensitivity: %s", PchzQolCameraSensitivity());
    std::snprintf(g_achzRespk99, sizeof(g_achzRespk99), "Pause When Unfocused: %s", FQolPauseOnFocusLoss() ? "On" : "Off");
    std::snprintf(g_achzRespk67, sizeof(g_achzRespk67), "DLSS Model: %s",
        FDlssSupported() ? PchzDlssPreset() : "N/A");
    if (FDlssActive())
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: Off (DLSS)");
    else if (FFsrTemporal())
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: Off (FSR)");
    else if (g_fMsaa)
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: %dx", g_msaaSamples);
    else
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: Off");
    if (FRetailTiming())
        std::snprintf(g_achzRespk28, sizeof(g_achzRespk28), "Frame Rate: 59.94 (Retail)");
    else if (g_targetFrameRate <= 0)
        std::snprintf(g_achzRespk28, sizeof(g_achzRespk28), "Frame Rate: Uncapped");
    else
        std::snprintf(g_achzRespk28, sizeof(g_achzRespk28), "Frame Rate: %d FPS", g_targetFrameRate);
    if (FDlssActive())
    {
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30),
            "Resolution: DLSS (%d x %d)", g_gl.renderWidth, g_gl.renderHeight);
    }
    else if (FFsrActive())
    {
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30),
            "Resolution: FSR (%d x %d)", g_gl.renderWidth, g_gl.renderHeight);
    }
    else if (g_internalResolutionHeight == 0)
    {
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30),
            "Resolution: Native (%d x %d)", g_gl.renderWidth, g_gl.renderHeight);
    }
    else
    {
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30),
            "Resolution: %d x %d", g_gl.renderWidth, g_gl.renderHeight);
    }

    const char* fogName = g_fogType == 2 ? "PS3 Style" : "PS2 Style";
    std::snprintf(g_achzRespk31, sizeof(g_achzRespk31), "Fog: %s", fogName);

    const float drawDistance = g_drawDistanceMultiplier;
    const char* drawDistanceName = nullptr;
    if (drawDistance > 0.749f && drawDistance < 0.751f)
        drawDistanceName = "Low";
    else if (drawDistance > 0.999f && drawDistance < 1.001f)
        drawDistanceName = "Original";
    else if (drawDistance > 1.499f && drawDistance < 1.501f)
        drawDistanceName = "High";
    else if (drawDistance > 1.999f && drawDistance < 2.001f)
        drawDistanceName = "Very High";
    else if (drawDistance > 2.999f && drawDistance < 3.001f)
        drawDistanceName = "Extreme";

    if (drawDistanceName != nullptr)
        std::snprintf(g_achzRespk32, sizeof(g_achzRespk32), "Draw Distance: %s", drawDistanceName);
    else
        std::snprintf(g_achzRespk32, sizeof(g_achzRespk32), "Draw Distance: Custom (%.2fx)", drawDistance);

    constexpr const char* windowModeNames[] = { "Windowed", "Borderless", "Fullscreen" };
    const int windowModeIndex = (std::clamp)(static_cast<int>(g_windowMode), 0, 2);
    std::snprintf(g_achzRespk33, sizeof(g_achzRespk33), "Window Mode: %s",
        windowModeNames[windowModeIndex]);
    std::snprintf(g_achzRespk34, sizeof(g_achzRespk34), "VSync: %s",
        g_fVsync ? "On" : "Off");
    const char* aspectRatioName = "Fit to Screen";
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
    std::snprintf(g_achzRespk35, sizeof(g_achzRespk35), "Aspect Ratio: %s", aspectRatioName);
    std::snprintf(g_achzRespk40, sizeof(g_achzRespk40), "GUI Scale: %d%%",
        static_cast<int>(std::lround(g_guiScale * 100.0f)));
    std::snprintf(g_achzRespk60, sizeof(g_achzRespk60), "GUI Style: %s",
        g_guiStyle == GuiStyle_Modern ? "Modern" : "PS2");
	std::snprintf(g_achzRespk61, sizeof(g_achzRespk61),
        "Disable Controller Input When Unfocused: %s",
        g_fDisableControllerInputWhenUnfocused ? "On" : "Off");
    std::snprintf(g_achzButtonIcons, sizeof(g_achzButtonIcons), "Button Icons: %s", PchzQolButtonIcons());
	std::snprintf(g_achzCutsceneSkip, sizeof(g_achzCutsceneSkip), "Dialog Skip Controls: %s",
		g_fSkipBinocDialogs ? "On" : "Off");
	std::snprintf(g_achzRetailTiming, sizeof(g_achzRetailTiming), "Retail Timing: %s",
		FRetailTiming() ? "On" : "Off");

    // -------------------------------------------------------------------------
    // Choose initial response
    // -------------------------------------------------------------------------

    pprompt->irespk = 0;

    switch (prk)
    {
        case PRK_QuitConfirm:
		case PRK_ResetConfirm:
		case PRK_ImportConfirm:
		case PRK_DisplayConfirm:
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

        default:
        break;
    }

    // -------------------------------------------------------------------------
    // Measure prompt
    // -------------------------------------------------------------------------

    PRD& prd = PrdPrompt(prk);
	RESPK* const promptResponses = ArespkPrompt(prk, prd);
	const int promptResponseCount = CrespkPrompt(prk, prd);
	NormalizeSelection(pprompt, promptResponses, promptResponseCount);

    float titleWidth = 0.0f;
    float titleHeight = 0.0f;
    float responseWidth = 0.0f;

    if (prd.pchz != nullptr)
    {
        pprompt->pfont->PushScaling(TitleBaseScale(prd), TitleBaseScale(prd));
        // GetExtents word-wraps by writing '\n' into the string, and titles are
        // string literals (read-only), so measure a writable copy.
        std::string titleCopy(prd.pchz);
        pprompt->pfont->GetExtents(titleCopy.data(), &titleWidth, &titleHeight, 500.0f);
        if (FMenuFontUsable(MenuStyleTtfFont(), prd.pchz))
            titleWidth = DxPromptText(prd.pchz, pprompt->pfont);
        pprompt->pfont->PopScaling();

        titleWidth = std::max(titleWidth, 0.0f);
    }

    // Tab strip under the title (Video / Customization / Mods pages).
    if (const TABGROUP* ptg = PtabgroupFromPrk(prk, nullptr))
    {
        pprompt->pfont->PushScaling(RespkBaseScale(prd) * kTabScale, RespkBaseScale(prd) * kTabScale);
        const float space = static_cast<float>(pprompt->pfont->m_dxSpaceUnscaled) * pprompt->pfont->m_rxScale;
        float widestTab = 0.0f;
        for (int t = 0; t < ptg->c; ++t)
            widestTab = std::max(widestTab, DxPromptText(ptg->apchz[t], pprompt->pfont));
        titleWidth = std::max(titleWidth, (widestTab + space * 4.0f) * static_cast<float>(ptg->c));
        titleHeight += LineHeightFont(pprompt->pfont) * kTabLines;
        pprompt->pfont->PopScaling();
    }

    pprompt->pfont->PushScaling(RespkBaseScale(prd), RespkBaseScale(prd));

    const float lineHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    const bool isBindingMenu = FMappingPrompt(prk);
    const int visibleResponseCount = isBindingMenu ? std::min(promptResponseCount, 9) : promptResponseCount;
    float responseHeight = prd.fVertical != 0 ? lineHeight * static_cast<float>(visibleResponseCount) : lineHeight;

    if (promptResponseCount == 0)
        responseHeight = prd.fVertical != 0 ? 0.0f : lineHeight;

    // Modern layout rows need room for the name, a gap and the value widget.
    const bool fModernMeasure = FMenuStyleModern() && prd.fVertical != 0;

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
            itemWidth = fModernMeasure
                ? DxModernRow(response, AchzFromRespk(response), pprompt->pfont)
                : DxPromptText(AchzFromRespk(response), pprompt->pfont);
        }
        else
        {
            for (int variant = 0; variant < responseData.cpchz; ++variant)
            {
                if (responseData.apchz[variant] == nullptr)
                    continue;

                itemWidth = std::max(itemWidth, fModernMeasure
                    ? DxModernRow(response, responseData.apchz[variant], pprompt->pfont)
                    : DxPromptText(responseData.apchz[variant], pprompt->pfont));
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

    s_arScaleRespkFit[prk] = 0.0f;
    const PROMPTLAYOUT layout = LayoutPrompt(prk, prd, promptResponses, promptResponseCount);
    if (layout.fTwoColumn)
    {
        // responseWidth is the widest item (vertical menu).
        pprompt->pfont->PushScaling(RespkBaseScale(prd), RespkBaseScale(prd));
        const float gap = static_cast<float>(pprompt->pfont->m_dxSpaceUnscaled) * pprompt->pfont->m_rxScale * 6.0f;
        pprompt->pfont->PopScaling();

        const float cols = static_cast<float>(layout.cCols);
        float width = responseWidth * cols + gap * (cols - 1.0f);
        float rows = static_cast<float>(layout.cRows + (layout.fBack ? 1 : 0));
        float fit = 1.0f;
        const float maxWidth = PromptMaxWidth();
        if (width + 10.0f > maxWidth)
            fit = (maxWidth - 10.0f) / width;
        s_arScaleRespkFit[prk] = RespkBaseScale(prd) * fit;
        responseWidth = width * fit;
        responseHeight = lineHeight * fit * rows;
    }

    ResizeBlot(pprompt, std::max(titleWidth, responseWidth) + 10.0f, titleHeight + responseHeight);

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

// Switches to another tab of the same menu without the close/open animation.
void SwitchPromptTab(PROMPT* pprompt, PRK prk, int dir)
{
    if (pprompt == nullptr)
        return;
    const PRK oldPrk = pprompt->prk;
    pprompt->mpprpprk[PRP_Basic] = prk;
    pprompt->fReshow = 0;
    pprompt->prkReshow = PRK_Nil;
    HandlePromptPrkTransition(pprompt, oldPrk, prk);
    ChangePromptPrk(pprompt, prk);
    SetBlotBlots(pprompt, BLOTS_Visible);
    s_dirSlide = dir;
    s_tSlide = g_clock.tReal;
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

    // Numeric settings open the value editor (type / drag / arrows).
    if (OpenValueEditor(pprompt, response))
    {
        if (FMenuStyleSounds())
            StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        return;
    }

    // Section headers can't be selected.
    if (FHeaderRespk(response))
        return;

    if (pprompt->prk == PRK_DisplayConfirm)
    {
        FinishDisplayConfirm(pprompt, response == RESPK_Yes);
        return;
    }

    // Options > Gameplay
    if (pprompt->prk == PRK_Gameplay)
    {
        switch (response)
        {
        case RESPK_PauseOnFocusLoss: ToggleQolPauseOnFocusLoss(); break;
        case RESPK_Vibration: ToggleVibration(g_pgsCur); break;
        case RESPK_FpsCounter: ToggleDlssFpsCounter(); break;
        case RESPK_GuiStyle: SelectNextGuiStyle(); SaveSystemSettings(); break;
        case RESPK_GuiScale: SelectGuiScale(1); SaveSystemSettings(); break;
        case RESPK_TimerEnable: ModTimerOption(MODTIMEROPT_Enable); RefreshModsMenuStrings(); break;
        case RESPK_CutsceneSkipControls: g_fSkipBinocDialogs = !g_fSkipBinocDialogs; SaveSystemSettings(); break;
        case RESPK_RetailTiming: SetRetailTiming(!FRetailTiming()); break;
        default: return;
        }
        RebuildKeepingSelection(pprompt);
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
        case PRK_VideoGraphics:
        case PRK_VideoUpscaling:
        switch (response)
        {
            case RESPK_Dlss:
            if (!FDlssSupported())
            {
                // Status (e.g. "No DLSS capable (RTX) GPU found") is in the log.
                std::printf("DLSS unavailable: %s\n", PchzDlssStatus());
                playInvalidSelectionSound();
                return;
            }
            SelectNextDlssQuality();
            // DLSS and FSR are alternatives: turning DLSS on turns FSR off.
            if (FDlssActive() && FsrQuality() != FSRQ_Off)
                SetFsrQuality(FSRQ_Off);
            rebuildPromptKeepingSelection();
            return;

            case RESPK_Fsr:
            SelectNextFsrQuality();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_Fov:
            SelectNextQolFov();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_Anisotropy:
            SelectNextQolAnisotropy();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_FsrSharpness:
            if (!FFsrActive())
            {
                playInvalidSelectionSound();
                return;
            }
            SelectNextFsrSharpness();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_FrameGen:
            if (!FDlssFrameGenAvailable())
            {
                playInvalidSelectionSound();
                return;
            }
            SelectNextDlssFrameGen();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_FpsCounter:
            ToggleDlssFpsCounter();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_DlssPreset:
            if (!FDlssSupported())
            {
                playInvalidSelectionSound();
                return;
            }
            SelectNextDlssPreset();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_Msaa:
            {
                constexpr int samples[] = { 0, 2, 4, 8, 16 };
                const int currentSamples = g_fMsaa ? g_msaaSamples : 0;
                auto current = std::find(std::begin(samples), std::end(samples), currentSamples);
                int index = current == std::end(samples)
                    ? 0
                    : static_cast<int>(current - std::begin(samples));
                constexpr int count = static_cast<int>(std::size(samples));
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
            if (FRetailTiming())
            {
                // Retail Timing (Options > Gameplay) locks the game to the PS2's
                // 59.94 Hz update rate so physics and glitches match the console.
                QolShowNotice("Retail Timing is on (Options > Gameplay)", 2.5);
                playInvalidSelectionSound();
                return;
            }
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
            // Ask to keep it; reverts automatically if the screen went black.
            s_displayconfirm.before = CaptureCurrentVideoSettings();
            SelectWindowMode(direction);
            SaveSystemSettings();
            BeginDisplayConfirm(pprompt, s_displayconfirm.before);
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

		case PRK_ResetConfirm:
		SetPrompt(pprompt, PRP_Basic, response == RESPK_Yes ? PRK_ResetExport : PRK_OptionsMenu);
		return;

		case PRK_ResetExport:
		{
			char achzMessage[160];
			std::snprintf(achzMessage, sizeof(achzMessage), "Settings reset to default.");
			if (response == RESPK_Yes)
			{
				char achzFolder[96] = {};
				if (ExportSettingsBackup(achzFolder, sizeof(achzFolder)))
					std::snprintf(achzMessage, sizeof(achzMessage), "Settings saved to %s and reset to default.", achzFolder);
				else
					std::snprintf(achzMessage, sizeof(achzMessage), "Export failed - settings were NOT reset.");
			}
			const bool fReset = std::strncmp(achzMessage, "Export failed", 13) != 0;
			if (fReset)
				ResetAllSettings();
			QolShowNotice(achzMessage, 5.0);
			SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
			return;
		}

		case PRK_Backups:
		if (response == RESPK_ExportNow)
		{
			char achzFolder[96] = {};
			char achzMessage[160];
			if (ExportSettingsBackup(achzFolder, sizeof(achzFolder)))
				std::snprintf(achzMessage, sizeof(achzMessage), "Settings exported to %s", achzFolder);
			else
				std::snprintf(achzMessage, sizeof(achzMessage), "Export failed");
			QolShowNotice(achzMessage, 5.0);
			RebuildKeepingSelection(pprompt);   // the new backup shows up in the list
			return;
		}
		if (response >= RESPK_Backup0 && response < RESPK_Backup0 + 8)
		{
			s_iImportBackup = response - RESPK_Backup0;
			SetPrompt(pprompt, PRP_Basic, PRK_ImportConfirm);
		}
		return;

		case PRK_ImportConfirm:
		if (response == RESPK_Yes && s_iImportBackup >= 0)
		{
			const bool fOk = ImportSettingsBackup(s_iImportBackup);
			if (fOk)
				ReloadAllSettings();
			QolShowNotice(fOk ? "Settings imported." : "Import failed: no settings files in that backup.", 5.0);
		}
		SetPrompt(pprompt, PRP_Basic, PRK_Backups);
		return;

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
            OnModsNewGame();
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
                OnModsNewGame();
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

                case RESPK_GameplayMenu:
                SetPrompt(pprompt, PRP_Basic, PRK_Gameplay);
                return;

                case RESPK_Video:
                SetPrompt(pprompt, PRP_Basic, PRK_VideoMenu);
                return;

                case RESPK_Mods:
                SetPrompt(pprompt, PRP_Basic, PRK_ModsMenu);
                return;

                case RESPK_ResetDefaults:
                SetPrompt(pprompt, PRP_Basic, PRK_ResetConfirm);
                return;

                case RESPK_BackupsMenu:
                SetPrompt(pprompt, PRP_Basic, PRK_Backups);
                return;

                case RESPK_PauseOnFocusLoss:
                ToggleQolPauseOnFocusLoss();
                rebuildPromptKeepingSelection();
                return;

                case RESPK_MenuStyle:
                SetPrompt(pprompt, PRP_Basic, PRK_Custom);
                return;

                default:
                return;
            }
        }

        case PRK_ModsMenu:
        switch (response)
        {
            case RESPK_ModEnable:
            ToggleModsEnabled();
            break;

            case RESPK_ModFfSpeed:
            SelectNextModFastForwardSpeed();
            break;

            case RESPK_ModStatesMenu:
            SetPrompt(pprompt, PRP_Basic, PRK_ModsStates);
            return;

            case RESPK_ModTimerMenu:
            SetPrompt(pprompt, PRP_Basic, PRK_ModsTimer);
            return;

            case RESPK_ModCombosMenu:
            SetPrompt(pprompt, PRP_Basic, PRK_ModsCombos);
            return;

            default:
            return;
        }
        rebuildPromptKeepingSelection();
        return;

        case PRK_Custom:
        case PRK_CustText:
        case PRK_CustBox:
        case PRK_CustBorder:
        case PRK_CustHighlight:
        case PRK_CustEffects:
        if (response >= RESPK_MsOpt0 && response < RESPK_MsOpt0 + MSOPT_Max)
        {
            if (FFocusableRow(response))
            {
                // Select the row; Left/Right then change it.
                s_focus.fActive = true;
                s_focus.prk = pprompt->prk;
                s_focus.irespk = pprompt->irespk;
                if (FMenuStyleSounds())
                    StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                return;
            }
            AdjustMenuStyle(static_cast<MSOPT>(response - RESPK_MsOpt0), +1, true);
            rebuildPromptKeepingSelection();
            return;
        }
        switch (response)
        {
            case RESPK_MsTextMenu: SetPrompt(pprompt, PRP_Basic, PRK_CustText); return;
            case RESPK_MsBoxMenu: SetPrompt(pprompt, PRP_Basic, PRK_CustBox); return;
            case RESPK_MsBorderMenu: SetPrompt(pprompt, PRP_Basic, PRK_CustBorder); return;
            case RESPK_MsHighlightMenu: SetPrompt(pprompt, PRP_Basic, PRK_CustHighlight); return;
            case RESPK_MsEffectsMenu: SetPrompt(pprompt, PRP_Basic, PRK_CustEffects); return;
            case RESPK_MsReset:
            ResetMenuStyle();
            rebuildPromptKeepingSelection();
            return;
            default:
            return;
        }

        case PRK_Mouse:
        if (response >= RESPK_MouseOpt0 && response < RESPK_MouseOpt0 + MOUSEOPT_Max)
        {
            AdjustMouseOpt(static_cast<MOUSEOPT>(response - RESPK_MouseOpt0), +1, true);
            rebuildPromptKeepingSelection();
        }
        return;

        case PRK_ModsTimer:
        if (response >= RESPK_TimerEnable && response <= RESPK_TimerReset)
        {
            ModTimerOption(static_cast<MODTIMEROPT>(response - RESPK_TimerEnable));
            rebuildPromptKeepingSelection();
        }
        return;

        case PRK_ModsStates:
        case PRK_ModsCombos:
        switch (response)
        {
            case RESPK_ModSaveState:
            if (!FModsEnabled())
            {
                playInvalidSelectionSound();
                return;
            }
            FSaveModState(ModStateSlot());
            break;

            case RESPK_ModLoadState:
            if (!FModsEnabled())
            {
                playInvalidSelectionSound();
                return;
            }
            // Close the menu (same as Return to Game), then reload the level.
            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            FLoadModState(ModStateSlot());
            return;

            case RESPK_ModSlot:
            SelectNextModStateSlot();
            break;

            case RESPK_ModBindResetTimer:
            BeginModComboCapture(MODACT_ResetTimer);
            break;

            case RESPK_ModBindSave:
            case RESPK_ModBindLoad:
            case RESPK_ModBindPrevSlot:
            case RESPK_ModBindNextSlot:
            case RESPK_ModBindFfHold:
            case RESPK_ModBindFfToggle:
            BeginModComboCapture(static_cast<MODACT>(response - RESPK_ModBindSave));
            break;

            default:
            return;
        }
        rebuildPromptKeepingSelection();
        return;

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
        case PRK_ControllerSettings:
        switch (response)
        {
			case RESPK_ControllerMenu:
			SetPrompt(pprompt, PRP_Basic, PRK_ControllerSettings);
			return;

			case RESPK_KeyboardMapping:
			s_fControllerBindingCapture = false;
			OpenMappingPrompt(pprompt, PRK_KeyboardMapping);
			return;

			case RESPK_ControllerMapping:
			s_fControllerBindingCapture = true;
			OpenMappingPrompt(pprompt, PRK_ControllerMapping);
			return;

			case RESPK_MouseMenu:
			SetPrompt(pprompt, PRP_Basic, PRK_Mouse);
			return;

			case RESPK_DisableControllerWhenUnfocused:
			g_fDisableControllerInputWhenUnfocused =
				!g_fDisableControllerInputWhenUnfocused;
			SaveSystemSettings();
			break;

			case RESPK_ButtonIcons:
			SelectNextQolButtonIcons();
			break;

			case RESPK_CameraSensitivity:
			SelectNextQolCameraSensitivity();
			break;

			case RESPK_CameraInvert:
			g_pgsCur->grfgs ^= 0x1000;
			break;

			case RESPK_MouseCamera:
			g_fMouseCamera = !g_fMouseCamera;
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
			SetPrompt(pprompt, PRP_Basic, pprompt->prk == PRK_ControllerMapping ? PRK_ControllerSettings : PRK_ControlsMenu);
			return;
		}

		if ((response >= RESPK_KeyUp && response <= RESPK_KeyR3) ||
			(!s_fControllerBindingCapture && response >= RESPK_KeyFfHold && response <= RESPK_KeyScreenshot))
		{
			s_iKeyboardBindingCapture = response <= RESPK_KeyR3
				? static_cast<int>(response) - static_cast<int>(RESPK_KeyUp)
				: BTN_MAX + (static_cast<int>(response) - static_cast<int>(RESPK_KeyFfHold));
			s_fKeyboardBindingCaptureArmed = false;
			std::snprintf(BindingLabel(s_iKeyboardBindingCapture), 64,
				"%s: Press %s", BindingActionName(s_iKeyboardBindingCapture),
				s_fControllerBindingCapture ? "a button" : "a key or mouse button");
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
	if (respk == RESPK_Speakers)
		std::snprintf(g_achzRespk16, sizeof(g_achzRespk16), "Speakers: %s", FUserStereoEnabled() ? "Stereo" : "Mono");
	else if (respk == RESPK_MusicVolume)
		std::snprintf(g_achzRespk63, sizeof(g_achzRespk63), "Music Volume: %d%%", static_cast<int>(GetUserMusicVolume() * 100.0f + 0.5f));
	else if (respk == RESPK_SfxVolume)
		std::snprintf(g_achzRespk64, sizeof(g_achzRespk64), "Sound Effects: %d%%", static_cast<int>(GetUserSfxVolume() * 100.0f + 0.5f));
	else if (respk == RESPK_DialogueVolume)
		std::snprintf(g_achzRespk65, sizeof(g_achzRespk65), "Dialogue: %d%%", static_cast<int>(GetUserDialogueVolume() * 100.0f + 0.5f));
    return s_mprespkachz[respk];
}

void BuildSaveSlotText(GS* pgs, char* pchzDst)
{
    int percentComplete = CalculatePercentCompletion(pgs);
    const char* pchzLevel = PchzFriendlyFromLevelId((pgs->gameWorldCur << 8) | pgs->worldLevelCur);
    int totalSeconds = std::max(static_cast<int>(pgs->dt / 60.0f), 1);

    std::snprintf(pchzDst, 64, "%d%% - %s - %d:%02d", percentComplete, pchzLevel, totalSeconds / 60, totalSeconds % 60);
}

void OnPromptActive(PROMPT* pprompt, int fActive)
{
    if (pprompt->fActive == fActive)
        return;

    const int levelId = (g_pgsCur->gameWorldCur << 8) | g_pgsCur->worldLevelCur;

    if (fActive == 0)
    {
        if (levelId == 0x0308)
        {
            ResumeVag();
            ContinueMusicSequencer();
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

	if (prk == PRK_ModsCombos && FModComboCapturing())
	{
		// Recording a "modifier + button" combo; the menu itself ignores input.
		// Rows of the Button Combos menu are in MODACT order.
		const int selection = pprompt->irespk;
		const int row = 5 + std::clamp(selection, 0, MODACT_Max - 1);
		char before[64];
		std::snprintf(before, sizeof(before), "%s", g_achzModsMenu[row]);
		const bool fDone = UpdateModComboCapture();
		RefreshModsMenuStrings();
		if (fDone || std::strcmp(before, g_achzModsMenu[row]) != 0)
		{
			SetPromptPrk(pprompt);
			pprompt->irespk = selection;
		}
		return;
	}

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
			: KeyboardKeyPressed(g_gl.window);
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
			else if (s_iKeyboardBindingCapture >= BTN_MAX)
				SetExtraBindingKey(s_iKeyboardBindingCapture - BTN_MAX, key);
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
	NormalizeSelection(pprompt, promptResponses, promptResponseCount);

	// "Keep these display settings?" counts down and reverts on its own.
	if (prk == PRK_DisplayConfirm && s_displayconfirm.fPending)
	{
		UpdateDisplayConfirmTitle();
		if (g_clock.tReal - s_displayconfirm.tStart >= kDisplayConfirmSeconds)
		{
			FinishDisplayConfirm(pprompt, false);
			return;
		}
	}

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

    if (s_edit.fActive)
    {
        UpdateValueEditor(pprompt, pjoy);
        return;
    }

    // A selected choice row: Left/Right (or the wheel) change it, Confirm or
    // Back release it. Nothing else reacts while a row is selected.
    if (s_focus.fActive)
    {
        if (s_focus.prk != prk || s_focus.irespk != pprompt->irespk || promptResponses == nullptr ||
            pprompt->irespk < 0 || pprompt->irespk >= promptResponseCount ||
            !FFocusableRow(promptResponses[pprompt->irespk]))
        {
            s_focus.fActive = false;
        }
        else
        {
            const RESPK selected = promptResponses[pprompt->irespk];
            int dir = pjoy->DxSelectionJoy(g_clock.tReal);
            const int wheel = MouseWheelNotches();
            if (dir == 0 && wheel != 0)
                dir = wheel > 0 ? 1 : -1;
            if (dir != 0)
            {
                AdjustMenuStyle(static_cast<MSOPT>(selected - RESPK_MsOpt0), dir, true);
                if (FMenuStyleSounds())
                    StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                RebuildKeepingSelection(pprompt);
                return;
            }

            const bool fEscapeDown = g_gl.window != nullptr && glfwGetKey(g_gl.window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
            const bool fEscapePressed = fEscapeDown && !s_fPromptEscapeWasDown;
            s_fPromptEscapeWasDown = fEscapeDown;
            if (canAcceptInput && (pjoy->IsPressed(BTN_CROSS) || pjoy->IsPressed(BTN_CIRCLE) ||
                pjoy->IsPressed(BTN_TRIANGLE) || pjoy->IsPressed(BTN_START) || fEscapePressed ||
                FMouseLeftPressed() || FMouseRightPressed()))
            {
                ConsumeAllButtons(pjoy);
                s_focus.fActive = false;
                if (FMenuStyleSounds())
                    StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            }
            return;
        }
    }

    // Tabs: L1 / R1 (Q / E on the keyboard by default) or click a tab.
    {
        int itab = 0;
        const TABGROUP* ptg = PtabgroupFromPrk(prk, &itab);
        if (ptg != nullptr && canAcceptInput)
        {
            int target = -1;
            int dir = 0;
            if (pjoy->IsPressed(BTN_L1))
            {
                pjoy->SetHandled(BTN_L1);
                dir = -1;
                target = (itab + ptg->c - 1) % ptg->c;
            }
            else if (pjoy->IsPressed(BTN_R1))
            {
                pjoy->SetHandled(BTN_R1);
                dir = 1;
                target = (itab + 1) % ptg->c;
            }
            else if (FMouseLeftPressed())
            {
                const int itabHit = ItabFromMouse(prk);
                if (itabHit >= 0 && itabHit != itab)
                {
                    target = itabHit;
                    dir = itabHit > itab ? 1 : -1;
                }
            }
            if (target >= 0 && target != itab)
            {
                if (FMenuStyleSounds())
                    StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                ConsumeAllButtons(pjoy);
                SwitchPromptTab(pprompt, ptg->aprk[target], dir);
                return;
            }
        }
    }

    // Delete / middle click: reset a setting, or erase a save in Load Game.
    const bool fDeletePressed = FDeleteKeyPressed();
    const bool fMiddlePressed = FMiddleClickPressed();

    // Reset the highlighted setting to its default: Square / Delete / middle click.
    {
        if (canAcceptInput && promptResponses != nullptr && pprompt->irespk >= 0 && pprompt->irespk < promptResponseCount &&
            (fDeletePressed || fMiddlePressed || pjoy->IsPressed(BTN_SQUARE)))
        {
            const RESPK selected = promptResponses[pprompt->irespk];
            bool fDefault = true;
            if (FRowDefaultInfo(prk, selected, &fDefault) && !fDefault)
            {
                pjoy->SetHandled(BTN_SQUARE);
                const bool fDisplay = selected == RESPK_WindowMode;
                if (fDisplay)
                    s_displayconfirm.before = CaptureCurrentVideoSettings();
                ResetRowToDefault(prk, selected);
                if (FMenuStyleSounds())
                    StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                if (fDisplay)
                {
                    BeginDisplayConfirm(pprompt, s_displayconfirm.before);
                    return;
                }
                RebuildKeepingSelection(pprompt);
                return;
            }
        }
    }

    // Mouse: hover highlights, left click selects, right click goes back and
    // the wheel changes slider values (or moves the highlight elsewhere).
    if (promptResponseCount > 0 && promptResponses != nullptr)
    {
        const int iHover = IrespkFromMouse(prk);
        if (iHover >= 0 && iHover < promptResponseCount && iHover != pprompt->irespk && FMouseMenuMoved())
        {
            pprompt->irespk = iHover;
            if (FMenuStyleSounds())
                StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        }

        if (canAcceptInput && FMouseLeftPressed() && iHover >= 0 && iHover < promptResponseCount)
        {
            pprompt->irespk = iHover;
            ConsumeAllButtons(pjoy);
            ExecutePrompt(pprompt);
            return;
        }

        if (canAcceptInput && FMouseRightPressed())
        {
            ConsumeAllButtons(pjoy);
            CancelPrompt(pprompt);
            return;
        }

        const int wheel = MouseWheelNotches();
        if (wheel != 0 && pprompt->irespk >= 0 && pprompt->irespk < promptResponseCount)
        {
            const int dir = wheel > 0 ? 1 : -1;
            const RESPK selected = promptResponses[pprompt->irespk];
            (void)selected;   // the wheel only moves the highlight (values change after selecting a row)
            {
                // Move the highlight, skipping section headers.
                for (int step = 0; step < promptResponseCount; ++step)
                {
                    pprompt->irespk = (pprompt->irespk - dir + promptResponseCount) % promptResponseCount;
                    if (!FHeaderRespk(promptResponses[pprompt->irespk]))
                        break;
                }
            }
            if (FMenuStyleSounds())
                StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            const int selection = pprompt->irespk;
            SetPromptPrk(pprompt);
            pprompt->irespk = selection;
            return;
        }
    }

    const PROMPTLAYOUT promptLayout = LayoutPrompt(prk, promptData, promptResponses, promptResponseCount);
    if (promptResponseCount > 0 && promptLayout.fTwoColumn)
    {
        const int dySelection = pjoy->DySelectionJoy(g_clock.tReal);
        const int dxSelection = pjoy->DxSelectionJoy(g_clock.tReal);
        const int next = MoveTwoColumnSelection(promptLayout, promptResponses, pprompt->irespk,
            dySelection != 0 ? 0 : dxSelection, dySelection);
        if (next != pprompt->irespk)
        {
            pprompt->irespk = next;
            if (FMenuStyleSounds())
                StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            return;
        }
    }
    else if (promptResponseCount > 0)
    {
        const int selectionDelta = promptData.fVertical != 0 ? pjoy->DySelectionJoy(g_clock.tReal) : pjoy->DxSelectionJoy(g_clock.tReal);

        if (selectionDelta > 0)
        {
            pprompt->irespk = (pprompt->irespk + 1) % promptResponseCount;
            if (FMenuStyleSounds())
                StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            return;
        }

        if (selectionDelta < 0)
        {
            pprompt->irespk = (pprompt->irespk + promptResponseCount - 1) % promptResponseCount;
            if (FMenuStyleSounds())
                StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            return;
        }
    }

    if (!canAcceptInput)
        return;

    // Back: Start / Triangle (PS2), plus Circle (B on Xbox-style pads) and the
    // Escape key, so menus can be left without scrolling down to "Back".
    bool& s_fEscapeWasDown = s_fPromptEscapeWasDown;
    const bool fEscapeDown = g_gl.window != nullptr &&
        glfwGetKey(g_gl.window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    const bool fEscapePressed = fEscapeDown && !s_fEscapeWasDown;
    s_fEscapeWasDown = fEscapeDown;

    if (pjoy->IsPressed(BTN_START) || pjoy->IsPressed(BTN_TRIANGLE) ||
        pjoy->IsPressed(BTN_CIRCLE) || fEscapePressed)
    {
        pjoy->SetHandled(BTN_START);
        pjoy->SetHandled(BTN_TRIANGLE);
        pjoy->SetHandled(BTN_CIRCLE);
        CancelPrompt(pprompt);
        return;
    }

    if (pjoy->IsPressed(BTN_SQUARE) ||
        (prk == PRK_MemcardChooseLoadSlot && (fDeletePressed || fMiddlePressed)))
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

    if (pjoy->IsPressed(BTN_CROSS))
    {
        // Consume the confirmation before ExecutePrompt can close the menu or
        // reload a level. Otherwise JT sees this same press as a jump.
        pjoy->SetHandled(BTN_CROSS);
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

		case PRK_ResetConfirm:
		case PRK_ResetExport:
		case PRK_Backups:
		SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
		break;

		case PRK_ImportConfirm:
		SetPrompt(pprompt, PRP_Basic, PRK_Backups);
		break;

        case PRK_MemcardChooseSaveSlot:
        case PRK_MemcardChooseNewSlot:
        case PRK_MemcardChooseLoadSlot:
        case PRK_ControlsMenu:
        case PRK_VideoMenu:
        case PRK_VideoGraphics:
        case PRK_VideoUpscaling:
        case PRK_Gameplay:
		case PRK_SoundMenu:
		case PRK_ModsMenu:
		case PRK_ModsStates:
		case PRK_ModsTimer:
		case PRK_ModsCombos:
        SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
        break;

		case PRK_DisplayConfirm:
		FinishDisplayConfirm(pprompt, false);
		break;

		case PRK_Custom:
		SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
		break;

		case PRK_Mouse:
		SetPrompt(pprompt, PRP_Basic, PRK_ControlsMenu);
		break;

		case PRK_CustText:
		case PRK_CustBox:
		case PRK_CustBorder:
		case PRK_CustHighlight:
		case PRK_CustEffects:
		SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);   // tabs share one Back
		break;

		case PRK_KeyboardMapping:
		case PRK_ControllerMapping:
		{
			const PRK prkParent = pprompt->prk == PRK_ControllerMapping ? PRK_ControllerSettings : PRK_ControlsMenu;
			s_iKeyboardBindingCapture = -1;
			s_fKeyboardBindingCaptureArmed = false;
			SetPrompt(pprompt, PRP_Basic, prkParent);
			break;
		}

		case PRK_ControllerSettings:
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

    ApplyMenuStyleFont(pprompt);
    // Customization > Effects > Open Animation off: draw at full size at once.
    const float uOn = FMenuStyleOpenAnim() ? pprompt->uOn : 1.0f;
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
    float dx = dxUnscaled * promptScaleX;
    const float dy = dyUnscaled * promptScaleY;
    // RepositionBlot currently centers xOn/yOn in output-pixel space while
    // retaining the retail (unscaled) dx/dy.  Preserve that already-correct
    // output-space center and scale only the offset from it.  Scaling xOn or
    // yOn again would move the prompt toward the lower-right corner.
    float x = centerX + (xUnscaled - centerX) * promptScaleX;
    float y = centerY + (yUnscaled - centerY) * promptScaleY;

    // Customization: box width and screen position.
    {
        const float dxWide = dx * (static_cast<float>(MenuStyleValue(MSOPT_BoxWidth)) / 100.0f);
        x -= (dxWide - dx) * 0.5f;
        dx = dxWide;
        const int position = MenuStyleValue(MSOPT_Position);
        const float margin = 70.0f * promptScaleX;
        if (position == 1)
            x = margin;
        else if (position == 2)
            x = static_cast<float>(g_gl.width) - dx - margin;
    }

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
    // Customization colors (defaults reproduce the retail values above).
    const glm::vec3 textRgb = glm::vec3(static_cast<float>(MenuStyleValue(MSOPT_TextR)),
        static_cast<float>(MenuStyleValue(MSOPT_TextG)), static_cast<float>(MenuStyleValue(MSOPT_TextB))) / 255.0f;
    glm::vec3 titleRgb(127.0f * kGsColorScale);
    switch (MenuStyleValue(MSOPT_TitleColor))
    {
        case 1: titleRgb = textRgb; break;
        case 2: titleRgb = glm::vec3(1.0f, 0.82f, 0.3f); break;
        case 3: titleRgb = glm::vec3(0.4f, 0.7f, 1.0f); break;
        case 4: titleRgb = glm::vec3(1.0f, 0.38f, 0.32f); break;
        default: break;
    }
    glm::vec4 titleColor(MenuStyleGrade(titleRgb), alpha);
    glm::vec4 responseColor(MenuStyleGrade(textRgb), alpha);
    const bool fTextShadow = MenuStyleValue(MSOPT_TextShadow) != 0;
    const float shadowOffset = 1.5f * promptScaleY;
    const auto DrawRich = [&](CRichText& text, CRichText& shadowText, CTextBox& box, const glm::vec4& color)
        {
            // Customization > Text > Font: TrueType text (plain strings only).
            const int ttf = MenuStyleTtfFont();
            if (FMenuFontUsable(ttf, text.m_text.c_str()))
            {
                const char* pchz = text.m_text.c_str();
                const float dyLine = LineHeightFont(pprompt->pfont);
                float tx = box.m_x;
                if (box.m_jh == JH_Center)
                    tx = box.m_x + (box.m_dx - DxMenuFontText(ttf, pchz, dyLine)) * 0.5f;
                if (fTextShadow)
                    DrawMenuFontText(ttf, pchz, tx + shadowOffset, box.m_y + shadowOffset, dyLine,
                        glm::vec4(0.0f, 0.0f, 0.0f, color.a * 0.8f));
                DrawMenuFontText(ttf, pchz, tx, box.m_y, dyLine, color);
                return;
            }
            if (fTextShadow)
            {
                glm::vec4 shadowColor(0.0f, 0.0f, 0.0f, color.a * 0.8f);
                CTextBox shadowBox = box;
                shadowBox.SetPos(box.m_x + shadowOffset, box.m_y + shadowOffset);
                shadowBox.SetTextColor(&shadowColor);
                shadowText.Draw(&shadowBox, nullptr);
            }
            text.Draw(&box, nullptr);
        };

    const auto GetSelectedColor = [&](float pulseValue)
        {
            // Default (222, 222, 94) pulsing toward its brightest channel is the
            // retail "blue rises from 47 to 111" highlight.
            const glm::vec3 base = glm::vec3(static_cast<float>(MenuStyleValue(MSOPT_HlR)),
                static_cast<float>(MenuStyleValue(MSOPT_HlG)), static_cast<float>(MenuStyleValue(MSOPT_HlB))) / 255.0f;
            glm::vec3 rgb = base;
            if (MenuStyleValue(MSOPT_HlPulse) != 0)
            {
                const float brightest = (std::max)(base.r, (std::max)(base.g, base.b));
                rgb = glm::mix(base, glm::vec3(brightest), pulseValue);
            }
            return glm::vec4(MenuStyleGrade(rgb), alpha);
        };

    CTextBox textBox;

    // -------------------------------------------------------------------------
    // Background/edge rectangle
    // -------------------------------------------------------------------------

    // Customization > Effects > Dim Background.
    const int dimPercent = MenuStyleValue(MSOPT_ScreenDim);
    if (dimPercent > 0)
    {
        const float dimAlpha = static_cast<float>(dimPercent) / 100.0f * alpha * std::clamp(pprompt->uOn, 0.0f, 1.0f);
        FillScreenRect(0, 0, 0, static_cast<int>(dimAlpha * 255.0f), 0.0f, 0.0f,
            static_cast<float>(g_gl.width), static_cast<float>(g_gl.height));
    }

    const int boxStyle = MenuStyleValue(MSOPT_BoxStyle);
    const float boxOpacity = static_cast<float>(MenuStyleValue(MSOPT_BoxOpacity)) / 100.0f;
    const bool fBoxRainbow = MenuStyleValue(MSOPT_BoxRainbow) != 0;
    const float boxPadX = 22.0f * promptScaleX;
    const float boxPadY = 14.0f * promptScaleY;
    const float boxX0 = x - boxPadX;
    const float boxY0 = y - boxPadY;
    const float boxX1 = x + dx + boxPadX;
    const float boxY1 = y + dy + boxPadY;
    const auto ToByte = [](float v) { return static_cast<int>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f); };

    if (boxStyle == 0)
    {
        if (pprompt->pte != nullptr)
        {
            textBox.SetPos(x, y);
            textBox.SetSize(dx, dy);
            textBox.SetTextColor(&edgeBoxColor);
            textBox.SetHorizontalJust(JH_Left);
            textBox.SetVerticalJust(JV_Top);

            const glm::vec4 savedEdgeColor = pprompt->pte->m_rgba;

            glm::vec4 edgeColor = savedEdgeColor;
            edgeColor.a = alpha * boxOpacity;
            if (fBoxRainbow)
                edgeColor = glm::vec4(MenuStyleRainbow(0.0f), edgeColor.a);
            edgeColor = glm::vec4(MenuStyleGrade(glm::vec3(edgeColor)), edgeColor.a);

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
    }
    else
    {
        glm::vec3 boxRgb = fBoxRainbow ? MenuStyleRainbow(0.0f) * 0.45f
            : glm::vec3(static_cast<float>(MenuStyleValue(MSOPT_BoxR)), static_cast<float>(MenuStyleValue(MSOPT_BoxG)),
                static_cast<float>(MenuStyleValue(MSOPT_BoxB))) / 255.0f;
        boxRgb = MenuStyleGrade(boxRgb);
        const int boxAlpha = ToByte(boxOpacity * alpha);
        if (boxStyle == 1)
        {
            FillScreenRect(ToByte(boxRgb.r), ToByte(boxRgb.g), ToByte(boxRgb.b), boxAlpha, boxX0, boxY0, boxX1, boxY1);
        }
        else
        {
            // Gradient: full color at the top fading darker toward the bottom.
            constexpr int strips = 24;
            for (int strip = 0; strip < strips; ++strip)
            {
                const float t = static_cast<float>(strip) / static_cast<float>(strips - 1);
                const glm::vec3 c = boxRgb * (1.0f - 0.6f * t);
                const float sy0 = boxY0 + (boxY1 - boxY0) * static_cast<float>(strip) / strips;
                const float sy1 = boxY0 + (boxY1 - boxY0) * static_cast<float>(strip + 1) / strips;
                FillScreenRect(ToByte(c.r), ToByte(c.g), ToByte(c.b), boxAlpha, boxX0, sy0, boxX1, sy1);
            }
        }
    }

    // Customization > Border (any box style).
    if (MenuStyleValue(MSOPT_Border) != 0)
    {
        glm::vec3 borderRgb = MenuStyleValue(MSOPT_BorderRainbow) != 0 ? MenuStyleRainbow(0.5f)
            : glm::vec3(static_cast<float>(MenuStyleValue(MSOPT_BorderR)), static_cast<float>(MenuStyleValue(MSOPT_BorderG)),
                static_cast<float>(MenuStyleValue(MSOPT_BorderB))) / 255.0f;
        borderRgb = MenuStyleGrade(borderRgb);
        float borderAlpha = static_cast<float>(MenuStyleValue(MSOPT_BorderOpacity)) / 100.0f * alpha;
        if (MenuStyleValue(MSOPT_BorderPulse) != 0)
            borderAlpha *= 0.45f + 0.55f * pulse;
        const float t = static_cast<float>(MenuStyleValue(MSOPT_BorderThickness)) * (std::max)(1.0f, static_cast<float>(g_gl.height) / 720.0f);
        const int r = ToByte(borderRgb.r), g = ToByte(borderRgb.g), b = ToByte(borderRgb.b), a = ToByte(borderAlpha);
        FillScreenRect(r, g, b, a, boxX0 - t, boxY0 - t, boxX1 + t, boxY0);      // top
        FillScreenRect(r, g, b, a, boxX0 - t, boxY1, boxX1 + t, boxY1 + t);      // bottom
        FillScreenRect(r, g, b, a, boxX0 - t, boxY0, boxX0, boxY1);              // left
        FillScreenRect(r, g, b, a, boxX1, boxY0, boxX1 + t, boxY1);              // right
    }

    // -------------------------------------------------------------------------
    // Prompt title
    // -------------------------------------------------------------------------

    if (prd.pchz != nullptr)
    {
        const float titleScaleX = TitleBaseScale(prd) * uOn * promptScaleX;
        const float titleScaleY = TitleBaseScale(prd) * uOn * promptScaleY;

        pprompt->pfont->PushScaling(titleScaleX, titleScaleY);

        textBox.SetPos(x, y);
        textBox.SetSize(dx, dy);
        textBox.SetTextColor(&titleColor);
        textBox.SetHorizontalJust(JH_Center);
        textBox.SetVerticalJust(JV_Top);

        CRichText titleText(prd.pchz, pprompt->pfont);
        CRichText titleShadow(prd.pchz, pprompt->pfont);

        // Wrap long titles first (on these copies) so they draw on several lines.
        const float dyTitle = titleText.DyWrap(500.0f * promptScaleX);
        titleShadow.DyWrap(500.0f * promptScaleX);
        DrawRich(titleText, titleShadow, textBox, titleColor);
        y += dyTitle;

        pprompt->pfont->PopScaling();
    }

    // -------------------------------------------------------------------------
    // Tab strip (Video / Customization / Mods pages)
    // -------------------------------------------------------------------------

    const bool fRecordHits = pprompt == &g_prompt;
    if (fRecordHits)
    {
        s_chitPrompt = 0;
        s_chitTab = 0;
        s_prkHit = pprompt->prk;
        s_tHit = g_clock.tReal;
    }

    int itabCur = 0;
    if (const TABGROUP* ptg = PtabgroupFromPrk(pprompt->prk, &itabCur))
    {
        const float tabScale = RespkBaseScale(prd) * kTabScale * uOn;
        pprompt->pfont->PushScaling(tabScale * promptScaleX, tabScale * promptScaleY);
        const float tabLine = LineHeightFont(pprompt->pfont);
        const float stripH = tabLine * kTabLines;
        const float cellW = dx / static_cast<float>(ptg->c);
        const glm::vec4 accent = GetSelectedColor(0.0f);
        const float underline = (std::max)(2.0f, 1.2f * promptScaleY);

        FillScreenRect(255, 255, 255, ToByte(0.05f * alpha), x, y + tabLine * 0.05f, x + dx, y + stripH - tabLine * 0.1f);
        for (int t = 0; t < ptg->c; ++t)
        {
            const float cx0 = x + cellW * static_cast<float>(t);
            const bool fActive = t == itabCur;
            glm::vec4 color = fActive ? accent : glm::vec4(glm::vec3(responseColor), responseColor.a * 0.55f);
            const char* pchzTab = ptg->apchz[t];
            const float w = DxPromptText(pchzTab, pprompt->pfont);
            CRichText tabText(pchzTab, pprompt->pfont);
            CRichText tabShadow(pchzTab, pprompt->pfont);
            textBox.SetPos(cx0 + (cellW - w) * 0.5f, y + (stripH - tabLine) * 0.5f);
            textBox.SetSize(w + 2.0f, tabLine);
            textBox.SetTextColor(&color);
            textBox.SetHorizontalJust(JH_Left);
            textBox.SetVerticalJust(JV_Top);
            DrawRich(tabText, tabShadow, textBox, color);
            if (fActive)
            {
                FillScreenRect(ToByte(accent.r), ToByte(accent.g), ToByte(accent.b), ToByte(alpha),
                    cx0 + cellW * 0.12f, y + stripH - tabLine * 0.1f - underline, cx0 + cellW * 0.88f, y + stripH - tabLine * 0.1f);
            }
            if (fRecordHits && s_chitTab < static_cast<int>(std::size(s_ahitTab)))
                s_ahitTab[s_chitTab++] = PROMPTHIT{ cx0, y, cx0 + cellW, y + stripH, t };
        }
        pprompt->pfont->PopScaling();
        y += stripH;
    }

    // Retail places the divider one PS2-canvas unit above the response row.
    const float dividerY = y - promptScaleY;

    // -------------------------------------------------------------------------
    // Responses
    // -------------------------------------------------------------------------

    const PROMPTLAYOUT layout = LayoutPrompt(pprompt->prk, prd, promptResponses, promptResponseCount);
    const float responseScaleX = ScaleRespkPrompt(pprompt->prk, prd) * uOn * promptScaleX;
    const float responseScaleY = ScaleRespkPrompt(pprompt->prk, prd) * uOn * promptScaleY;

    pprompt->pfont->PushScaling(responseScaleX, responseScaleY);

    const float baseLineHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    const float spaceWidth = static_cast<float>(pprompt->pfont->m_dxSpaceUnscaled) * pprompt->pfont->m_rxScale;
    const glm::vec4 accentColor = GetSelectedColor(0.0f);
    const int cursor = MenuStyleValue(MSOPT_HlCursor);

    // Modern layout: name left, value right (only for vertical menus).
    const bool fModern = FMenuStyleModern() && prd.fVertical != 0;
    bool fSplitMenu = false;
    if (fModern && promptResponses != nullptr)
    {
        for (int i = 0; i < promptResponseCount; ++i)
            fSplitMenu = fSplitMenu || FSplitLabel(AchzFromRespk(promptResponses[i]), nullptr, nullptr);
    }

    // Slide-in after switching tabs.
    float slideX = 0.0f;
    if (s_dirSlide != 0 && FMenuStyleOpenAnim())
    {
        const float u = std::clamp((g_clock.tReal - s_tSlide) / 0.2f, 0.0f, 1.0f);
        const float ease = 1.0f - (1.0f - u) * (1.0f - u) * (1.0f - u);
        slideX = (1.0f - ease) * static_cast<float>(s_dirSlide) * 40.0f * promptScaleX;
        if (u >= 1.0f)
            s_dirSlide = 0;
    }
    else
        s_dirSlide = 0;

    // Draws one line of text with its top-left at (tx, ty).
    const auto DrawTextAt = [&](const char* pchz, float tx, float ty, const glm::vec4& colorText)
        {
            glm::vec4 c = colorText;
            CRichText rich(pchz, pprompt->pfont);
            CRichText shadow(pchz, pprompt->pfont);
            textBox.SetPos(tx, ty);
            textBox.SetSize(DxPromptText(pchz, pprompt->pfont) + 2.0f, LineHeightFont(pprompt->pfont));
            textBox.SetTextColor(&c);
            textBox.SetHorizontalJust(JH_Left);
            textBox.SetVerticalJust(JV_Top);
            DrawRich(rich, shadow, textBox, c);
        };

    // Column cell of a response: [x0, x1] and the row's top.
    const auto CellOf = [&](int i, float* px0, float* py0, float* px1)
        {
            if (layout.fTwoColumn)
            {
                if (layout.fBack && i == layout.cItems)
                {
                    *px0 = x;
                    *px1 = x + dx;
                    *py0 = y + baseLineHeight * static_cast<float>(layout.cRows);
                }
                else
                {
                    const float cols = static_cast<float>((std::max)(layout.cCols, 1));
                    *px0 = x + dx * static_cast<float>(ColumnOf(layout, i)) / cols;
                    *px1 = *px0 + dx / cols;
                    *py0 = y + baseLineHeight * static_cast<float>(RowOf(layout, i));
                }
            }
            else
            {
                *px0 = x;
                *px1 = x + dx;
                *py0 = y + baseLineHeight * static_cast<float>(i);
            }
        };

    float ySelectedItem = y;

    if (prd.fVertical == 0)
    {
        // Horizontal row (Yes / No and similar): centered, as in the original.
        float totalWidth = 0.0f;
        for (int i = 0; i < promptResponseCount; ++i)
        {
            const char* responseText = promptResponses[i] != RESPK_Nil ? AchzFromRespk(promptResponses[i]) : nullptr;
            if (responseText == nullptr)
                continue;
            if (i > 0)
                totalWidth += spaceWidth * 2.0f;
            totalWidth += DxPromptText(responseText, pprompt->pfont);
        }

        float cursorX = x + (dx - totalWidth) * 0.5f;
        for (int i = 0; i < promptResponseCount; ++i)
        {
            const RESPK response = promptResponses[i];
            const char* responseText = response != RESPK_Nil ? AchzFromRespk(response) : nullptr;
            if (responseText == nullptr)
                continue;

            const float unselectedWidth = DxPromptText(responseText, pprompt->pfont);
            const bool selected = i == pprompt->irespk;
            const glm::vec4 color = selected ? GetSelectedColor(pulse) : responseColor;
            const float selectionScale = (selected && MenuStyleValue(MSOPT_HlBounce) != 0) ? 1.0f + (1.0f - pulse) * 0.1f : 1.0f;

            pprompt->pfont->PushScaling(responseScaleX * selectionScale, responseScaleY * selectionScale);
            const float selectedWidth = DxPromptText(responseText, pprompt->pfont);
            const float selectedHeight = LineHeightFont(pprompt->pfont);
            const float drawX = cursorX + (unselectedWidth - selectedWidth) * 0.5f;
            const float drawY = y + (baseLineHeight - selectedHeight) * 0.5f;

            if (fRecordHits && s_chitPrompt < kMaxPromptHits)
                s_ahitPrompt[s_chitPrompt++] = PROMPTHIT{ cursorX - spaceWidth, y, cursorX + unselectedWidth + spaceWidth, y + baseLineHeight, i };
            if (selected)
                ySelectedItem = y;

            if (selected && FMenuStyleModern())
            {
                FillScreenRect(ToByte(accentColor.r), ToByte(accentColor.g), ToByte(accentColor.b), ToByte(0.16f * alpha),
                    cursorX - spaceWidth, y, cursorX + unselectedWidth + spaceWidth, y + baseLineHeight);
            }
            DrawTextAt(responseText, drawX, drawY, color);

            if (selected && cursor != 0)
            {
                const char* markerL = cursor == 3 ? "-" : ">";
                const char* markerR = cursor == 3 ? "-" : "<";
                DrawTextAt(markerL, drawX - DxPromptText(markerL, pprompt->pfont) - spaceWidth, drawY, color);
                if (cursor >= 2)
                    DrawTextAt(markerR, drawX + selectedWidth + spaceWidth, drawY, color);
            }
            pprompt->pfont->PopScaling();

            cursorX += unselectedWidth + spaceWidth * 2.0f;
        }
    }
    else
    {
        for (int i = 0; i < promptResponseCount; ++i)
        {
            const RESPK response = promptResponses[i];
            if (response == RESPK_Nil)
                continue;
            const char* responseText = AchzFromRespk(response);
            if (responseText == nullptr)
                continue;

            float cx0 = 0.0f;
            float cy0 = 0.0f;
            float cx1 = 0.0f;
            CellOf(i, &cx0, &cy0, &cx1);
            cx0 += slideX;
            cx1 += slideX;
            const float cy1 = cy0 + baseLineHeight;
            const float pad = spaceWidth * 1.5f;
            const bool selected = i == pprompt->irespk;
            const bool fBackRow = response == RESPK_Back;

            // Section header: small accent label (+ a thin rule in Modern).
            if (FHeaderRespk(response))
            {
                pprompt->pfont->PushScaling(responseScaleX * 0.8f, responseScaleY * 0.8f);
                const float headerLine = LineHeightFont(pprompt->pfont);
                const float w = DxPromptText(responseText, pprompt->pfont);
                const glm::vec4 headerColor(glm::vec3(accentColor), alpha * 0.85f);
                const float tx = fModern ? cx0 + pad : cx0 + ((cx1 - cx0) - w) * 0.5f;
                const float ty = cy0 + (baseLineHeight - headerLine) * 0.65f;
                DrawTextAt(responseText, tx, ty, headerColor);
                pprompt->pfont->PopScaling();
                if (fModern)
                {
                    const float ruleY = ty + headerLine * 0.55f;
                    const float rule = (std::max)(1.0f, 0.6f * promptScaleY);
                    FillScreenRect(ToByte(accentColor.r), ToByte(accentColor.g), ToByte(accentColor.b), ToByte(0.35f * alpha),
                        tx + w + spaceWidth, ruleY, cx1 - pad, ruleY + rule);
                }
                continue;
            }

            if (fRecordHits && s_chitPrompt < kMaxPromptHits)
                s_ahitPrompt[s_chitPrompt++] = PROMPTHIT{ cx0, cy0, cx1, cy1, i };
            if (selected)
                ySelectedItem = cy0;

            const glm::vec4 color = selected ? GetSelectedColor(pulse) : responseColor;
            bool fAtDefault = true;
            const bool fChanged = FRowDefaultInfo(pprompt->prk, response, &fAtDefault) && !fAtDefault;
            const float dot = baseLineHeight * 0.16f;

            if (fModern)
            {
                // Highlight bar, gliding toward the selected row.
                if (selected)
                {
                    const float target[4] = { cx0 + pad * 0.3f, cy0, cx1 - pad * 0.3f, cy1 };
                    if (!s_fGlideValid || s_prkGlide != pprompt->prk || !FMenuStyleOpenAnim())
                    {
                        for (int k = 0; k < 4; ++k)
                            s_aglide[k] = target[k];
                        s_fGlideValid = true;
                        s_prkGlide = pprompt->prk;
                    }
                    else
                    {
                        const float blend = 1.0f - std::exp(-18.0f * g_clock.dtReal);
                        for (int k = 0; k < 4; ++k)
                            s_aglide[k] += (target[k] - s_aglide[k]) * blend;
                    }
                    const int hr = ToByte(accentColor.r), hg = ToByte(accentColor.g), hb = ToByte(accentColor.b);
                    FillScreenRect(hr, hg, hb, ToByte(0.16f * alpha), s_aglide[0], s_aglide[1], s_aglide[2], s_aglide[3]);
                    FillScreenRect(hr, hg, hb, ToByte(0.9f * alpha), s_aglide[0], s_aglide[1],
                        s_aglide[0] + (std::max)(2.0f, 1.5f * promptScaleX), s_aglide[3]);
                }

                std::string name;
                std::string value;
                const bool fSplit = FSplitLabel(responseText, &name, &value);
                const float nameX = cx0 + pad + dot * 2.5f;
                const float textY = cy0;

                // Changed-from-default marker.
                if (fChanged)
                {
                    const float dy0 = cy0 + (baseLineHeight - dot) * 0.5f;
                    FillScreenRect(ToByte(accentColor.r), ToByte(accentColor.g), ToByte(accentColor.b), ToByte(alpha),
                        cx0 + pad, dy0, cx0 + pad + dot, dy0 + dot);
                }

                if (fBackRow || !fSplitMenu)
                {
                    const float w = DxPromptText(responseText, pprompt->pfont);
                    DrawTextAt(responseText, cx0 + ((cx1 - cx0) - w) * 0.5f, textY, color);
                }
                else if (!fSplit)
                    DrawTextAt(responseText, nameX, textY, color);
                else
                {
                    DrawTextAt(name.c_str(), nameX, textY, color);

                    const float right = cx1 - pad;
                    const float lineH = baseLineHeight;
                    if (FToggleValue(value))
                    {
                        // On / Off switch.
                        const bool fOn = value == "On";
                        const float pw = lineH * 1.5f;
                        const float ph = lineH * 0.5f;
                        const float px0 = right - pw;
                        const float py0 = cy0 + (lineH - ph) * 0.5f;
                        if (fOn)
                            FillScreenRect(ToByte(accentColor.r), ToByte(accentColor.g), ToByte(accentColor.b), ToByte(0.9f * alpha), px0, py0, px0 + pw, py0 + ph);
                        else
                            FillScreenRect(70, 74, 86, ToByte(0.9f * alpha), px0, py0, px0 + pw, py0 + ph);
                        const float knob = ph * 0.8f;
                        const float kx = fOn ? px0 + pw - ph * 0.1f - knob : px0 + ph * 0.1f;
                        FillScreenRect(255, 255, 255, ToByte(alpha), kx, py0 + ph * 0.1f, kx + knob, py0 + ph * 0.1f + knob);
                    }
                    else
                    {
                        NUMROW num;
                        const bool fNumeric = FNumericRow(response, &num);
                        const glm::vec4 dim(glm::vec3(color), color.a * 0.6f);
                        const float vw = DxPromptText(value.c_str(), pprompt->pfont);
                        float vx = right - vw;
                        if (!fNumeric && FFocusRow(pprompt->prk, i))
                        {
                            const float aw = DxPromptText(">", pprompt->pfont);
                            DrawTextAt(">", right - aw, textY, dim);
                            vx = right - aw - spaceWidth - vw;
                            DrawTextAt("<", vx - spaceWidth - DxPromptText("<", pprompt->pfont), textY, dim);
                        }
                        DrawTextAt(value.c_str(), vx, textY, color);

                        if (fNumeric)
                        {
                            // Inline slider to the left of the value.
                            const float bw = lineH * 2.4f;
                            const float bh = (std::max)(2.0f, lineH * 0.1f);
                            const float bx1 = vx - spaceWidth * 1.2f;
                            const float bx0 = bx1 - bw;
                            const float by0 = cy0 + (lineH - bh) * 0.5f;
                            const float range = static_cast<float>((std::max)(num.max - num.min, 1));
                            const float u = std::clamp(static_cast<float>(num.value - num.min) / range, 0.0f, 1.0f);
                            FillScreenRect(90, 94, 108, ToByte(0.8f * alpha), bx0, by0, bx1, by0 + bh);
                            FillScreenRect(ToByte(accentColor.r), ToByte(accentColor.g), ToByte(accentColor.b), ToByte(alpha),
                                bx0, by0, bx0 + bw * u, by0 + bh);
                            const float kx = bx0 + bw * u;
                            const float kw = (std::max)(2.0f, lineH * 0.08f);
                            FillScreenRect(255, 255, 255, ToByte(alpha), kx - kw, by0 - bh * 1.5f, kx + kw, by0 + bh * 2.5f);
                        }
                    }
                }
                continue;
            }

            // Classic: centered in its cell (original look), optional bounce.
            const float unselectedWidth = DxPromptText(responseText, pprompt->pfont);
            const float selectionScale = (selected && MenuStyleValue(MSOPT_HlBounce) != 0) ? 1.0f + (1.0f - pulse) * 0.1f : 1.0f;
            pprompt->pfont->PushScaling(responseScaleX * selectionScale, responseScaleY * selectionScale);
            const float selectedWidth = DxPromptText(responseText, pprompt->pfont);
            const float selectedHeight = LineHeightFont(pprompt->pfont);
            const float center = (cx0 + cx1) * 0.5f;
            const float drawX = center - selectedWidth * 0.5f;
            const float drawY = cy0 + (baseLineHeight - selectedHeight) * 0.5f;

            DrawTextAt(responseText, drawX, drawY, color);

            if (FFocusRow(pprompt->prk, i))
            {
                // Selected choice row: "<  Theme: Midnight  >".
                DrawTextAt("<", drawX - DxPromptText("<", pprompt->pfont) - spaceWidth, drawY, color);
                DrawTextAt(">", drawX + selectedWidth + spaceWidth, drawY, color);
            }
            else if (selected && cursor != 0)
            {
                const char* markerL = cursor == 3 ? "-" : ">";
                const char* markerR = cursor == 3 ? "-" : "<";
                DrawTextAt(markerL, drawX - DxPromptText(markerL, pprompt->pfont) - spaceWidth, drawY, color);
                if (cursor >= 2)
                    DrawTextAt(markerR, drawX + selectedWidth + spaceWidth, drawY, color);
            }
            if (fChanged)
            {
                const float dx0 = drawX + selectedWidth + spaceWidth * (selected && cursor >= 2 ? 2.5f : 0.6f) +
                    (selected && cursor >= 2 ? DxPromptText("<", pprompt->pfont) : 0.0f);
                const float dy0 = cy0 + (baseLineHeight - dot) * 0.5f;
                FillScreenRect(ToByte(accentColor.r), ToByte(accentColor.g), ToByte(accentColor.b), ToByte(alpha),
                    dx0, dy0, dx0 + dot, dy0 + dot);
            }
            pprompt->pfont->PopScaling();
        }
    }

    pprompt->pfont->PopScaling();

    // -------------------------------------------------------------------------
    // Title/response divider
    // -------------------------------------------------------------------------

    if (prd.pchz != nullptr && promptResponseCount > 0 && MenuStyleValue(MSOPT_Divider) != 0)
    {
        const glm::vec4 dividerColor(95.0f * kGsColorScale, 95.0f * kGsColorScale, 95.0f * kGsColorScale, alpha);

        DrawLineScreen(x, dividerY, 0.0f, x + dx, dividerY, 0.0f, dividerColor, false);
    }

    // -------------------------------------------------------------------------
    // Info box: explains the highlighted option, beside the menu.
    // -------------------------------------------------------------------------

    const char* pchzHelp = nullptr;
    const char* pchzHelpLabel = nullptr;
    if (promptResponses != nullptr && pprompt->irespk >= 0 && pprompt->irespk < promptResponseCount && uOn >= 0.98f)
    {
        pchzHelp = PchzRespkHelp(promptResponses[pprompt->irespk]);
        pchzHelpLabel = AchzFromRespk(promptResponses[pprompt->irespk]);
    }

    if (pchzHelp != nullptr && !s_edit.fActive)
    {
        const float helpScale = RespkBaseScale(prd) * 0.72f;
        pprompt->pfont->PushScaling(helpScale * promptScaleX, helpScale * promptScaleY);

        const float pad = 12.0f * promptScaleX;
        const float panelW = 250.0f * promptScaleX;
        const float lineH = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;

        // Header: the option name without its value ("Mouse Camera: On" -> "Mouse Camera").
        std::string header = pchzHelpLabel != nullptr ? pchzHelpLabel : "";
        const size_t colon = header.find(':');
        if (colon != std::string::npos)
            header.resize(colon);
        header = SettingName(promptResponses[pprompt->irespk]);

        int cLines = 1;
        const std::string body = WrapText(pchzHelp, pprompt->pfont, panelW - pad * 2.0f, &cLines);
        const float headerH = header.empty() ? 0.0f : lineH * 1.25f;
        const float panelH = pad * 2.0f + headerH + lineH * static_cast<float>(cLines);

        const float gap = 18.0f * promptScaleX;
        const float screenW = static_cast<float>(g_gl.width);
        const float screenH = static_cast<float>(g_gl.height);
        // Beside the menu (right, else left); if the menu is too wide for
        // either, underneath it (or above when there is no room below).
        float px0 = boxX1 + gap;
        float py0 = ySelectedItem - pad;
        if (px0 + panelW > screenW - 8.0f)
        {
            px0 = boxX0 - gap - panelW;
            if (px0 < 8.0f)
            {
                px0 = (boxX0 + boxX1 - panelW) * 0.5f;
                py0 = boxY1 + gap;
                if (py0 + panelH > screenH - 8.0f)
                    py0 = boxY0 - gap - panelH;
            }
        }
        px0 = std::clamp(px0, 8.0f, (std::max)(8.0f, screenW - panelW - 8.0f));
        py0 = std::clamp(py0, 8.0f, (std::max)(8.0f, screenH - panelH - 8.0f));
        const float px1 = px0 + panelW;
        const float py1 = py0 + panelH;

        // Panel follows the menu's Customization colors.
        glm::vec3 panelRgb(0.06f, 0.07f, 0.11f);
        if (boxStyle != 0)
            panelRgb = MenuStyleGrade(fBoxRainbow ? MenuStyleRainbow(0.0f) * 0.45f
                : glm::vec3(static_cast<float>(MenuStyleValue(MSOPT_BoxR)), static_cast<float>(MenuStyleValue(MSOPT_BoxG)),
                    static_cast<float>(MenuStyleValue(MSOPT_BoxB))) / 255.0f);
        const float panelAlpha = (std::max)(boxOpacity, 0.82f) * alpha;
        FillScreenRect(ToByte(panelRgb.r), ToByte(panelRgb.g), ToByte(panelRgb.b), ToByte(panelAlpha), px0, py0, px1, py1);

        const glm::vec4 accent = GetSelectedColor(0.0f);
        const float edge = (std::max)(1.0f, 2.0f * screenH / 1080.0f);
        const int ar = ToByte(accent.r), ag = ToByte(accent.g), ab = ToByte(accent.b), aa = ToByte(0.75f * alpha);
        FillScreenRect(ar, ag, ab, aa, px0, py0, px1, py0 + edge);
        FillScreenRect(ar, ag, ab, aa, px0, py1 - edge, px1, py1);
        FillScreenRect(ar, ag, ab, aa, px0, py0, px0 + edge, py1);
        FillScreenRect(ar, ag, ab, aa, px1 - edge, py0, px1, py1);

        float ty = py0 + pad;
        if (!header.empty())
        {
            CRichText headerText(header.c_str(), pprompt->pfont);
            CRichText headerShadow(header.c_str(), pprompt->pfont);
            glm::vec4 headerColor = accent;
            textBox.SetPos(px0 + pad, ty);
            textBox.SetSize(panelW - pad * 2.0f, lineH);
            textBox.SetTextColor(&headerColor);
            textBox.SetHorizontalJust(JH_Left);
            textBox.SetVerticalJust(JV_Top);
            DrawRich(headerText, headerShadow, textBox, headerColor);
            ty += headerH;
        }

        CRichText bodyText(body.c_str(), pprompt->pfont);
        CRichText bodyShadow(body.c_str(), pprompt->pfont);
        glm::vec4 bodyColor = responseColor;
        textBox.SetPos(px0 + pad, ty);
        textBox.SetSize(panelW - pad * 2.0f, lineH * static_cast<float>(cLines));
        textBox.SetTextColor(&bodyColor);
        textBox.SetHorizontalJust(JH_Left);
        textBox.SetVerticalJust(JV_Top);
        DrawRich(bodyText, bodyShadow, textBox, bodyColor);

        pprompt->pfont->PopScaling();
    }

    // -------------------------------------------------------------------------
    // Value editor (numeric settings): name, value, slider, hints.
    // -------------------------------------------------------------------------

    if (s_edit.fActive && s_edit.prk == pprompt->prk && pprompt == &g_prompt)
    {
        // Drawn entirely on the ImGui overlay so it sits above everything in
        // the menu (including TrueType menu text, which is also drawn there).
        const float sx = promptScaleX;
        const float sy = promptScaleY;
        const float pad = 14.0f * sx;
        const float panelW = 360.0f * sx;
        const float base = RespkBaseScale(prd);
        const int ifont = MenuStyleTtfFont() >= 0 ? MenuStyleTtfFont() : (CMenuFonts() > 0 ? 0 : -1);

        const auto LineAt = [&](float scale)
            {
                pprompt->pfont->PushScaling(scale * sx, scale * sy);
                const float h = LineHeightFont(pprompt->pfont);
                pprompt->pfont->PopScaling();
                return h;
            };
        const float lineName = LineAt(base * 1.0f);
        const float lineValue = LineAt(base * 1.7f);
        const float lineHint = LineAt(base * 0.7f);
        const float barH = 12.0f * sy;
        const float gapY = 12.0f * sy;
        const float panelH = pad * 2.0f + lineName + lineValue + gapY + barH + gapY * 1.5f + lineHint * 2.0f;

        const float screenW = static_cast<float>(g_gl.width);
        const float screenH = static_cast<float>(g_gl.height);
        const float px0 = std::clamp((boxX0 + boxX1 - panelW) * 0.5f, 8.0f, (std::max)(8.0f, screenW - panelW - 8.0f));
        const float py0 = std::clamp((boxY0 + boxY1 - panelH) * 0.5f, 8.0f, (std::max)(8.0f, screenH - panelH - 8.0f));
        const float px1 = px0 + panelW;
        const float py1 = py0 + panelH;

        glm::vec3 panelRgb(0.06f, 0.07f, 0.11f);
        if (boxStyle != 0)
            panelRgb = MenuStyleGrade(fBoxRainbow ? MenuStyleRainbow(0.0f) * 0.45f
                : glm::vec3(static_cast<float>(MenuStyleValue(MSOPT_BoxR)), static_cast<float>(MenuStyleValue(MSOPT_BoxG)),
                    static_cast<float>(MenuStyleValue(MSOPT_BoxB))) / 255.0f);
        const glm::vec4 accent = GetSelectedColor(0.0f);
        const float edge = (std::max)(2.0f, 3.0f * screenH / 1080.0f);

        // Dim the whole screen, then the panel with an accent border.
        FillOverlayRect(0.0f, 0.0f, screenW, screenH, glm::vec4(0.0f, 0.0f, 0.0f, 0.55f * alpha));
        FillOverlayRect(px0 - edge, py0 - edge, px1 + edge, py1 + edge, glm::vec4(glm::vec3(accent), alpha));
        FillOverlayRect(px0, py0, px1, py1, glm::vec4(panelRgb, alpha));

        const auto DrawCentered = [&](const char* text, float yTop, float dy, const glm::vec4& color)
            {
                if (text == nullptr || text[0] == '\0')
                    return;
                const float w = DxOverlayText(ifont, text, dy);
                DrawOverlayText(ifont, text, px0 + (panelW - w) * 0.5f, yTop, dy, color);
            };

        float ty = py0 + pad;
        const std::string name = SettingName(s_edit.respk);
        DrawCentered(name.c_str(), ty, lineName, responseColor);
        ty += lineName;

        const char* pchzRow = AchzFromRespk(s_edit.respk);
        const bool fPercent = s_edit.respk == RESPK_Fov || (pchzRow != nullptr && std::strchr(pchzRow, '%') != nullptr);
        char achzValue[24];
        if (s_edit.fTyping)
        {
            const bool fCaret = std::fmod(g_clock.tReal, 1.0f) < 0.5f;
            std::snprintf(achzValue, sizeof(achzValue), "%s%s%s", s_edit.achzTyped, fCaret ? "_" : " ", fPercent ? "%" : "");
        }
        else
            std::snprintf(achzValue, sizeof(achzValue), "%d%s", s_edit.value, fPercent ? "%" : "");
        DrawCentered(achzValue, ty, lineValue, glm::vec4(glm::vec3(accent), alpha));
        ty += lineValue + gapY;

        // Slider: track, fill, knob.
        const float bx0 = px0 + pad * 1.5f;
        const float bx1 = px1 - pad * 1.5f;
        const float by0 = ty;
        const float by1 = ty + barH;
        const float range = static_cast<float>((std::max)(s_edit.num.max - s_edit.num.min, 1));
        const float u = std::clamp(static_cast<float>(s_edit.value - s_edit.num.min) / range, 0.0f, 1.0f);
        const float xKnob = bx0 + (bx1 - bx0) * u;
        FillOverlayRect(bx0, by0, bx1, by1, glm::vec4(0.18f, 0.19f, 0.24f, alpha));
        FillOverlayRect(bx0, by0, xKnob, by1, glm::vec4(glm::vec3(accent), 0.9f * alpha));
        const float knobW = 5.0f * sx;
        FillOverlayRect(xKnob - knobW, by0 - 6.0f * sy, xKnob + knobW, by1 + 6.0f * sy, glm::vec4(1.0f, 1.0f, 1.0f, alpha));
        ty = by1 + gapY * 1.5f;

        char achzHint[96];
        std::snprintf(achzHint, sizeof(achzHint), "Type a number or drag the slider (%d - %d)", s_edit.num.min, s_edit.num.max);
        DrawCentered(achzHint, ty, lineHint, responseColor);
        ty += lineHint;
        std::snprintf(achzHint, sizeof(achzHint), "Left/Right +%d   Up/Down +1   Enter = OK   Esc = Cancel", s_edit.num.step);
        DrawCentered(achzHint, ty, lineHint, responseColor);

        s_edit.xBar0 = bx0;
        s_edit.xBar1 = bx1;
        s_edit.yBar0 = by0;
        s_edit.yBar1 = by1;
        s_edit.xPanel0 = px0;
        s_edit.yPanel0 = py0;
        s_edit.xPanel1 = px1;
        s_edit.yPanel1 = py1;
        s_edit.fRects = true;
    }

    // -------------------------------------------------------------------------
    // Button prompt bar (bottom of the screen): what you can press right now,
    // shown for the device used last (controller glyphs, keys or mouse).
    // -------------------------------------------------------------------------

    if (pprompt == &g_prompt && MenuStyleValue(MSOPT_PromptBar) != 0 && !s_edit.fActive &&
        uOn >= 0.98f && promptResponses != nullptr && promptResponseCount > 0)
    {
        const RESPK selectedRespk = (pprompt->irespk >= 0 && pprompt->irespk < promptResponseCount)
            ? promptResponses[pprompt->irespk] : RESPK_Nil;
        bool fAtDefault = true;
        const bool fReset = selectedRespk != RESPK_Nil &&
            FRowDefaultInfo(pprompt->prk, selectedRespk, &fAtDefault) && !fAtDefault;
        NUMROW numSelected;
        const bool fEdit = selectedRespk != RESPK_Nil && FNumericRow(selectedRespk, &numSelected);
        const bool fFocused = FFocusRow(pprompt->prk, pprompt->irespk);
        const bool fTabs = PtabgroupFromPrk(pprompt->prk, nullptr) != nullptr;
        // Load Game: the selected slot holds a save that can be erased.
        bool fErase = false;
        if (pprompt->prk == PRK_MemcardChooseLoadSlot)
        {
            for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
                if (s_arespkSlots[slot] == selectedRespk && g_saveData.saveData[slot].dt != 0.0f)
                    fErase = true;
        }
        const std::vector<BARITEM> items = BuildPromptBarItems(fTabs, fFocused, fReset, fEdit, fErase);
        const float iconSize = 24.0f * promptScaleY * MenuStyleTextScale();
        DrawPromptBar(items, iconSize, alpha, glm::vec4(0.92f, 0.93f, 0.96f, alpha), GetSelectedColor(0.0f));
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

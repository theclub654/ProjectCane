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
#include <algorithm>
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

    if (g_joy.joystickId < GLFW_JOYSTICK_1 || g_joy.joystickId > GLFW_JOYSTICK_LAST)
        return -1;

    GLFWgamepadstate state{};
    if (!glfwGetGamepadState(g_joy.joystickId, &state))
        return -1;

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
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid)
    {
        if (glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid))
            return true;
    }
    return false;
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

	if (prk == PRK_KeyboardMapping)
		return keyboardMapping;
	if (prk == PRK_ControllerMapping)
		return controllerMapping;
	return s_mpprkprd[prk];
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
    std::vector<int> frameRates;
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

    if (frameRates.empty())
        frameRates.push_back(60);

    return frameRates;
}

void SelectNextSupportedFrameRate()
{
    const std::vector<int> frameRates = SupportedFrameRates();
    auto next = std::upper_bound(frameRates.begin(), frameRates.end(), g_targetFrameRate);

    if (next == frameRates.end())
        next = frameRates.begin();

    g_targetFrameRate = *next;
}

void SelectNextInternalResolution()
{
    constexpr int resolutions[] = { 0, 360, 480, 720, 1080, 1440, 2160 };
    auto current = std::find(std::begin(resolutions), std::end(resolutions),
        g_internalResolutionHeight);

    if (current == std::end(resolutions) || ++current == std::end(resolutions))
        current = std::begin(resolutions);

    g_internalResolutionHeight = *current;
    ApplyInternalResolutionSettings();
}

void SelectNextGuiScale()
{
    constexpr float ps2Scales[] = { 0.75f, 1.0f };
    constexpr float modernScales[] = { 1.0f, 1.25f };

    if (g_guiStyle == GuiStyle_Modern)
    {
        auto next = std::upper_bound(std::begin(modernScales), std::end(modernScales),
            g_guiScale + 0.001f);
        g_guiScale = next == std::end(modernScales) ? modernScales[0] : *next;
    }
    else
    {
        auto next = std::upper_bound(std::begin(ps2Scales), std::end(ps2Scales),
            g_guiScale + 0.001f);
        g_guiScale = next == std::end(ps2Scales) ? ps2Scales[0] : *next;
    }

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

void SelectNextDrawDistance()
{
    if (g_pcm == nullptr)
        return;

    constexpr float multipliers[] = { 0.75f, 1.0f, 1.5f, 2.0f, 3.0f };
    auto next = std::upper_bound(std::begin(multipliers), std::end(multipliers),
        g_pcm->rMRD + 0.001f);

    if (next == std::end(multipliers))
        next = std::begin(multipliers);

    SetCmMrdRatio(g_pcm, *next);
    g_drawDistanceMultiplier = *next;
}

void SelectNextWindowMode()
{
    const int next = (static_cast<int>(g_windowMode) + 1) % 3;
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

        case PRK_MemcardChooseLoadSlot:
        g_pnote.pvtblot->pfnSetBlotAchzDraw(&g_pnote, (char*)"&2S&.: Erase Game");
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
            std::snprintf(slotStrings[slot], 64, "%s", choosingSlot ? "New Game" : "Empty");
        }
    }

    std::snprintf(g_achzRespk13, sizeof(g_achzRespk13), "%s", s_apchzRespk13[FVibrationEnabled(g_pgsCur) ? 0 : 1]);
    std::snprintf(g_achzRespk15, sizeof(g_achzRespk15), "%s", s_apchzRespk15[(g_pgsCur->grfgs & 0x80U) != 0 ? 1 : 0]);
    std::snprintf(g_achzRespk16, sizeof(g_achzRespk16), "%s", s_apchzRespk16[(g_pgsCur->grfgs & 0x40U) != 0 ? 1 : 0]);
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
    if (g_fMsaa)
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: %dx", g_msaaSamples);
    else
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: Off");
    std::snprintf(g_achzRespk28, sizeof(g_achzRespk28), "Frame Rate: %d FPS", g_targetFrameRate);
    if (g_internalResolutionHeight == 0)
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

    if (prd.pchz != nullptr)
    {
        pprompt->pfont->PushScaling(prd.rScaleTitle, prd.rScaleTitle);
        pprompt->pfont->GetExtents(const_cast<char*>(prd.pchz), &titleWidth, &titleHeight, 500.0f);
        pprompt->pfont->PopScaling();

        titleWidth = std::max(titleWidth, 0.0f);
    }

    pprompt->pfont->PushScaling(prd.rScaleRespk, prd.rScaleRespk);

    const float lineHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    const bool isBindingMenu = FMappingPrompt(prk);
    const int visibleResponseCount = isBindingMenu ? std::min(promptResponseCount, 9) : promptResponseCount;
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
                if (responseData.apchz[variant] == nullptr)
                    continue;

                CRichText text(const_cast<char*>(responseData.apchz[variant]), pprompt->pfont);
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

void ExecutePrompt(PROMPT* pprompt)
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
            if (!g_fMsaa)
            {
                g_fMsaa = true;
                g_msaaSamples = 2;
            }
            else if (g_msaaSamples < 4)
                g_msaaSamples = 4;
            else if (g_msaaSamples < 8)
                g_msaaSamples = 8;
            else if (g_msaaSamples < 16)
                g_msaaSamples = 16;
            else
                g_fMsaa = false;

            ApplyMsaaSettings();
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_FrameRate:
            SelectNextSupportedFrameRate();
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_InternalResolution:
            SelectNextInternalResolution();
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_GuiScale:
            SelectNextGuiScale();
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_GuiStyle:
            SelectNextGuiStyle();
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_DrawDistance:
            SelectNextDrawDistance();
            SaveSystemSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_WindowMode:
            SelectNextWindowMode();
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
            switch (g_gl.aspectMode)
            {
                case FitToScreen:
                ApplyAspectRatioSettings(Fixed_16_9);
                break;
                case Fixed_16_9:
                ApplyAspectRatioSettings(Fixed_16_10);
                break;
                case Fixed_16_10:
                ApplyAspectRatioSettings(Fixed_4_3);
                break;
                case Fixed_4_3:
                ApplyAspectRatioSettings(PS2_4_3);
                break;
                case PS2_4_3:
                ApplyAspectRatioSettings(PS2_16_9);
                break;
                case PS2_16_9:
                default:
                ApplyAspectRatioSettings(FitToScreen);
                break;
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
            SaveCurrentGameToDisk(&g_saveData);
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
                SaveCurrentGameToDisk(&g_saveData);
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
                g_autosave.pvtblot->pfnShowBlot(&g_autosave);
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

                case RESPK_Music:
                g_pgsCur->grfgs ^= 0x80;
                SetAttractVolume((g_pgsCur->grfgs & 0x80) != 0);
                rebuildPromptKeepingSelection();
                return;
    
                case RESPK_Speakers:
                g_pgsCur->grfgs ^= 0x40;
                SetAttractSoundOption((g_pgsCur->grfgs & 0x40) != 0);
                rebuildPromptKeepingSelection();
                return;

                case RESPK_Controls:
                SetPrompt(pprompt, PRP_Basic, PRK_ControlsMenu);
                return;

                case RESPK_Video:
                SetPrompt(pprompt, PRP_Basic, PRK_VideoMenu);
                return;

                default:
                return;
            }
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

		if (response >= RESPK_KeyUp && response <= RESPK_KeyR3)
		{
			s_iKeyboardBindingCapture = static_cast<int>(response) - static_cast<int>(RESPK_KeyUp);
			s_fKeyboardBindingCaptureArmed = false;
			std::snprintf(g_aachzKeyboardBindings[s_iKeyboardBindingCapture],
				sizeof(g_aachzKeyboardBindings[s_iKeyboardBindingCapture]),
				"%s: Press %s", s_apchzKeyboardActions[s_iKeyboardBindingCapture],
				s_fControllerBindingCapture ? "a button" : "a key");
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

    if (pjoy->IsPressed(BTN_START) || pjoy->IsPressed(BTN_TRIANGLE))
    {
        pjoy->SetHandled(BTN_START);
        pjoy->SetHandled(BTN_TRIANGLE);
        CancelPrompt(pprompt);
        return;
    }

    if (pjoy->IsPressed(BTN_SQUARE))
    {
        pjoy->SetHandled(BTN_SQUARE);

        if (prk == PRK_MemcardChooseLoadSlot &&
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

        case PRK_MemcardChooseSaveSlot:
        case PRK_MemcardChooseNewSlot:
        case PRK_MemcardChooseLoadSlot:
        case PRK_ControlsMenu:
        case PRK_VideoMenu:
        SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
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
    // RepositionBlot currently centers xOn/yOn in output-pixel space while
    // retaining the retail (unscaled) dx/dy.  Preserve that already-correct
    // output-space center and scale only the offset from it.  Scaling xOn or
    // yOn again would move the prompt toward the lower-right corner.
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

    if (prd.pchz != nullptr)
    {
        const float titleScaleX = prd.rScaleTitle * uOn * promptScaleX;
        const float titleScaleY = prd.rScaleTitle * uOn * promptScaleY;

        pprompt->pfont->PushScaling(titleScaleX, titleScaleY);

        textBox.SetPos(x, y);
        textBox.SetSize(dx, dy);
        textBox.SetTextColor(&titleColor);
        textBox.SetHorizontalJust(JH_Center);
        textBox.SetVerticalJust(JV_Top);

        CRichText titleText(prd.pchz, pprompt->pfont);

        titleText.Draw(&textBox, nullptr);
        y += titleText.DyWrap(500.0f * promptScaleX);

        pprompt->pfont->PopScaling();
    }

    // Retail places the divider one PS2-canvas unit above the response row.
    const float dividerY = y - promptScaleY;

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

    // The original prompt was designed for short menus. Keep mapping menus
    // within that layout and scroll the visible window around the selection.
    int firstResponse = 0;
    int lastResponse = promptResponseCount;
    if (FMappingPrompt(pprompt->prk))
    {
        constexpr int visibleBindings = 9;
        firstResponse = std::clamp(pprompt->irespk - visibleBindings / 2,
            0, std::max(0, promptResponseCount - visibleBindings));
        lastResponse = std::min(promptResponseCount, firstResponse + visibleBindings);
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

    if (prd.pchz != nullptr && promptResponseCount > 0)
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

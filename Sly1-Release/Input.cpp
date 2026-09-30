#include "Input.h"
#include "clock.h"
#include "credit.h"
#include "pzo.h"
#include "sound.h"
#include "transition.h"
#include "ui.h"
#include <cstdio>
#include <iterator>

namespace
{
constexpr uint16_t JBC_SELECT   = 0x0001;
constexpr uint16_t JBC_L3       = 0x0002;
constexpr uint16_t JBC_START    = 0x0008;
constexpr uint16_t JBC_UP       = 0x0010;
constexpr uint16_t JBC_RIGHT    = 0x0020;
constexpr uint16_t JBC_DOWN     = 0x0040;
constexpr uint16_t JBC_LEFT     = 0x0080;
constexpr uint16_t JBC_L2       = 0x0100;
constexpr uint16_t JBC_R2       = 0x0200;
constexpr uint16_t JBC_R1       = 0x0800;
constexpr uint16_t JBC_TRIANGLE = 0x1000;
constexpr uint16_t JBC_CIRCLE   = 0x2000;
constexpr uint16_t JBC_CROSS    = 0x4000;
constexpr uint16_t JBC_SQUARE   = 0x8000;

CODE* s_pcode = nullptr;
float s_tCodeCheck = 0.0f;

uint16_t GrfbtnFromState(const std::unordered_map<JOY_BUTTON, bool>& state)
{
    uint16_t grfbtn = 0;
    const auto held = [&state](JOY_BUTTON button)
    {
        const auto it = state.find(button);
        return it != state.end() && it->second;
    };

    if (held(BTN_SELECT))   grfbtn |= JBC_SELECT;
    if (held(BTN_L3))       grfbtn |= JBC_L3;
    if (held(BTN_START))    grfbtn |= 0x0008;
    if (held(BTN_UP))       grfbtn |= JBC_UP;
    if (held(BTN_RIGHT))    grfbtn |= JBC_RIGHT;
    if (held(BTN_DOWN))     grfbtn |= JBC_DOWN;
    if (held(BTN_LEFT))     grfbtn |= JBC_LEFT;
    if (held(BTN_L2))       grfbtn |= JBC_L2;
    if (held(BTN_R2))       grfbtn |= 0x0200;
    if (held(BTN_L1))       grfbtn |= 0x0400;
    if (held(BTN_R1))       grfbtn |= JBC_R1;
    if (held(BTN_TRIANGLE)) grfbtn |= JBC_TRIANGLE;
    if (held(BTN_CIRCLE))   grfbtn |= JBC_CIRCLE;
    if (held(BTN_CROSS))    grfbtn |= JBC_CROSS;
    if (held(BTN_SQUARE))   grfbtn |= JBC_SQUARE;
    return grfbtn;
}

void CodeResetWorld(uint32_t nParam)
{
    g_transition.ResetWorld(static_cast<int>(nParam));
}

void CodeResetCheats(uint32_t)
{
    g_grfcht = 0;
    g_transition.ResetWorld(0);
}

void CodeSetCheatFlags(uint32_t nParam)
{
    g_grfcht |= static_cast<GRFCHT>(nParam & ~0x4000U);
    if ((nParam & 0x4000U) != 0)
        g_transition.ResetWorld(0);
}

void CodeCollectAllClues(uint32_t)
{
    if (g_psw != nullptr)
        CollectAllClues();
}

void CodeSetVaultFlags(uint32_t nParam)
{
    if (g_pgsCur != nullptr)
        g_pgsCur->grfvault = nParam;
}

void CompleteWorld(GAMEWORLD gameWorld)
{
    if (g_pgsCur == nullptr)
        return;

    WS& world = g_pgsCur->aws[static_cast<int>(gameWorld)];
    for (LS& level : world.als)
        level.grfls = static_cast<GRFLS>(level.grfls | 1U);
    world.fws |= 31;
}

void CodeCompleteAllWorlds(uint32_t)
{
    for (int i = 0; i < 6; ++i)
        CompleteWorld(static_cast<GAMEWORLD>(i));

    // Retail compares this field against the raw UIS_Hub value.
    if (g_ui.uisPlaying == UIS_Hub)
        g_transition.ResetWorld(0);
}

void CodeShowPassword(uint32_t)
{
    if (g_pgsCur == nullptr)
        return;

    const int worldLevel =
        (static_cast<int>(g_pgsCur->gameWorldCur) << 8) |
        static_cast<int>(g_pgsCur->worldLevelCur);

    if (worldLevel != 0x0400 || (GetGameProgress() & 6U) != 6U ||
        g_pgsCur->ccoin != 99 || g_pgsCur->clife != 0)
        return;

    char message[64]{};
    std::snprintf(message, sizeof(message), "The password is: %s", "chetkido");
    g_pnote.pvtblot->pfnSetBlotAchzDraw(&g_pnote, message);
    SetBlotDtVisible(&g_pnote, 10.0f);
    g_pnote.pvtblot->pfnShowBlot(&g_pnote);
}

constexpr uint16_t s_codeResetWorld[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_SELECT };
constexpr uint16_t s_codeResetCheats[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_START };
constexpr uint16_t s_codeCheat4[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_LEFT };
constexpr uint16_t s_codeCheat8[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_RIGHT };
constexpr uint16_t s_codeCheat2[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_CIRCLE };
constexpr uint16_t s_codeClues[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_UP };
constexpr uint16_t s_codeVaultFlags[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_DOWN };
constexpr uint16_t s_codeWorlds[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_TRIANGLE };
constexpr uint16_t s_codePassword[] =
    { JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_DOWN, JBC_L2, JBC_R2, JBC_CROSS, JBC_LEFT, JBC_CIRCLE, JBC_SQUARE, JBC_SQUARE, JBC_SQUARE };

CODE s_codes[] =
{
    { s_codeResetWorld,  static_cast<int>(std::size(s_codeResetWorld)),  CodeResetWorld,        0 },
    { s_codeResetCheats, static_cast<int>(std::size(s_codeResetCheats)), CodeResetCheats,       0 },
    { s_codeCheat4,      static_cast<int>(std::size(s_codeCheat4)),      CodeSetCheatFlags, 0x4004 },
    { s_codeCheat8,      static_cast<int>(std::size(s_codeCheat8)),      CodeSetCheatFlags,      8 },
    { s_codeCheat2,      static_cast<int>(std::size(s_codeCheat2)),      CodeSetCheatFlags,      2 },
    { s_codeClues,       static_cast<int>(std::size(s_codeClues)),       CodeCollectAllClues,    0 },
    { s_codeVaultFlags,  static_cast<int>(std::size(s_codeVaultFlags)),  CodeSetVaultFlags, 0xF000FFFF },
    { s_codeWorlds,      static_cast<int>(std::size(s_codeWorlds)),      CodeCompleteAllWorlds,  0 },
    { s_codePassword,    static_cast<int>(std::size(s_codePassword)),    CodeShowPassword,       0 }
};

constexpr std::array<int, BTN_MAX> kDefaultKeyboardBindings =
{
    GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
    GLFW_KEY_J, GLFW_KEY_L, GLFW_KEY_K, GLFW_KEY_I,
    GLFW_KEY_ENTER, GLFW_KEY_BACKSPACE,
    GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_Z, GLFW_KEY_C,
    GLFW_KEY_UNKNOWN, GLFW_KEY_UNKNOWN
};

constexpr std::array<int, BTN_MAX> kDefaultGamepadBindings =
{
    GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
    GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
    GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_B,
    GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_Y,
    GLFW_GAMEPAD_BUTTON_START, GLFW_GAMEPAD_BUTTON_BACK,
    GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
    GAMEPAD_BINDING_LEFT_TRIGGER, GAMEPAD_BINDING_RIGHT_TRIGGER,
    GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB
};

float GamepadActivity(const GLFWgamepadstate& state)
{
    float activity = 0.0f;

    for (int button = 0; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button)
    {
        if (state.buttons[button] == GLFW_PRESS)
            activity = (std::max)(activity, 2.0f);
    }

    activity = (std::max)(activity, std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]));
    activity = (std::max)(activity, std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]));
    activity = (std::max)(activity, std::abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]));
    activity = (std::max)(activity, std::abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]));

    // GLFW trigger axes rest at -1. Convert them to a zero-to-one range so
    // an untouched trigger does not make every connected pad look active.
    activity = (std::max)(activity,
        (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1.0f) * 0.5f);
    activity = (std::max)(activity,
        (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) * 0.5f);

    return activity;
}

const char* PchzKeyboardKeyNameInternal(int key)
{
    switch (key)
    {
    case GLFW_KEY_UNKNOWN: return "Unbound";
    case GLFW_KEY_SPACE: return "Space";
    case GLFW_KEY_ENTER: return "Enter";
    case GLFW_KEY_TAB: return "Tab";
    case GLFW_KEY_BACKSPACE: return "Backspace";
    case GLFW_KEY_LEFT_SHIFT: return "Left Shift";
    case GLFW_KEY_RIGHT_SHIFT: return "Right Shift";
    case GLFW_KEY_LEFT_CONTROL: return "Left Ctrl";
    case GLFW_KEY_RIGHT_CONTROL: return "Right Ctrl";
    case GLFW_KEY_LEFT_ALT: return "Left Alt";
    case GLFW_KEY_RIGHT_ALT: return "Right Alt";
    case GLFW_KEY_UP: return "Up Arrow";
    case GLFW_KEY_DOWN: return "Down Arrow";
    case GLFW_KEY_LEFT: return "Left Arrow";
    case GLFW_KEY_RIGHT: return "Right Arrow";
    default:
        if (const char* name = glfwGetKeyName(key, 0))
            return name;
        return "Key";
    }
}
}

const char* PchzKeyboardKeyName(int key)
{
    return PchzKeyboardKeyNameInternal(key);
}

const char* PchzGamepadBindingName(int binding)
{
    switch (binding)
    {
    case GLFW_GAMEPAD_BUTTON_A: return "A / Cross";
    case GLFW_GAMEPAD_BUTTON_B: return "B / Circle";
    case GLFW_GAMEPAD_BUTTON_X: return "X / Square";
    case GLFW_GAMEPAD_BUTTON_Y: return "Y / Triangle";
    case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return "Left Bumper";
    case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return "Right Bumper";
    case GLFW_GAMEPAD_BUTTON_BACK: return "Back / Select";
    case GLFW_GAMEPAD_BUTTON_START: return "Start";
    case GLFW_GAMEPAD_BUTTON_GUIDE: return "Guide";
    case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return "Left Stick";
    case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return "Right Stick";
    case GLFW_GAMEPAD_BUTTON_DPAD_UP: return "D-Pad Up";
    case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return "D-Pad Right";
    case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return "D-Pad Down";
    case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return "D-Pad Left";
    case GAMEPAD_BINDING_LEFT_TRIGGER: return "Left Trigger";
    case GAMEPAD_BINDING_RIGHT_TRIGGER: return "Right Trigger";
    default: return "Unbound";
    }
}

void JOY::Update(GLFWwindow* window)
{
    const uint16_t grfbtnPrev = GrfbtnFromState(current);
    previous = current;
    handled.clear();
    forcedHeld.clear();

    for (int button = 0; button < BTN_MAX; ++button)
        current[static_cast<JOY_BUTTON>(button)] = false;

    for (int button = 0; button < BTN_MAX; ++button)
    {
        const int key = g_keyboardBindings[button];
        if (key != GLFW_KEY_UNKNOWN)
            current[static_cast<JOY_BUTTON>(button)] = glfwGetKey(window, key) == GLFW_PRESS;
    }

    float xRaw = 0.0f;
    float yRaw = 0.0f;
    float x2Raw = 0.0f;
    float y2Raw = 0.0f;

    GLFWgamepadstate state{};
    bool haveGamepadState = false;
    int activeJoystickId = JOYID_Nil;
    float bestActivity = -1.0f;
    const bool allowControllerInput = !g_fDisableControllerInputWhenUnfocused ||
        glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;

    // Steam may leave an idle virtual gamepad in a lower joystick slot than
    // the physical controller. Poll every mapped gamepad and prefer a device
    // that is actually receiving input instead of permanently binding to the
    // first slot returned by GLFW.
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid)
    {
        if (!allowControllerInput)
            break;

        if (!glfwJoystickPresent(jid) || !glfwJoystickIsGamepad(jid))
            continue;

        GLFWgamepadstate candidate{};
        if (!glfwGetGamepadState(jid, &candidate))
            continue;

        float activity = GamepadActivity(candidate);
        if (jid == joystickId)
            activity += 0.01f;

        if (!haveGamepadState || activity > bestActivity)
        {
            state = candidate;
            activeJoystickId = jid;
            bestActivity = activity;
            haveGamepadState = true;
        }
    }

    if (haveGamepadState)
    {
        if (!gamepadConnected || activeJoystickId != joystickId)
        {
            calibration = StickCalibration{};
            calibration2 = StickCalibration{};
        }

        joystickId = activeJoystickId;
        gamepadConnected = true;
        for (int button = 0; button < BTN_MAX; ++button)
        {
            const int binding = g_gamepadBindings[button];
            bool held = false;
            if (binding >= 0 && binding <= GLFW_GAMEPAD_BUTTON_LAST)
                held = state.buttons[binding] == GLFW_PRESS;
            else if (binding == GAMEPAD_BINDING_LEFT_TRIGGER)
                held = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.25f;
            else if (binding == GAMEPAD_BINDING_RIGHT_TRIGGER)
                held = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.25f;
            current[static_cast<JOY_BUTTON>(button)] = current[static_cast<JOY_BUTTON>(button)] || held;
        }

        xRaw = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        // STEP/JT expects the original PlayStation convention: forward is
        // positive Y. GLFW reports stick-up as -1, so invert it here.
        yRaw = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        x2Raw = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        y2Raw = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
    }
    else
    {
        joystickId = JOYID_Nil;
        gamepadConnected = false;
    }

    // Keyboard movement emulates the left analog stick.  The base CPMAN
    // policy is normally active during gameplay, so it must not suppress
    // these values or the player receives a permanently centered stick.
    if (current[BTN_LEFT] && !current[BTN_RIGHT]) xRaw = -1.0f;
    if (current[BTN_RIGHT] && !current[BTN_LEFT]) xRaw = 1.0f;
    if (current[BTN_UP] && !current[BTN_DOWN]) yRaw = 1.0f;
    if (current[BTN_DOWN] && !current[BTN_UP]) yRaw = -1.0f;

    // Keyboard camera input emulates the right analog stick. UpdateCptn
    // consumes x2 for manual orbiting around the player.
    const bool cameraLeft = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool cameraRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    const bool cameraUp = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    const bool cameraDown = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

    if (cameraLeft && !cameraRight) x2Raw = -1.0f;
    if (cameraRight && !cameraLeft) x2Raw = 1.0f;
    if (cameraUp && !cameraDown) y2Raw = 1.0f;
    if (cameraDown && !cameraUp) y2Raw = -1.0f;

    ApplyStickDeadZone(xRaw, yRaw, &calibration, &x, &y, &uDeflect);
    ApplyStickDeadZone(x2Raw, y2Raw, &calibration2, &x2, &y2, &uDeflect2);

    _MatchCodes(this, grfbtnPrev);

    joys = JOYS_Ready;
}

void AddCode(CODE* pcode)
{
    for (CODE* current = s_pcode; current != nullptr; current = current->pcodeNext)
        if (current == pcode)
            return;

    pcode->pcodeNext = s_pcode;
    s_pcode = pcode;
    pcode->ijbcCur = GrfbtnFromState(g_joy.current) == pcode->ajbc[0] ? 1 : 0;
}

void _ResetCodes()
{
    for (CODE* code = s_pcode; code != nullptr; code = code->pcodeNext)
        code->ijbcCur = 0;
}

void _MatchCodes(JOY* pjoy, uint16_t grfbtnPrev)
{
    const uint16_t grfbtn = GrfbtnFromState(g_joy.current);
    if (s_pcode == nullptr || pjoy != &g_joy || grfbtnPrev == grfbtn || grfbtn == 0)
        return;

    for (CODE* code = s_pcode; code != nullptr; code = code->pcodeNext)
    {
        if (code->ijbcCur < code->cjbc)
            code->ijbcCur = grfbtn == code->ajbc[code->ijbcCur] ? code->ijbcCur + 1 : 0;
    }

    s_tCodeCheck = g_clock.tReal + 1.0f;
}

void UpdateCodes()
{
    if (s_tCodeCheck == 0.0f || s_tCodeCheck > g_clock.tReal)
        return;

    CODE* matched = nullptr;
    CODE** link = &s_pcode;
    while (*link != nullptr)
    {
        CODE* code = *link;
        if (code->fRemove != 0)
        {
            *link = code->pcodeNext;
            code->pcodeNext = nullptr;
            code->ijbcCur = 0;
            code->fRemove = 0;
            continue;
        }

        if (code->ijbcCur >= code->cjbc &&
            (matched == nullptr || matched->cjbc < code->cjbc))
            matched = code;

        link = &code->pcodeNext;
    }

    if (matched != nullptr)
    {
        StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr,
            3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        matched->pfn(matched->nParam);
    }

    _ResetCodes();
    s_tCodeCheck = 0.0f;
}

void StartupCodes()
{
    for (CODE& code : s_codes)
        AddCode(&code);
}

bool JOY::IsPressed(JOY_BUTTON button)
{
    return current[button] && !previous[button] && !handled[button];
}

bool JOY::IsHeld(JOY_BUTTON button)
{
    return current[button] || forcedHeld[button];
}

bool JOY::IsReleased(JOY_BUTTON button)
{
    return !current[button] && previous[button] && !handled[button];
}

void JOY::SetHandled(JOY_BUTTON button)
{
    handled[button] = true;
}

int JOY::DxSelectionJoy(float currentTime)
{
    if (dxLatch == 0)
    {
        if (IsPressed(BTN_LEFT))
        {
            SetHandled(BTN_LEFT);
            dxLatch = -1;
        }
        else if (IsPressed(BTN_RIGHT))
        {
            SetHandled(BTN_RIGHT);
            dxLatch = 1;
        }
        else if (uDeflect >= 0.8f && std::abs(y) < std::abs(x))
        {
            dxLatch = x < 0.0f ? -1 : 1;
        }

        if (dxLatch == 0)
            return 0;

        dtLatchX = 0.3333f;
    }
    else
    {
        bool fStillHeld;
        if (dxLatch < 0)
        {
            fStillHeld = IsHeld(BTN_LEFT) ||
                (uDeflect >= 0.25f && std::abs(y) < std::abs(x) && x < 0.0f);
        }
        else
        {
            fStillHeld = IsHeld(BTN_RIGHT) ||
                (uDeflect >= 0.25f && std::abs(y) < std::abs(x) && x > 0.0f);
        }

        if (!fStillHeld)
        {
            dxLatch = 0;
            return 0;
        }

        if (currentTime < tLatchX)
            return 0;

        dtLatchX = glm::clamp(dtLatchX * 0.6666667f, 0.1f, 0.3333f);
    }

    tLatchX = currentTime + dtLatchX;
    return dxLatch;
}

int JOY::DySelectionJoy(float currentTime)
{
    if (dyLatch == 0)
    {
        if (IsPressed(BTN_UP))
        {
            SetHandled(BTN_UP);
            dyLatch = -1;
        }
        else if (IsPressed(BTN_DOWN))
        {
            SetHandled(BTN_DOWN);
            dyLatch = 1;
        }
        else if (uDeflect >= 0.8f && std::abs(x) < std::abs(y))
        {
            dyLatch = y > 0.0f ? -1 : 1;
        }

        if (dyLatch == 0)
            return 0;

        dtLatchY = 0.3333f;
    }
    else
    {
        bool fStillHeld;
        if (dyLatch < 0)
        {
            fStillHeld = IsHeld(BTN_UP) ||
                (uDeflect >= 0.25f && std::abs(x) < std::abs(y) && y > 0.0f);
        }
        else
        {
            fStillHeld = IsHeld(BTN_DOWN) ||
                (uDeflect >= 0.25f && std::abs(x) < std::abs(y) && y < 0.0f);
        }

        if (!fStillHeld)
        {
            dyLatch = 0;
            return 0;
        }

        if (currentTime < tLatchY)
            return 0;

        dtLatchY = glm::clamp(dtLatchY * 0.6666667f, 0.1f, 0.3333f);
    }

    tLatchY = currentTime + dtLatchY;
    return dyLatch;
}

void JOY::StartJoySelection()
{
    dxLatch = 0;
    dyLatch = 0;
    tLatchX = 0.0f;
    tLatchY = 0.0f;
    dtLatchX = 0.3333f;
    dtLatchY = 0.3333f;
}

void AddGrfusr(GRFUSR grfusr)
{
    g_grfusr |= grfusr;
    UpdateGrfjoytFromGrfusr();
}

void RemoveGrfusr(GRFUSR grfusr)
{
    g_grfusr &= ~grfusr;
    UpdateGrfjoytFromGrfusr();
}

void UpdateGrfjoytFromGrfusr()
{
    if ((g_grfusr & 4) != 0)
        g_grfjoyt = 0;
    else if ((g_grfusr & 1) != 0)
        g_grfjoyt = 5;
    else if ((g_grfusr & 2) != 0)
        g_grfjoyt = 4;
    else
        g_grfjoyt = 7;
}

void EnableVibration()
{
    if (vibrationSetting == 0)
        vibrationSetting = 1;
}

void ToggleVibration(GS* pgs)
{
    constexpr uint32_t GRFGS_VibrationDisabled = 0x20;

    switch (vibrationSetting)
    {
    case 1:
        // Toggle the saved vibration flag.
        pgs->grfgs ^= GRFGS_VibrationDisabled;
        break;

    case 2:
        // Currently forced on; switch it off.
        pgs->grfgs |= GRFGS_VibrationDisabled;
        break;

    case 3:
        // Currently forced off; switch it on.
        pgs->grfgs &= ~GRFGS_VibrationDisabled;
        break;

    default:
        break;
    }

    if ((pgs->grfgs & GRFGS_VibrationDisabled) == 0)
        vibrationSetting = 2; // Enabled
    else
        vibrationSetting = 3; // Disabled
}

void ApplyVibrationSetting(GS* pgs)
{
    if (vibrationSetting == 2) {
        pgs->grfgs = pgs->grfgs & 4294967263;
        return;
    }
    if ((2 < vibrationSetting) && (vibrationSetting == 3)) {
        pgs->grfgs = pgs->grfgs | 32;
        return;
    }
}

bool FVibrationEnabled(GS* pgs)
{
    if (vibrationSetting == 0 || vibrationSetting == 3)
        return 0;

    if (vibrationSetting == 2)
        return 1;

    return (pgs->grfgs & 0x20) == 0;
}

JOY g_joy{};
JOY g_joyZero{};
int g_grfjoyt;
GRFUSR g_grfusr;
int vibrationSetting = 1;
std::array<int, BTN_MAX> g_keyboardBindings = kDefaultKeyboardBindings;
std::array<int, BTN_MAX> g_gamepadBindings = kDefaultGamepadBindings;
bool g_fDisableControllerInputWhenUnfocused = false;

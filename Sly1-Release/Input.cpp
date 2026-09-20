#include "Input.h"

namespace
{
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

    // Steam may leave an idle virtual gamepad in a lower joystick slot than
    // the physical controller. Poll every mapped gamepad and prefer a device
    // that is actually receiving input instead of permanently binding to the
    // first slot returned by GLFW.
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid)
    {
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

    ApplyStickDeadZone(xRaw, yRaw, &x, &y, &uDeflect);
    ApplyStickDeadZone(x2Raw, y2Raw, &x2, &y2, &uDeflect2);

    joys = JOYS_Ready;
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
        if (IsPressed(BTN_LEFT) || x <= -0.8f)
        {
            SetHandled(BTN_LEFT);
            dxLatch = -1;
        }
        else if (IsPressed(BTN_RIGHT) || x >= 0.8f)
        {
            SetHandled(BTN_RIGHT);
            dxLatch = 1;
        }

        if (dxLatch == 0)
            return 0;

        dtLatchX = 0.3333f;
    }
    else
    {
        const bool fStillHeld = dxLatch < 0 ? IsHeld(BTN_LEFT) || x <= -0.25f : IsHeld(BTN_RIGHT) || x >= 0.25f;

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
        if (IsPressed(BTN_UP) || y >= 0.8f)
        {
            SetHandled(BTN_UP);
            dyLatch = -1;
        }
        else if (IsPressed(BTN_DOWN) || y <= -0.8f)
        {
            SetHandled(BTN_DOWN);
            dyLatch = 1;
        }

        if (dyLatch == 0)
            return 0;

        dtLatchY = 0.3333f;
    }
    else
    {
        const bool fStillHeld = dyLatch < 0 ? IsHeld(BTN_UP) || y >= 0.25f : IsHeld(BTN_DOWN) || y <= -0.25f;

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

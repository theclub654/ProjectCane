#include "Input.h"

bool JOY::FindGamepad()
{
    if (gamepadConnected && joystickId >= GLFW_JOYSTICK_1 && joystickId <= GLFW_JOYSTICK_LAST &&
        glfwJoystickPresent(joystickId) && glfwJoystickIsGamepad(joystickId))
    {
        return true;
    }

    joystickId = JOYID_Nil;
    gamepadConnected = false;

    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid)
    {
        if (glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid))
        {
            joystickId = jid;
            gamepadConnected = true;
            return true;
        }
    }

    return false;
}

void JOY::Update(GLFWwindow* window)
{
    previous = current;
    handled.clear();
    forcedHeld.clear();

    for (int button = 0; button < BTN_MAX; ++button)
        current[static_cast<JOY_BUTTON>(button)] = false;

    current[BTN_UP] = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    current[BTN_DOWN] = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    current[BTN_LEFT] = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    current[BTN_RIGHT] = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    current[BTN_SQUARE] = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
    current[BTN_CROSS] = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
    current[BTN_CIRCLE] = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    current[BTN_TRIANGLE] = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    current[BTN_START] = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
    current[BTN_SELECT] = glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS;
    current[BTN_L1] = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    current[BTN_R1] = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
    current[BTN_L2] = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;
    current[BTN_R2] = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;

    float xRaw = 0.0f;
    float yRaw = 0.0f;
    float x2Raw = 0.0f;
    float y2Raw = 0.0f;

    GLFWgamepadstate state{};

    if (FindGamepad() && glfwGetGamepadState(joystickId, &state))
    {
        current[BTN_UP] = current[BTN_UP] || state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS;
        current[BTN_DOWN] = current[BTN_DOWN] || state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS;
        current[BTN_LEFT] = current[BTN_LEFT] || state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS;
        current[BTN_RIGHT] = current[BTN_RIGHT] || state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS;
        current[BTN_CROSS] = current[BTN_CROSS] || state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
        current[BTN_CIRCLE] = current[BTN_CIRCLE] || state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS;
        current[BTN_SQUARE] = current[BTN_SQUARE] || state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS;
        current[BTN_TRIANGLE] = current[BTN_TRIANGLE] || state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS;
        current[BTN_L1] = current[BTN_L1] || state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS;
        current[BTN_R1] = current[BTN_R1] || state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS;
        current[BTN_SELECT] = current[BTN_SELECT] || state.buttons[GLFW_GAMEPAD_BUTTON_BACK] == GLFW_PRESS;
        current[BTN_START] = current[BTN_START] || state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS;
        current[BTN_L3] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB] == GLFW_PRESS;
        current[BTN_R3] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB] == GLFW_PRESS;
        current[BTN_L2] = current[BTN_L2] || state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.25f;
        current[BTN_R2] = current[BTN_R2] || state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.25f;

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

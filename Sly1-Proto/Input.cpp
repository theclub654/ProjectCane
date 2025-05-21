#include "Input.h"

void JOY::Update(GLFWwindow* window)
{
    previous = current;

    current[BTN_UP]     = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    current[BTN_DOWN]   = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    current[BTN_LEFT]   = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    current[BTN_RIGHT]  = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    current[BTN_A]      = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
    current[BTN_B]      = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
    current[BTN_X]      = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;
    current[BTN_Y]      = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    current[BTN_START]  = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
    current[BTN_SELECT] = glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS;
    current[BTN_L1]     = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    current[BTN_R1]     = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
}

bool JOY::IsPressed(JOY_BUTTON button)
{
	return current[button] && !previous[button];
}

bool JOY::IsHeld(JOY_BUTTON button)
{
    return current[button];
}

bool JOY::IsReleased(JOY_BUTTON button)
{
    return !current[button] && previous[button];
}

int JOY::DxSelectionJoy(float currentTime)
{
    if (dxLatch == 0) {
        if (IsPressed(BTN_LEFT)) {
            dxLatch = -1;
            tLatchX = currentTime + dtLatchX;
            dtLatchX = 0.3333f;
            return dxLatch;
        }
        else if (IsPressed(BTN_RIGHT)) {
            dxLatch = 1;
            tLatchX = currentTime + dtLatchX;
            dtLatchX = 0.3333f;
            return dxLatch;
        }
    }
    else {
        // Reset latch if input released
        if ((dxLatch < 0 && !IsHeld(BTN_LEFT)) ||
            (dxLatch > 0 && !IsHeld(BTN_RIGHT))) {
            dxLatch = 0;
            return 0;
        }

        // Handle repeated input after hold
        if (currentTime >= tLatchX) {
            float speed = dtLatchX * 2.0f * (1.0f / 3.0f);
            dtLatchX = glm::clamp(speed, 0.1f, 0.3333f);
            tLatchX = currentTime + dtLatchX;
            return dxLatch;
        }
    }

    return 0;
}

int JOY::DySelectionJoy(float currentTime)
{
    if (dyLatch == 0) {
        if (IsPressed(BTN_UP)) {
            dyLatch = -1;
            tLatchY = currentTime + dtLatchY;
            dtLatchY = 0.3333f;
            return dyLatch;
        }
        else if (IsPressed(BTN_DOWN)) {
            dyLatch = 1;
            tLatchY = currentTime + dtLatchY;
            dtLatchY = 0.3333f;
            return dyLatch;
        }
    }
    else {
        // Latch clears when directional button is released
        if ((dyLatch < 0 && !IsHeld(BTN_UP)) ||
            (dyLatch > 0 && !IsHeld(BTN_DOWN))) {
            dyLatch = 0;
            return 0;
        }

        if (currentTime >= tLatchY) {
            float speed = dtLatchY * 2.0f * (1.0f / 3.0f);  // Gradual repeat acceleration
            dtLatchY = glm::clamp(speed, 0.1f, 0.3333f);
            tLatchY = currentTime + dtLatchY;
            return dyLatch;
        }
    }

    return 0;
}

void JOY::StartJoySelection()
{
    dyLatch = 0;
    dxLatch = 0;
}

JOY g_joy;
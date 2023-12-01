#include "mouse.h"

/*
    define initial static values
*/

// x posiiton
double MOUSE::x = 0;
// y position
double MOUSE::y = 0;

// previous x position
double MOUSE::lastX = 0;
// previous y position
double MOUSE::lastY = 0;

// change in x position from lastX
double MOUSE::dx = 0;
// change in y position from lastY
double MOUSE::dy = 0;

// change in scroll x
double MOUSE::scrollDx = 0;
// change in scroll y
double MOUSE::scrollDy = 0;

// if this is the first change in the mouse position
bool MOUSE::firstMouse = true;

// button state array (true for down, false for up)
bool MOUSE::buttons[GLFW_MOUSE_BUTTON_LAST] = { 0 };
// button changed array (true if changed)
bool MOUSE::buttonsChanged[GLFW_MOUSE_BUTTON_LAST] = { 0 };

/*
    callbacks
*/

// cursor position changed
void MOUSE::CursorPosCallback(GLFWwindow* window, double _x, double _y) {
    x = _x;
    y = _y;

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    dx = x - lastX;
    dy = lastY - y; // y coordinates are inverted
    lastX = x;
    lastY = y;
}

// mouse button state changed
void MOUSE::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action != GLFW_RELEASE) {
        if (!buttons[button]) {
            buttons[button] = true;
        }
    }
    else {
        buttons[button] = false;
    }
    buttonsChanged[button] = action != GLFW_REPEAT;
}

// scroll wheel moved
void MOUSE::MouseWheelCallback(GLFWwindow* window, double dx, double dy) {
    scrollDx = dx;
    scrollDy = dy;
}

/*
    accessors
*/

// get mouse x position
double MOUSE::GetMouseX() {
    return x;
}

// get mouse y position
double MOUSE::GetMouseY() {
    return y;
}

// get mouse change in x
double MOUSE::GetDX() {
    double _dx = dx;
    // set to 0 because change no longer new
    dx = 0;
    return _dx;
}

// get mouse change in y
double MOUSE::GetDY() {
    double _dy = dy;
    // set to 0 because change no longer new
    dy = 0;
    return _dy;
}

// get scroll value in x
double MOUSE::GetScrollDX() {
    double _scrollDx = scrollDx;
    // set to 0 because change no longer new
    scrollDx = 0;
    return _scrollDx;
}

// get scroll value in y
double MOUSE::GetScrollDY() {
    double _scrollDy = scrollDy;
    // set to 0 because change no longer new
    scrollDy = 0;
    return _scrollDy;
}

// get button state
bool MOUSE::Button(int button) {
    return buttons[button];
}

// return if button changed then reset it in the changed array
bool MOUSE::ButtonChanged(int button) {
    bool ret = buttonsChanged[button];
    // set to false because change no longer new
    buttonsChanged[button] = false;
    return ret;
}

// return if button changed and is now up
bool MOUSE::ButtonWentUp(int button) {
    return !buttons[button] && ButtonChanged(button);
}

// return if button changed and is now down
bool MOUSE::ButtonWentDown(int button) {
    return buttons[button] && ButtonChanged(button);
}
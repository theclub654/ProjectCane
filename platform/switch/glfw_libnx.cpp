

#include <switch.h>
#include "runtime.h"
#include "deko/backend.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>
#include <thread>

struct GLFWmonitor {};
struct GLFWcursor {};
struct GLFWwindow
{
    bool close = false;
    bool focused = true;
    double x = 0, y = 0;
    int cursorMode = GLFW_CURSOR_NORMAL;
    bool touching = false;
    GLFWframebuffersizefun framebuffer = nullptr;
    GLFWcursorposfun cursor = nullptr;
    GLFWmousebuttonfun mouse = nullptr;
    GLFWscrollfun scroll = nullptr;
    GLFWkeyfun key = nullptr;
    GLFWcharfun character = nullptr;
    GLFWcursorenterfun enter = nullptr;
    GLFWwindowfocusfun focus = nullptr;
};

namespace
{
    GLFWwindow* window = nullptr;
    GLFWmonitor monitor;
    GLFWmonitor* monitors[] = { &monitor };
    GLFWvidmode mode{1280, 720, 8, 8, 8, 60};
    PadState pad;
    HidKeyboardState keyboard{};
    GLFWerrorfun errorCallback = nullptr;
    std::string errorDescription;
    int errorCode = 0;
    std::string clipboard;
    const auto epoch = std::chrono::steady_clock::now();

    void error(const char* message)
    {
        errorCode = GLFW_PLATFORM_ERROR;
        errorDescription = message;
        std::fprintf(stderr, "[WINDOW] %s\n", message);
        if (errorCallback) errorCallback(errorCode, errorDescription.c_str());
    }
    void updateDisplayMode()
    {
        const bool docked = appletGetOperationMode() == AppletOperationMode_Console;
        const int width = docked ? 1920 : 1280, height = docked ? 1080 : 720;
        if (mode.width == width && mode.height == height) return;
        if (window) CaneDeko::ResizeOutput(width, height);
        mode.width = width; mode.height = height;
        if (window && window->framebuffer) window->framebuffer(window, width, height);
    }
    int keyboardUsage(int key)
    {
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) return 4 + key - GLFW_KEY_A;
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) return 30 + key - GLFW_KEY_1;
        if (key == GLFW_KEY_0) return 39;
        if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12) return 58 + key - GLFW_KEY_F1;
        switch (key)
        {
            case GLFW_KEY_ENTER: return 40; case GLFW_KEY_ESCAPE: return 41;
            case GLFW_KEY_BACKSPACE: return 42; case GLFW_KEY_TAB: return 43;
            case GLFW_KEY_SPACE: return 44; case GLFW_KEY_MINUS: return 45;
            case GLFW_KEY_EQUAL: return 46; case GLFW_KEY_LEFT_BRACKET: return 47;
            case GLFW_KEY_RIGHT_BRACKET: return 48; case GLFW_KEY_BACKSLASH: return 49;
            case GLFW_KEY_SEMICOLON: return 51; case GLFW_KEY_APOSTROPHE: return 52;
            case GLFW_KEY_GRAVE_ACCENT: return 53; case GLFW_KEY_COMMA: return 54;
            case GLFW_KEY_PERIOD: return 55; case GLFW_KEY_SLASH: return 56;
            case GLFW_KEY_INSERT: return 73; case GLFW_KEY_HOME: return 74;
            case GLFW_KEY_PAGE_UP: return 75; case GLFW_KEY_DELETE: return 76;
            case GLFW_KEY_END: return 77; case GLFW_KEY_PAGE_DOWN: return 78;
            case GLFW_KEY_RIGHT: return 79; case GLFW_KEY_LEFT: return 80;
            case GLFW_KEY_DOWN: return 81; case GLFW_KEY_UP: return 82;
            case GLFW_KEY_LEFT_CONTROL: return 224; case GLFW_KEY_LEFT_SHIFT: return 225;
            case GLFW_KEY_LEFT_ALT: return 226; case GLFW_KEY_LEFT_SUPER: return 227;
            case GLFW_KEY_RIGHT_CONTROL: return 228; case GLFW_KEY_RIGHT_SHIFT: return 229;
            case GLFW_KEY_RIGHT_ALT: return 230; case GLFW_KEY_RIGHT_SUPER: return 231;
            default: return -1;
        }
    }
    bool keyHeld(const HidKeyboardState& state, int key)
    {
        const int usage = keyboardUsage(key);
        return usage >= 0 && (state.keys[usage / 64] & (u64(1) << (usage % 64)));
    }
}

void CaneSwitchRumble(float low, float high)
{
    const auto id = padIsHandheld(&pad) ? HidNpadIdType_Handheld : HidNpadIdType_No1;
    const u32 styles = padGetStyleSet(&pad);
    const auto style = id == HidNpadIdType_Handheld ? HidNpadStyleTag_NpadHandheld :
        (styles & HidNpadStyleTag_NpadFullKey) ? HidNpadStyleTag_NpadFullKey :
        HidNpadStyleTag_NpadJoyDual;
    HidVibrationDeviceHandle handles[2];
    if (!padIsConnected(&pad) || R_FAILED(hidInitializeVibrationDevices(handles, 2, id, style))) return;
    const HidVibrationValue value{low, 160.0f, high, 320.0f};
    const HidVibrationValue values[] = {value, value};
    hidSendVibrationValues(handles, values, 2);
}

extern "C"
{
int glfwInit(void)
{
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    padUpdate(&pad);
    hidInitializeTouchScreen();
    hidInitializeKeyboard();
    return GLFW_TRUE;
}
void glfwWindowHint(int, int) {}
GLFWwindow* glfwCreateWindow(int, int, const char*, GLFWmonitor*, GLFWwindow*)
{
    if (window) { error("Only one Switch window is available"); return nullptr; }
    updateDisplayMode();
    auto* result = new GLFWwindow;
    CaneDeko::Initialize(mode.width, mode.height);
    window = result;
    return window;
}
void glfwDestroyWindow(GLFWwindow* w)
{
    if (!w) return;
    CaneSwitchRumble(0.0f, 0.0f);
    CaneDeko::Shutdown();
    if (window == w) window = nullptr;
    delete w;
}
void glfwTerminate(void) { if (window) glfwDestroyWindow(window); }
void glfwMakeContextCurrent(GLFWwindow*) {}
GLFWglproc glfwGetProcAddress(const char*) { return nullptr; }
void glfwSwapInterval(int interval) { CaneDeko::SetSwapInterval(interval); }
void glfwSwapBuffers(GLFWwindow* w) { if (w) CaneDeko::Present(); }
int glfwWindowShouldClose(GLFWwindow* w) { return !w || w->close; }
void glfwSetWindowShouldClose(GLFWwindow* w, int close) { if (w) w->close = close != 0; }
void glfwPollEvents(void)
{
    if (!window) return;
    if (!appletMainLoop()) { window->close = true; return; }
    updateDisplayMode();
    padUpdate(&pad);
    const bool focused = appletGetFocusState() == AppletFocusState_InFocus;
    if (focused != window->focused)
    {
        if (!focused) CaneSwitchRumble(0.0f, 0.0f);
        window->focused = focused;
        if (window->focus) window->focus(window, focused);
    }

    constexpr u64 exitChord = HidNpadButton_StickL | HidNpadButton_StickR | HidNpadButton_Minus;
    if ((padGetButtons(&pad) & exitChord) == exitChord) window->close = true;
    const auto oldKeyboard = keyboard;
    if (!hidGetKeyboardStates(&keyboard, 1)) keyboard = {};
    if (window->key)
    {
        for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key)
        {
            const bool previous = keyHeld(oldKeyboard, key), current = keyHeld(keyboard, key);
            if (previous != current) window->key(window, key, keyboardUsage(key), current ? GLFW_PRESS : GLFW_RELEASE, 0);
            if (!previous && current && window->character && key >= GLFW_KEY_SPACE && key <= GLFW_KEY_GRAVE_ACCENT)
            {
                unsigned int character = unsigned(key);
                if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z && !keyHeld(keyboard, GLFW_KEY_LEFT_SHIFT) && !keyHeld(keyboard, GLFW_KEY_RIGHT_SHIFT)) character += 32;
                window->character(window, character);
            }
        }
    }
    HidTouchScreenState touch{};
    const bool touching = hidGetTouchScreenStates(&touch, 1) && touch.count > 0;
    if (touching)
    {
        window->x = touch.touches[0].x; window->y = touch.touches[0].y;
        if (window->cursor) window->cursor(window, window->x, window->y);
    }
    if (touching != window->touching && window->mouse)
        window->mouse(window, GLFW_MOUSE_BUTTON_LEFT, touching ? GLFW_PRESS : GLFW_RELEASE, 0);
    window->touching = touching;
}
int glfwGetKey(GLFWwindow* w, int key) { return w && w->focused && keyHeld(keyboard, key) ? GLFW_PRESS : GLFW_RELEASE; }
void glfwWaitEventsTimeout(double seconds)
{
    if (std::isfinite(seconds) && seconds > 0)
        std::this_thread::sleep_for(std::chrono::duration<double>(std::min(seconds, 1.0)));
    glfwPollEvents();
}
int glfwGetMouseButton(GLFWwindow* w, int button) { return w && button == GLFW_MOUSE_BUTTON_LEFT && w->touching ? GLFW_PRESS : GLFW_RELEASE; }
int glfwJoystickPresent(int jid) { return jid == GLFW_JOYSTICK_1 && padIsConnected(&pad); }
int glfwJoystickIsGamepad(int jid) { return glfwJoystickPresent(jid); }
int glfwGetGamepadState(int jid, GLFWgamepadstate* state)
{
    if (!state || !glfwJoystickPresent(jid)) return GLFW_FALSE;
    std::memset(state, 0, sizeof(*state));
    const u64 keys = (!window || window->focused) ? padGetButtons(&pad) : 0;
    const u64 mapping[] = { HidNpadButton_B, HidNpadButton_A, HidNpadButton_Y, HidNpadButton_X,
        HidNpadButton_L, HidNpadButton_R, HidNpadButton_Minus, HidNpadButton_Plus, 0,
        HidNpadButton_StickL, HidNpadButton_StickR, HidNpadButton_Up, HidNpadButton_Right,
        HidNpadButton_Down, HidNpadButton_Left };
    for (unsigned i = 0; i < std::size(mapping); ++i) state->buttons[i] = (keys & mapping[i]) ? GLFW_PRESS : GLFW_RELEASE;
    if (!window || window->focused)
    {
        const auto left = padGetStickPos(&pad, 0), right = padGetStickPos(&pad, 1);
        state->axes[GLFW_GAMEPAD_AXIS_LEFT_X] = left.x / 32767.0f;
        state->axes[GLFW_GAMEPAD_AXIS_LEFT_Y] = -left.y / 32767.0f;
        state->axes[GLFW_GAMEPAD_AXIS_RIGHT_X] = right.x / 32767.0f;
        state->axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] = -right.y / 32767.0f;
    }
    state->axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] = (keys & HidNpadButton_ZL) ? 1.0f : -1.0f;
    state->axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] = (keys & HidNpadButton_ZR) ? 1.0f : -1.0f;
    return GLFW_TRUE;
}
const float* glfwGetJoystickAxes(int jid, int* count)
{
    static GLFWgamepadstate state;
    const bool available = glfwGetGamepadState(jid, &state) == GLFW_TRUE;
    if (count) *count = available ? GLFW_GAMEPAD_AXIS_LAST + 1 : 0;
    return available ? state.axes : nullptr;
}
const unsigned char* glfwGetJoystickButtons(int jid, int* count)
{
    static GLFWgamepadstate state;
    const bool available = glfwGetGamepadState(jid, &state) == GLFW_TRUE;
    if (count) *count = available ? GLFW_GAMEPAD_BUTTON_LAST + 1 : 0;
    return available ? state.buttons : nullptr;
}
const unsigned char* glfwGetJoystickHats(int jid, int* count)
{
    static unsigned char hat;
    GLFWgamepadstate state;
    const bool available = glfwGetGamepadState(jid, &state) == GLFW_TRUE;
    hat = GLFW_HAT_CENTERED;
    if (available)
    {
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP]) hat |= GLFW_HAT_UP;
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]) hat |= GLFW_HAT_RIGHT;
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]) hat |= GLFW_HAT_DOWN;
        if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]) hat |= GLFW_HAT_LEFT;
    }
    if (count) *count = available ? 1 : 0;
    return available ? &hat : nullptr;
}
const char* glfwGetGamepadName(int jid) { return glfwJoystickPresent(jid) ? "Nintendo Switch" : nullptr; }
double glfwGetTime(void) { return std::chrono::duration<double>(std::chrono::steady_clock::now() - epoch).count(); }
int glfwGetError(const char** description) { if (description) *description = errorDescription.c_str(); return std::exchange(errorCode, 0); }
GLFWerrorfun glfwSetErrorCallback(GLFWerrorfun callback) { return std::exchange(errorCallback, callback); }
void glfwGetFramebufferSize(GLFWwindow*, int* w, int* h) { if (w) *w = mode.width; if (h) *h = mode.height; }
void glfwGetWindowSize(GLFWwindow* win, int* w, int* h) { glfwGetFramebufferSize(win, w, h); }
void glfwGetWindowPos(GLFWwindow*, int* x, int* y) { if (x) *x = 0; if (y) *y = 0; }
void glfwSetWindowPos(GLFWwindow*, int, int) {}
void glfwIconifyWindow(GLFWwindow*) {}
const char* glfwGetMonitorName(GLFWmonitor*) { return "Nintendo Switch"; }
void glfwGetWindowContentScale(GLFWwindow*, float* x, float* y) { if (x) *x = 1; if (y) *y = 1; }
int glfwGetWindowAttrib(GLFWwindow* w, int attribute)
{
    if (attribute == GLFW_FOCUSED) return w && w->focused;
    if (attribute == GLFW_ICONIFIED || attribute == GLFW_DECORATED) return GLFW_FALSE;
    return GLFW_TRUE;
}
void glfwSetWindowAttrib(GLFWwindow*, int, int) {}
GLFWmonitor* glfwGetWindowMonitor(GLFWwindow*) { return &monitor; }
GLFWmonitor* glfwGetPrimaryMonitor(void) { return &monitor; }
GLFWmonitor** glfwGetMonitors(int* count) { if (count) *count = 1; return monitors; }
const GLFWvidmode* glfwGetVideoMode(GLFWmonitor*) { return &mode; }
const GLFWvidmode* glfwGetVideoModes(GLFWmonitor*, int* count) { if (count) *count = 1; return &mode; }
void glfwGetMonitorPos(GLFWmonitor*, int* x, int* y) { if (x) *x = 0; if (y) *y = 0; }
void glfwGetMonitorWorkarea(GLFWmonitor* m, int* x, int* y, int* w, int* h) { glfwGetMonitorPos(m, x, y); glfwGetFramebufferSize(window, w, h); }
void glfwSetWindowMonitor(GLFWwindow* w, GLFWmonitor*, int, int, int, int, int) { if (w && w->framebuffer) w->framebuffer(w, mode.width, mode.height); }
int glfwGetInputMode(GLFWwindow* w, int mode) { return w && mode == GLFW_CURSOR ? w->cursorMode : GLFW_FALSE; }
void glfwSetInputMode(GLFWwindow* w, int mode, int value) { if (w && mode == GLFW_CURSOR) w->cursorMode = value; }
void glfwGetCursorPos(GLFWwindow* w, double* x, double* y) { if (x) *x = w ? w->x : 0; if (y) *y = w ? w->y : 0; }
void glfwSetCursorPos(GLFWwindow* w, double x, double y) { if (w) { w->x = x; w->y = y; } }
GLFWcursor* glfwCreateStandardCursor(int) { return new GLFWcursor; }
void glfwDestroyCursor(GLFWcursor* cursor) { delete cursor; }
void glfwSetCursor(GLFWwindow*, GLFWcursor*) {}
const char* glfwGetClipboardString(GLFWwindow*) { return clipboard.c_str(); }
void glfwSetClipboardString(GLFWwindow*, const char* text) { clipboard = text ? text : ""; }
const char* glfwGetKeyName(int key, int)
{
    static char name[2];
    if (key >= GLFW_KEY_SPACE && key <= GLFW_KEY_GRAVE_ACCENT) { name[0] = char(key); name[1] = 0; return name; }
    return nullptr;
}
#define CANE_CALLBACK(function, type, field) type function(GLFWwindow* w, type cb) { return w ? std::exchange(w->field, cb) : nullptr; }
CANE_CALLBACK(glfwSetFramebufferSizeCallback, GLFWframebuffersizefun, framebuffer)
CANE_CALLBACK(glfwSetCursorPosCallback, GLFWcursorposfun, cursor)
CANE_CALLBACK(glfwSetMouseButtonCallback, GLFWmousebuttonfun, mouse)
CANE_CALLBACK(glfwSetScrollCallback, GLFWscrollfun, scroll)
CANE_CALLBACK(glfwSetKeyCallback, GLFWkeyfun, key)
CANE_CALLBACK(glfwSetCharCallback, GLFWcharfun, character)
CANE_CALLBACK(glfwSetCursorEnterCallback, GLFWcursorenterfun, enter)
CANE_CALLBACK(glfwSetWindowFocusCallback, GLFWwindowfocusfun, focus)
GLFWmonitorfun glfwSetMonitorCallback(GLFWmonitorfun cb) { static GLFWmonitorfun callback = nullptr; return std::exchange(callback, cb); }
}

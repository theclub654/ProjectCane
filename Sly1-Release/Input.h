#pragma once
#include "unordered_map"
#include <array>
#include "gl.h"
#include "game.h"

typedef int GRFUSR;

struct CODE
{
    const uint16_t* ajbc;
    int cjbc;
    void (*pfn)(uint32_t);
    uint32_t nParam;
    int ijbcCur = 0;
    int fRemove = 0;
    CODE* pcodeNext = nullptr;
};

enum JOY_BUTTON
{
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_SQUARE,
    BTN_CIRCLE,
    BTN_CROSS,
    BTN_TRIANGLE,
    BTN_START,
    BTN_SELECT,
    BTN_L1,
    BTN_R1,
    BTN_L2,
    BTN_R2,
    BTN_L3,
    BTN_R3,
    BTN_MAX
};

enum RUMK 
{
    RUMK_Nil = -1,
    RUMK_SteadyBuzz = 0,
    RUMK_LowThrob = 1,
    RUMK_MediumThrob = 2,
    RUMK_HardThrob = 3,
    RUMK_Blunt = 4,
    RUMK_Electric = 5,
    RUMK_Fire = 6,
    RUMK_Water = 7,
    RUMK_Crush = 8,
    RUMK_Break = 9,
    RUMK_Bomb = 10,
    RUMK_Max = 11
};

enum RUMS
{
    RUMS_Nil = 0,
    RUMS_Idle = 1,
    RUMS_Rumble = 2,
    RUMS_Stop = 3,
    RUMS_Kill = 4,
    RUMS_Dead = 5
};

struct RUMINS
{
    float dt;
    uint8_t fHighSpeedMotor;
    uint8_t bLowSpeedMotor;
};

struct RUMPAT
{
    const RUMINS* arumins;
    int crumins;
};

struct RUMBLE
{
    RUMS rums = RUMS_Idle;
    const RUMPAT* prumpat = nullptr;
    int irumins = 0;
    float dtRumins = 0.0f;
    float dtRumble = 0.0f;
};

enum JOYID : int
{
    JOYID_Nil = -1,
    JOYID_Left = 0,
    JOYID_Right = 1,
    JOYID_Max = 2
};

enum JOYS 
{
    JOYS_Nil = -1,
    JOYS_Initing = 0,
    JOYS_Searching = 1,
    JOYS_Waiting = 2,
    JOYS_Ready = 3,
    JOYS_Max = 4
};

struct JOY 
{
    struct StickCalibration
    {
        std::array<float, 4> minimum{ -0.75f, -0.75f, -0.75f, -0.75f };
        std::array<float, 4> maximum{  0.75f,  0.75f,  0.75f,  0.75f };
        bool hasPreviousSample = false;
        bool moved = false;
        uint8_t previousX = 128;
        uint8_t previousY = 128;
    };

    std::unordered_map<JOY_BUTTON, bool> current;
    std::unordered_map<JOY_BUTTON, bool> previous;
    std::unordered_map<JOY_BUTTON, bool> handled;
    std::unordered_map<JOY_BUTTON, bool> forcedHeld;

    float stick = 0.0f;
    float stickLatch = 0.0f;

    int dxLatch = 0;
    float tLatchX = 0.0f;
    float dtLatchX = 0.3333f;

    int dyLatch = 0;
    float tLatchY = 0.0f;
    float dtLatchY = 0.3333f;

    float uDeflect = 0.0f;
    float uDeflect2 = 0.0f;
    float x = 0.0f;
    float y = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;

    StickCalibration calibration;
    StickCalibration calibration2;

    JOYS joys = JOYS_Ready;
    int joystickId = JOYID_Nil;
    bool gamepadConnected = false;
    // The binding UI reads the same normalized state selected by gameplay.
    // joystickId stores SDL's stable joystick instance ID.
    GLFWgamepadstate gamepadState{};
    bool hasGamepadState = false;
    RUMBLE rumble;

    void Update(GLFWwindow* window);
    bool IsPressed(JOY_BUTTON button);
    bool IsHeld(JOY_BUTTON button);
    bool IsReleased(JOY_BUTTON button);
    void SetHandled(JOY_BUTTON button);
    int  DxSelectionJoy(float currentTime);
    int  DySelectionJoy(float currentTime);
    void StartJoySelection();
};

void AddGrfusr(GRFUSR grfusr);
void RemoveGrfusr(GRFUSR grfusr);
void UpdateGrfjoytFromGrfusr();
void AddCode(CODE* pcode);
void _ResetCodes();
void _MatchCodes(JOY* pjoy, uint16_t grfbtnPrev);
void UpdateCodes();
void StartupCodes();
bool StartupControllerInput();
void ShutdownControllerInput();
bool FControllerInputAvailable();
void TriggerJoyRumbleRumk(JOY* pjoy, RUMK rumk, float dt);
void StopJoyRumble(JOY* pjoy);
void UpdateRumble(RUMBLE* prumble, JOY* pjoy);
void EnableVibration();
void ToggleVibration(GS* pgs);
void ApplyVibrationSetting(GS* pgs);
bool FVibrationEnabled(GS* pgs);


extern float g_controllerDeadzone;

static void ApplyStickDeadZone(float xRaw, float yRaw, JOY::StickCalibration* calibration,
    float* px, float* py, float* puDeflect)
{
    // Retail GetJoyXYDeflection measures the stick against the cardinal and
    // diagonal gate directions, expanding a separate positive/negative limit
    // for each direction. SetJoyJoys initializes every limit to +/-0.75.
    constexpr std::array<std::array<float, 2>, 4> axes =
    {{
        {{ 1.0f,  0.0f }},
        {{ 0.0f,  1.0f }},
        {{ 0.7f,  0.7f }},
        {{ 0.7f, -0.7f }}
    }};

    const uint8_t rawX = static_cast<uint8_t>(glm::clamp(
        std::lround((xRaw + 1.0f) * 127.5f), 0L, 255L));
    const uint8_t rawY = static_cast<uint8_t>(glm::clamp(
        std::lround((1.0f - yRaw) * 127.5f), 0L, 255L));

    if (!calibration->moved)
    {
        if (calibration->hasPreviousSample &&
            (rawX != calibration->previousX || rawY != calibration->previousY ||
             rawX < 96 || rawX >= 161 || rawY < 96 || rawY >= 161))
        {
            calibration->moved = true;
        }

        calibration->hasPreviousSample = true;
        calibration->previousX = rawX;
        calibration->previousY = rawY;

        if (!calibration->moved)
        {
            *px = 0.0f;
            *py = 0.0f;
            *puDeflect = 0.0f;
            return;
        }
    }
    else
    {
        calibration->previousX = rawX;
        calibration->previousY = rawY;
    }

    float normalizedDeflection = 0.0f;

    for (size_t i = 0; i < axes.size(); ++i)
    {
        const float projection = xRaw * axes[i][0] + yRaw * axes[i][1];
        float limit;

        if (projection >= 0.0f)
        {
            calibration->maximum[i] = (std::max)(calibration->maximum[i], projection);
            limit = calibration->maximum[i];
        }
        else
        {
            calibration->minimum[i] = (std::min)(calibration->minimum[i], projection);
            limit = calibration->minimum[i];
        }

        normalizedDeflection = (std::max)(normalizedDeflection, projection / limit);
    }

    const float magnitudeRaw = std::sqrt(xRaw * xRaw + yRaw * yRaw);
    const float deadzone = glm::clamp(g_controllerDeadzone, 0.0f, 0.50f);
    // Preserve retail ProjectCane's existing outer response: with the default
    // 35% deadzone, full deflection is reached at 85%, exactly as before this
    // setting became configurable.
    const float deflection = glm::clamp(
        (normalizedDeflection - deadzone) * 2.0f, 0.0f, 1.0f);

    if (magnitudeRaw > 0.0001f)
    {
        *px = xRaw * deflection / magnitudeRaw;
        *py = yRaw * deflection / magnitudeRaw;
    }
    else
    {
        *px = 0.0f;
        *py = 0.0f;
    }

    *puDeflect = deflection;
}

extern JOY g_joy;
extern JOY g_joyZero;
extern int g_grfjoyt;
extern GRFUSR g_grfusr;
extern int vibrationSetting;
extern std::array<int, BTN_MAX> g_keyboardBindings;
extern std::array<int, BTN_MAX> g_gamepadBindings;
void ResetKeyboardMouseBindings();
void ResetControllerBindings();
extern bool g_fDisableControllerInputWhenUnfocused;
extern bool g_fMouseCamera;
extern float g_mouseCameraSensitivity;

inline constexpr int GAMEPAD_BINDING_LEFT_TRIGGER = 1000;
inline constexpr int GAMEPAD_BINDING_RIGHT_TRIGGER = 1001;
// Mouse buttons share the keyboard binding table so each gameplay action still
// has one PC-side binding. Values are kept outside GLFW's key range and away
// from GLFW_KEY_UNKNOWN (-1).
inline constexpr int MOUSE_BINDING_BASE = -1000;

inline constexpr int MouseBindingFromButton(int button)
{
    return MOUSE_BINDING_BASE - button;
}

inline constexpr bool FMouseBinding(int binding)
{
    return binding <= MOUSE_BINDING_BASE &&
        binding >= MOUSE_BINDING_BASE - GLFW_MOUSE_BUTTON_LAST;
}

inline constexpr int MouseButtonFromBinding(int binding)
{
    return MOUSE_BINDING_BASE - binding;
}

const char* PchzKeyboardKeyName(int key);
const char* PchzGamepadBindingName(int binding);

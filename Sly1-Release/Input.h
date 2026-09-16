#pragma once
#include "unordered_map"
#include "gl.h"
#include "game.h"

typedef int GRFUSR;

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

enum JOYID 
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

    JOYS joys = JOYS_Ready;
    int joystickId = JOYID_Nil;
    bool gamepadConnected = false;

    void Update(GLFWwindow* window);
    bool FindGamepad();
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
void EnableVibration();
void ToggleVibration(GS* pgs);
void ApplyVibrationSetting(GS* pgs);
bool FVibrationEnabled(GS* pgs);


static void ApplyStickDeadZone(float xRaw, float yRaw, float* px, float* py, float* puDeflect)
{
    constexpr float deadZone = 0.35f;

    const float magnitudeRaw = std::sqrt(xRaw * xRaw + yRaw * yRaw);

    if (magnitudeRaw <= deadZone)
    {
        *px = 0.0f;
        *py = 0.0f;
        *puDeflect = 0.0f;
        return;
    }

    const float magnitude = glm::clamp((magnitudeRaw - deadZone) / (1.0f - deadZone), 0.0f, 1.0f);
    const float inverseMagnitude = magnitudeRaw > 0.0001f ? 1.0f / magnitudeRaw : 0.0f;

    *px = xRaw * inverseMagnitude * magnitude;
    *py = yRaw * inverseMagnitude * magnitude;
    *puDeflect = magnitude;
}

extern JOY g_joy;
extern JOY g_joyZero;
extern int g_grfjoyt;
extern GRFUSR g_grfusr;
extern int vibrationSetting;

#pragma once
#include "unordered_map"
#include "gl.h"

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
    BTN_MAX
};

struct JOY 
{
    std::unordered_map <JOY_BUTTON, bool> current;
    std::unordered_map <JOY_BUTTON, bool> previous;

    float stick;      // -1 to 1 for analog simulation
    float stickLatch; // velocity or acceleration if needed

    int   dxLatch = 0;
    float tLatchX = 0.0f;
    float dtLatchX = 0.3333f; // Initial delay

    int   dyLatch = 0;
    float tLatchY = 0.0f;
    float dtLatchY = 0.3333f;

    void Update(GLFWwindow* window);
    bool IsPressed(JOY_BUTTON button);
    bool IsHeld(JOY_BUTTON button);
    bool IsReleased(JOY_BUTTON button);
    int  DxSelectionJoy(float currentTime);
    int  DySelectionJoy(float currentTime);
    void StartJoySelection();
};

extern JOY g_joy;
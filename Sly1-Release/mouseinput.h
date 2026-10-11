#pragma once

// -----------------------------------------------------------------------------
// Mouse support (Mouse.ini next to the exe):
//
//   * Mouse camera      While playing (no menu open) the cursor is captured and
//                       mouse movement orbits the third-person camera and aims
//                       the binoc-u-com / turrets. Separate sensitivities,
//                       invert X / Y, raw input.
//   * Mouse bindings    Any mouse button (1-8) and the wheel can be bound in
//                       Controls > Keyboard Mapping like a key.
//   * Mouse menus       Hover highlights an item, left click selects it, right
//                       click goes back, the wheel adjusts slider options.
// -----------------------------------------------------------------------------

struct GLFWwindow;

enum MOUSEOPT
{
	MOUSEOPT_Camera = 0,       // mouse camera on/off
	MOUSEOPT_Sensitivity,      // third-person camera, percent
	MOUSEOPT_AimSensitivity,   // binoc-u-com / turrets, percent
	MOUSEOPT_InvertX,
	MOUSEOPT_InvertY,
	MOUSEOPT_RawInput,
	MOUSEOPT_WheelZoom,        // wheel zooms the binoc-u-com (when the wheel is unbound)
	MOUSEOPT_Menus,            // mouse hover / click in menus
	MOUSEOPT_Max
};

// Keyboard bindings store these codes for mouse buttons (outside GLFW key range).
inline constexpr int MOUSE_BINDING_BASE = 2000;                       // + GLFW mouse button 0..7
inline constexpr int MOUSE_BINDING_WHEEL_UP = MOUSE_BINDING_BASE + 8;
inline constexpr int MOUSE_BINDING_WHEEL_DOWN = MOUSE_BINDING_BASE + 9;
inline constexpr int MOUSE_BINDING_LAST = MOUSE_BINDING_WHEEL_DOWN;
inline bool FMouseBinding(int binding) { return binding >= MOUSE_BINDING_BASE && binding <= MOUSE_BINDING_LAST; }

void StartupMouseInput();
void UpdateMouseInput(GLFWwindow* window);     // once per frame, before g_joy.Update

// Bindings
bool FMouseBindingHeld(int binding);            // state for this frame
int MouseBindingPressed(GLFWwindow* window);    // for binding capture: code or -1
const char* PchzMouseBindingName(int binding);

// Camera (values for this frame, already scaled; 0 when the mouse camera is off)
bool FMouseCaptured();
float MouseCameraOrbit();                       // radians; + = same direction as "stick right"
void MouseAimDelta(float* pdxRight, float* pdyUp); // radians for binoc-u-com / turrets
float MouseWheelZoom();                         // wheel notches (+ = zoom in), 0 if disabled/bound

// Menus
bool FMouseMenuEnabled();
bool MouseMenuCursor(float* px, float* py);     // cursor in menu (output) pixels; false if unavailable
bool FMouseMenuMoved();
bool FMouseLeftPressed();
bool FMouseRightPressed();
bool FMouseLeftHeld();
int MouseWheelNotches();                        // raw notches this frame (+ = up)

// Options menu
int MouseOptValue(MOUSEOPT opt);
void AdjustMouseOpt(MOUSEOPT opt, int dir, bool fWrap);
void RefreshMouseStrings();
bool MouseOptNumericRange(MOUSEOPT opt, int* pmin, int* pmax, int* pstep);   // percent options only
void SetMouseOpt(MOUSEOPT opt, int value);
void ResetMouseInput();                         // Options > Reset to Default
int MouseOptDefault(MOUSEOPT opt);

// Last input device used (for the button prompt bar).
enum INPUTDEV { INPUTDEV_Keyboard = 0, INPUTDEV_Mouse, INPUTDEV_Gamepad };
void NoteInputDevice(INPUTDEV dev);
INPUTDEV LastInputDevice();
extern char g_achzMouseOpt[MOUSEOPT_Max][64];

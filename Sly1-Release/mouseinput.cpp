// Mouse camera, mouse bindings and mouse menu input. See mouseinput.h.

#include "mouseinput.h"

#include "main.h"
#include "gl.h"
#include "cm.h"
#include "ui.h"
#include "prompt.h"
#include "Input.h"
#include "imgui/imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <string>

namespace
{
struct MOUSEDEF
{
	const char* iniKey;
	const char* label;
	int def;
	int min;
	int max;
	int step;
	bool fPercent;
};

const MOUSEDEF s_amousedef[MOUSEOPT_Max] =
{
	{ "Camera",         "Mouse Camera",           1,  0,   1,  1, false },
	{ "Sensitivity",    "Mouse Sensitivity",    100,  1, 1000, 5, true  },
	{ "AimSensitivity", "Mouse Aim Sensitivity", 100, 1, 1000, 5, true  },
	{ "InvertX",        "Invert X",               0,  0,   1,  1, false },
	{ "InvertY",        "Invert Y",               0,  0,   1,  1, false },
	{ "RawInput",       "Raw Input",              1,  0,   1,  1, false },
	{ "WheelZoom",      "Wheel Zooms Binoc-u-com", 1, 0,   1,  1, false },
	{ "Menus",          "Mouse In Menus",         1,  0,   1,  1, false },
};

// Radians of camera turn per mouse count at 100%.
constexpr float kRadPerCountOrbit = 0.0030f;
constexpr float kRadPerCountAim = 0.0012f;

int s_av[MOUSEOPT_Max] = {};

bool s_fCaptured = false;
bool s_fHaveLast = false;
double s_xLast = 0.0;
double s_yLast = 0.0;
float s_dxCounts = 0.0f;   // + = mouse moved right
float s_dyCounts = 0.0f;   // + = mouse moved down
int s_wheel = 0;           // + = wheel up

constexpr int kMouseButtons = GLFW_MOUSE_BUTTON_LAST + 1;
bool s_afButton[kMouseButtons] = {};
bool s_afButtonPrev[kMouseButtons] = {};
bool s_fImGuiMouse = false;

bool s_fMenuCursorValid = false;
float s_xMenu = 0.0f;
float s_yMenu = 0.0f;
float s_xMenuLast = -1.0f;
float s_yMenuLast = -1.0f;
bool s_fMenuMoved = false;

std::wstring IniPath()
{
	wchar_t path[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	std::wstring dir(path);
	const size_t slash = dir.find_last_of(L"\\/");
	if (slash != std::wstring::npos)
		dir.resize(slash + 1);
	return dir + L"Mouse.ini";
}

void Save()
{
	const std::wstring ini = IniPath();
	for (int i = 0; i < MOUSEOPT_Max; ++i)
	{
		const std::string key = s_amousedef[i].iniKey;
		WritePrivateProfileStringW(L"Mouse", std::wstring(key.begin(), key.end()).c_str(),
			std::to_wstring(s_av[i]).c_str(), ini.c_str());
	}
}

bool FWheelBound()
{
	for (int key : g_keyboardBindings)
		if (key == MOUSE_BINDING_WHEEL_UP || key == MOUSE_BINDING_WHEEL_DOWN)
			return true;
	return false;
}

bool FWantCapture(GLFWwindow* window)
{
	if (s_av[MOUSEOPT_Camera] == 0 || window == nullptr)
		return false;
	if (glfwGetWindowAttrib(window, GLFW_FOCUSED) != GLFW_TRUE)
		return false;
	if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) == GLFW_TRUE)
		return false;
	if (g_fDebugGuiOpen)
		return false;
	if (g_psw == nullptr || g_ui.uis != UIS_Playing)
		return false;
	if (g_prompt.blots != BLOTS_Hidden)
		return false;
	// The debug free camera manages the cursor itself.
	if (g_pcm != nullptr && g_pcm->ccpr > 0 && g_pcm->acpr[0].cpp == CPP_ManualOverride)
		return false;
	return true;
}

void SetCapture(GLFWwindow* window, bool fCapture)
{
	if (fCapture)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, s_av[MOUSEOPT_RawInput] != 0 ? GLFW_TRUE : GLFW_FALSE);
	}
	else
	{
		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	s_fCaptured = fCapture;
	s_fHaveLast = false;
}
} // namespace

char g_achzMouseOpt[MOUSEOPT_Max][64] = {};

void StartupMouseInput()
{
	const std::wstring ini = IniPath();
	for (int i = 0; i < MOUSEOPT_Max; ++i)
	{
		const MOUSEDEF& def = s_amousedef[i];
		const std::string key = def.iniKey;
		const int value = static_cast<int>(GetPrivateProfileIntW(L"Mouse",
			std::wstring(key.begin(), key.end()).c_str(), def.def, ini.c_str()));
		s_av[i] = std::clamp(value, def.min, def.max);
	}
	Save();
	RefreshMouseStrings();
}

void UpdateMouseInput(GLFWwindow* window)
{
	s_dxCounts = 0.0f;
	s_dyCounts = 0.0f;
	s_wheel = 0;
	s_fMenuMoved = false;
	s_fMenuCursorValid = false;
	if (window == nullptr)
		return;

	// Wheel notches since last frame (mouse.cpp accumulates them).
	const double wheel = MOUSE::GetScrollDY();
	s_wheel = static_cast<int>(std::lround(wheel));
	if (s_wheel == 0 && wheel != 0.0)
		s_wheel = wheel > 0.0 ? 1 : -1;

	// ImGui (debug menu) owns the mouse while it is hovering its windows.
	s_fImGuiMouse = g_fDebugGuiOpen && ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse;

	const bool fFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
	for (int button = 0; button < kMouseButtons; ++button)
	{
		s_afButtonPrev[button] = s_afButton[button];
		s_afButton[button] = fFocused && !s_fImGuiMouse && glfwGetMouseButton(window, button) == GLFW_PRESS;
	}

	const bool fWant = FWantCapture(window);
	if (fWant != s_fCaptured)
		SetCapture(window, fWant);

	double x = 0.0;
	double y = 0.0;
	glfwGetCursorPos(window, &x, &y);

	if (s_fCaptured)
	{
		if (s_fHaveLast)
		{
			const double dx = x - s_xLast;
			const double dy = y - s_yLast;
			// Ignore the jump GLFW can report right after capture changes.
			if (std::abs(dx) < 4000.0 && std::abs(dy) < 4000.0)
			{
				s_dxCounts = static_cast<float>(dx);
				s_dyCounts = static_cast<float>(dy);
			}
		}
		s_xLast = x;
		s_yLast = y;
		s_fHaveLast = true;
		return;
	}

	// Free cursor: convert window coordinates to menu (output) pixels.
	int windowW = 0, windowH = 0, fbW = 0, fbH = 0;
	glfwGetWindowSize(window, &windowW, &windowH);
	glfwGetFramebufferSize(window, &fbW, &fbH);
	if (!fFocused || windowW <= 0 || windowH <= 0 || fbW <= 0 || fbH <= 0)
		return;

	const float fbX = static_cast<float>(x) * static_cast<float>(fbW) / static_cast<float>(windowW);
	const float fbY = static_cast<float>(y) * static_cast<float>(fbH) / static_cast<float>(windowH);
	// The menu is drawn in the present viewport; scale in case the output
	// framebuffer differs from the window framebuffer (DLSS present window).
	const float outScaleX = static_cast<float>((std::max)(g_gl.outputWidth, 1)) / static_cast<float>(fbW);
	const float outScaleY = static_cast<float>((std::max)(g_gl.outputHeight, 1)) / static_cast<float>(fbH);
	s_xMenu = fbX * outScaleX - static_cast<float>(g_gl.presentX);
	s_yMenu = fbY * outScaleY - static_cast<float>(g_gl.presentY);
	s_fMenuCursorValid = true;

	if (std::abs(s_xMenu - s_xMenuLast) >= 1.0f || std::abs(s_yMenu - s_yMenuLast) >= 1.0f)
	{
		s_fMenuMoved = s_xMenuLast >= 0.0f;   // first sample is not a move
		s_xMenuLast = s_xMenu;
		s_yMenuLast = s_yMenu;
		if (s_fMenuMoved)
			NoteInputDevice(INPUTDEV_Mouse);
	}
	for (int button = 0; button < kMouseButtons; ++button)
		if (s_afButton[button] && !s_afButtonPrev[button])
			NoteInputDevice(INPUTDEV_Mouse);
}

// -----------------------------------------------------------------------------
// Bindings
// -----------------------------------------------------------------------------

bool FMouseBindingHeld(int binding)
{
	if (binding == MOUSE_BINDING_WHEEL_UP)
		return s_wheel > 0;
	if (binding == MOUSE_BINDING_WHEEL_DOWN)
		return s_wheel < 0;
	const int button = binding - MOUSE_BINDING_BASE;
	if (button < 0 || button >= kMouseButtons)
		return false;
	return s_afButton[button];
}

int MouseBindingPressed(GLFWwindow* window)
{
	if (window == nullptr || glfwGetWindowAttrib(window, GLFW_FOCUSED) != GLFW_TRUE)
		return -1;
	for (int button = 0; button < kMouseButtons; ++button)
		if (glfwGetMouseButton(window, button) == GLFW_PRESS)
			return MOUSE_BINDING_BASE + button;
	if (s_wheel > 0)
		return MOUSE_BINDING_WHEEL_UP;
	if (s_wheel < 0)
		return MOUSE_BINDING_WHEEL_DOWN;
	return -1;
}

const char* PchzMouseBindingName(int binding)
{
	switch (binding)
	{
	case MOUSE_BINDING_BASE + 0: return "Left Mouse";
	case MOUSE_BINDING_BASE + 1: return "Right Mouse";
	case MOUSE_BINDING_BASE + 2: return "Middle Mouse";
	case MOUSE_BINDING_BASE + 3: return "Mouse 4 (Back)";
	case MOUSE_BINDING_BASE + 4: return "Mouse 5 (Forward)";
	case MOUSE_BINDING_BASE + 5: return "Mouse 6";
	case MOUSE_BINDING_BASE + 6: return "Mouse 7";
	case MOUSE_BINDING_BASE + 7: return "Mouse 8";
	case MOUSE_BINDING_WHEEL_UP: return "Wheel Up";
	case MOUSE_BINDING_WHEEL_DOWN: return "Wheel Down";
	default: return "Mouse";
	}
}

// -----------------------------------------------------------------------------
// Camera
// -----------------------------------------------------------------------------

bool FMouseCaptured()
{
	return s_fCaptured;
}

float MouseCameraOrbit()
{
	if (!s_fCaptured)
		return 0.0f;
	const float sign = s_av[MOUSEOPT_InvertX] != 0 ? -1.0f : 1.0f;
	return s_dxCounts * kRadPerCountOrbit * (static_cast<float>(s_av[MOUSEOPT_Sensitivity]) / 100.0f) * sign;
}

void MouseAimDelta(float* pdxRight, float* pdyUp)
{
	float dx = 0.0f;
	float dy = 0.0f;
	if (s_fCaptured)
	{
		const float scale = kRadPerCountAim * (static_cast<float>(s_av[MOUSEOPT_AimSensitivity]) / 100.0f);
		dx = s_dxCounts * scale * (s_av[MOUSEOPT_InvertX] != 0 ? -1.0f : 1.0f);
		dy = -s_dyCounts * scale * (s_av[MOUSEOPT_InvertY] != 0 ? -1.0f : 1.0f);
	}
	if (pdxRight != nullptr)
		*pdxRight = dx;
	if (pdyUp != nullptr)
		*pdyUp = dy;
}

float MouseWheelZoom()
{
	if (!s_fCaptured || s_av[MOUSEOPT_WheelZoom] == 0 || FWheelBound())
		return 0.0f;
	return static_cast<float>(s_wheel);
}

// -----------------------------------------------------------------------------
// Menus
// -----------------------------------------------------------------------------

bool FMouseMenuEnabled()
{
	return s_av[MOUSEOPT_Menus] != 0 && !s_fCaptured && !s_fImGuiMouse;
}

bool MouseMenuCursor(float* px, float* py)
{
	if (!FMouseMenuEnabled() || !s_fMenuCursorValid)
		return false;
	*px = s_xMenu;
	*py = s_yMenu;
	return true;
}

bool FMouseMenuMoved()
{
	return FMouseMenuEnabled() && s_fMenuMoved;
}

bool FMouseLeftPressed()
{
	return FMouseMenuEnabled() && s_afButton[GLFW_MOUSE_BUTTON_LEFT] && !s_afButtonPrev[GLFW_MOUSE_BUTTON_LEFT];
}

bool FMouseLeftHeld()
{
	return FMouseMenuEnabled() && s_afButton[GLFW_MOUSE_BUTTON_LEFT];
}

bool FMouseRightPressed()
{
	return FMouseMenuEnabled() && s_afButton[GLFW_MOUSE_BUTTON_RIGHT] && !s_afButtonPrev[GLFW_MOUSE_BUTTON_RIGHT];
}

int MouseWheelNotches()
{
	return FMouseMenuEnabled() ? s_wheel : 0;
}

// -----------------------------------------------------------------------------
// Options
// -----------------------------------------------------------------------------

int MouseOptValue(MOUSEOPT opt)
{
	return (opt >= 0 && opt < MOUSEOPT_Max) ? s_av[opt] : 0;
}

void AdjustMouseOpt(MOUSEOPT opt, int dir, bool fWrap)
{
	if (opt < 0 || opt >= MOUSEOPT_Max || dir == 0)
		return;
	const MOUSEDEF& def = s_amousedef[opt];
	int value = s_av[opt] + (dir > 0 ? def.step : -def.step);
	if (value > def.max)
		value = fWrap ? def.min : def.max;
	if (value < def.min)
		value = fWrap ? def.max : def.min;
	s_av[opt] = value;

	if (opt == MOUSEOPT_RawInput && s_fCaptured && g_gl.window != nullptr && glfwRawMouseMotionSupported())
		glfwSetInputMode(g_gl.window, GLFW_RAW_MOUSE_MOTION, value != 0 ? GLFW_TRUE : GLFW_FALSE);

	Save();
	RefreshMouseStrings();
}

bool MouseOptNumericRange(MOUSEOPT opt, int* pmin, int* pmax, int* pstep)
{
	if (opt < 0 || opt >= MOUSEOPT_Max || !s_amousedef[opt].fPercent)
		return false;
	*pmin = s_amousedef[opt].min;
	*pmax = s_amousedef[opt].max;
	*pstep = s_amousedef[opt].step;
	return true;
}

void SetMouseOpt(MOUSEOPT opt, int value)
{
	if (opt < 0 || opt >= MOUSEOPT_Max)
		return;
	s_av[opt] = std::clamp(value, s_amousedef[opt].min, s_amousedef[opt].max);
	Save();
	RefreshMouseStrings();
}

int MouseOptDefault(MOUSEOPT opt)
{
	return (opt >= 0 && opt < MOUSEOPT_Max) ? s_amousedef[opt].def : 0;
}

namespace
{
INPUTDEV s_inputdevLast = INPUTDEV_Keyboard;
}

void NoteInputDevice(INPUTDEV dev)
{
	s_inputdevLast = dev;
}

INPUTDEV LastInputDevice()
{
	return s_inputdevLast;
}

void ResetMouseInput()
{
	for (int i = 0; i < MOUSEOPT_Max; ++i)
		s_av[i] = s_amousedef[i].def;
	if (s_fCaptured && g_gl.window != nullptr && glfwRawMouseMotionSupported())
		glfwSetInputMode(g_gl.window, GLFW_RAW_MOUSE_MOTION, s_av[MOUSEOPT_RawInput] != 0 ? GLFW_TRUE : GLFW_FALSE);
	Save();
	RefreshMouseStrings();
}

void RefreshMouseStrings()
{
	for (int i = 0; i < MOUSEOPT_Max; ++i)
	{
		const MOUSEDEF& def = s_amousedef[i];
		if (def.fPercent)
			std::snprintf(g_achzMouseOpt[i], sizeof(g_achzMouseOpt[i]), "%s: %d%%", def.label, s_av[i]);
		else
			std::snprintf(g_achzMouseOpt[i], sizeof(g_achzMouseOpt[i]), "%s: %s", def.label, s_av[i] != 0 ? "On" : "Off");
	}
}

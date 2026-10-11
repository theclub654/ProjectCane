#pragma once

// -----------------------------------------------------------------------------
// Menu customization (Options > Customization...). Settings live in
// MenuStyle.ini next to the exe. Every option is a number with a range:
// Left/Right adjust the highlighted option, Confirm steps it forward (wraps).
// -----------------------------------------------------------------------------

#include <glm/glm.hpp>

enum MSOPT
{
	MSOPT_Theme = 0,        // preset (changing anything else switches to Custom)

	// Text
	MSOPT_Font,             // -1 = game default, else index into the loaded fonts
	MSOPT_TextSize,         // percent
	MSOPT_TitleSize,        // percent
	MSOPT_TextR, MSOPT_TextG, MSOPT_TextB,
	MSOPT_TitleColor,       // White / Match Text / Gold / Sly Blue / Red
	MSOPT_TextShadow,

	// Background
	MSOPT_BoxStyle,         // Classic / Flat / Gradient
	MSOPT_BoxR, MSOPT_BoxG, MSOPT_BoxB,
	MSOPT_BoxOpacity,       // percent
	MSOPT_BoxRainbow,
	MSOPT_BoxWidth,         // percent

	// Border
	MSOPT_Border,
	MSOPT_BorderR, MSOPT_BorderG, MSOPT_BorderB,
	MSOPT_BorderOpacity,
	MSOPT_BorderThickness,  // pixels at 720p
	MSOPT_BorderRainbow,
	MSOPT_BorderPulse,

	// Highlight (selected item)
	MSOPT_HlR, MSOPT_HlG, MSOPT_HlB,
	MSOPT_HlPulse,
	MSOPT_HlBounce,
	MSOPT_HlCursor,         // None / Arrow / Dot / Brackets
	MSOPT_Divider,

	// Effects
	MSOPT_Position,         // Center / Left / Right
	MSOPT_OpenAnim,
	MSOPT_ScreenDim,        // percent
	MSOPT_Sounds,
	MSOPT_Brightness,       // percent
	MSOPT_Contrast,         // percent

	// Layout (Customization > General)
	MSOPT_Layout,           // Classic (centered rows) / Modern (name left, value right)
	MSOPT_PromptBar,        // button prompt bar at the bottom of the screen

	MSOPT_Max
};

void StartupMenuStyle();

int MenuStyleValue(MSOPT opt);
void AdjustMenuStyle(MSOPT opt, int dir, bool fWrap);   // dir = +1 / -1
void ResetMenuStyle();
// Numeric (slider) options: range for the value editor. False for choices / toggles / Font.
bool MenuStyleNumericRange(MSOPT opt, int* pmin, int* pmax, int* pstep);
// fRaw = store only (no theme side effects); used to restore values exactly.
void SetMenuStyleValue(MSOPT opt, int value, bool fRaw);
void RefreshMenuStyleStrings();
extern char g_achzMenuStyle[MSOPT_Max][64];

// Helpers for drawing --------------------------------------------------------------
glm::vec3 MenuStyleGrade(glm::vec3 rgb);            // brightness / contrast, clamped
glm::vec3 MenuStyleRainbow(float phase);            // animated hue (0..1 phase offset)
float MenuStyleTextScale();
float MenuStyleTitleScale();
bool FMenuStyleSounds();
bool FMenuStyleOpenAnim();
int MenuStyleDefault(MSOPT opt);                     // default value (for "changed" markers / reset)
bool FMenuStyleModern();
bool FMenuStyleToggle(MSOPT opt);                     // On/Off option                              // Layout = Modern
void* MenuStyleFont(void* pfontDefault);
int MenuStyleTtfFont();                              // -1 = game font, else menufont.h index             // CFontBrx* (void* keeps this header light)

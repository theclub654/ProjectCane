// Menu customization. See menustyle.h.

#include "menustyle.h"

#include "main.h"
#include "font.h"
#include "menufont.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iterator>
#include <string>

char g_achzMenuStyle[MSOPT_Max][64]{};

namespace
{
enum MSKIND { MSK_Slider, MSK_Choice, MSK_Toggle };

struct MSDEF
{
	const char* label;
	const char* iniKey;
	MSKIND kind;
	int min;
	int max;
	int step;
	int def;
	const char* const* choices;   // MSK_Choice labels (index = value - min)
	const char* suffix;           // "%", "px", ""
};

const char* const s_apchzTheme[] = { "Custom", "Classic", "Midnight", "Sly Blue", "Crimson", "Emerald", "Gold", "Vaporwave", "High Contrast", "Modern" };
const char* const s_apchzLayout[] = { "Classic", "Modern" };
const char* const s_apchzTitleColor[] = { "White", "Match Text", "Gold", "Sly Blue", "Red" };
const char* const s_apchzBoxStyle[] = { "Classic", "Flat", "Gradient" };
const char* const s_apchzCursor[] = { "None", "Arrow", "Both Sides", "Dashes" };
const char* const s_apchzPosition[] = { "Center", "Left", "Right" };

// Defaults reproduce the original ProjectCane look ("Classic").
const MSDEF s_amsdef[MSOPT_Max] =
{
	{ "Theme",            "Theme",          MSK_Choice, 0, 9, 1, 1, s_apchzTheme, "" },

	{ "Font",             "Font",           MSK_Slider, -1, 63, 1, -1, nullptr, "" },
	{ "Text Size",        "TextSize",       MSK_Slider, 70, 150, 5, 100, nullptr, "%" },
	{ "Title Size",       "TitleSize",      MSK_Slider, 70, 150, 5, 100, nullptr, "%" },
	{ "Text Red",         "TextR",          MSK_Slider, 0, 255, 5, 189, nullptr, "" },
	{ "Text Green",       "TextG",          MSK_Slider, 0, 255, 5, 189, nullptr, "" },
	{ "Text Blue",        "TextB",          MSK_Slider, 0, 255, 5, 189, nullptr, "" },
	{ "Title Color",      "TitleColor",     MSK_Choice, 0, 4, 1, 0, s_apchzTitleColor, "" },
	{ "Text Shadow",      "TextShadow",     MSK_Toggle, 0, 1, 1, 0, nullptr, "" },

	{ "Box Style",        "BoxStyle",       MSK_Choice, 0, 2, 1, 0, s_apchzBoxStyle, "" },
	{ "Box Red",          "BoxR",           MSK_Slider, 0, 255, 5, 15, nullptr, "" },
	{ "Box Green",        "BoxG",           MSK_Slider, 0, 255, 5, 25, nullptr, "" },
	{ "Box Blue",         "BoxB",           MSK_Slider, 0, 255, 5, 50, nullptr, "" },
	{ "Box Opacity",      "BoxOpacity",     MSK_Slider, 0, 100, 5, 100, nullptr, "%" },
	{ "Rainbow Box",      "BoxRainbow",     MSK_Toggle, 0, 1, 1, 0, nullptr, "" },
	{ "Box Width",        "BoxWidth",       MSK_Slider, 100, 160, 5, 100, nullptr, "%" },

	{ "Border",           "Border",         MSK_Toggle, 0, 1, 1, 0, nullptr, "" },
	{ "Border Red",       "BorderR",        MSK_Slider, 0, 255, 5, 0, nullptr, "" },
	{ "Border Green",     "BorderG",        MSK_Slider, 0, 255, 5, 150, nullptr, "" },
	{ "Border Blue",      "BorderB",        MSK_Slider, 0, 255, 5, 250, nullptr, "" },
	{ "Border Opacity",   "BorderOpacity",  MSK_Slider, 0, 100, 5, 100, nullptr, "%" },
	{ "Border Thickness", "BorderThick",    MSK_Slider, 1, 8, 1, 2, nullptr, "px" },
	{ "Rainbow Border",   "BorderRainbow",  MSK_Toggle, 0, 1, 1, 0, nullptr, "" },
	{ "Border Pulse",     "BorderPulse",    MSK_Toggle, 0, 1, 1, 0, nullptr, "" },

	{ "Highlight Red",    "HlR",            MSK_Slider, 0, 255, 5, 222, nullptr, "" },
	{ "Highlight Green",  "HlG",            MSK_Slider, 0, 255, 5, 222, nullptr, "" },
	{ "Highlight Blue",   "HlB",            MSK_Slider, 0, 255, 5, 94, nullptr, "" },
	{ "Highlight Pulse",  "HlPulse",        MSK_Toggle, 0, 1, 1, 1, nullptr, "" },
	{ "Highlight Bounce", "HlBounce",       MSK_Toggle, 0, 1, 1, 1, nullptr, "" },
	{ "Cursor",           "Cursor",         MSK_Choice, 0, 3, 1, 0, s_apchzCursor, "" },
	{ "Divider Line",     "Divider",        MSK_Toggle, 0, 1, 1, 1, nullptr, "" },

	{ "Menu Position",    "Position",       MSK_Choice, 0, 2, 1, 0, s_apchzPosition, "" },
	{ "Open Animation",   "OpenAnim",       MSK_Toggle, 0, 1, 1, 1, nullptr, "" },
	{ "Dim Background",   "ScreenDim",      MSK_Slider, 0, 90, 10, 0, nullptr, "%" },
	{ "Menu Sounds",      "Sounds",         MSK_Toggle, 0, 1, 1, 1, nullptr, "" },
	{ "Brightness",       "Brightness",     MSK_Slider, 50, 150, 5, 100, nullptr, "%" },
	{ "Contrast",         "Contrast",       MSK_Slider, 50, 150, 5, 100, nullptr, "%" },

	{ "Layout",           "Layout",         MSK_Choice, 0, 1, 1, 1, s_apchzLayout, "" },
	{ "Button Prompts",   "PromptBar",      MSK_Toggle, 0, 1, 1, 1, nullptr, "" },
};

int s_av[MSOPT_Max];

struct PRESETVAL { MSOPT opt; int value; };

// Themes: start from Classic, then override.
const PRESETVAL s_aMidnight[] = {
	{ MSOPT_BoxStyle, 2 }, { MSOPT_BoxR, 10 }, { MSOPT_BoxG, 12 }, { MSOPT_BoxB, 30 }, { MSOPT_BoxOpacity, 90 },
	{ MSOPT_Border, 1 }, { MSOPT_BorderR, 90 }, { MSOPT_BorderG, 110 }, { MSOPT_BorderB, 200 }, { MSOPT_BorderOpacity, 80 },
	{ MSOPT_TextR, 200 }, { MSOPT_TextG, 205 }, { MSOPT_TextB, 225 },
	{ MSOPT_HlR, 150 }, { MSOPT_HlG, 180 }, { MSOPT_HlB, 255 }, { MSOPT_ScreenDim, 40 }, { MSOPT_HlCursor, 1 } };
const PRESETVAL s_aSlyBlue[] = {
	{ MSOPT_BoxStyle, 1 }, { MSOPT_BoxR, 20 }, { MSOPT_BoxG, 45 }, { MSOPT_BoxB, 95 }, { MSOPT_BoxOpacity, 85 },
	{ MSOPT_Border, 1 }, { MSOPT_BorderR, 255 }, { MSOPT_BorderG, 200 }, { MSOPT_BorderB, 40 }, { MSOPT_BorderThickness, 3 },
	{ MSOPT_TextR, 220 }, { MSOPT_TextG, 225 }, { MSOPT_TextB, 235 }, { MSOPT_TitleColor, 2 },
	{ MSOPT_HlR, 255 }, { MSOPT_HlG, 205 }, { MSOPT_HlB, 60 }, { MSOPT_TextShadow, 1 } };
const PRESETVAL s_aCrimson[] = {
	{ MSOPT_BoxStyle, 2 }, { MSOPT_BoxR, 70 }, { MSOPT_BoxG, 10 }, { MSOPT_BoxB, 15 }, { MSOPT_BoxOpacity, 90 },
	{ MSOPT_Border, 1 }, { MSOPT_BorderR, 255 }, { MSOPT_BorderG, 70 }, { MSOPT_BorderB, 70 }, { MSOPT_BorderPulse, 1 },
	{ MSOPT_TextR, 235 }, { MSOPT_TextG, 210 }, { MSOPT_TextB, 210 }, { MSOPT_TitleColor, 4 },
	{ MSOPT_HlR, 255 }, { MSOPT_HlG, 120 }, { MSOPT_HlB, 100 }, { MSOPT_ScreenDim, 30 } };
const PRESETVAL s_aEmerald[] = {
	{ MSOPT_BoxStyle, 1 }, { MSOPT_BoxR, 10 }, { MSOPT_BoxG, 55 }, { MSOPT_BoxB, 40 }, { MSOPT_BoxOpacity, 85 },
	{ MSOPT_Border, 1 }, { MSOPT_BorderR, 90 }, { MSOPT_BorderG, 230 }, { MSOPT_BorderB, 150 },
	{ MSOPT_TextR, 210 }, { MSOPT_TextG, 235 }, { MSOPT_TextB, 220 },
	{ MSOPT_HlR, 150 }, { MSOPT_HlG, 255 }, { MSOPT_HlB, 170 }, { MSOPT_HlCursor, 2 } };
const PRESETVAL s_aGold[] = {
	{ MSOPT_BoxStyle, 2 }, { MSOPT_BoxR, 45 }, { MSOPT_BoxG, 30 }, { MSOPT_BoxB, 10 }, { MSOPT_BoxOpacity, 90 },
	{ MSOPT_Border, 1 }, { MSOPT_BorderR, 255 }, { MSOPT_BorderG, 200 }, { MSOPT_BorderB, 70 }, { MSOPT_BorderThickness, 3 },
	{ MSOPT_TextR, 240 }, { MSOPT_TextG, 225 }, { MSOPT_TextB, 190 }, { MSOPT_TitleColor, 2 },
	{ MSOPT_HlR, 255 }, { MSOPT_HlG, 215 }, { MSOPT_HlB, 90 }, { MSOPT_TextShadow, 1 }, { MSOPT_HlCursor, 3 } };
const PRESETVAL s_aVapor[] = {
	{ MSOPT_BoxStyle, 2 }, { MSOPT_BoxR, 40 }, { MSOPT_BoxG, 10 }, { MSOPT_BoxB, 60 }, { MSOPT_BoxOpacity, 85 },
	{ MSOPT_Border, 1 }, { MSOPT_BorderRainbow, 1 }, { MSOPT_BorderThickness, 3 },
	{ MSOPT_TextR, 255 }, { MSOPT_TextG, 190 }, { MSOPT_TextB, 240 },
	{ MSOPT_HlR, 120 }, { MSOPT_HlG, 255 }, { MSOPT_HlB, 255 }, { MSOPT_ScreenDim, 30 }, { MSOPT_HlCursor, 1 } };
const PRESETVAL s_aHighContrast[] = {
	{ MSOPT_BoxStyle, 1 }, { MSOPT_BoxR, 0 }, { MSOPT_BoxG, 0 }, { MSOPT_BoxB, 0 }, { MSOPT_BoxOpacity, 95 },
	{ MSOPT_Border, 1 }, { MSOPT_BorderR, 255 }, { MSOPT_BorderG, 255 }, { MSOPT_BorderB, 255 }, { MSOPT_BorderThickness, 2 },
	{ MSOPT_TextR, 255 }, { MSOPT_TextG, 255 }, { MSOPT_TextB, 255 },
	{ MSOPT_HlR, 255 }, { MSOPT_HlG, 230 }, { MSOPT_HlB, 0 }, { MSOPT_HlPulse, 0 }, { MSOPT_HlCursor, 1 },
	{ MSOPT_TextSize, 110 }, { MSOPT_ScreenDim, 60 } };

// Modern: dark frosted panel, thin accent outline, Poppins, no pulsing.
const PRESETVAL s_aModern[] = {
	{ MSOPT_Font, 0 }, { MSOPT_TextR, 225 }, { MSOPT_TextG, 228 }, { MSOPT_TextB, 236 }, { MSOPT_TitleColor, 0 },
	{ MSOPT_BoxStyle, 1 }, { MSOPT_BoxR, 16 }, { MSOPT_BoxG, 18 }, { MSOPT_BoxB, 26 }, { MSOPT_BoxOpacity, 90 }, { MSOPT_BoxWidth, 115 },
	{ MSOPT_Border, 1 }, { MSOPT_BorderR, 110 }, { MSOPT_BorderG, 170 }, { MSOPT_BorderB, 255 }, { MSOPT_BorderOpacity, 45 }, { MSOPT_BorderThickness, 1 },
	{ MSOPT_HlR, 120 }, { MSOPT_HlG, 190 }, { MSOPT_HlB, 255 }, { MSOPT_HlPulse, 0 }, { MSOPT_HlBounce, 0 }, { MSOPT_HlCursor, 0 },
	{ MSOPT_Divider, 1 }, { MSOPT_ScreenDim, 40 }, { MSOPT_Layout, 1 }, { MSOPT_PromptBar, 1 } };

struct PRESET { const PRESETVAL* a; int c; };
const PRESET s_apreset[] =
{
	{ nullptr, 0 },                                                     // Custom (no change)
	{ nullptr, 0 },                                                     // Classic colors (defaults; Layout is separate)
	{ s_aMidnight, static_cast<int>(std::size(s_aMidnight)) },
	{ s_aSlyBlue, static_cast<int>(std::size(s_aSlyBlue)) },
	{ s_aCrimson, static_cast<int>(std::size(s_aCrimson)) },
	{ s_aEmerald, static_cast<int>(std::size(s_aEmerald)) },
	{ s_aGold, static_cast<int>(std::size(s_aGold)) },
	{ s_aVapor, static_cast<int>(std::size(s_aVapor)) },
	{ s_aHighContrast, static_cast<int>(std::size(s_aHighContrast)) },
	{ s_aModern, static_cast<int>(std::size(s_aModern)) },
};
static_assert(std::size(s_apreset) == std::size(s_apchzTheme), "one preset per theme name");

std::wstring IniPath()
{
	wchar_t path[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	return (std::filesystem::path(path).parent_path() / L"MenuStyle.ini").wstring();
}

void Save()
{
	const std::wstring ini = IniPath();
	for (int i = 0; i < MSOPT_Max; ++i)
	{
		const std::string key = s_amsdef[i].iniKey;
		WritePrivateProfileStringW(L"MenuStyle", std::wstring(key.begin(), key.end()).c_str(),
			std::to_wstring(s_av[i]).c_str(), ini.c_str());
	}
}

void SetDefaults()
{
	for (int i = 0; i < MSOPT_Max; ++i)
		s_av[i] = s_amsdef[i].def;
}

void ApplyTheme(int theme)
{
	if (theme <= 0 || theme >= static_cast<int>(std::size(s_apreset)))
		return;
	SetDefaults();
	const PRESET& preset = s_apreset[theme];
	for (int i = 0; i < preset.c; ++i)
		s_av[preset.a[i].opt] = preset.a[i].value;
	s_av[MSOPT_Theme] = theme;
}

// Number of TrueType menu fonts (menufont.cpp); -1 = the game's own font.
int FontCount()
{
	return CMenuFonts();
}

glm::vec3 Hsv(float h, float s, float v)
{
	h = h - std::floor(h);
	const float i = std::floor(h * 6.0f);
	const float f = h * 6.0f - i;
	const float p = v * (1.0f - s);
	const float q = v * (1.0f - f * s);
	const float t = v * (1.0f - (1.0f - f) * s);
	switch (static_cast<int>(i) % 6)
	{
		case 0: return { v, t, p };
		case 1: return { q, v, p };
		case 2: return { p, v, t };
		case 3: return { p, q, v };
		case 4: return { t, p, v };
		default: return { v, p, q };
	}
}
} // namespace

void StartupMenuStyle()
{
	StartupMenuFonts();
	SetDefaults();
	const std::wstring ini = IniPath();
	for (int i = 0; i < MSOPT_Max; ++i)
	{
		const std::string key = s_amsdef[i].iniKey;
		const int value = static_cast<int>(GetPrivateProfileIntW(L"MenuStyle",
			std::wstring(key.begin(), key.end()).c_str(), s_amsdef[i].def, ini.c_str()));
		s_av[i] = std::clamp(value, s_amsdef[i].min, s_amsdef[i].max);
	}
	if (s_av[MSOPT_Font] >= FontCount())
		s_av[MSOPT_Font] = -1;
	Save();
}

int MenuStyleValue(MSOPT opt)
{
	return (opt >= 0 && opt < MSOPT_Max) ? s_av[opt] : 0;
}

void AdjustMenuStyle(MSOPT opt, int dir, bool fWrap)
{
	if (opt < 0 || opt >= MSOPT_Max || dir == 0)
		return;
	const MSDEF& def = s_amsdef[opt];
	int maxValue = def.max;
	if (opt == MSOPT_Font)
		maxValue = (std::min)(def.max, FontCount() - 1);

	int value = s_av[opt] + (dir > 0 ? def.step : -def.step);
	if (value > maxValue)
		value = fWrap ? def.min : maxValue;
	if (value < def.min)
		value = fWrap ? maxValue : def.min;
	s_av[opt] = value;

	if (opt == MSOPT_Theme)
		ApplyTheme(value);
	else if (opt != MSOPT_Layout && opt != MSOPT_PromptBar && opt != MSOPT_Sounds)
		s_av[MSOPT_Theme] = 0;   // any manual color/style change makes the theme "Custom"
	Save();
}

bool MenuStyleNumericRange(MSOPT opt, int* pmin, int* pmax, int* pstep)
{
	if (opt < 0 || opt >= MSOPT_Max || opt == MSOPT_Font || s_amsdef[opt].kind != MSK_Slider)
		return false;
	*pmin = s_amsdef[opt].min;
	*pmax = s_amsdef[opt].max;
	*pstep = s_amsdef[opt].step;
	return true;
}

void SetMenuStyleValue(MSOPT opt, int value, bool fRaw)
{
	if (opt < 0 || opt >= MSOPT_Max)
		return;
	s_av[opt] = std::clamp(value, s_amsdef[opt].min, s_amsdef[opt].max);
	if (!fRaw)
	{
		if (opt == MSOPT_Theme)
			ApplyTheme(s_av[opt]);
		else
			if (opt != MSOPT_Layout && opt != MSOPT_PromptBar && opt != MSOPT_Sounds)
				s_av[MSOPT_Theme] = 0;   // manual change -> Custom
	}
	Save();
}

void ResetMenuStyle()
{
	SetDefaults();
	Save();
}

void RefreshMenuStyleStrings()
{
	for (int i = 0; i < MSOPT_Max; ++i)
	{
		const MSDEF& def = s_amsdef[i];
		const int v = s_av[i];
		char value[32];
		if (i == MSOPT_Font)
			std::snprintf(value, sizeof(value), "%s", v < 0 ? "Game Font" : PchzMenuFontName(v));
		else if (def.kind == MSK_Toggle)
			std::snprintf(value, sizeof(value), "%s", v ? "On" : "Off");
		else if (def.kind == MSK_Choice)
			std::snprintf(value, sizeof(value), "%s", def.choices[v - def.min]);
		else if (i == MSOPT_ScreenDim && v == 0)
			std::snprintf(value, sizeof(value), "Off");
		else
			std::snprintf(value, sizeof(value), "%d%s", v, def.suffix);

		std::snprintf(g_achzMenuStyle[i], 64, "%s: %s", def.label, value);
	}
}

glm::vec3 MenuStyleGrade(glm::vec3 rgb)
{
	const float brightness = s_av[MSOPT_Brightness] / 100.0f;
	const float contrast = s_av[MSOPT_Contrast] / 100.0f;
	rgb = (rgb - glm::vec3(0.5f)) * contrast + glm::vec3(0.5f);
	rgb *= brightness;
	return glm::clamp(rgb, glm::vec3(0.0f), glm::vec3(1.0f));
}

glm::vec3 MenuStyleRainbow(float phase)
{
	return Hsv(static_cast<float>(glfwGetTime()) * 0.12f + phase, 0.75f, 1.0f);
}

float MenuStyleTextScale()
{
	return s_av[MSOPT_TextSize] / 100.0f;
}

float MenuStyleTitleScale()
{
	return s_av[MSOPT_TitleSize] / 100.0f;
}

bool FMenuStyleSounds()
{
	return s_av[MSOPT_Sounds] != 0;
}

bool FMenuStyleOpenAnim()
{
	return s_av[MSOPT_OpenAnim] != 0;
}

int MenuStyleDefault(MSOPT opt)
{
	return (opt >= 0 && opt < MSOPT_Max) ? s_amsdef[opt].def : 0;
}

bool FMenuStyleToggle(MSOPT opt)
{
	return opt >= 0 && opt < MSOPT_Max && s_amsdef[opt].kind == MSK_Toggle;
}

bool FMenuStyleModern()
{
	return s_av[MSOPT_Layout] != 0;
}

// The game's bitmap fonts other than the menu font are icon/glyph sets, so the
// prompt always measures and lays out with the default font; TrueType fonts
// are drawn on top through menufont.cpp (see MenuStyleTtfFont).
void* MenuStyleFont(void* pfontDefault)
{
	return pfontDefault;
}

int MenuStyleTtfFont()
{
	const int font = s_av[MSOPT_Font];
	return (font >= 0 && font < FontCount()) ? font : -1;
}

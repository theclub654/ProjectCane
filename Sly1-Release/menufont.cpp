// TrueType menu fonts. See menufont.h.

#include "menufont.h"

#include "main.h"
#include "gl.h"
#include "imgui/imgui.h"

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
namespace fs = std::filesystem;

constexpr float kBakePixels = 56.0f;    // atlas size; drawn smaller or larger as needed
constexpr int kMaxFonts = 32;
// TrueType glyphs fill more of the line than the game's bitmap font.
constexpr float kSizeOfLine = 0.80f;

struct MENUFONT
{
	std::string name;
	ImFont* pfont = nullptr;
};

std::vector<MENUFONT> s_afont;

struct KNOWNFONT
{
	const char* file;
	const char* name;
};

// Friendly names for the fonts bundled in Libraries\fonts.
const KNOWNFONT s_aknownBundled[] =
{
	{ "Poppins-Regular.ttf", "Poppins" },
	{ "Poppins-Bold.ttf", "Poppins Bold" },
	{ "Lora-Variable.ttf", "Lora" },
	{ "Lato-Regular.ttf", "Lato" },
	{ "Lato-Black.ttf", "Lato Black" },
	{ "DejaVuSansMono-Bold.ttf", "DejaVu Mono" },
	{ "DejaVuSerif-Bold.ttf", "DejaVu Serif" },
	{ "LiberationSerif-Bold.ttf", "Liberation Serif" },
	{ "Carlito-Bold.ttf", "Carlito" },
};

// Common Windows fonts, used when installed.
const KNOWNFONT s_aknownWindows[] =
{
	{ "arialbd.ttf", "Arial" },
	{ "ariblk.ttf", "Arial Black" },
	{ "segoeuib.ttf", "Segoe UI" },
	{ "verdanab.ttf", "Verdana" },
	{ "tahomabd.ttf", "Tahoma" },
	{ "trebucbd.ttf", "Trebuchet MS" },
	{ "georgiab.ttf", "Georgia" },
	{ "timesbd.ttf", "Times New Roman" },
	{ "impact.ttf", "Impact" },
	{ "comicbd.ttf", "Comic Sans MS" },
	{ "consolab.ttf", "Consolas" },
	{ "courbd.ttf", "Courier New" },
	{ "bahnschrift.ttf", "Bahnschrift" },
	{ "framd.ttf", "Franklin Gothic" },
	{ "calibrib.ttf", "Calibri" },
	{ "cambriab.ttf", "Cambria" },
	{ "Inkfree.ttf", "Ink Free" },
	{ "segoepr.ttf", "Segoe Print" },
	{ "segoesc.ttf", "Segoe Script" },
	{ "Gabriola.ttf", "Gabriola" },
};

std::string Lower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return s;
}

std::string FriendlyName(const fs::path& path)
{
	const std::string file = path.filename().string();
	for (const KNOWNFONT& known : s_aknownBundled)
		if (Lower(file) == Lower(known.file))
			return known.name;
	std::string stem = path.stem().string();
	std::replace(stem.begin(), stem.end(), '-', ' ');
	std::replace(stem.begin(), stem.end(), '_', ' ');
	return stem;
}

fs::path ExeDir()
{
	wchar_t path[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	return fs::path(path).parent_path();
}

void TryAdd(const fs::path& path, const std::string& name)
{
	if (static_cast<int>(s_afont.size()) >= kMaxFonts)
		return;
	for (const MENUFONT& font : s_afont)
		if (font.name == name)
			return;

	std::error_code ec;
	if (!fs::is_regular_file(path, ec))
		return;

	static const ImWchar s_aranges[] = { 0x0020, 0x007E, 0x00A0, 0x00FF, 0 };
	ImFontConfig config;
	config.OversampleH = 1;
	config.OversampleV = 1;
	config.PixelSnapH = false;
	// ImGui takes UTF-8 paths (it converts back to wide on Windows).
	const std::wstring wide = path.wstring();
	const int cb = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (cb <= 0)
		return;
	std::string utf8(static_cast<size_t>(cb), '\0');
	WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), cb, nullptr, nullptr);
	ImFont* pfont = ImGui::GetIO().Fonts->AddFontFromFileTTF(utf8.c_str(), kBakePixels, &config, s_aranges);
	if (pfont != nullptr)
		s_afont.push_back({ name, pfont });
}

bool FValid(int ifont)
{
	return ifont >= 0 && ifont < static_cast<int>(s_afont.size()) && s_afont[ifont].pfont != nullptr &&
		ImGui::GetCurrentContext() != nullptr;
}

// "&&" is an escaped '&' in the game's rich text; any other '&' code is markup.
bool FPlainText(const char* text)
{
	for (const char* pch = text; *pch != '\0'; ++pch)
	{
		if (*pch != '&')
			continue;
		if (pch[1] != '&')
			return false;
		++pch;
	}
	return true;
}

std::string Unescape(const char* text)
{
	std::string out;
	for (const char* pch = text; *pch != '\0'; ++pch)
	{
		out += *pch;
		if (pch[0] == '&' && pch[1] == '&')
			++pch;
	}
	return out;
}

// Menu (output) pixels -> ImGui display coordinates.
float ScaleX()
{
	const float display = ImGui::GetIO().DisplaySize.x;   // -1 before the first ImGui frame
	return display > 0.0f ? display / static_cast<float>((std::max)(g_gl.outputWidth, 1)) : 1.0f;
}

float ScaleY()
{
	const float display = ImGui::GetIO().DisplaySize.y;
	return display > 0.0f ? display / static_cast<float>((std::max)(g_gl.outputHeight, 1)) : 1.0f;
}
} // namespace

void StartupMenuFonts()
{
	if (ImGui::GetCurrentContext() == nullptr || !s_afont.empty())
		return;

	ImFontAtlas* atlas = ImGui::GetIO().Fonts;
	// Keep ImGui's built-in font first: it stays the default for the
	// debug menu, FPS counter and timer overlays.
	if (atlas->Fonts.Size == 0)
		atlas->AddFontDefault();

	// 1) <exe>\Fonts : bundled fonts first (known order), then anything else.
	std::error_code ec;
	const fs::path dir = ExeDir() / "Fonts";
	for (const KNOWNFONT& known : s_aknownBundled)
		TryAdd(dir / known.file, known.name);
	if (fs::is_directory(dir, ec))
	{
		std::vector<fs::path> extra;
		for (const fs::directory_entry& entry : fs::directory_iterator(dir, ec))
		{
			const std::string ext = Lower(entry.path().extension().string());
			if (ext == ".ttf" || ext == ".otf")
				extra.push_back(entry.path());
		}
		std::sort(extra.begin(), extra.end());
		for (const fs::path& path : extra)
			TryAdd(path, FriendlyName(path));
	}

	// 2) Installed Windows fonts.
	wchar_t windowsDir[MAX_PATH] = {};
	if (GetWindowsDirectoryW(windowsDir, MAX_PATH) > 0)
	{
		const fs::path fontsDir = fs::path(windowsDir) / "Fonts";
		for (const KNOWNFONT& known : s_aknownWindows)
			TryAdd(fontsDir / known.file, known.name);
	}

	atlas->Build();
}

int CMenuFonts()
{
	return static_cast<int>(s_afont.size());
}

const char* PchzMenuFontName(int ifont)
{
	return (ifont >= 0 && ifont < static_cast<int>(s_afont.size())) ? s_afont[ifont].name.c_str() : "Default";
}

bool FMenuFontUsable(int ifont, const char* text)
{
	return FValid(ifont) && text != nullptr && FPlainText(text);
}

float DxMenuFontText(int ifont, const char* text, float dyLine)
{
	if (!FValid(ifont) || text == nullptr)
		return 0.0f;
	const std::string plain = Unescape(text);
	text = plain.c_str();
	const float sizeImGui = dyLine * kSizeOfLine * ScaleY();
	float widest = 0.0f;
	const char* line = text;
	while (true)
	{
		const char* end = std::strchr(line, '\n');
		const ImVec2 size = s_afont[ifont].pfont->CalcTextSizeA(sizeImGui, FLT_MAX, 0.0f, line, end);
		widest = (std::max)(widest, size.x);
		if (end == nullptr)
			break;
		line = end + 1;
	}
	return widest / ScaleX();
}

void DrawMenuFontText(int ifont, const char* text, float x, float y, float dyLine, const glm::vec4& rgba)
{
	if (!FValid(ifont) || text == nullptr || rgba.a <= 0.0f)
		return;
	const std::string plain = Unescape(text);
	text = plain.c_str();
	ImFont* pfont = s_afont[ifont].pfont;
	const float sx = ScaleX();
	const float sy = ScaleY();
	const float sizeMenu = dyLine * kSizeOfLine;
	const float sizeImGui = sizeMenu * sy;
	const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(
		std::clamp(rgba.r, 0.0f, 1.0f), std::clamp(rgba.g, 0.0f, 1.0f),
		std::clamp(rgba.b, 0.0f, 1.0f), std::clamp(rgba.a, 0.0f, 1.0f)));

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	float yLine = y + (dyLine - sizeMenu) * 0.5f;
	const char* line = text;
	while (true)
	{
		const char* end = std::strchr(line, '\n');
		const ImVec2 pos((x + static_cast<float>(g_gl.presentX)) * sx, (yLine + static_cast<float>(g_gl.presentY)) * sy);
		draw->AddText(pfont, sizeImGui, pos, color, line, end);
		if (end == nullptr)
			break;
		line = end + 1;
		yLine += dyLine;
	}
}

// -----------------------------------------------------------------------------
// Overlay text (value editor): drawn through ImGui so it is always on top of
// the menu panels. ifont < 0 uses ImGui's built-in font.
// -----------------------------------------------------------------------------
namespace
{
ImFont* PfontOverlay(int ifont)
{
	if (ImGui::GetCurrentContext() == nullptr)
		return nullptr;
	if (ifont >= 0 && ifont < static_cast<int>(s_afont.size()) && s_afont[ifont].pfont != nullptr)
		return s_afont[ifont].pfont;
	return ImGui::GetFont();
}
} // namespace

float DxOverlayText(int ifont, const char* text, float dyLine)
{
	ImFont* pfont = PfontOverlay(ifont);
	if (pfont == nullptr || text == nullptr)
		return 0.0f;
	const std::string plain = Unescape(text);
	const float sizeImGui = dyLine * kSizeOfLine * ScaleY();
	return pfont->CalcTextSizeA(sizeImGui, FLT_MAX, 0.0f, plain.c_str()).x / ScaleX();
}

void DrawOverlayText(int ifont, const char* text, float x, float y, float dyLine, const glm::vec4& rgba)
{
	ImFont* pfont = PfontOverlay(ifont);
	if (pfont == nullptr || text == nullptr || rgba.a <= 0.0f)
		return;
	const std::string plain = Unescape(text);
	const float sx = ScaleX();
	const float sy = ScaleY();
	const float sizeMenu = dyLine * kSizeOfLine;
	const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(
		std::clamp(rgba.r, 0.0f, 1.0f), std::clamp(rgba.g, 0.0f, 1.0f),
		std::clamp(rgba.b, 0.0f, 1.0f), std::clamp(rgba.a, 0.0f, 1.0f)));
	const ImVec2 pos((x + static_cast<float>(g_gl.presentX)) * sx,
		(y + (dyLine - sizeMenu) * 0.5f + static_cast<float>(g_gl.presentY)) * sy);
	ImGui::GetForegroundDrawList()->AddText(pfont, sizeMenu * sy, pos, color, plain.c_str());
}

void FillOverlayRect(float x0, float y0, float x1, float y1, const glm::vec4& rgba)
{
	if (ImGui::GetCurrentContext() == nullptr || rgba.a <= 0.0f)
		return;
	const float sx = ScaleX();
	const float sy = ScaleY();
	const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(
		std::clamp(rgba.r, 0.0f, 1.0f), std::clamp(rgba.g, 0.0f, 1.0f),
		std::clamp(rgba.b, 0.0f, 1.0f), std::clamp(rgba.a, 0.0f, 1.0f)));
	ImGui::GetForegroundDrawList()->AddRectFilled(
		ImVec2((x0 + static_cast<float>(g_gl.presentX)) * sx, (y0 + static_cast<float>(g_gl.presentY)) * sy),
		ImVec2((x1 + static_cast<float>(g_gl.presentX)) * sx, (y1 + static_cast<float>(g_gl.presentY)) * sy), color);
}

// -----------------------------------------------------------------------------
// Overlay shapes (button icons). Coordinates / sizes in menu (output) pixels.
// thickness <= 0 draws filled.
// -----------------------------------------------------------------------------
namespace
{
ImVec2 OverlayPoint(float x, float y)
{
	return ImVec2((x + static_cast<float>(g_gl.presentX)) * ScaleX(), (y + static_cast<float>(g_gl.presentY)) * ScaleY());
}

ImU32 OverlayColor(const glm::vec4& rgba)
{
	return ImGui::ColorConvertFloat4ToU32(ImVec4(
		std::clamp(rgba.r, 0.0f, 1.0f), std::clamp(rgba.g, 0.0f, 1.0f),
		std::clamp(rgba.b, 0.0f, 1.0f), std::clamp(rgba.a, 0.0f, 1.0f)));
}
} // namespace

void OverlayRect(float x0, float y0, float x1, float y1, const glm::vec4& rgba, float rounding, float thickness)
{
	if (ImGui::GetCurrentContext() == nullptr || rgba.a <= 0.0f)
		return;
	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const float s = ScaleY();
	if (thickness <= 0.0f)
		draw->AddRectFilled(OverlayPoint(x0, y0), OverlayPoint(x1, y1), OverlayColor(rgba), rounding * s);
	else
		draw->AddRect(OverlayPoint(x0, y0), OverlayPoint(x1, y1), OverlayColor(rgba), rounding * s, 0, thickness * s);
}

void OverlayCircle(float cx, float cy, float radius, const glm::vec4& rgba, float thickness)
{
	if (ImGui::GetCurrentContext() == nullptr || rgba.a <= 0.0f)
		return;
	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const float s = ScaleY();
	if (thickness <= 0.0f)
		draw->AddCircleFilled(OverlayPoint(cx, cy), radius * s, OverlayColor(rgba), 32);
	else
		draw->AddCircle(OverlayPoint(cx, cy), radius * s, OverlayColor(rgba), 32, thickness * s);
}

void OverlayLine(float x0, float y0, float x1, float y1, const glm::vec4& rgba, float thickness)
{
	if (ImGui::GetCurrentContext() == nullptr || rgba.a <= 0.0f)
		return;
	ImGui::GetForegroundDrawList()->AddLine(OverlayPoint(x0, y0), OverlayPoint(x1, y1), OverlayColor(rgba), thickness * ScaleY());
}

void OverlayTriangle(float x0, float y0, float x1, float y1, float x2, float y2, const glm::vec4& rgba, float thickness)
{
	if (ImGui::GetCurrentContext() == nullptr || rgba.a <= 0.0f)
		return;
	ImDrawList* draw = ImGui::GetForegroundDrawList();
	if (thickness <= 0.0f)
		draw->AddTriangleFilled(OverlayPoint(x0, y0), OverlayPoint(x1, y1), OverlayPoint(x2, y2), OverlayColor(rgba));
	else
		draw->AddTriangle(OverlayPoint(x0, y0), OverlayPoint(x1, y1), OverlayPoint(x2, y2), OverlayColor(rgba), thickness * ScaleY());
}

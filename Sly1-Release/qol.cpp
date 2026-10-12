// Quality-of-life options. See qol.h.

#include "qol.h"
#include "clock.h"
#include "mouseinput.h"
#include "Input.h"

#include "main.h"
#include "gl.h"
#include "cm.h"
#include "sound.h"
#include "ui.h"
#include "prompt.h"
#include "wipe.h"
#include "imgui/imgui.h"
#include <stb/stb_image_write.h>   // implementation lives in debug.cpp

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

void RecalcCm(CM* pcm);   // cm.cpp

namespace
{
constexpr GLenum kGL_TEXTURE_MAX_ANISOTROPY = 0x84FE;
constexpr GLenum kGL_MAX_TEXTURE_MAX_ANISOTROPY = 0x84FF;

const int s_aAniso[] = { 1, 2, 4, 8, 16 };
const char* const s_apchzAniso[] = { "Off", "2x", "4x", "8x", "16x" };
constexpr int kAnisoCount = static_cast<int>(std::size(s_aAniso));

const float s_aFov[] = { 1.0f, 1.1f, 1.2f, 1.3f, 1.4f };
const char* const s_apchzFov[] = { "Default", "+10%", "+20%", "+30%", "+40%" };
constexpr int kFovCount = static_cast<int>(std::size(s_aFov));
constexpr int kFovPercentMax = 60;

const float s_aSens[] = { 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f };
const char* const s_apchzSens[] = { "50%", "75%", "100%", "125%", "150%", "200%" };
constexpr int kSensCount = static_cast<int>(std::size(s_aSens));
constexpr int kSensPercentMin = 10;
constexpr int kSensPercentMax = 400;

struct QOLSTATE
{
	int aniso = 4;          // index: 16x
	int fovPercent = 0;     // extra field of view, percent (0 = default)
	int sensPercent = 100;  // camera sensitivity, percent
	int screenshotKey = GLFW_KEY_F12;
	bool fPauseOnFocusLoss = true;
	int buttonIcons = 0;    // 0 Auto, 1 Xbox, 2 PlayStation, 3 Nintendo
	bool fMutedByFocus = false;
	float volumeBeforeMute = 1.0f;

	bool fAnisoChecked = false;
	bool fAnisoSupported = false;
	float maxAniso = 1.0f;
	std::vector<GLuint> textures;

	bool fKeyWasDown = false;
	bool fPendingNoHud = false;
	bool fPendingFinal = false;
	GLuint resolveFbo = 0;
	GLuint resolveRbo = 0;
	int resolveW = 0;
	int resolveH = 0;

	std::string notice;
	double tNoticeEnd = 0.0;
};
QOLSTATE s_qol;

std::filesystem::path ExeDirectory()
{
	wchar_t path[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	return std::filesystem::path(path).parent_path();
}

std::wstring IniPath()
{
	return (ExeDirectory() / L"QOL.ini").wstring();
}

void SaveQol()
{
	const std::wstring ini = IniPath();
	WritePrivateProfileStringW(L"QOL", L"Anisotropy", std::to_wstring(s_aAniso[s_qol.aniso]).c_str(), ini.c_str());
	WritePrivateProfileStringW(L"QOL", L"FieldOfView", std::to_wstring(s_qol.fovPercent).c_str(), ini.c_str());
	WritePrivateProfileStringW(L"QOL", L"CameraSensitivity", std::to_wstring(s_qol.sensPercent).c_str(), ini.c_str());
	WritePrivateProfileStringW(L"QOL", L"ScreenshotKey", std::to_wstring(s_qol.screenshotKey).c_str(), ini.c_str());
	WritePrivateProfileStringW(L"QOL", L"PauseOnFocusLoss", s_qol.fPauseOnFocusLoss ? L"1" : L"0", ini.c_str());
	WritePrivateProfileStringW(L"QOL", L"ButtonIcons", std::to_wstring(s_qol.buttonIcons).c_str(), ini.c_str());
	WritePrivateProfileStringW(L"QOL", L"RetailTiming", g_fRetailTiming ? L"1" : L"0", ini.c_str());
}

void CheckAniso()
{
	if (s_qol.fAnisoChecked)
		return;
	s_qol.fAnisoChecked = true;
	GLint count = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &count);
	for (GLint i = 0; i < count; ++i)
	{
		const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i)));
		if (ext != nullptr && (std::strcmp(ext, "GL_EXT_texture_filter_anisotropic") == 0 ||
			std::strcmp(ext, "GL_ARB_texture_filter_anisotropic") == 0))
		{
			s_qol.fAnisoSupported = true;
			break;
		}
	}
	if (s_qol.fAnisoSupported)
		glGetFloatv(kGL_MAX_TEXTURE_MAX_ANISOTROPY, &s_qol.maxAniso);
	std::cout << "[QOL] Anisotropic filtering: " << (s_qol.fAnisoSupported ? "supported" : "not supported")
		<< " (max " << s_qol.maxAniso << "x)" << std::endl;
}

float AnisoValue()
{
	return (std::min)(static_cast<float>(s_aAniso[s_qol.aniso]), (std::max)(1.0f, s_qol.maxAniso));
}

void ApplyAnisoToAll()
{
	CheckAniso();
	if (!s_qol.fAnisoSupported)
		return;
	GLint prev = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev);
	const float value = AnisoValue();
	size_t live = 0;
	for (GLuint tex : s_qol.textures)
	{
		if (!glIsTexture(tex))
			continue;
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameterf(GL_TEXTURE_2D, kGL_TEXTURE_MAX_ANISOTROPY, value);
		s_qol.textures[live++] = tex;
	}
	s_qol.textures.resize(live);   // drop textures that were deleted
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prev));
}

void Notice(const std::string& text)
{
	s_qol.notice = text;
	s_qol.tNoticeEnd = glfwGetTime() + 2.5;
}

std::filesystem::path ScreenshotPath(bool fNoHud)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	char name[96];
	std::snprintf(name, sizeof(name), "Sly_%04d-%02d-%02d_%02d-%02d-%02d-%03d%s.png",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		fNoHud ? "_nohud" : "");
	const std::filesystem::path dir = ExeDirectory() / L"Screenshots";
	std::error_code ec;
	std::filesystem::create_directories(dir, ec);
	return dir / name;
}

// Reads the given framebuffer region (bottom-left origin) and saves it as a
// PNG on a worker thread so the game does not hitch while compressing.
void Capture(GLuint fbo, int x, int y, int width, int height, bool fNoHud)
{
	if (width <= 0 || height <= 0)
		return;

	GLint prevRead = 0;
	GLint prevDraw = 0;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
	GLuint readFbo = fbo;

	// Multisampled framebuffers cannot be read directly: resolve first.
	// (GL_SAMPLE_BUFFERS describes the bound draw framebuffer.)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	GLint sampleBuffers = 0;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	if (sampleBuffers > 0)
	{
		if (s_qol.resolveFbo == 0 || s_qol.resolveW != width || s_qol.resolveH != height)
		{
			if (s_qol.resolveFbo != 0) glDeleteFramebuffers(1, &s_qol.resolveFbo);
			if (s_qol.resolveRbo != 0) glDeleteRenderbuffers(1, &s_qol.resolveRbo);
			glGenRenderbuffers(1, &s_qol.resolveRbo);
			glBindRenderbuffer(GL_RENDERBUFFER, s_qol.resolveRbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glGenFramebuffers(1, &s_qol.resolveFbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_qol.resolveFbo);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, s_qol.resolveRbo);
			s_qol.resolveW = width;
			s_qol.resolveH = height;
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_qol.resolveFbo);
		const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
		glDisable(GL_SCISSOR_TEST);
		glBlitFramebuffer(x, y, x + width, y + height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		if (scissor) glEnable(GL_SCISSOR_TEST);
		readFbo = s_qol.resolveFbo;
		x = 0;
		y = 0;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
	glReadBuffer(readFbo == 0 ? GL_BACK : GL_COLOR_ATTACHMENT0);
	std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 3);
	GLint prevPack = 4;
	glGetIntegerv(GL_PACK_ALIGNMENT, &prevPack);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	glPixelStorei(GL_PACK_ALIGNMENT, prevPack);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));

	const std::filesystem::path path = ScreenshotPath(fNoHud);
	Notice("Screenshot saved: " + path.filename().string());

	std::thread([pixels = std::move(pixels), path, width, height]() mutable
	{
		// OpenGL rows are bottom-up; PNG rows are top-down.
		const size_t row = static_cast<size_t>(width) * 3;
		std::vector<unsigned char> flipped(pixels.size());
		for (int r = 0; r < height; ++r)
			std::memcpy(&flipped[static_cast<size_t>(r) * row], &pixels[static_cast<size_t>(height - 1 - r) * row], row);
		stbi_write_png(path.string().c_str(), width, height, 3, flipped.data(), static_cast<int>(row));
	}).detach();
}
} // namespace

// =============================================================================
void StartupQol()
{
	const std::wstring ini = IniPath();
	const int aniso = static_cast<int>(GetPrivateProfileIntW(L"QOL", L"Anisotropy", 16, ini.c_str()));
	s_qol.aniso = 0;
	for (int i = 0; i < kAnisoCount; ++i)
	{
		if (s_aAniso[i] <= aniso)
			s_qol.aniso = i;
	}
	// Percent (older builds stored a step index in FovStep: +10% per step).
	const int legacyFov = std::clamp(static_cast<int>(GetPrivateProfileIntW(L"QOL", L"FovStep", 0, ini.c_str())), 0, kFovCount - 1) * 10;
	s_qol.fovPercent = std::clamp(static_cast<int>(GetPrivateProfileIntW(L"QOL", L"FieldOfView", legacyFov, ini.c_str())),
		0, kFovPercentMax);
	// Percent (older builds stored a step index in SensitivityStep).
	const int legacyStep = std::clamp(static_cast<int>(GetPrivateProfileIntW(L"QOL", L"SensitivityStep", 2, ini.c_str())), 0, kSensCount - 1);
	const int legacyPercent = static_cast<int>(s_aSens[legacyStep] * 100.0f + 0.5f);
	s_qol.sensPercent = std::clamp(static_cast<int>(GetPrivateProfileIntW(L"QOL", L"CameraSensitivity", legacyPercent, ini.c_str())),
		kSensPercentMin, kSensPercentMax);
	s_qol.screenshotKey = static_cast<int>(GetPrivateProfileIntW(L"QOL", L"ScreenshotKey", GLFW_KEY_F12, ini.c_str()));
	s_qol.fPauseOnFocusLoss = GetPrivateProfileIntW(L"QOL", L"PauseOnFocusLoss", 1, ini.c_str()) != 0;
	s_qol.buttonIcons = std::clamp(static_cast<int>(GetPrivateProfileIntW(L"QOL", L"ButtonIcons", 0, ini.c_str())), 0, 3);
	SetGamepadIconOverride(s_qol.buttonIcons);
	g_fRetailTiming = GetPrivateProfileIntW(L"QOL", L"RetailTiming", 1, ini.c_str()) != 0;
	SaveQol();   // writes defaults the first time so the keys are discoverable
	CheckAniso();
}

void QolRegisterTexture(GLuint texture)
{
	if (texture == 0)
		return;
	CheckAniso();
	s_qol.textures.push_back(texture);
	if (s_qol.fAnisoSupported)
		glTexParameterf(GL_TEXTURE_2D, kGL_TEXTURE_MAX_ANISOTROPY, AnisoValue());   // texture is bound by the caller
}

void SelectNextQolAnisotropy()
{
	s_qol.aniso = (s_qol.aniso + 1) % kAnisoCount;
	SaveQol();
	ApplyAnisoToAll();
}

const char* PchzQolAnisotropy()
{
	CheckAniso();
	return s_qol.fAnisoSupported ? s_apchzAniso[s_qol.aniso] : "Unsupported";
}

float QolFovScale()
{
	return 1.0f + static_cast<float>(s_qol.fovPercent) / 100.0f;
}

void SelectNextQolFov()
{
	s_qol.fovPercent = s_qol.fovPercent >= kFovPercentMax ? 0 : (std::min)(s_qol.fovPercent + 10, kFovPercentMax);
	SaveQol();
	if (g_pcm != nullptr)
		RecalcCm(g_pcm);
}

const char* PchzQolFov()
{
	static char s_achz[16];
	if (s_qol.fovPercent == 0)
		return "Default";
	std::snprintf(s_achz, sizeof(s_achz), "+%d%%", s_qol.fovPercent);
	return s_achz;
}

float QolCameraSensitivity()
{
	return static_cast<float>(s_qol.sensPercent) / 100.0f;
}

void SelectNextQolCameraSensitivity()
{
	s_qol.sensPercent = s_qol.sensPercent >= kSensPercentMax ? 25 : (std::min)(s_qol.sensPercent + 25, kSensPercentMax);
	SaveQol();
}

const char* PchzQolCameraSensitivity()
{
	static char s_achz[16];
	std::snprintf(s_achz, sizeof(s_achz), "%d%%", s_qol.sensPercent);
	return s_achz;
}

void UpdateQolInput()
{
	if (g_gl.window == nullptr)
		return;
	const int key = s_qol.screenshotKey;
	const bool fDown = FMouseBinding(key) ? FMouseBindingHeld(key)
		: (key > GLFW_KEY_UNKNOWN && key <= GLFW_KEY_LAST && glfwGetKey(g_gl.window, key) == GLFW_PRESS);
	if (fDown && !s_qol.fKeyWasDown)
	{
		const bool fShift = glfwGetKey(g_gl.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(g_gl.window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
		if (fShift)
			s_qol.fPendingNoHud = true;
		else
			s_qol.fPendingFinal = true;
	}
	s_qol.fKeyWasDown = fDown;
}

bool FQolScreenshotPending()
{
	return s_qol.fPendingNoHud || s_qol.fPendingFinal;
}

void QolCaptureBeforeHud(int width, int height)
{
	if (!s_qol.fPendingNoHud)
		return;
	s_qol.fPendingNoHud = false;
	GLint drawFbo = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
	Capture(static_cast<GLuint>(drawFbo), 0, 0, width, height, true);
}

void QolCaptureFinal(GLuint drawFbo, int x, int y, int width, int height)
{
	// A "without HUD" request in a frame with no 3D scene (menus) falls back
	// to a normal capture.
	if (!s_qol.fPendingFinal && !s_qol.fPendingNoHud)
		return;
	const bool fNoHud = s_qol.fPendingNoHud;
	s_qol.fPendingFinal = false;
	s_qol.fPendingNoHud = false;
	Capture(drawFbo, x, y, width, height, fNoHud);
}

namespace
{
bool FWindowFocused()
{
	return g_gl.window == nullptr || glfwGetWindowAttrib(g_gl.window, GLFW_FOCUSED) == GLFW_TRUE;
}
}

bool FQolPauseOnFocusLoss()
{
	return s_qol.fPauseOnFocusLoss;
}

int QolFovPercent()
{
	return s_qol.fovPercent;
}

void SetQolFovPercent(int percent)
{
	s_qol.fovPercent = std::clamp(percent, 0, kFovPercentMax);
	SaveQol();
	if (g_pcm != nullptr)
		RecalcCm(g_pcm);
}

int QolScreenshotKey()
{
	return s_qol.screenshotKey;
}

void SetQolScreenshotKey(int key)
{
	s_qol.screenshotKey = key;
	SaveQol();
}

int QolCameraSensitivityPercent()
{
	return s_qol.sensPercent;
}

void SetQolCameraSensitivityPercent(int percent)
{
	s_qol.sensPercent = std::clamp(percent, kSensPercentMin, kSensPercentMax);
	SaveQol();
}

void QolShowNotice(const char* text, double seconds)
{
	s_qol.notice = text != nullptr ? text : "";
	s_qol.tNoticeEnd = glfwGetTime() + seconds;
}

void ReloadQolSettings()
{
	StartupQol();
	ApplyAnisoToAll();
	if (g_pcm != nullptr)
		RecalcCm(g_pcm);
}

bool FQolAnisotropyDefault()
{
	int def = 0;
	for (int i = 0; i < kAnisoCount; ++i)
		if (s_aAniso[i] <= 16)
			def = i;
	return s_qol.aniso == def;
}

void ResetQolAnisotropy()
{
	for (int i = 0; i < kAnisoCount; ++i)
		if (s_aAniso[i] <= 16)
			s_qol.aniso = i;
	SaveQol();
	ApplyAnisoToAll();
}

int QolButtonIcons()
{
	return s_qol.buttonIcons;
}

void SetQolButtonIcons(int setting)
{
	s_qol.buttonIcons = std::clamp(setting, 0, 3);
	SetGamepadIconOverride(s_qol.buttonIcons);
	SaveQol();
}

void SelectNextQolButtonIcons()
{
	SetQolButtonIcons((s_qol.buttonIcons + 1) % 4);
}

const char* PchzQolButtonIcons()
{
	static const char* const s_apchzType[] = { "Xbox", "PlayStation", "Nintendo" };
	static char s_achz[32];
	if (s_qol.buttonIcons == 0)
		std::snprintf(s_achz, sizeof(s_achz), "Auto (%s)", s_apchzType[std::clamp(static_cast<int>(DetectedGamepadType()), 0, 2)]);
	else
		std::snprintf(s_achz, sizeof(s_achz), "%s", s_apchzType[s_qol.buttonIcons - 1]);
	return s_achz;
}

void ResetQolSettings()
{
	s_qol.aniso = 0;
	for (int i = 0; i < kAnisoCount; ++i)
		if (s_aAniso[i] <= 16)
			s_qol.aniso = i;
	s_qol.fovPercent = 0;
	s_qol.sensPercent = 100;
	s_qol.screenshotKey = GLFW_KEY_F12;
	s_qol.fPauseOnFocusLoss = true;
	s_qol.buttonIcons = 0;
	SetGamepadIconOverride(0);
	g_fRetailTiming = true;
	SaveQol();
	ApplyAnisoToAll();
	if (g_pcm != nullptr)
		RecalcCm(g_pcm);
}

bool FRetailTiming()
{
	return g_fRetailTiming;
}

void SetRetailTiming(bool fOn)
{
	g_fRetailTiming = fOn;
	SaveQol();
	QolShowNotice(fOn ? "Retail Timing on: game runs at the PS2's 59.94 Hz"
		: "Retail Timing off: game speed follows Frame Rate (physics may differ from PS2)", 3.0);
}

void ToggleQolPauseOnFocusLoss()
{
	s_qol.fPauseOnFocusLoss = !s_qol.fPauseOnFocusLoss;
	SaveQol();
}

bool FQolFocusLossPause()
{
	return s_qol.fPauseOnFocusLoss && !FWindowFocused();
}

namespace
{
void QolLog(const char* fmt, ...)
{
	static FILE* s_pf = nullptr;
	static bool s_fTried = false;
	if (!s_fTried)
	{
		s_fTried = true;
		_wfopen_s(&s_pf, (ExeDirectory() / L"QOL.log").c_str(), L"w");
	}
	char buffer[512];
	va_list args;
	va_start(args, fmt);
	std::vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	std::cout << "[QOL] " << buffer << std::endl;
	if (s_pf != nullptr)
	{
		std::fprintf(s_pf, "%s\n", buffer);
		std::fflush(s_pf);
	}
}
bool s_fWasFocused = true;
bool s_fPausePending = false;
}

void UpdateQolFocus()
{
	const bool fFocused = FWindowFocused();
	if (fFocused != s_fWasFocused)
	{
		s_fWasFocused = fFocused;
		QolLog("Window %s (uis %d, prompt blots %d prk %d, wipe %d, world %s)",
			fFocused ? "focused" : "lost focus", static_cast<int>(g_ui.uis),
			static_cast<int>(g_prompt.blots), static_cast<int>(g_prompt.prk),
			static_cast<int>(g_wipe.wipes), g_psw != nullptr ? "yes" : "no");
		s_fPausePending = !fFocused && s_qol.fPauseOnFocusLoss;
	}
	if (fFocused)
		s_fPausePending = false;

	// Open the pause menu exactly like pressing Start during gameplay. Keep
	// trying while unfocused in case gameplay was busy (wipe, cutscene).
	if (s_fPausePending && g_psw != nullptr && g_ui.uis == UIS_Playing &&
		g_prompt.blots == BLOTS_Hidden && g_wipe.wipes == WIPES_Idle)
	{
		SetPrompt(&g_prompt, PRP_Basic, PRK_PauseMenu);
		s_fPausePending = false;
		QolLog("Paused because the window lost focus");
	}

	const bool fMute = s_qol.fPauseOnFocusLoss && !fFocused;
	if (fMute && !s_qol.fMutedByFocus)
	{
		s_qol.volumeBeforeMute = GetMasterVolume();
		SetMasterVolume(0.0f);
		s_qol.fMutedByFocus = true;
	}
	else if (!fMute && s_qol.fMutedByFocus)
	{
		// Only restore if nothing else changed the volume in the meantime.
		if (GetMasterVolume() == 0.0f)
			SetMasterVolume(s_qol.volumeBeforeMute);
		s_qol.fMutedByFocus = false;
	}
}

void DrawQolOverlay()
{
	if (ImGui::GetCurrentContext() == nullptr || glfwGetTime() >= s_qol.tNoticeEnd || s_qol.notice.empty())
		return;
	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const ImVec2 display = ImGui::GetIO().DisplaySize;
	const float scale = (std::max)(1.0f, display.y / 720.0f);
	const float fontSize = ImGui::GetFontSize() * 1.1f * scale;
	const ImVec2 size = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, s_qol.notice.c_str());
	const float pad = 8.0f * scale;
	const ImVec2 pos(16.0f * scale, display.y - size.y - 16.0f * scale);
	draw->AddRectFilled(ImVec2(pos.x - pad, pos.y - pad),
		ImVec2(pos.x + size.x + pad, pos.y + size.y + pad), IM_COL32(0, 0, 0, 170), 6.0f * scale);
	draw->AddText(ImGui::GetFont(), fontSize, pos, IM_COL32(255, 255, 255, 255), s_qol.notice.c_str());
}

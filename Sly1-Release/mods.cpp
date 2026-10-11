// ProjectCane mods: position save states, fast forward, controller combos.
// See mods.h.

#include "mods.h"

#include "main.h"
#include "Input.h"
#include "game.h"
#include "chkpnt.h"
#include "wipe.h"
#include "po.h"
#include "so.h"
#include "cm.h"
#include "ui.h"
#include "prompt.h"
#include "clock.h"
#include "math.h"
#include "imgui/imgui.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

char g_achzModsMenu[24][64]{};

namespace
{
// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------
struct COMBO
{
	int btnModifier = -1;   // JOY_BUTTON, -1 = off
	int btnAction = -1;
};

struct MODSSETTINGS
{
	bool fEnabled = true;
	int slot = 0;                       // 0-based
	int ffSpeed = 3;                    // game updates per frame while fast forwarding
	std::array<COMBO, MODACT_Max> combos{};
	std::array<int, MODACT_Max> keys{}; // GLFW keys, GLFW_KEY_UNKNOWN = off
};

MODSSETTINGS s_mods;

const int s_affSpeeds[] = { 2, 3, 4, 8 };

const char* const s_apchzAction[MODACT_Max] =
{
	"Save State", "Load State", "Previous Slot", "Next Slot",
	"Fast Forward (Hold)", "Fast Forward (Toggle)", "Reset Timer"
};

const char* const s_apchzIniKey[MODACT_Max] =
{
	"SaveState", "LoadState", "PrevSlot", "NextSlot", "FastForwardHold", "FastForwardToggle", "ResetTimer"
};

const char* const s_apchzButton[BTN_MAX] =
{
	"Up", "Down", "Left", "Right", "Square", "Circle", "X", "Triangle",
	"Start", "Select", "L1", "R1", "L2", "R2", "L3", "R3"
};

void SetDefaultModsSettings()
{
	s_mods = MODSSETTINGS{};
	s_mods.combos[MODACT_SaveState] = { BTN_SELECT, BTN_R1 };
	s_mods.combos[MODACT_LoadState] = { BTN_SELECT, BTN_L1 };
	s_mods.combos[MODACT_PrevSlot] = { BTN_SELECT, BTN_LEFT };
	s_mods.combos[MODACT_NextSlot] = { BTN_SELECT, BTN_RIGHT };
	s_mods.combos[MODACT_FastForwardHold] = { BTN_SELECT, BTN_R2 };
	s_mods.combos[MODACT_FastForwardToggle] = { BTN_SELECT, BTN_L2 };
	s_mods.keys[MODACT_SaveState] = GLFW_KEY_F5;
	s_mods.keys[MODACT_LoadState] = GLFW_KEY_F8;
	s_mods.keys[MODACT_PrevSlot] = GLFW_KEY_F6;
	s_mods.keys[MODACT_NextSlot] = GLFW_KEY_F7;
	s_mods.keys[MODACT_FastForwardHold] = GLFW_KEY_TAB;
	s_mods.keys[MODACT_FastForwardToggle] = GLFW_KEY_F9;
	s_mods.keys[MODACT_ResetTimer] = GLFW_KEY_F4;
}

// -----------------------------------------------------------------------------
// Speedrun timer
// -----------------------------------------------------------------------------
struct MODTIMER
{
	// settings (Mods.ini [Timer])
	bool fEnabled = false;
	int mode = 0;               // 0 = Real Time, 1 = Loads Removed
	bool fLevel = true;
	bool fSplit = true;
	bool fMs = true;
	int position = 3;           // 0 TL, 1 TR, 2 BL, 3 BR
	bool fAutoStart = true;

	// run state
	bool fRunning = false;
	double total = 0.0;
	double level = 0.0;
	double tLast = 0.0;
	bool fSkipNext = false;     // drop the next frame's time (it contains a load)
	std::string levelName;
	std::string splitName;
	double splitTime = -1.0;
};
MODTIMER s_timer;

const char* const s_apchzTimerMode[] = { "Real Time", "Loads Removed" };
const char* const s_apchzTimerPos[] = { "Top Left", "Top Right", "Bottom Left", "Bottom Right" };

void FormatTime(char* buffer, size_t cb, double seconds, bool fMs)
{
	if (seconds < 0.0)
		seconds = 0.0;
	const long long totalMs = static_cast<long long>(seconds * 1000.0);
	const long long h = totalMs / 3600000;
	const int m = static_cast<int>((totalMs / 60000) % 60);
	const int s = static_cast<int>((totalMs / 1000) % 60);
	const int ms = static_cast<int>(totalMs % 1000);
	if (h > 0)
	{
		if (fMs) std::snprintf(buffer, cb, "%lld:%02d:%02d.%03d", h, m, s, ms);
		else std::snprintf(buffer, cb, "%lld:%02d:%02d", h, m, s);
	}
	else
	{
		if (fMs) std::snprintf(buffer, cb, "%d:%02d.%03d", m, s, ms);
		else std::snprintf(buffer, cb, "%d:%02d", m, s);
	}
}

std::filesystem::path ExeDirectory()
{
	wchar_t path[MAX_PATH]{};
	const DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
	if (len == 0 || len >= MAX_PATH)
		return std::filesystem::current_path();
	return std::filesystem::path(path).parent_path();
}

std::wstring IniPath()
{
	return (ExeDirectory() / L"Mods.ini").wstring();
}

std::filesystem::path StatePath(int slot)
{
	return ExeDirectory() / L"SaveStates" / (L"slot" + std::to_wstring(slot + 1) + L".state");
}

int ReadIniInt(const wchar_t* section, const char* key, int def)
{
	const std::wstring wkey(key, key + std::strlen(key));
	return static_cast<int>(GetPrivateProfileIntW(section, wkey.c_str(), def, IniPath().c_str()));
}

void WriteIniInt(const wchar_t* section, const char* key, int value)
{
	const std::wstring wkey(key, key + std::strlen(key));
	WritePrivateProfileStringW(section, wkey.c_str(), std::to_wstring(value).c_str(), IniPath().c_str());
}

void SaveModsSettings()
{
	WriteIniInt(L"Mods", "Enabled", s_mods.fEnabled ? 1 : 0);
	WriteIniInt(L"Mods", "Slot", s_mods.slot + 1);
	WriteIniInt(L"Mods", "FastForwardSpeed", s_mods.ffSpeed);
	WriteIniInt(L"Timer", "Enabled", s_timer.fEnabled ? 1 : 0);
	WriteIniInt(L"Timer", "Mode", s_timer.mode);
	WriteIniInt(L"Timer", "ShowLevel", s_timer.fLevel ? 1 : 0);
	WriteIniInt(L"Timer", "ShowSplit", s_timer.fSplit ? 1 : 0);
	WriteIniInt(L"Timer", "Milliseconds", s_timer.fMs ? 1 : 0);
	WriteIniInt(L"Timer", "Position", s_timer.position);
	WriteIniInt(L"Timer", "AutoStart", s_timer.fAutoStart ? 1 : 0);
	for (int i = 0; i < MODACT_Max; ++i)
	{
		const std::string key = s_apchzIniKey[i];
		WriteIniInt(L"Combos", (key + "Modifier").c_str(), s_mods.combos[i].btnModifier);
		WriteIniInt(L"Combos", (key + "Button").c_str(), s_mods.combos[i].btnAction);
		WriteIniInt(L"Keys", key.c_str(), s_mods.keys[i]);
	}
}

void LoadModsSettings()
{
	SetDefaultModsSettings();
	if (!std::filesystem::exists(IniPath()))
	{
		SaveModsSettings();
		return;
	}

	s_mods.fEnabled = ReadIniInt(L"Mods", "Enabled", 1) != 0;
	s_timer.fEnabled = ReadIniInt(L"Timer", "Enabled", 0) != 0;
	s_timer.mode = std::clamp(ReadIniInt(L"Timer", "Mode", 0), 0, 1);
	s_timer.fLevel = ReadIniInt(L"Timer", "ShowLevel", 1) != 0;
	s_timer.fSplit = ReadIniInt(L"Timer", "ShowSplit", 1) != 0;
	s_timer.fMs = ReadIniInt(L"Timer", "Milliseconds", 1) != 0;
	s_timer.position = std::clamp(ReadIniInt(L"Timer", "Position", 3), 0, 3);
	s_timer.fAutoStart = ReadIniInt(L"Timer", "AutoStart", 1) != 0;
	s_mods.slot = std::clamp(ReadIniInt(L"Mods", "Slot", 1) - 1, 0, kModStateSlots - 1);
	s_mods.ffSpeed = ReadIniInt(L"Mods", "FastForwardSpeed", s_mods.ffSpeed);
	if (std::find(std::begin(s_affSpeeds), std::end(s_affSpeeds), s_mods.ffSpeed) == std::end(s_affSpeeds))
		s_mods.ffSpeed = 3;

	for (int i = 0; i < MODACT_Max; ++i)
	{
		const std::string key = s_apchzIniKey[i];
		COMBO& combo = s_mods.combos[i];
		combo.btnModifier = ReadIniInt(L"Combos", (key + "Modifier").c_str(), combo.btnModifier);
		combo.btnAction = ReadIniInt(L"Combos", (key + "Button").c_str(), combo.btnAction);
		if (combo.btnModifier < 0 || combo.btnModifier >= BTN_MAX ||
			combo.btnAction < 0 || combo.btnAction >= BTN_MAX || combo.btnModifier == combo.btnAction)
			combo = COMBO{};
		s_mods.keys[i] = ReadIniInt(L"Keys", key.c_str(), s_mods.keys[i]);
	}
}

// -----------------------------------------------------------------------------
// On-screen notices
// -----------------------------------------------------------------------------
std::string s_notice;
double s_tNoticeEnd = 0.0;

void Notify(const char* fmt, ...)
{
	char buffer[160];
	va_list args;
	va_start(args, fmt);
	std::vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	s_notice = buffer;
	s_tNoticeEnd = glfwGetTime() + 2.5;
	std::printf("[MODS] %s\n", buffer);
}

// -----------------------------------------------------------------------------
// Position save states
// -----------------------------------------------------------------------------
constexpr uint32_t kStateMagic = 0x53435043; // 'CPCS'
constexpr uint32_t kStateVersion = 1;

struct MODSTATE
{
	uint32_t magic = kStateMagic;
	uint32_t version = kStateVersion;
	uint32_t cbGs = sizeof(GS);
	uint32_t levelSearchKey = 0;
	GS gs{};
	CBitArray4096 abitChkCur{};   // object state at the moment of saving
	CS csCheckpoint{};            // the real last checkpoint (death returns here)
	int fCheckpoint = 0;
	glm::vec3 pos{};
	glm::mat3 mat{ 1.0f };
	char levelName[64]{};
};

bool s_fLoadPending = false;
MODSTATE s_stateLoading{};

bool FWorldReadyForStates(const char** ppchzWhy)
{
	if (g_psw == nullptr || g_transition.m_plevelCurrent == nullptr || g_pgsCur == nullptr)
	{
		*ppchzWhy = "not in a level";
		return false;
	}
	if (g_transition.m_fPending != 0 || g_wipe.wipes != WIPES_Idle || s_fLoadPending)
	{
		*ppchzWhy = "a level is loading";
		return false;
	}
	if (PpoCur() == nullptr)
	{
		*ppchzWhy = "no player";
		return false;
	}
	return true;
}

bool FReadState(int slot, MODSTATE* pstate)
{
	std::ifstream in(StatePath(slot), std::ios::binary);
	if (!in)
		return false;
	in.read(reinterpret_cast<char*>(pstate), sizeof(*pstate));
	return in.gcount() == static_cast<std::streamsize>(sizeof(*pstate)) &&
		pstate->magic == kStateMagic && pstate->version == kStateVersion && pstate->cbGs == sizeof(GS);
}

void TeleportPlayer(const glm::vec3& pos, const glm::mat3& mat)
{
	PO* ppo = PpoCur();
	if (ppo == nullptr)
		return;

	glm::mat3 matCopy = mat;
	glm::vec3 posCopy = pos;
	ppo->pvtalo->pfnRotateAloToMat(ppo, &matCopy);
	TranslateSoToPosSafe(ppo, &posCopy, &g_normalZ);
	ppo->pvtalo->pfnSetAloVelocityVec(ppo, &g_vecZero);
	ppo->pvtalo->pfnSetAloAngularVelocityVec(ppo, &g_vecZero);
	// Snap the camera behind the restored position instead of panning there.
	SetCmCut(g_pcm, nullptr);
}

// -----------------------------------------------------------------------------
// Fast forward
// -----------------------------------------------------------------------------
bool s_fFastForwardToggled = false;
bool s_fFastForwardHeld = false;

// -----------------------------------------------------------------------------
// Combo / modifier handling
// -----------------------------------------------------------------------------
std::array<bool, BTN_MAX> s_rawNow{};
std::array<bool, BTN_MAX> s_rawPrev{};
std::array<bool, BTN_MAX> s_modifierDown{};      // modifier held and hidden from the game
std::array<bool, BTN_MAX> s_modifierUsed{};      // a combo fired during this hold
std::array<bool, BTN_MAX> s_suppressed{};        // hidden until physically released
std::array<bool, GLFW_KEY_LAST + 1> s_keyPrev{};

int s_captureAction = -1;   // MODACT being captured, -1 = none
bool s_captureArmed = false;
int s_captureFirst = -1;
double s_tCaptureStart = 0.0;

bool FIsModifier(int btn)
{
	for (const COMBO& combo : s_mods.combos)
		if (combo.btnModifier == btn && combo.btnAction >= 0)
			return true;
	return false;
}

void RunAction(MODACT modact)
{
	switch (modact)
	{
		case MODACT_SaveState: FSaveModState(s_mods.slot); break;
		case MODACT_LoadState: FLoadModState(s_mods.slot); break;
		case MODACT_PrevSlot:
			s_mods.slot = (s_mods.slot + kModStateSlots - 1) % kModStateSlots;
			SaveModsSettings();
			Notify("State slot %d", s_mods.slot + 1);
			break;
		case MODACT_NextSlot:
			SelectNextModStateSlot();
			Notify("State slot %d", s_mods.slot + 1);
			break;
		case MODACT_FastForwardToggle:
			s_fFastForwardToggled = !s_fFastForwardToggled;
			Notify(s_fFastForwardToggled ? "Fast forward %dx on" : "Fast forward off", s_mods.ffSpeed);
			break;
		case MODACT_ResetTimer:
			ResetModTimer();
			break;
		default:
			break;
	}
}

bool FStateActionsAllowed()
{
	// Combos and hotkeys act only during normal play, never inside menus.
	// Note: the prompt keeps its last PRK after it closes (it only fades the
	// blot out), so "is a menu open" must be judged by the blot's visibility.
	return g_psw != nullptr && g_ui.uis == UIS_Playing && g_prompt.blots == BLOTS_Hidden;
}
} // namespace

// =============================================================================
// Public API
// =============================================================================
void StartupMods()
{
	LoadModsSettings();
	std::error_code error;
	std::filesystem::create_directories(ExeDirectory() / L"SaveStates", error);
}

bool FModsEnabled()
{
	return s_mods.fEnabled;
}

void ToggleModsEnabled()
{
	s_mods.fEnabled = !s_mods.fEnabled;
	if (!s_mods.fEnabled)
	{
		s_fFastForwardToggled = false;
		s_fFastForwardHeld = false;
	}
	SaveModsSettings();
}

int ModStateSlot()
{
	return s_mods.slot;
}

void SelectNextModStateSlot()
{
	s_mods.slot = (s_mods.slot + 1) % kModStateSlots;
	SaveModsSettings();
}

int ModFastForwardSpeed()
{
	return s_mods.ffSpeed;
}

void SelectNextModFastForwardSpeed()
{
	const int count = static_cast<int>(std::size(s_affSpeeds));
	int index = 0;
	for (int i = 0; i < count; ++i)
		if (s_affSpeeds[i] == s_mods.ffSpeed)
			index = i;
	s_mods.ffSpeed = s_affSpeeds[(index + 1) % count];
	SaveModsSettings();
}

bool FSaveModState(int slot)
{
	const char* why = nullptr;
	if (!FWorldReadyForStates(&why))
	{
		Notify("Can't save state: %s", why);
		return false;
	}

	PO* ppo = PpoCur();
	MODSTATE state{};
	state.levelSearchKey = g_transition.m_plevelCurrent->searchKey;
	std::snprintf(state.levelName, sizeof(state.levelName), "%s", g_transition.m_worldCurrent.c_str());
	state.gs = *g_pgsCur;
	state.abitChkCur = g_chkmgr.abitChk;
	state.csCheckpoint = g_chkmgr.csSaved;
	state.fCheckpoint = g_chkmgr.fCheckpoint ? 1 : 0;
	state.pos = ppo->xf.posWorld;
	state.mat = ppo->xf.matWorld;

	std::error_code error;
	std::filesystem::create_directories(StatePath(slot).parent_path(), error);
	std::ofstream out(StatePath(slot), std::ios::binary | std::ios::trunc);
	if (!out)
	{
		Notify("Can't save state: could not write file");
		return false;
	}
	out.write(reinterpret_cast<const char*>(&state), sizeof(state));
	Notify("State %d saved (%s)", slot + 1, state.levelName);
	return true;
}

bool FLoadModState(int slot)
{
	const char* why = nullptr;
	if (!FWorldReadyForStates(&why))
	{
		Notify("Can't load state: %s", why);
		return false;
	}

	MODSTATE state{};
	if (!FReadState(slot, &state))
	{
		Notify("State %d is empty", slot + 1);
		return false;
	}

	LEVELINFO* plevel = PlevelinfoFromSearchKey(state.levelSearchKey);
	if (plevel == nullptr)
	{
		Notify("State %d: level not found", slot + 1);
		return false;
	}

	TRANS trans{};
	trans.fSet = 1;
	trans.plevel = plevel;
	trans.oidWarp = state.csCheckpoint.oidWarp;
	trans.oidWarpContext = state.csCheckpoint.oidWarpContext;
	trans.grftrans = static_cast<GRFTRANS>(FTRANS_Checkpoint);
	ActivateWipe(&g_wipe, &trans, WIPEK_Fade);

	if (g_wipe.wipes == WIPES_Idle)
	{
		Notify("Can't load state right now");
		return false;
	}

	// The fade has started; the level reloads when it reaches black.
	// Progress first (coins, keys, bottles, lives...). Keep the player's
	// current settings flags (camera inversion etc.) instead of the saved ones.
	const GRFGS grfgsSettings = g_pgsCur->grfgs;
	*g_pgsCur = state.gs;
	g_pgsCur->grfgs = grfgsSettings;

	// Reload through the game's own checkpoint path: SetupGame restores the
	// object bits from csSaved, so put the save-state bits there for this load.
	g_chkmgr.csSaved.abitChk = state.abitChkCur;
	g_chkmgr.csSaved.oidWarp = state.csCheckpoint.oidWarp;
	g_chkmgr.csSaved.oidWarpContext = state.csCheckpoint.oidWarpContext;
	g_chkmgr.fCheckpoint = true;

	s_stateLoading = state;
	s_fLoadPending = true;
	s_fFastForwardToggled = false;
	Notify("Loading state %d...", slot + 1);
	return true;
}

void OnModsWorldLoaded()
{
	// Speedrun timer: the load itself is not counted in Loads Removed mode,
	// and moving to a different level records a split.
	s_timer.fSkipNext = true;
	const std::string world = g_transition.m_worldCurrent;
	if (!s_fLoadPending && world != s_timer.levelName)
	{
		if (s_timer.fRunning && !s_timer.levelName.empty())
		{
			s_timer.splitName = s_timer.levelName;
			s_timer.splitTime = s_timer.level;
		}
		s_timer.level = 0.0;
		s_timer.levelName = world;
	}

	if (!s_fLoadPending)
		return;
	s_fLoadPending = false;

	if (g_psw == nullptr || g_transition.m_plevelCurrent == nullptr ||
		g_transition.m_plevelCurrent->searchKey != s_stateLoading.levelSearchKey)
	{
		Notify("State load interrupted");
		return;
	}

	TeleportPlayer(s_stateLoading.pos, s_stateLoading.mat);

	// Dying after a state load returns to the real checkpoint, not the state.
	g_chkmgr.csSaved = s_stateLoading.csCheckpoint;
	g_chkmgr.fCheckpoint = s_stateLoading.fCheckpoint != 0;
	g_chkmgr.fChkDirty = true;

	Notify("State loaded");
}

int CModsGameUpdates()
{
	if (!s_mods.fEnabled || g_psw == nullptr || s_fLoadPending)
		return 1;
	if (g_transition.m_fPending != 0 || g_wipe.wipes != WIPES_Idle)
		return 1;
	return (s_fFastForwardHeld || s_fFastForwardToggled) ? (std::max)(1, s_mods.ffSpeed) : 1;
}

void UpdateModsInput()
{
	// Raw state as the game's input layer just produced it.
	s_rawPrev = s_rawNow;
	for (int b = 0; b < BTN_MAX; ++b)
		s_rawNow[b] = g_joy.current[static_cast<JOY_BUTTON>(b)];

	auto hideFromGame = [](int b)
	{
		g_joy.current[static_cast<JOY_BUTTON>(b)] = false;
		g_joy.previous[static_cast<JOY_BUTTON>(b)] = false;
	};

	// While a combo is being recorded, the game sees nothing.
	if (s_captureAction >= 0)
	{
		for (int b = 0; b < BTN_MAX; ++b)
			hideFromGame(b);
		return;
	}

	s_fFastForwardHeld = false;
	if (!s_mods.fEnabled)
	{
		s_modifierDown.fill(false);
		s_suppressed.fill(false);
		return;
	}

	const bool fActions = FStateActionsAllowed();

	// --- Keyboard hotkeys ------------------------------------------------------
	if (g_gl.window != nullptr && glfwGetWindowAttrib(g_gl.window, GLFW_FOCUSED) == GLFW_TRUE)
	{
		for (int i = 0; i < MODACT_Max; ++i)
		{
			// Keys or mouse buttons (Controls > Mouse & Keyboard Mapping).
			static bool s_afActionPrev[MODACT_Max] = {};
			const int key = s_mods.keys[i];
			bool down = false;
			if (FMouseBinding(key))
				down = FMouseBindingHeld(key);
			else if (key > GLFW_KEY_UNKNOWN && key <= GLFW_KEY_LAST)
				down = glfwGetKey(g_gl.window, key) == GLFW_PRESS;
			else
				continue;
			if (i == MODACT_FastForwardHold)
				s_fFastForwardHeld |= down && g_psw != nullptr;
			else if (down && !s_afActionPrev[i] && (fActions || i == MODACT_FastForwardToggle || i == MODACT_ResetTimer))
				RunAction(static_cast<MODACT>(i));
			s_afActionPrev[i] = down;
		}
	}

	// --- Buttons that stay hidden until released ----------------------------
	for (int b = 0; b < BTN_MAX; ++b)
	{
		if (s_suppressed[b])
		{
			if (s_rawNow[b]) hideFromGame(b);
			else s_suppressed[b] = false;
		}
	}

	// --- Modifiers: hidden while held, delivered as a tap on release ---------
	// Combos are only live during gameplay; in menus the modifier is a
	// normal button again.
	for (int b = 0; b < BTN_MAX; ++b)
	{
		if (!FIsModifier(b) || !fActions)
		{
			if (s_modifierDown[b] && !s_rawNow[b])
				s_modifierDown[b] = false;
			continue;
		}

		const bool pressed = s_rawNow[b] && !s_rawPrev[b];
		if (pressed && !s_suppressed[b])
		{
			s_modifierDown[b] = true;
			s_modifierUsed[b] = false;
		}

		if (!s_modifierDown[b])
			continue;

		if (s_rawNow[b])
		{
			hideFromGame(b);
		}
		else
		{
			s_modifierDown[b] = false;
			if (!s_modifierUsed[b])
			{
				// Tap: let the game see a press this frame and the release next.
				g_joy.current[static_cast<JOY_BUTTON>(b)] = true;
				g_joy.previous[static_cast<JOY_BUTTON>(b)] = false;
			}
		}
	}

	// --- Combos ------------------------------------------------------------------
	for (int i = 0; i < MODACT_Max; ++i)
	{
		const COMBO& combo = s_mods.combos[i];
		if (combo.btnModifier < 0 || combo.btnAction < 0 || !s_modifierDown[combo.btnModifier])
			continue;

		const int act = combo.btnAction;
		const bool held = s_rawNow[act];
		const bool pressed = held && !s_rawPrev[act];

		if (i == MODACT_FastForwardHold)
		{
			if (held)
			{
				s_fFastForwardHeld = true;
				s_modifierUsed[combo.btnModifier] = true;
				s_suppressed[act] = true;
				hideFromGame(act);
			}
			continue;
		}

		if (pressed)
		{
			s_modifierUsed[combo.btnModifier] = true;
			s_suppressed[act] = true;
			hideFromGame(act);
			RunAction(static_cast<MODACT>(i));
		}
	}
}

namespace
{
void DrawModsTimer()
{
	if (!s_timer.fEnabled)
		return;

	char total[32];
	FormatTime(total, sizeof(total), s_timer.total, s_timer.fMs);
	char lines[2][96] = {};
	int cLines = 0;
	if (s_timer.fLevel)
	{
		char level[32];
		FormatTime(level, sizeof(level), s_timer.level, s_timer.fMs);
		std::snprintf(lines[cLines++], sizeof(lines[0]), "Level  %s", level);
	}
	if (s_timer.fSplit && s_timer.splitTime >= 0.0)
	{
		char split[32];
		FormatTime(split, sizeof(split), s_timer.splitTime, s_timer.fMs);
		std::snprintf(lines[cLines++], sizeof(lines[0]), "%s  %s", s_timer.splitName.c_str(), split);
	}

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const ImVec2 display = ImGui::GetIO().DisplaySize;
	const float scale = (std::max)(1.0f, display.y / 720.0f);
	const float bigSize = ImGui::GetFontSize() * 1.9f * scale;
	const float smallSize = ImGui::GetFontSize() * 1.0f * scale;
	ImFont* font = ImGui::GetFont();

	const ImVec2 bigExt = font->CalcTextSizeA(bigSize, FLT_MAX, 0.0f, total);
	float width = bigExt.x;
	for (int i = 0; i < cLines; ++i)
		width = (std::max)(width, font->CalcTextSizeA(smallSize, FLT_MAX, 0.0f, lines[i]).x);
	const float lineH = smallSize * 1.2f;
	const float height = bigExt.y + lineH * cLines;
	const float pad = 10.0f * scale;
	const float margin = 16.0f * scale;

	const bool fRight = s_timer.position == 1 || s_timer.position == 3;
	const bool fBottom = s_timer.position >= 2;
	float x = fRight ? display.x - margin - width : margin;
	float y = fBottom ? display.y - margin - height : margin;
	// Stay clear of the FPS counter (top right) and the mod notices (top left).
	if (!fBottom)
		y += 48.0f * scale;

	draw->AddRectFilled(ImVec2(x - pad, y - pad), ImVec2(x + width + pad, y + height + pad),
		IM_COL32(0, 0, 0, 170), 8.0f * scale);
	const ImU32 colBig = !s_timer.fRunning ? IM_COL32(170, 170, 170, 255)
		: (s_timer.mode == 1 ? IM_COL32(120, 200, 255, 255) : IM_COL32(120, 255, 140, 255));
	draw->AddText(font, bigSize, ImVec2(fRight ? x + width - bigExt.x : x, y), colBig, total);
	for (int i = 0; i < cLines; ++i)
	{
		const float w = font->CalcTextSizeA(smallSize, FLT_MAX, 0.0f, lines[i]).x;
		draw->AddText(font, smallSize, ImVec2(fRight ? x + width - w : x, y + bigExt.y + lineH * i),
			IM_COL32(220, 220, 220, 255), lines[i]);
	}
}
} // namespace

void ResetModTimer()
{
	s_timer.fRunning = true;
	s_timer.total = 0.0;
	s_timer.level = 0.0;
	s_timer.splitTime = -1.0;
	s_timer.splitName.clear();
	s_timer.tLast = glfwGetTime();
	if (s_timer.fEnabled)
		Notify("Timer reset");
}

void OnModsNewGame()
{
	if (s_timer.fAutoStart)
	{
		ResetModTimer();
		s_timer.levelName.clear();   // the first level load is not a split
	}
}

void UpdateModsTimer()
{
	const double now = glfwGetTime();
	double dt = s_timer.tLast > 0.0 ? now - s_timer.tLast : 0.0;
	s_timer.tLast = now;
	const bool fSkip = s_timer.fSkipNext;
	s_timer.fSkipNext = false;
	if (!s_timer.fEnabled || !s_timer.fRunning)
		return;
	if (dt < 0.0 || dt > 60.0)
		dt = 0.0;
	if (s_timer.mode == 1 && (fSkip || g_psw == nullptr || g_transition.m_fPending != 0))
		return;   // loads removed
	s_timer.total += dt;
	s_timer.level += dt;
}

void ModTimerOption(MODTIMEROPT opt)
{
	switch (opt)
	{
		case MODTIMEROPT_Enable:
			s_timer.fEnabled = !s_timer.fEnabled;
			if (s_timer.fEnabled && !s_timer.fRunning)
				ResetModTimer();
			break;
		case MODTIMEROPT_Mode: s_timer.mode = (s_timer.mode + 1) % 2; break;
		case MODTIMEROPT_Level: s_timer.fLevel = !s_timer.fLevel; break;
		case MODTIMEROPT_Split: s_timer.fSplit = !s_timer.fSplit; break;
		case MODTIMEROPT_Ms: s_timer.fMs = !s_timer.fMs; break;
		case MODTIMEROPT_Position: s_timer.position = (s_timer.position + 1) % 4; break;
		case MODTIMEROPT_AutoStart: s_timer.fAutoStart = !s_timer.fAutoStart; break;
		case MODTIMEROPT_Reset: ResetModTimer(); break;
		default: break;
	}
	SaveModsSettings();
}

void DrawModsOverlay()
{
	if (ImGui::GetCurrentContext() == nullptr)
		return;

	DrawModsTimer();

	std::string text;
	if (glfwGetTime() < s_tNoticeEnd)
		text = s_notice;
	if (CModsGameUpdates() > 1)
	{
		char ff[32];
		std::snprintf(ff, sizeof(ff), ">> %dx", CModsGameUpdates());
		text = text.empty() ? std::string(ff) : std::string(ff) + "   " + text;
	}
	if (text.empty())
		return;

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const float scale = (std::max)(1.0f, ImGui::GetIO().DisplaySize.y / 720.0f);
	const float fontSize = ImGui::GetFontSize() * 1.25f * scale;
	const ImVec2 size = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.c_str());
	const ImVec2 pos(16.0f * scale, 16.0f * scale);
	const float pad = 8.0f * scale;
	draw->AddRectFilled(ImVec2(pos.x - pad, pos.y - pad),
		ImVec2(pos.x + size.x + pad, pos.y + size.y + pad), IM_COL32(0, 0, 0, 170), 6.0f * scale);
	draw->AddText(ImGui::GetFont(), fontSize, pos, IM_COL32(255, 255, 255, 255), text.c_str());
}

const char* PchzModAction(MODACT modact)
{
	return (modact >= 0 && modact < MODACT_Max) ? s_apchzAction[modact] : "";
}

const char* PchzModCombo(MODACT modact)
{
	static char buffer[64];
	if (modact == s_captureAction)
	{
		if (s_captureFirst < 0)
			return "Hold a button...";
		std::snprintf(buffer, sizeof(buffer), "%s + ...", s_apchzButton[s_captureFirst]);
		return buffer;
	}
	const COMBO& combo = s_mods.combos[modact];
	if (combo.btnModifier < 0 || combo.btnAction < 0)
		return "Off";
	std::snprintf(buffer, sizeof(buffer), "%s + %s", s_apchzButton[combo.btnModifier], s_apchzButton[combo.btnAction]);
	return buffer;
}

void BeginModComboCapture(MODACT modact)
{
	s_captureAction = modact;
	s_captureArmed = false;
	s_captureFirst = -1;
	s_tCaptureStart = glfwGetTime();
}

bool FModComboCapturing()
{
	return s_captureAction >= 0;
}

bool UpdateModComboCapture()
{
	if (s_captureAction < 0)
		return true;

	// Escape cancels and leaves the old binding.
	if (g_gl.window != nullptr && glfwGetKey(g_gl.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		s_captureAction = -1;
		return true;
	}

	bool anyHeld = false;
	for (int b = 0; b < BTN_MAX; ++b)
		anyHeld |= s_rawNow[b];

	// Wait until the button that opened the capture is released.
	if (!s_captureArmed)
	{
		if (!anyHeld)
			s_captureArmed = true;
		return false;
	}

	if (s_captureFirst < 0)
	{
		for (int b = 0; b < BTN_MAX; ++b)
		{
			if (s_rawNow[b] && !s_rawPrev[b])
			{
				s_captureFirst = b;
				break;
			}
		}
		// Nothing pressed for 8 seconds: cancel.
		if (s_captureFirst < 0 && glfwGetTime() - s_tCaptureStart > 8.0)
		{
			s_captureAction = -1;
			return true;
		}
		return false;
	}

	if (!s_rawNow[s_captureFirst])
	{
		// Pressed and released one button on its own: clear this combo.
		s_mods.combos[s_captureAction] = COMBO{};
		SaveModsSettings();
		s_captureAction = -1;
		return true;
	}

	for (int b = 0; b < BTN_MAX; ++b)
	{
		if (b != s_captureFirst && s_rawNow[b] && !s_rawPrev[b])
		{
			s_mods.combos[s_captureAction] = { s_captureFirst, b };
			s_suppressed[s_captureFirst] = true;
			s_suppressed[b] = true;
			SaveModsSettings();
			s_captureAction = -1;
			return true;
		}
	}
	return false;
}

void RefreshModsMenuStrings()
{
	std::snprintf(g_achzModsMenu[0], 64, "Mods: %s", s_mods.fEnabled ? "On" : "Off");

	MODSTATE state{};
	if (FReadState(s_mods.slot, &state))
		std::snprintf(g_achzModsMenu[1], 64, "Load State %d (%s)", s_mods.slot + 1, state.levelName);
	else
		std::snprintf(g_achzModsMenu[1], 64, "Load State %d (empty)", s_mods.slot + 1);
	std::snprintf(g_achzModsMenu[2], 64, "Save State %d", s_mods.slot + 1);
	std::snprintf(g_achzModsMenu[3], 64, "State Slot: %d", s_mods.slot + 1);
	std::snprintf(g_achzModsMenu[4], 64, "Fast Forward Speed: %dx", s_mods.ffSpeed);
	for (int i = 0; i < MODACT_Max; ++i)
		std::snprintf(g_achzModsMenu[5 + i], 64, "%s: %s", s_apchzAction[i], PchzModCombo(static_cast<MODACT>(i)));

	char (*t)[64] = &g_achzModsMenu[kModsMenuTimer];
	std::snprintf(t[MODTIMEROPT_Enable], 64, "Speedrun Timer: %s", s_timer.fEnabled ? "On" : "Off");
	std::snprintf(t[MODTIMEROPT_Mode], 64, "Timing: %s", s_apchzTimerMode[s_timer.mode]);
	std::snprintf(t[MODTIMEROPT_Level], 64, "Level Time: %s", s_timer.fLevel ? "Show" : "Hide");
	std::snprintf(t[MODTIMEROPT_Split], 64, "Last Split: %s", s_timer.fSplit ? "Show" : "Hide");
	std::snprintf(t[MODTIMEROPT_Ms], 64, "Milliseconds: %s", s_timer.fMs ? "On" : "Off");
	std::snprintf(t[MODTIMEROPT_Position], 64, "Position: %s", s_apchzTimerPos[s_timer.position]);
	std::snprintf(t[MODTIMEROPT_AutoStart], 64, "Start on New Game: %s", s_timer.fAutoStart ? "On" : "Off");
	std::snprintf(t[MODTIMEROPT_Reset], 64, "Reset Timer");
}

// Options > Reset to Default: mod settings, combos, hotkeys and timer options
// back to their defaults (the timer's current time is kept).
void ResetModsSettings()
{
	SetDefaultModsSettings();
	s_timer.fEnabled = false;
	s_timer.mode = 0;
	s_timer.fLevel = true;
	s_timer.fSplit = true;
	s_timer.fMs = true;
	s_timer.position = 3;
	s_timer.fAutoStart = true;
	SaveModsSettings();
	RefreshModsMenuStrings();
}

// Settings import: re-read Mods.ini.
void ReloadModsSettings()
{
	LoadModsSettings();
	RefreshModsMenuStrings();
}

// Keyboard / mouse hotkey for an action (shown in Mouse & Keyboard Mapping).
int ModActionKey(MODACT modact)
{
	return (modact >= 0 && modact < MODACT_Max) ? s_mods.keys[modact] : GLFW_KEY_UNKNOWN;
}

void SetModActionKey(MODACT modact, int key)
{
	if (modact < 0 || modact >= MODACT_Max)
		return;
	s_mods.keys[modact] = key;
	SaveModsSettings();
	RefreshModsMenuStrings();
}

// Changed-setting markers / single-setting reset (Options menus).
// which: 0 = Mods On, 1 = Fast Forward Speed, 2 + MODTIMEROPT = timer options.
bool FModOptionDefault(int which)
{
	switch (which)
	{
	case 0: return s_mods.fEnabled;
	case 1: return s_mods.ffSpeed == 3;
	case 2 + MODTIMEROPT_Enable: return !s_timer.fEnabled;
	case 2 + MODTIMEROPT_Mode: return s_timer.mode == 0;
	case 2 + MODTIMEROPT_Level: return s_timer.fLevel;
	case 2 + MODTIMEROPT_Split: return s_timer.fSplit;
	case 2 + MODTIMEROPT_Ms: return s_timer.fMs;
	case 2 + MODTIMEROPT_Position: return s_timer.position == 3;
	case 2 + MODTIMEROPT_AutoStart: return s_timer.fAutoStart;
	default: return true;
	}
}

void ResetModOption(int which)
{
	switch (which)
	{
	case 0: s_mods.fEnabled = true; break;
	case 1: s_mods.ffSpeed = 3; break;
	case 2 + MODTIMEROPT_Enable: s_timer.fEnabled = false; break;
	case 2 + MODTIMEROPT_Mode: s_timer.mode = 0; break;
	case 2 + MODTIMEROPT_Level: s_timer.fLevel = true; break;
	case 2 + MODTIMEROPT_Split: s_timer.fSplit = true; break;
	case 2 + MODTIMEROPT_Ms: s_timer.fMs = true; break;
	case 2 + MODTIMEROPT_Position: s_timer.position = 3; break;
	case 2 + MODTIMEROPT_AutoStart: s_timer.fAutoStart = true; break;
	default: return;
	}
	SaveModsSettings();
	RefreshModsMenuStrings();
}

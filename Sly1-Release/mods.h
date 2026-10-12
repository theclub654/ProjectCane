#pragma once

// -----------------------------------------------------------------------------
// ProjectCane "Mods": quality-of-life features layered on the port.
//
//   * Position save states  - level, Sly's exact position/facing, the level's
//     checkpoint bits (broken/collected/disabled objects) and game progress
//     (coins, keys, bottles, lives, charms). Loading reloads the level at that
//     state; guards and moving objects restart from their spawn.
//   * Fast forward          - hold or toggle, 2x/3x/4x/8x game speed.
//   * Controller combos     - "modifier + button" chords (default Select + X)
//     plus keyboard hotkeys. A tap of the modifier alone still reaches the game
//     (on release), so Select keeps opening the map.
//   * Mods menu             - Options > Mods... in the pause menu.
//
// Settings live in Mods.ini, states in SaveStates\slotN.state next to the exe.
// -----------------------------------------------------------------------------

enum MODACT
{
	MODACT_SaveState = 0,
	MODACT_LoadState,
	MODACT_PrevSlot,
	MODACT_NextSlot,
	MODACT_FastForwardHold,
	MODACT_FastForwardToggle,
	MODACT_ResetTimer,          // speedrun timer: restart from 0:00
	MODACT_Max
};

constexpr int kModStateSlots = 5;

// Lifetime
void StartupMods();

// Call right after g_joy.Update() every frame: hotkeys, combos and modifier
// handling (it may hide combo buttons from the game for this frame).
void UpdateModsInput();

// Number of game-logic updates to run this frame (1 = normal speed).
int CModsGameUpdates();

// Call right after a level transition has executed.
void OnModsWorldLoaded();

// Draw on-screen notices (call between ImGui::NewFrame and ImGui::Render).
void DrawModsOverlay();

// Actions (also used by the Mods menu)
bool FModsEnabled();
void ToggleModsEnabled();
bool FSaveModState(int slot);
bool FLoadModState(int slot);
int  ModStateSlot();
void SelectNextModStateSlot();
int  ModFastForwardSpeed();
void SelectNextModFastForwardSpeed();

// Combo binding UI
const char* PchzModAction(MODACT modact);
const char* PchzModCombo(MODACT modact);   // "Select + R1", "Off", "Press buttons..."
void BeginModComboCapture(MODACT modact);
bool FModComboCapturing();
// Returns true when capture finished (bound, cleared or cancelled) this call.
bool UpdateModComboCapture();

// Text buffers for the Mods menu, refreshed by RefreshModsMenuStrings().
// Indices into g_achzModsMenu:
//   0 enable, 1 load, 2 save, 3 slot, 4 ff speed,
//   5 .. 5+MODACT_Max-1 combo bindings,
//   kModsMenuTimer + MODTIMEROPT timer options.
constexpr int kModsMenuTimer = 5 + MODACT_Max;
extern char g_achzModsMenu[24][64];
void RefreshModsMenuStrings();

// Speedrun timer ---------------------------------------------------------------
enum MODTIMEROPT
{
	MODTIMEROPT_Enable = 0,     // show + run the timer
	MODTIMEROPT_Mode,           // Real Time / Loads Removed
	MODTIMEROPT_Level,          // show the current level's time
	MODTIMEROPT_Split,          // show the last level split
	MODTIMEROPT_Ms,             // milliseconds
	MODTIMEROPT_Position,       // corner of the screen
	MODTIMEROPT_AutoStart,      // restart automatically on New Game
	MODTIMEROPT_Reset,          // action: restart now
	MODTIMEROPT_Max
};
void ModTimerOption(MODTIMEROPT opt);   // toggle / cycle / run (menu)
void ResetModTimer();
void OnModsNewGame();                   // a fresh game was just started
void UpdateModsTimer();                 // once per frame

// Options > Reset to Default.
void ResetModsSettings();
void ReloadModsSettings();   // settings import
int ModActionKey(MODACT modact);             // GLFW key or mouse binding
void SetModActionKey(MODACT modact, int key);
// Changed-setting markers: 0 = Mods On, 1 = FF speed, 2 + MODTIMEROPT = timer options.
bool FModOptionDefault(int which);
void ResetModOption(int which);

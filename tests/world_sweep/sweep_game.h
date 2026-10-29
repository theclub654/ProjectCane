#pragma once
// Entry points between the sweep runner, its child modes and the game code.

#include <string>
#include <vector>

namespace sweep_game
{
// The level the single-level checks (saves, determinism, display) load
// unless --level names another.
inline constexpr const char* kDefaultLevel = "A Stealthy Approach";

// main.cpp's RunGameFrame and FlushTransitionFrame (game_main.cpp).
void RunFrame();
void FlushTransition();

// The 46 levels in g_levelTable order.
std::vector<std::string> LevelNames();

// `cane_world_sweep child <mode> ...`: runs inside a fresh process.
int RunChild(const std::vector<std::string>& args);

// `cane_world_sweep units`: game-code checks that need no data (sweep_units.cpp).
int RunUnits();
}

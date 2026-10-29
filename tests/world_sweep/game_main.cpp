// The game's own main.cpp, compiled into the sweep with its entry point
// renamed. Every other game object is linked unchanged from the ProjectCane
// build; main.cpp.o is the one left out, so this file supplies Startup(),
// fQuitGame and g_fDeveloperDebugMode exactly as the game defines them, and
// exposes the frame step that main.cpp keeps in an anonymous namespace.
#define main cane_game_main
#include "../../Sly1-Release/main.cpp"
#undef main

#include "sweep_game.h"

namespace sweep_game
{
void RunFrame()
{
    RunGameFrame();
}

void FlushTransition()
{
    FlushTransitionFrame();
}
}

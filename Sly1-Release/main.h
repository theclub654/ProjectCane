#pragma once
#include <iostream>
#include <windows.h>
#include <chrono>
#include <thread>

#include "transition.h"
#include "debug.h"
#include "mouse.h"
#include "render.h"

// Game loop
int  main(int cphzArgs, char* aphzArgs[]);
// Initializing all the things needed for the game to run
void Startup();

extern bool fQuitGame;
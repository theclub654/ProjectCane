#pragma once
#include <iostream>
#include <windows.h>

#include "transition.h"
#include "debug.h"
#include "mouse.h"
#include "update.h"
#include "render.h"

// Game loop
int  main(int cphzArgs, char* aphzArgs[]);
// Initializing all the things needed for the game to run
void Startup();

extern float prevTime;
extern float accumulator;
extern unsigned int counter;
extern bool fQuitGame;
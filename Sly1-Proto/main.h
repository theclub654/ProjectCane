#pragma once
#include <iostream>
#include <windows.h>

#include "transition.h"
#include "debug.h"
#include "mouse.h"
#include "update.h"


void RenderSw(SW *psw, CM *pcm);
void RenderSwGlobset(SW *psw, CM *pcm);
void RenderSwAloAll(SW* psw, CM* pcm);
void DrawSw(SW *psw, CM *pcm);
void DrawSwCollisionAll(CM *pcm);
void DeleteWorld(SW *psw);

// Game loop
int  main(int cphzArgs, char* aphzArgs[]);
// Initializing all the things needed for the game to run
void Startup();

extern bool fQuitGame;
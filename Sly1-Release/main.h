#pragma once
#include <iostream>
#include <windows.h>
#include <chrono>
#include <thread>

#include "transition.h"
#include "debug.h"
#include "mouse.h"

// Temporary flag
bool loadEmitMesh = 0;
GL g_gl;
GLSHADER glScreenShader;
GLSHADER glGlobShader;
GLSHADER glCollisionShader;
std::string file;
std::string levelName;
CTransition g_transition;
CLOCK g_clock;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void RenderSw(SW *psw, CM *pcm);
void RenderSwAloAll(SW* psw, CM* pcm);
void RenderSwGlobset(SW *psw, CM *pcm);
void DrawSw(SW *psw, CM *pcm);
void DrawSwCollisionAll(CM *pcm);
void DeleteWorld(SW *psw);

// Game loop
int  main(int cphzArgs, char* aphzArgs[]);
// Initializing all the things needed for the game to run
void Startup();
#pragma once
#include <iostream>
#include <windows.h>

#include "transition.h"
#include "debug.h"
#include "mouse.h"

// Temporary flag
bool loadEmitMesh = 0;
GL g_gl;
GLSHADER glScreenShader;
GLSHADER glGlobShader;
GLSHADER glCelBorderShader;
GLSHADER glCollisionShader;
std::string file;
std::string levelName;
CTransition g_transition;
bool firstClick = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void RenderSw(SW *psw, CM *pcm);
void RenderSwGlobset(SW *psw, CM *pcm);
void DrawSw(SW *psw, CM *pcm);
void DrawSwCollisionAll();
void DeleteWorld(SW *psw);

// Game loop
int  main(int cphzArgs, char* aphzArgs[]);
// Initializing all the things needed for the game to run
void Startup();
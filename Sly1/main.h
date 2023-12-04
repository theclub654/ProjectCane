#pragma once
#include <iostream>
#include "gl.h"
#include "transition.h"
#include "debug.h"
#include "freecamera.h"
#include "mouse.h"

void RenderSw(SW* psw, CM* pcm);
void DrawSwAll();
void DrawSwCollisionAll();
void DeleteSw(SW* psw);

// Game loop
int main(int cphzArgs, char* aphzArgs[]);
// Initializing all the things needed for the game to run
void Startup();
// Processes user input
void ProcessInput(GLFWwindow* window, double dt);
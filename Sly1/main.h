#pragma once
#include <iostream>
#include "shd.h"
#include "transition.h"
#include "debug.h"

void RenderSw(SW* psw, CM* pcm);
void DrawSwAll(SW* psw, CM* pcm);
void FreeGLBuffers(SW* psw);

int main(int cphzArgs, char* aphzArgs[]);
// Initializing all the things needed for the game to run
void Startup();
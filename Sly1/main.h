#pragma once
#include <iostream>
#include "gl.h"
#include "transition.h"

void RenderSw(SW* psw, CM* pcm);

int main(int cphzArgs, char* aphzArgs[]);
// Initializing all the things needed for the game to run
void Startup();
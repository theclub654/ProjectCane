﻿#pragma once
#include "gl.h"
#include "so.h"
#include "light.h"
#include "debug.h"

extern std::vector <ALO*> allSWAloObjs;
extern std::vector <LIGHT*> allSwLights;
std::vector<SO*> allSWSoObjs;

// Loops through all objects in a level to see which object is in camera view and other conditions and stores all objects
// in a render list
void RenderSw(SW* psw, CM* pcm);
void RenderSwAll(SW* psw, CM* pcm);
void RenderSwGlobsetAll(SW* psw, CM* pcm);
// Loops through that render list of objects to be rendered on the screen
void DrawSw(SW* psw, CM* pcm);
// Draws all 3D objects in SW
void DrawSwAll(SW *psw, GLFWwindow* window);
// Draws all collision models in SW
void DrawSwCollisionAll();
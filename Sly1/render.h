#pragma once
#include "sw.h"

extern std::vector<ALO*> allSWAloObjs;
extern std::vector <GEOM*> allcollisionModels;

// Loops through all objects in a level to see which object is in camera view and stores all objects
// in camera view in a render list
void RenderSw(SW* psw, CM* pcm);
// Loops through that render list of objects to be rendered on the screen
void DrawSw(SW* psw, CM* pcm);
// Draws all 3D objects in SW
void DrawSwAll();
// Draws all collision models in SW
void DrawSwCollisionAll();
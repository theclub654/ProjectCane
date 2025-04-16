#pragma once
#include "so.h"
#include "light.h"
#include "debug.h"

extern std::vector <ALO*> allSWAloObjs;
extern std::vector <LIGHT*> allSwLights;
std::vector <SO*> allSWSoObjs;
std::vector <RPL> renderBuffer;

int g_dynamicTextureCount = 0;
int g_backGroundCount = 0;
int g_blotContextCount = 0;
int g_opaqueCount = 0;
int g_cutOutCount = 0;
int g_celBorderCount = 0;
int g_projVolumeCount = 0;
int g_opaqueAfterProjVolumeCount = 0;
int g_cutoutAfterProjVolumeCount = 0;
int g_celBorderAfterProjVolumeCount = 0;
int g_murkClearCount = 0;
int g_murkOpaqueCount = 0;
int g_murkFillCount = 0;
int g_translucentCount = 0;
int g_translucentCelBorderCount = 0;
int g_blipCount = 0;
int g_foreGroundCount = 0;
int g_worldMapCount = 0;
int g_maxCount = 0;

// Loops through all objects in a level to see which object is in camera view and other conditions and stores all objects
// in a render list
void RenderSw(SW *psw, CM *pcm);
void RenderSwAloAll(SW* psw, CM* pcm);
void RenderSwGlobset(SW *psw, CM *pcm);
// Adds a object to a render list in a sorted order
void SubmitRpl(RPL *prpl);
// Sorts the draw list
void SortRenderRpl();
inline bool compareRP(RPL &prpl0, RPL &prpl1);
inline bool compareZ(RPL &prpl0, RPL &prpl1);
// Loops through that render list of objects to be rendered on the screen
void DrawSw(SW *psw, CM *pcm);
// Draws all collision models in SW
void DrawSwCollisionAll();

glm::vec4 g_rgbaCel = glm::vec4(0.16, 0.16, 0.16, 1.0);
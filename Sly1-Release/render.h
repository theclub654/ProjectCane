#pragma once
#include "so.h"
#include "light.h"
#include "debug.h"

std::vector <RPL> renderBuffer;

int numRo;

int g_dynamicTextureCount;
int g_backGroundCount;
int g_blotContextCount;
int g_opaqueCount;
int g_cutOutCount;
int g_celBorderCount;
int g_projVolumeCount;
int g_opaqueAfterProjVolumeCount;
int g_cutoutAfterProjVolumeCount;
int g_celBorderAfterProjVolumeCount;
int g_murkClearCount;
int g_murkOpaqueCount;
int g_murkFillCount;
int g_translucentCount;
int g_translucentCelBorderCount;
int g_blipCount;
int g_foreGroundCount;
int g_worldMapCount;
int g_maxCount;

void AllocateRpl();
// Loops through all objects in a level to see which object is in camera view and other conditions and stores all objects
// in a render list
void RenderSw(SW* psw, CM* pcm);
void RenderSwAloAll(SW* psw, CM* pcm);
void RenderSwGlobset(SW* psw, CM* pcm);
// Adds a object to a render list in a sorted order
void SubmitRpl(RPL* prpl);
// Sorts the draw list
void SortRenderRpl();
inline bool compareRP(RPL& prpl0, RPL& prpl1);
inline bool compareZ(RPL& prpl0, RPL& prpl1);
// Loops through that render list of objects to be rendered on the screen
void DrawSw(SW* psw, CM* pcm);
// Draws all collision models in SW
void DrawSwCollisionAll(CM* pcm);

glm::vec4 g_rgbaCel = glm::vec4(16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f, 1.0);
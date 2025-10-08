#pragma once
#include "so.h"
#include "light.h"
#include "debug.h"

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
inline bool compareGrfShd(RPL& prpl0, RPL& prpl1);
inline bool compareZ(RPL& prpl0, RPL& prpl1);
// Loops through that render list of objects to be rendered on the screen
void DrawSw(SW* psw, CM* pcm);
void BindRenderObject(RPL* prpl);
void BindRenderObjectCel(RPL* prpl);
void DrawGlob(RPL* prpl);
void DrawCutout(RPL* prpl);
void DrawCelBorder(RPL* prpl);
void DrawProjVolume(RPL* prpl);
void DrawMurkClear(RPL* prpl);
void DrawMurkFill(RPL* prpl);
void DrawTranslucent(RPL* prpl);
// Draws all collision models in SW
void DrawSwCollisionAll(CM* pcm);

extern std::vector <RPL> renderBuffer;
extern int numRo;

extern int g_dynamicTextureCount;
extern int g_backGroundCount;
extern int g_backGroundBlendCount;
extern int g_blotContextCount;
extern int g_opaqueCount;
extern int g_cutOutCount;
extern int g_cutOutBlendAddCount;
extern int g_celBorderCount;
extern int g_projVolumeCount;
extern int g_projVolumeBlendAddCount;
extern int g_projVolumeBlendCount;
extern int g_opaqueAfterProjVolumeCount;
extern int g_cutOutAfterProjVolumeCount;
extern int g_cutOutAfterProjVolumeAddCount;
extern int g_celBorderAfterProjVolumeCount;
extern int g_murkClearCount;
extern int g_murkOpaqueCount;
extern int g_murkFillCount;
extern int g_translucentCount;
extern int g_translucentAddCount;
extern int g_transluscentOnScreen;
extern int g_translucentCelBorderCount;
extern int g_blipCount;
extern int g_foreGroundCount;
extern int g_worldMapCount;
extern int g_maxCount;

extern glm::vec4 g_rgbaCel;
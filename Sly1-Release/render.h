#pragma once
#include "so.h"
#include "dysh.h"
#include "light.h"
#include "debug.h"

void SetGlobDraw(GLOB* pglob);
void AllocateRpl();
// Loops through all objects in a level to see which object is in camera view and other conditions and stores all objects
// in a render list
void RenderSw(SW* psw, CM* pcm);
void RenderSwAloAll(SW* psw, CM* pcm);
void RenderSwGlobset(SW* psw, CM* pcm);
// Adds a object to a render list in a sorted order
void SubmitRpl(RPL* prpl);
void SubmitRplCel(RPLCEL* prplcel);
// Sorts the draw list
void SortRenderRpl();
inline bool compareZ(const RPL& prpl0, const RPL& prpl1);
// Loops through that render list of objects to be rendered on the screen
void DrawSw(SW* psw, CM* pcm);
void FillScreenRect(int r, int g, int b, int alpha, float xLeft, float yTop, float xRight, float yBottom, bool fAdditive = false);
void DrawDysh(RPL* prpl);
void DrawGlob(RPL* prpl);
void DrawCelBorder(RPLCEL* prplcel);
void DrawSubGlob(int baseVertex, int firstIndex, int indexCount);
void DrawProjVolume(int baseVertex, int firstIndex, int indexCount);
void DrawProjVolumeAlphaAdd(int baseVertex, int firstIndex, int indexCount);
void DrawProjVolumeAdd(int baseVertex, int firstIndex, int indexCount);
void DrawMurkClear(int baseVertex, int firstIndex, int indexCount);
void DrawMurkFill(int baseVertex, int firstIndex, int indexCount);
void DrawTranslucent(int baseVertex, int firstIndex, int indexCount);
void DrawBlip(RPL* prplblip);
// Draws all collision models in SW
void DrawSwCollisionAll(CM* pcm);

extern int numRo;
extern int numRoCel;

extern int g_cFrameGlobs;

extern int g_cFrameCelGlobs;

extern int g_cframe;

// Frame at which world/static illumination was last changed. Static three-way
// caches older than this generation must be rebuilt before they are reused.
extern int g_cframeStaticLightsInvalid;

extern int g_boundVAO;

extern int g_shdIDBound;
extern int g_lastAnimateUv;

extern int g_grfshd;

extern int g_dynamicTextureCount;
extern std::vector <RPL> g_dynamicTexturePrpl;

extern int g_backGroundCount;
extern std::vector <RPL> g_backGroundPrpl;

extern int g_backGroundBlendCount;
extern std::vector <RPL> g_backGroundBlendPrpl;

extern int g_blotContextCount;
extern std::vector <RPL> g_blotContextPrpl;

extern int g_opaqueCount;
extern std::vector <RPL> g_opaquePrpl;

extern int g_cutOutCount;
extern std::vector <RPL> g_cutOutPrpl;

extern int g_cutOutBlendAddCount;
extern std::vector <RPL> g_cutOutBlendAddPrpl;

extern int g_celBorderCount;
extern std::vector <RPLCEL> g_celBorderPrpl;

extern int g_projVolumeCount;
extern std::vector <RPL> g_projVolumePrpl;

extern int g_projVolumeAlphaAddCount;
extern std::vector <RPL> g_projVolumeAlphaAddPrpl;

extern int g_projVolumeAddCount;
extern std::vector <RPL> g_projVolumeAddPrpl;

extern int g_opaqueAfterProjVolumeCount;
extern std::vector <RPL> g_opaqueAfterProjVolumePrpl;

extern int g_cutOutAfterProjVolumeCount;
extern std::vector <RPL> g_cutOutAfterProjVolumePrpl;

extern int g_cutOutAfterProjVolumeAddCount;
extern std::vector <RPL> g_cutOutAfterProjVolumeAddPrpl;

extern int g_celBorderAfterProjVolumeCount;
extern std::vector <RPLCEL> g_celBorderAfterProjVolumePrpl;

extern int g_murkClearCount;
extern std::vector <RPL> g_murkClearPrpl;

extern int g_murkOpaqueCount;
extern std::vector <RPL> g_murkOpaquePrpl;

extern int g_murkFillCount;
extern std::vector <RPL> g_murkFillPrpl;

extern int g_translucentCount;
extern std::vector <RPL> g_translucentPrpl;

extern int g_translucentAddCount;
extern std::vector <RPL> g_translucentAddPrpl;

extern int g_translucentCelBorderCount;
extern std::vector <RPLCEL> g_translucentCelBorderPrpl;

extern int g_blipCount;
extern std::vector <RPL> g_blipPrpl;

extern int g_foreGroundCount;
extern std::vector <RPL> g_foreGroundPrpl;

extern int g_worldMapCount;
extern std::vector <RPL> g_worldMapPrpl;

extern int g_maxCount;
extern std::vector <RPL> g_maxPrpl;

extern bool g_fVsync;

extern int g_fDisableSkin;
extern int g_fDisablePoses;

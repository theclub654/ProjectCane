#pragma once
#include "blot.h"

#include <string>

inline constexpr int CUTSCENE_COUNT = 18;

struct CUTSCENEINFO
{
    int cutsceneId;
    OID identityId;
    std::string videoFileName;
    std::string audioFileName;
};

struct FMVMENU : public BLOT
{
    int iCutscene = 0;
};

void StartupFmvMenu(FMVMENU* pfmvmenu);
void PostFmvMenuLoad(FMVMENU* pfmvmenu);
void OnFmvMenuActive(FMVMENU* pfmvmenu, int fActive);
void UpdateFmvMenuActive(FMVMENU* pfmvmenu, JOY* poy);
void DrawFmvMenu(FMVMENU* pfmvmenu);
uint32_t FFmvUnlocked(int iCutscene);
const char* PchzFriendlyFromCid(int iCutscene);
const CUTSCENEINFO* PcutsceneinfoFromId(int cutsceneId);
const CUTSCENEINFO* PcutsceneinfoFromIdentityId(OID identityId);
void QueueCutscene(OID identityId);
void QueueCutsceneDescriptor(const char* descriptor);
bool FCutscenePending();
void ExecutePendingCutscenes();

struct VTFMV
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostFmvMenuLoad)(FMVMENU*) = PostFmvMenuLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnFmvMenuActive)(FMVMENU*, int) = OnFmvMenuActive;
    void (*pfnUpdateFmvMenuActive)(FMVMENU*, JOY*) = UpdateFmvMenuActive;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush)(BLOT*) = nullptr;
    void (*pfnOnBlotPop)(BLOT*) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawFmvMenu)(FMVMENU*) = DrawFmvMenu;
    void(*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern CTextEdge g_teFmv;
extern VTFMV g_vtfmvmenu;
extern FMVMENU g_fmvmenu;
extern CUTSCENEINFO g_cutsceneTable[CUTSCENE_COUNT];

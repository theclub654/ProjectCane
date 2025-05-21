#pragma once
#include "blotdec.h"
#include "glm/vec3.hpp"
#include "font.h"
#include "clock.h"
#include "Input.h"

enum BLOTS
{
    BLOTS_Nil = -1,
    BLOTS_Hidden = 0,
    BLOTS_Appearing = 1,
    BLOTS_Visible = 2,
    BLOTS_Disappearing = 3,
    BLOTS_Max = 4
};
enum BLOTK
{
    BLOTK_Nil = -1,
    BLOTK_Letterbox = 0,
    BLOTK_Binoc = 1,
    BLOTK_LightGun = 2,
    BLOTK_Lives = 3,
    BLOTK_Clue = 4,
    BLOTK_Key = 5,
    BLOTK_Gold = 6,
    BLOTK_Coin = 7,
    BLOTK_Trunk = 8,
    BLOTK_Crusher = 9,
    BLOTK_Lap = 10,
    BLOTK_Boost = 11,
    BLOTK_Place = 12,
    BLOTK_Boss = 13,
    BLOTK_PuffCharge = 14,
    BLOTK_Timer = 15,
    BLOTK_Note = 16,
    BLOTK_Title = 17,
    BLOTK_Totals = 18,
    BLOTK_Call = 19,
    BLOTK_Wmc = 20,
    BLOTK_Prompt = 21,
    BLOTK_TvLeft = 22,
    BLOTK_TvRight = 23,
    BLOTK_Scores = 24,
    BLOTK_Logo = 25,
    BLOTK_Attract = 26,
    BLOTK_Credit = 27,
    BLOTK_Debug = 28,
    BLOTK_Max = 29
};
enum BLOTE
{
    BLOTE_Nil = -1,
    BLOTE_Left = 0,
    BLOTE_Right = 1,
    BLOTE_Top = 2,
    BLOTE_Bottom = 3,
    BLOTE_Max = 4,
    BLOTE_TopLeft = 5,
    BLOTE_TopRight = 6,
    BLOTE_BottomRight = 7,
    BLOTE_BottomLeft = 8,
    BLOTE_Center = 9
};

struct BLOTI
{
    float x;
    float y;
    BLOTE blote;
    BLOTK blotkPeg;
    BLOTE blotePeg;
};

struct BLOT
{
    union
    {
        struct VTBLOT* pvtblot;
        struct VTCTR* pvtctr;
        struct VTLIFECTR* pvtlifectr;
        struct VTCLUECTR* pvtcluectr;
        struct VTKEYCTR* pvtkeyctr;
        struct VTGOLDCTR* pvtgoldctr;
        struct VTCOINCTR* pvtcoinctr;
        struct VTTRUNKCTR* pvttrunkctr;
        struct VTCRUSHERCTR* pvtcrusherctr;
        struct VTBOOSTCTR* pvtboostctr;
        struct VTPLACECTR* pvtplacectr;
        struct VTLAPCTR* pvtlapctr;
        struct VTPUFFCHARGECTR* pvtpuffchargectr;
        struct VTBOSSCTR* pvtbossctr;
        struct VTNOTE* pvtnote;
        struct VTTITLE* pvttitle;
        struct VTTOTALS* pvttotals;
        struct VTCALL* pvtcall;
        struct VTSCORES* pvtscores;
        struct VTLOGO* pvtlogo;
        struct VTATTRACT* pvtattract;
        struct VTTIMER* pvttimer;
        struct VTLETTERBOX* pvtletterbox;
        struct VTDEBUGMENU* pvtdebugmenu;
        struct VTPROMPT* pvtprompt;
        struct VTCREDIT* pvtcredit;
        struct VTTV* pvttv;
        struct VTBINOC* pvtbinoc;
        struct VTLGNR* pvtlgnr;
        struct VTWMC* pvtwmc;

    };
    CFontBrx* pfont;          // Font to draw with
    char achzDraw[512];       // Text or formatted string to draw
    glm::vec4 rgba;           // Text color (or overlay color)
    float rFontScale;         // Font scaling factor
    CTextEdge* pte;           // Optional edge effect for text

    BLOTK blotk;              // Type of UI element
    BLOTI* pbloti;            // Pointer to position/anchor info

    float x, y;               // Position on screen
    float xOn, yOn;           // Target position when "on"
    float xOff, yOff;         // Target position when "off"
    float dx, dy;             // Current offset used during transitions (tweening)

    float uOn;                // Unknown, likely opacity or timing progress

    float dtAppear;           // How long it takes to appear
    float dtVisible;          // How long it stays fully visible
    float dtDisappear;        // How long it takes to fade out
    float dtBlotsExtra;       // Additional timing, maybe for delay

    BLOTS blots;              // Current visibility state
    float tBlots;             // Time accumulator for animation transitions

    float* ptNow;             // Pointer to global time (used for timing animations)
    int fActive;              // Whether this BLOT is active
};

void  InitBlot(BLOT* pblot, BLOTK blotk);
void  PostBlotsLoad();
void  PostBlotLoad(BLOT* pblot);
// Sets the text to be drawn on the blot and resizes it to fit the text
void  SetBlotAchzDraw(BLOT *pblot, char *pchz);
void  OnBlotActive(BLOT* pblot, int fActive);
void  ShowBlot(BLOT* pblot);
void  HideBlot(BLOT* pblot);
void  SetBlotClock(BLOT* pblot, float* pt);
void  SetBlotFontScale(BLOT* pblot, float rFontScale);
void  SetBlotDtVisible(BLOT* pblot, float dtVisible);
void  SetBlotDtAppear(BLOT* pblot, float dtAppear);
void  SetBlotDtDisappear(BLOT* pblot, float dtDisappear);
void  SetBlotBlots(BLOT* pblot, BLOTS blots);
float DtAppearBlot(BLOT* pblot);
float DtVisibleBlot(BLOT* pblot);
float DtDisappearBlot(BLOT* pblot);
void  ResizeBlot(BLOT* pblot, float dx, float dy);
void  RepositionBlotDependents(BLOT* pblot);
void  RepositionBlot(BLOT* pblot);
int   FIncludeBlotForPeg(BLOT* pblot, BLOT* pblotOther);
void  OnBlotReset(BLOT* pblot);
void  UpdateBlot(BLOT* pblot);
void  UpdateBlotActive(BLOT* pblot, JOY* pjoy);
void  UpdateBlots();
void  DrawBlot(BLOT *pblot);
void  DrawBlots();
void  ResetBlots();

struct VTLIFECTR
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTLIFECTR g_vtlifectr;

struct VTKEYCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTKEYCTR g_vtkeyctr;

struct VTLETTERBOX 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostBlotLoad)(BLOT*) = PostBlotLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawLetterbox)(LETTERBOX*) = DrawLetterBox;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTLETTERBOX g_vtletterbox;

struct VTCLUECTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTCLUECTR g_vtcluectr;

struct VTCOINCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTCOINCTR g_vtcoinctr;

struct VTTITLE 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostTitleLoad)(TITLE*) = PostTitleLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetTitleAchzDraw) = nullptr;
    void (*pfnDrawTitle)(TITLE*) = DrawTitle;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetTitleBlots)(TITLE*, BLOTS) = SetTitleBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeTitleForPeg)(TITLE*, BLOT*) = FIncludeTitleForPeg;
};

extern VTTITLE g_vttitle;

struct VTWMC 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostWmcLoad)(WMC*) = PostWmcLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnWmcActive) = nullptr;
    void (*pfnUpdateWmcActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawWmc)(WMC*) = DrawWmc;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTWMC g_vtwmc;

struct VTBINOC 
{
    void (*pfnInitBinoc)(BINOC*, BLOTK) = InitBinoc;
    void (*pfnPostBinocLoad)(BINOC*) = PostBinocLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBinocActive)(BINOC*, int) = OnBinocActive;
    void (*pfnUpdateBinocActive)(BINOC*, JOY*) = UpdateBinocActive;
    void (*pfnOnBinocReset)(BINOC*) = OnBinocReset;
    void (*pfnOnBinocPush)(BINOC*) = OnBinocPush;
    void (*pfnOnBinocPop)(BINOC*) = OnBinocPop;
    void (*pfnSetBinocAchzDraw)(BINOC*, char*) = SetBinocAchzDraw;
    void (*pfnDrawBinoc)(BINOC*) = DrawBinoc;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBinoc)(BINOC*) = DtAppearBinoc;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBinoc)(BINOC*) = DtDisappearBinoc;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBinocBlots)(BINOC*, BLOTS) = SetBinocBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTBINOC g_vtbinoc;

struct VTATTRACT 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostAttractLoad)(ATTRACT*) = PostAttractLoad;
    void (*pfnUpdateAttract)(ATTRACT*) = UpdateAttract;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetAttractAchzDraw)(ATTRACT*, char*) = SetAttractAchzDraw;
    void (*pfnDrawAttract)(ATTRACT*) = DrawAttract;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetAttractBlots)(ATTRACT*, BLOTS) = SetAttractBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTATTRACT g_vtattract;

struct VTBLOT 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostBlotLoad)(BLOT*) = PostBlotLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive)(BLOT*, int) = OnBlotActive;
    void (*pfnUpdateBlotActive)(BLOT*, JOY*) = UpdateBlotActive;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawBlot)(BLOT*) = DrawBlot;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTBLOT g_vtblot;

struct VTTIMER 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostTimerLoad)(TIMER*) = PostTimerLoad;
    void (*pfnUpdateTimer)(TIMER*) = UpdateTimer;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawTimer)(TIMER*) = DrawTimer;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTTIMER g_vttimer;

struct VTTRUNKCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleTrunkctr)(TRUNKCTR*) = DtVisibleTrunkctr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTTRUNKCTR g_vttrunkctr;

struct VTPUFFCHARGECTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisiblePuffchargectr)(PUFFCHARGERCTR*) = DtVisiblePuffchargerctr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTPUFFCHARGECTR g_vtpuffchargectr;

struct VTBOOSTCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTBOOSTCTR g_vtboostctr;

struct VTCREDIT 
{
    void (*pfnInitCredit)(CREDIT*, BLOTK) = InitCredit;
    void (*pfnPostCreditLoad)(CREDIT*) = PostCreditLoad;
    void (*pfnUpdateCredit)(CREDIT*) = UpdateCredit;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCredit)(CREDIT*) = DrawCredit;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetCreditClock)(CREDIT*, float*) = SetCreditClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTCREDIT g_vtcredit;

struct VTCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTCTR g_vtctr;

struct VTCRUSHERCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCrusherctr)(CRUSHERCTR*) = DtVisibleCrusherctr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTCRUSHERCTR g_vtcrusherctr;

struct VTTOTALS 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostTotalsLoad)(TOTALS*) = PostTotalsLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawTotals)(TOTALS*) = DrawTotals;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetTotalsBlots)(TOTALS*, BLOTS) = SetTotalsBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTTOTALS g_vttotals;

struct VTTV 
{
    void (*pfnInitTv)(TV*, BLOTK) = InitTv;
    void (*pfnPostTvLoad)(TV*) = PostTvLoad;
    void (*pfnUpdateTv) = nullptr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnTvReset)(TV*) = OnTvReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawTv)(TV*) = DrawTv;
    void (*pfnRenderTv) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetTvBlots)(TV*, BLOTS) = SetTvBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTTV g_vttv;

struct VTPLACECTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTPLACECTR g_vtplacectr;

struct VTDEBUGMENU 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostBlotLoad)(BLOT*) = PostBlotLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnDebugmenuActive)(DEBUGMENU*, int) = OnDebugmenuActive;
    void (*pfnUpdateDebugmenuActive)(DEBUGMENU*, JOY*) = UpdateDebugmenuActive;
    void (*pfnOnDebugmenuReset)(DEBUGMENU*) = OnDebugmenuReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawDebugmenu)(DEBUGMENU*) = DrawDebugMenu;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTDEBUGMENU g_vtdebugmenu;

struct VTLOGO 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostLogoLoad)(LOGO*) = PostLogoLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawLogo)(LOGO*) = DrawLogo;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTLOGO g_vtlogo;

struct VTBOSSCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostBossCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawBossctr)(BOSSCTR*) = DrawBossCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTBOSSCTR g_vtbossctr;

struct VTNOTE 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostNoteLoad)(NOTE*) = PostNoteLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetNoteAchzDraw) = nullptr;
    void (*pfnDrawNote)(NOTE*) = DrawNote;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTNOTE g_vtnote;

struct VTCALL 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCallLoad)(CALL*) = PostCallLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCall)(CALL*) = DrawCall;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTCALL g_vtcall;

struct VTSCORES 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostScoresLoad)(SCORES*) = PostScoresLoad;
    void (*pfnUpdateScores)(SCORES*) = UpdateScores;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawScores)(SCORES*) = DrawScores;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTSCORES g_vtscores;

struct VTLGNR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostBlotLoad)(BLOT*) = PostBlotLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawLgnr)(LGNR*) = DrawLgnr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTLGNR g_vtlgnr;

struct VTGOLDCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTGOLDCTR g_vtgoldctr;

struct VTLAPCTR 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostCtrLoad)(CTR*) = PostCtrLoad;
    void (*pfnUpdateCtr)(CTR*) = UpdateCtr;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawCtr)(CTR*) = DrawCtr;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleCtr)(CTR*) = DtVisibleCtr;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTLAPCTR g_vtlapctr;

struct VTPROMPT 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostPromptLoad)(PROMPT*) = PostPromptLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnPromptActive)(PROMPT*, int) = OnPromptActive;
    void (*pfnUpdatePromptActive)(PROMPT*, JOY*) = UpdatePromptActive;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawPrompt)(PROMPT*) = DrawPrompt;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetPromptBlots)(PROMPT*, BLOTS) = SetPromptBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTPROMPT g_vtprompt;

inline BLOTI s_abloti[29] = 
{
    {0.0f,   0.0f,   BLOTE_Nil,    BLOTK_Nil,     BLOTE_Nil},    // 0
    {0.0f,   0.0f,   BLOTE_Nil,    BLOTK_Nil,     BLOTE_Nil},    // 1
    {0.0f,   0.0f,   BLOTE_Nil,    BLOTK_Nil,     BLOTE_Nil},    // 2
    {0.0f,   20.0f,  BLOTE_Top,    BLOTK_Nil,     BLOTE_Nil},    // 3
    {-18.0f, 20.0f,  BLOTE_Right,  BLOTK_Nil,     BLOTE_Nil},    // 4
    {-18.0f, 20.0f,  BLOTE_Right,  BLOTK_Clue,    BLOTE_Top},    // 5
    {-18.0f, 20.0f,  BLOTE_Right,  BLOTK_Key,     BLOTE_Top},    // 6
    {18.0f,  20.0f,  BLOTE_Left,   BLOTK_Nil,     BLOTE_Nil},    // 7
    {-18.0f, 20.0f,  BLOTE_Right,  BLOTK_Gold,    BLOTE_Nil},    // 8
    {-18.0f, 20.0f,  BLOTE_Right,  BLOTK_Gold,    BLOTE_Nil},    // 9
    {18.0f,  20.0f,  BLOTE_Top,    BLOTK_Coin,    BLOTE_Top},    // 10
    {0.0f,   20.0f,  BLOTE_Top,    BLOTK_Lives,   BLOTE_Top},    // 11
    {-18.0f, 20.0f,  BLOTE_Top,    BLOTK_Key,     BLOTE_Top},    // 12
    {0.0f,   -5.0f,  BLOTE_Bottom, BLOTK_Nil,     BLOTE_Nil},    // 13
    {-18.0f, 20.0f,  BLOTE_Right,  BLOTK_Crusher, BLOTE_Top},    // 14
    {18.0f,  -20.0f, BLOTE_Left,   BLOTK_Nil,     BLOTE_Nil},    // 15
    {18.0f,  -20.0f, BLOTE_Nil,    BLOTK_Nil,     BLOTE_Nil},    // 16
    {-18.0f, -20.0f, BLOTE_Right,  BLOTK_Nil,     BLOTE_Nil},    // 17
    {-18.0f, -20.0f, BLOTE_Right,  BLOTK_Title,   BLOTE_Bottom}, // 18
    {18.0f,  -20.0f, BLOTE_Left,   BLOTK_Note,    BLOTE_Bottom}, // 19
    {18.0f,  -20.0f, BLOTE_Nil,    BLOTK_Nil,     BLOTE_Nil},    // 20
    {0.0f,    0.0f,  BLOTE_Nil,    BLOTK_Nil,     BLOTE_Nil},    // 21
    {1.0f,   -20.0f, BLOTE_Left,   BLOTK_Nil,     BLOTE_Nil},    // 22
    {-1.0f,  -20.0f, BLOTE_Right,  BLOTK_Nil,     BLOTE_Nil},    // 23
    {-18.0f,  20.0f, BLOTE_Right,  BLOTK_Gold,    BLOTE_Nil},    // 24
    {18.0f,   20.0f, BLOTE_Top,    BLOTK_Nil,     BLOTE_Nil},    // 25
    {-18.0f, -20.0f, BLOTE_Bottom, BLOTK_Nil,     BLOTE_Nil},    // 26
    {18.0f,   20.0f, BLOTE_Nil,    BLOTK_Nil,     BLOTE_Nil},    // 27
    {16.0f,   16.0f, BLOTE_Left,   BLOTK_Nil,     BLOTE_Nil},    // 28
};

inline BLOT* s_apblot[29] =
{
    (BLOT*)&g_letterbox,     // [0]
    (BLOT*)&g_binoc,         // [1]
    (BLOT*)&g_lgnr,          // [2]
    (BLOT*)&g_lifectr,       // [3]
    (BLOT*)&g_cluectr,       // [4]
    (BLOT*)&g_keyctr,        // [5]
    (BLOT*)&g_goldctr,       // [6]
    (BLOT*)&g_coinctr,       // [7]
    (BLOT*)&g_trunkctr,      // [8]
    (BLOT*)&g_crusherctr,    // [9]
    (BLOT*)&g_lapctr,        // [10]
    (BLOT*)&g_boostctr,      // [11]
    (BLOT*)&g_placectr,      // [12]
    (BLOT*)&g_bossctr,       // [13]
    (BLOT*)&g_puffchargectr, // [14]
    (BLOT*)&g_timer,         // [15]
    (BLOT*)&g_note,          // [16]
    (BLOT*)&g_title,         // [17]
    (BLOT*)&g_totals,        // [18]
    (BLOT*)&g_call,          // [19]
    (BLOT*)&g_wmc,           // [20]
    (BLOT*)&g_prompt,        // [21]
    (BLOT*)&g_tvLeft,        // [22]
    (BLOT*)&g_tvRight,       // [23]
    (BLOT*)&g_scores,        // [24]
    (BLOT*)&g_logo,          // [25]
    (BLOT*)&g_attract,       // [26]
    (BLOT*)&g_credit,        // [27] 
    (BLOT*)&g_debugmenu      // [28]
};
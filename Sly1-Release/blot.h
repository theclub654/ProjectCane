#pragma once
#include "blotdec.h"
#include "glm/vec3.hpp"
#include "font.h"
#include "clock.h"
#include "Input.h"
#include <unordered_set>
#include <unordered_map>
#include "game.h"

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
    BLOTK_Coin = 6,
    BLOTK_Gold = 7,
    BLOTK_PuffCharge = 8,
    BLOTK_Lap = 9,
    BLOTK_Boost = 10,
    BLOTK_Place = 11,
    BLOTK_SecurityCode = 12,
    BLOTK_MgvHealth = 13,
    BLOTK_Boss = 14,
    BLOTK_Crusher = 15,
    BLOTK_Timer = 16,
    BLOTK_Note = 17,
    BLOTK_Unused18 = 18,
    BLOTK_Title = 19,
    BLOTK_Totals = 20,
    BLOTK_Call = 21,
    BLOTK_Wmc = 22,
    BLOTK_Hub = 23,
    BLOTK_FmvMenu = 24,
    BLOTK_Prompt = 25,
    BLOTK_TvLeft = 26,
    BLOTK_TvRight = 27,
    BLOTK_VanComputer = 28,
    BLOTK_Scores = 29,
    BLOTK_Logo = 30,
    BLOTK_Attract = 31,
    BLOTK_Credit = 32,
    BLOTK_RubyIcon = 33,
    BLOTK_JtIcon = 34,
    BLOTK_Autosave = 35,
    BLOTK_Percent = 36,
    BLOTK_Max = 37
};
enum BLOTE
{
    BLOTE_Nil = -1,
    BLOTE_Left = 0,
    BLOTE_Right = 1,
    BLOTE_Top = 2,
    BLOTE_Bottom = 3,
    BLOTE_Max = 4,
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
        struct VTBOSS* pvtboss;
        struct VTNOTE* pvtnote;
        struct VTTITLE* pvttitle;
        struct VTTOTALS* pvttotals;
        struct VTCALL* pvtcall;
        struct VTSCORES* pvtscores;
        struct VTVAN* pvtvan;
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
        struct VTSAVEBLOT* pvtsaveblot;
        struct VTHUBBLOT* pvthubblot;
        struct VTFMV* pvtfmv;
        struct VTJTICON* pvtjticon;

    };
    CFontBrx* pfont;          // Font to draw with
    char achzDraw[512];       // Text or formatted string to draw
    glm::vec4 rgba;           // Text color (or overlay color)
    float rFontScale;         // Font scaling factor
    CTextEdge* pte;           // Optional edge effect for text

    BLOTK blotk;              // Type of UI element
    BLOTI *pbloti;            // Pointer to position/anchor info

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

    float *ptNow;             // Pointer to global time (used for timing animations)
    int fActive;              // Whether this BLOT is active
};

void  InitBlot(BLOT* pblot, BLOTK blotk);
void  PostBlotsLoad();
void  PostBlotLoad(BLOT* pblot);
// Sets the text to be drawn on the blot and resizes it to fit the text
void  SetBlotAchzDraw(BLOT *pblot, char *pchz);
char* FormatBlotRichText(BLOT* pblot);
void  OnBlotActive(BLOT* pblot, int fActive);
void  ShowBlot(BLOT* pblot);
void  HideBlot(BLOT* pblot);
void  ForceHideBlots();
BLOTS GetBlot(BLOT* pblot);
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
void  RepositionAllBlots();
int   FIncludeBlotForPeg(BLOT* pblot, BLOT* pblotOther);
void  OnBlotReset(BLOT* pblot);
void  UpdateBlot(BLOT* pblot);
void  UpdateBlotActive(BLOT* pblot, JOY* pjoy);
void  UpdateBlots();
void  DrawBlot(BLOT *pblot);
void  RenderBlots();
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
    void (*pfnPostLetterboxLoad)(LETTERBOX*) = PostLetterBoxLoad;
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
    void (*pfnSetLetterboxBlots)(LETTERBOX*, BLOTS) = SetLetterboxBlots;
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
    void (*pfnSetTitleAchzDraw)(TITLE*, char*) = SetTitleAchzDraw;
    void (*pfnDrawTitle)(TITLE*) = DrawTitle;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowTitle)(TITLE*) = ShowTitle;
    void (*pfnHideTitle)(TITLE*) = HideTitle;
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
    void (*pfnOnWmcActive)(WMC*, int) = OnWmcActive;
    void (*pfnUpdateWmcActive)(WMC*, JOY*) = UpdateWmcActive;
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
    void (*pfnOnBlotPush)(BLOT*) = nullptr;
    void (*pfnOnBlotPop)(BLOT*) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawBlot)(BLOT*) = DrawBlot;
    void (*pfnRenderBlot)(BLOT*) = nullptr;
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
    void (*pfnSetTotalsAchzDraw)(TOTALS*, char*) = SetTotalsAchzDraw;
    void (*pfnDrawTotals)(TOTALS*) = DrawTotals;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(TOTALS*) = ShowTotals;
    void (*pfnHideBlot)(TOTALS*) = HideTotals;
    void (*pfnSetTotalsBlots)(TOTALS*, BLOTS) = SetTotalsBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTTOTALS g_vttotals;

struct VTTV 
{
    void (*pfnInitTv)(TV*, BLOTK) = InitTv;
    void (*pfnPostTvLoad)(TV*) = PostTvLoad;
    void (*pfnUpdateTv)(TV*) = UpdateTv;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnTvReset)(TV*) = OnTvReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawTv)(TV*) = DrawTv;
    void (*pfnRenderTv)(TV*) = RenderTv;
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

struct VTBOSS
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostBossLoad)(BOSS*) = PostBossLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawBoss)(BOSS*) = DrawBoss;
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

extern VTBOSS g_vtboss;

struct VTNOTE 
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostNoteLoad)(NOTE*) = PostNoteLoad;
    void (*pfnUpdateNote)(NOTE*) = UpdateNote;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
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
    void (*pfnUpdateCall)(CALL*) = UpdateCall;
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

extern VTSCORES g_vtscores;

struct VTVAN
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
    int (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTVAN g_vtvan;

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

struct VTSAVEBLOT
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostAutoSaveLoad)(SAVEBLOT*) = PostAutoSaveLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive)(BLOT*, int) = OnBlotActive;
    void (*pfnUpdateBlotActive)(BLOT*, JOY*) = UpdateBlotActive;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void(*pfnOnBlotPush) = nullptr;
    void(*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawBlot)(BLOT*) = DrawBlot;
    void(*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetSavesBlots)(SAVEBLOT*, BLOTS) = SetSaveBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int  (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTSAVEBLOT g_vtsaveblot;

extern BLOTI s_abloti[37];

struct JTICON;
extern JTICON g_rubyicon;
extern JTICON g_jticon;

inline std::unordered_map<int, BLOT*> s_apblot =
{
    {  0, (BLOT*)&g_letterbox },
    {  1, (BLOT*)&g_binoc },
    {  2, (BLOT*)&g_lgnr },
    {  3, (BLOT*)&g_lifectr },
    {  4, (BLOT*)&g_cluectr },
    {  5, (BLOT*)&g_keyctr },
    {  6, (BLOT*)&g_coinctr },
    {  7, (BLOT*)&g_goldctr },
    {  8, (BLOT*)&g_puffchargectr },
    {  9, (BLOT*)&g_lapctr },
    { 10, (BLOT*)&g_boostctr },
    { 11, (BLOT*)&g_placectr },
    { 12, (BLOT*)&g_securitycodectr },
    { 13, (BLOT*)&g_mgvhealthctr },
    { 14, (BLOT*)&g_boss },
    { 15, (BLOT*)&g_crusherctr },
    { 16, (BLOT*)&g_timer },
    { 17, (BLOT*)&g_note },
    { 19, (BLOT*)&g_title },
    { 20, (BLOT*)&g_totals },
    { 21, (BLOT*)&g_call },
    { 22, (BLOT*)&g_wmc },
    { 23, (BLOT*)&g_hubblot },
    { 24, (BLOT*)&g_fmvmenu },
    { 25, (BLOT*)&g_prompt },
    { 26, (BLOT*)&g_tvLeft },
    { 27, (BLOT*)&g_tvRight },
    { 28, (BLOT*)&g_van },
    { 29, (BLOT*)&g_scores },
    { 30, (BLOT*)&g_logo },
    { 31, (BLOT*)&g_attract },
    { 32, (BLOT*)&g_credit },
    { 33, (BLOT*)&g_rubyicon },
    { 34, (BLOT*)&g_jticon },
    { 35, (BLOT*)&g_autosave },
    { 36, (BLOT*)&g_percentctr }
};

inline BLOT* PblotFromBlotk(int blotk)
{
    const auto it = s_apblot.find(blotk);
    return it != s_apblot.end() ? it->second : nullptr;
}

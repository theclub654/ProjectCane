#pragma once
#include "alo.h"

constexpr int WORLDLEVEL_Count = 11;
constexpr int WMD_Count = 4;

using WmNavTable = WORLDLEVEL[WORLDLEVEL_Count][WMD_Count];

enum WMS 
{
    WMS_Nil = -1,
    WMS_Hidden = 0,
    WMS_Appearing = 1,
    WMS_Manual = 2,
    WMS_Warping = 3,
    WMS_Disappearing = 4,
    WMS_Max = 5
};
enum WMD 
{
    WMD_Nil = -1,
    WMD_Up = 0,
    WMD_Down = 1,
    WMD_Left = 2,
    WMD_Right = 3,
    WMD_Max = 4
};

struct WMFANVERTEX
{
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 rgba;
};

struct WMW 
{
    struct ASEG* pasegWarp;
    class ALO* paloPrimary;
    float uPrimaryAlphaTarget;
    class ALO* paloSecondary;
    float uAlphaTarget;
    class PNT* ppntPrimary;
    class PNT* ppntSecondary;
    int fNav;
    int fDot;
    float uDot;
    float uDotTarget;
    float radDot;
    float swDot;
};

struct WMC : public BLOT
{
    struct WM* pwmCurrent;
    struct WM* apwm[5];
    float uWarp;
    float uWarpTarget;
    float uWarpTargetGoal;
    GLuint gaoWmFan;
    GLuint gboWmFan;
};

class WM : public ALO
{
	public:
    WMS wms;
    float tWms;
    WORLDLEVEL worldlevelCur;
    WORLDLEVEL worldlevelGoal;
    WMS wmsActive;
    struct ASEG* pasegOpen;
    struct ASEG* pasegClose;
    struct ASEGA* pasegaOpenClose;
    struct ASEGA* pasegaMove;
    GAMEWORLD gameWorldCur;
    WMW awmw[11];
    WORLDLEVEL worldlevelCursor;
    glm::vec3 posCursor;
    glm::vec3 posCursorTarget;
    int fManualWarp;
    AMB* pambWmc;
};

WORLDLEVEL WorldLevelForWmDisplay();
bool FResolveWmLevel(WM* pwm, WORLDLEVEL worldLevelWm, GAMEWORLD* pgameWorld, WORLDLEVEL* pworldLevel);
bool FIsWmLevelAvailable(WM* pwm, WORLDLEVEL worldLevel);
WM*  NewWm();
int  GetWmSize();
void CloneWm(WM* pwm, WM* pwmBase);
void PostWmLoad(WM* pwm);
void BindWm(WM* pwm);
void RefreshWmMoveStats(WM* pwm, WORLDLEVEL worldlevel);
void ThrowWmDisplayState(WM* pwm, WORLDLEVEL worldlevel, int fReverse);
void CatchWmDisplayState(WM* pwm);
void UpdateWm(WM* pwm, float dt);
void RenderWmAll(WM* pwm, CM* pcm, RO* pro);
void HandleWmMessage(WM* pwm, MSGID msgid, void* pv);
void SetWmWms(WM* pwm, WMS wms);
void ShowWm(WM* pwm, WORLDLEVEL worldlevel, WMS wmsActive);
void HideWm(WM* pwm);
void MoveWm(WM* pwm, WMD wmd);
void GetWmWorldPosScreen(WM* pwm, WORLDLEVEL worldLevel, bool fSecondary, glm::vec3* pposScreen);
void SetWmCursor(WM* pwm, WORLDLEVEL worldlevel);
void DeleteWm(WM* pwm);

void StartupWmc(WMC* pwmc);
void PostWmcLoad(WMC* pwmc);
void OnWmcActive(WMC* pwmc, int fActive);
void UpdateWmcActive(WMC* pwmc, JOY* pjoy);
void DrawWmc(WMC* pwmc);
void DrawWmFan(WMC* pwmc, float xCenter, float yCenter, float sRadius, float rad, int cseg, glm::vec4* rgbaCenter, glm::vec4 rgbaEdge, int grfds);

extern WMC g_wmc;
extern CTextEdge g_teWmc;
extern glm::vec4 g_rgbaBoC;
extern glm::vec4 g_rgbaBoE;
extern WORLDLEVEL g_worldlevelWmPlayed;
extern int g_fWmReversePlayed;
extern int g_agrfwsSpecialMapUnlock[6];
extern SMP g_smpWmAlphaSlow;
extern SMP g_smpWmAlphaFast;
extern SMP g_smpWmDot;
extern SMP g_smpWmCursor;
extern const WmNavTable s_aaworldmapNavUnderwater;
extern const WmNavTable s_aaworldmapNavMuggshot;
extern const WmNavTable s_aaworldmapNavVoodoo;
extern const WmNavTable s_aaworldmapNavSnow;
extern const WmNavTable s_aaworldmapNavClockwerk;
extern const WmNavTable* s_mpaaworldmapNav[GAMEWORLD_Max];
extern SNIP s_asnipWm[52];
extern float g_rWmTaskMarkerScale;
extern float g_sWmDotOuter;       
extern float g_sWmDotInner;
extern const char* g_pchzWmTaskComplete;
extern const char* g_pchzWmTaskIncomplete;

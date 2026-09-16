#pragma once
#include "pnt.h"
#include "dialog.h"
#include "wipe.h"
#include "rose.h"

using PFNSPHERE = void (*)(LO*, glm::vec3*, float*);

enum BINOCS
{
    BINOCS_Nil = -1,
    BINOCS_None = 0,
    BINOCS_Peek = 1,
    BINOCS_Dialog = 2,
    BINOCS_Instruct = 3,
    BINOCS_Confront = 4,
    BINOCS_Sniper = 5,
    BINOCS_Max = 6
};
enum BFK
{
    BFK_Nil = -1,
    BFK_None = 0,
    BFK_Clue = 1,
    BFK_Breakable = 2,
    BFK_Gold = 3,
    BFK_Scan = 4,
    BFK_Max = 5
};

struct SLO
{
    LO* plo;
    float s;
    float sRadius;
    glm::vec3 pos;
};

class SCAN : public PNT
{
	public:
    std::string pchzScan;
    float uScan;       
    float uScanTarget;
};

struct BEI 
{
    CLQ clq;
    float cseg;
    float csegNotchHalf;
    int isegNotchFirst;
    int isegNotchMid;
    int isegNotchLast;
    float gNotchEdge;
    float gNotchCenter;
};

struct BRE
{
    struct LO *plo;
    struct ROSE *prose;
};

struct ReticleInstance {
    glm::mat4 model;
    glm::vec4 color;
};

struct BINOC : public BLOT
{
    BINOCS binocs;
    float tBinocs;
    float tAchzSet;
    int cichLR;
    int aichLR[32];
    float svch;
    glm::vec4 rgbaText;
    char chPause;
    class ALO* paloLookat;
    float uZoom;
    float dxReticle;
    float dyReticle;
    float radReticle;
    struct CFontBrx *pfontCompass;
    float uCompassBarOffset;
    int fTargeting;
    struct DIALOG *pdialogPlaying;
    BFK bfk;
    BFK bfkPrev;
    int mpinormalf[4];
    float mpinormalu[4];
    float tLastRose;
    int cbre;
    BRE abre[32];

    GLuint backGroundBinocVAO;
    GLuint backGroundBinocVBO;
    GLuint backGroundBinocEBO;

    std::vector <uint16_t> backGroundBinocIndices;

    GLuint outlineVAO;
    GLuint outlineVBO;
    GLuint outlineColorVBO = 0;
    GLuint outlineEBO;

    std::vector <uint16_t> outlineIndices;

    GLuint triangleBinocVAO;
    GLuint triangleBinocVBO;
    GLuint triangleBinocEBO;

    std::vector <uint16_t> triangleBinocIndices;

    GLuint binocIndicatorVAO;
    GLuint binocIndicatorVBO;

    GLuint reticleVAO = 0;
    GLuint reticleVBO = 0;
};

void StartupBinoc(BINOC* pbinoc);
void InitBinoc(BINOC* pbinoc, BLOTK blotk);
void PostBinocLoad(BINOC* pbinoc);
void ResetBinoc(BINOC* pbinoc);
void InitBei(BEI* pbei, const CLQ* pclq, float duWidth, float dgHeight, int cseg);
float GEvaluateBei(const BEI& bei, int iseg);
void GetBinocReticleFocus(BINOC* pbinoc, float* px, float* py);
void OnBinocPush(BINOC* pbinoc);
void OnBinocPop(BINOC* pbinoc);
void SetBinocBinocs(BINOC* pbinoc, BINOCS binocs);
void SetBinocBlots(BINOC* pbinoc, BLOTS blots);
int  NCmpSlo(const SLO* pslo1, const SLO* pslo2);
int  NCmpClueSlo(const SLO* pslo1, const SLO* pslo2);
void UpdateFilterRose(BINOC* pbinoc, CID cid, PFNSPHERE pfnSphere, PFNROSE pfnRose, int cploMax, float sMax, ALO* paloRender);
void GetSoSphere(LO* plo, glm::vec3* ppos, float* psRadius);
int  ChpFillAlo(void* pv, int chpMax, HP* ahp);
void SetBinocBfk(BINOC* pbinoc, BFK bfk, int fImmediate);
void UpdateBinocActiveFilter(BINOC* pbinoc, JOY* pjoy);
void UpdateBinocActive(BINOC* pbinoc, JOY* pjoy);
void OnBinocActive(BINOC* pbinoc, int fActive);
void OnBinocReset(BINOC* pbinoc);
void SetBinocAchzDraw(BINOC* pbinoc, char* pchz);
int  FDoneBinocAchz(BINOC* pbinoc);
void SetBinocLookat(BINOC* pbinoc, ALO* paloLookat);
void SetBinocZoom(BINOC* pbinoc, float zoomPercent);
float DtAppearBinoc(BINOC* pbinoc);
float DtDisappearBinoc(BINOC* pbinoc);


void BuildBinocBackGround(BINOC* pbinoc);
void BuildBinocOutline(BINOC* pbinoc);
void DrawBinocReticle(BINOC* pbinoc);
void DrawBinocBackground(BINOC *pbinoc);
void DrawBinocCompass(BINOC* pbinoc);
void DrawBinocZoom(BINOC* pbinoc);
void DrawBinocOutline(BINOC* pbinoc);
int  NCmpScanDistance(const void* pv0, const void* pv1);
void DrawBinocScan(BINOC* pbinoc);
void DrawBinocFilter(BINOC* pbinoc);
void DrawBinoc(BINOC* pbinoc);
void FreeBinocGL(BINOC* pbinoc);

SCAN*NewScan();
int  GetScanSize();
void CloneScan(SCAN* pscan, SCAN* pscanBase);
void LoadScanFromBrx(SCAN* pscan, CBinaryInputStream* pbis);
void InitializeScanDisplay(SCAN* pscan);
void DeleteScan(SCAN* pscan);

extern BINOC g_binoc;
extern CTextEdge g_teBinoc;
extern BEI s_beiUpper;
extern BEI s_beiLower;
extern BEI s_beiReticle;
extern CLQ s_clqUpper;
extern CLQ s_clqLower;
extern CLQ s_clqReticle;
extern float g_dxPointsMax;
extern const char *g_aachzPoints[8];
extern glm::vec4 RGBA_DarkBlue;
extern glm::vec4 RGBA_DarkRed;
extern glm::vec4 RGBA_LightRed;
extern glm::vec4 RGBA_Green;
extern glm::vec4 RGBA_LightBlue;
extern glm::vec4 RGBA_Overlay;
extern float DT_BinocRoseInterval;
extern int s_mpbfkgrfvault[2];

static constexpr float s_loFinderPulse = 8.0;

static SMP s_smpLoFinderAlpha =
{
    8, 0.0, 0.15
};

static constexpr float s_mpinormalax[4][4] =
{
    {  -5.0f, -15.0f, -35.0f, -15.0f }, // Left
    {   5.0f,  15.0f,  35.0f,  15.0f }, // Right
    {   0.0f,  10.0f,   0.0f, -10.0f }, // Vertical indicator
    {   0.0f, -10.0f,   0.0f,  10.0f }  // Vertical indicator
};

static constexpr float s_mpinormalay[4][4] =
{
    {   0.0f, -11.0f,   0.0f,  11.0f }, // Left
    {   0.0f,  11.0f,   0.0f, -11.0f }, // Right
    {  -4.4f, -15.4f, -35.2f, -15.4f }, // Up
    {   4.4f,  15.4f,  35.2f,  15.4f }  // Down
};

static constexpr glm::vec2 s_aBinocDirectionVertices[4][4] =
{
    // Left
    {
        { -5.0f,   0.0f },
        { -15.0f, -11.0f },
        { -35.0f,  0.0f },
        { -15.0f, 11.0f }
    },

    // Right
    {
        { 5.0f,   0.0f },
        { 15.0f, 11.0f },
        { 35.0f,  0.0f },
        { 15.0f, -11.0f }
    },

    // Up
    {
        { 0.0f,  -4.4f },
        { 10.0f, -15.4f },
        { 0.0f,  -35.2f },
        { -10.0f, -15.4f }
    },

    // Down
    {
        { 0.0f,  4.4f },
        { -10.0f, 15.4f },
        { 0.0f,  35.2f },
        { 10.0f, 15.4f }
    }
};

static constexpr glm::vec2 s_scanSelectionCenter =
{
    0.0, 0.3
};

static constexpr float s_scanSelectionDistanceMax = 3.4028235E+38;

static constexpr float s_scanAlphaBase = 1.5;

static constexpr float s_scanAlphaLinear = -1.5;

static constexpr float s_scanAlphaQuadratic = 0.0;

static constexpr float s_scanInactiveAlpha = 0.5;

static constexpr float s_scanSelectedScale = 1.0;

static constexpr float s_scanInactiveScale = 0.5;

static constexpr float s_scanTextWidth = 250.0;

static constexpr float s_scanTextOffsetX = 40.0;

static constexpr float s_scanTextOffsetY = -20.0;

static constexpr float s_scanLineOffsetX = -15.0;

static constexpr float s_scanLineOffsetY = 1.0;

static SMP s_smpScanAlpha
{
    5.0, 0.0, 0.1
};

static SMP s_smpScanScale
{
    50, 0.0, 0.1
};

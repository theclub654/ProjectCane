#pragma once
#include "pnt.h"
#include "dialog.h"
#include "wipe.h"

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

class SCAN : public PNT
{
	public:
		TBID tbidCaption;
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
    struct ALO* paloLookat;
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
    GLuint outlineEBO;

    std::vector <uint16_t> outlineIndices;

    GLuint triangleBinocVAO;
    GLuint triangleBinocVBO;
    GLuint triangleBinocEBO;

    std::vector <uint16_t> triangleBinocIndices;
};

void StartupBinoc(BINOC* pbinoc);
void InitBinoc(BINOC* pbinoc, BLOTK blotk);
void PostBinocLoad(BINOC* pbinoc);
void ResetBinoc(BINOC* pbinoc);
void InitBei(BEI* pbei, const CLQ* pclq, float duWidth, float dgHeight, int cseg);
float GEvaluateBei(const BEI& bei, int iseg);
void GetBinocReticleFocus(BINOC* pbinoc, float* px, float* py);
void SetBinocLookat(BINOC* pbinoc, ALO* paloLookat);
void OnBinocActive(BINOC* pbinoc, int fActive);
void UpdateBinocActive(BINOC* pbinoc, JOY* pjoy);
void OnBinocReset(BINOC* pbinoc);
void OnBinocPush(BINOC* pbinoc);
void OnBinocPop(BINOC* pbinoc);
void SetBinocAchzDraw(BINOC* pbinoc, char *pchz);
float DtAppearBinoc(BINOC* pbinoc);
float DtDisappearBinoc(BINOC* pbinoc);
void SetBinocBlots(BINOC* pbinoc, BLOTS blots);
void BuildBinocBackGround(BINOC* pbinoc);
void BuildBinocOutline(BINOC* pbinoc);
void DrawBinocReticle(BINOC* pbinoc);
void DrawBinocBackground(BINOC *pbinoc);
void DrawBinocCompass(BINOC* pbinoc);
void DrawBinocZoom(BINOC* pbinoc);
void DrawBinocOutline(BINOC* pbinoc);
void DrawBinocFilter(BINOC* pbinoc);
void DrawBinoc(BINOC* pbinoc);

SCAN*NewScan();
int  GetScanSize();
void CloneScan(SCAN* pscan, SCAN* pscanBase);
void LoadScanFromBrx(SCAN* pscan, CBinaryInputStream* pbis);
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
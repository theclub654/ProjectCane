#pragma once
#include "pnt.h"

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

struct BRE
{
    struct LO *plo;
    struct ROSE *prose;
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
    struct CFont* pfontCompass;
    float uCompassBarOffset;
    int cqwGifs;
    int fTargeting;
    struct DIALOG* pdialogPlaying;
    BFK bfk;
    BFK bfkPrev;
    int mpinormalf[4];
    float mpinormalu[4];
    float tLastRose;
    int cbre;
    BRE abre[32];
};

void StartupBinoc(BINOC* pbinoc);
void InitBinoc(BINOC* pbinoc, BLOTK blotk);
void PostBinocLoad(BINOC* pbinoc);
void OnBinocActive(BINOC* pbinoc, int fActive);
void UpdateBinocActive(BINOC* pbinoc, JOY* pjoy);
void OnBinocReset(BINOC* pbinoc);
void OnBinocPush(BINOC* pbinoc);
void OnBinocPop(BINOC* pbinoc);
void SetBinocAchzDraw(BINOC* pbinoc, char *pchz);
float DtAppearBinoc(BINOC* pbinoc);
float DtDisappearBinoc(BINOC* pbinoc);
void SetBinocBlots(BINOC* pbinoc, BLOTS blots);
void DrawBinoc(BINOC* pbinoc);

SCAN*NewScan();
int  GetScanSize();
void CloneScan(SCAN* pscan, SCAN* pscanBase);
void DeleteScan(SCAN* pscan);

extern BINOC g_binoc;
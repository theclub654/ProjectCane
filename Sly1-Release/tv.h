#pragma once
#include "blot.h"
#include "util.h"

enum TVS
{
    TVS_Nil = -1,
    TVS_Listen = 0,
    TVS_Talk = 1,
    TVS_Max = 2
};
enum TVGS 
{
    TVGS_Nil = -1,
    TVGS_Opening = 0,
    TVGS_Open = 1,
    TVGS_Closing = 2,
    TVGS_Closed = 3,
    TVGS_Max = 4
};

struct TVB
{
    float xCurveTop;
    float xCurveBottom;
    float yTop;
    float yBottom;
    glm::vec4 rgbaBase;
    glm::vec4 rgbaCur;
    float uDistort;
    float uDistortTarget;
    SMP smpDistort;
};

struct TV : public BLOT
{
    TVS tvs;
    float tTvs;
    TVGS tvgs;
    float tTvgs;
    struct SPEAKER* pspeaker;
    struct SPEAKER* pspeakerDraw;
    class ALO* paloReplace;
    struct LIGHT* plight;
    TVB atvb[57];
    float xCurveCenter;
    float xFlatBase;
    float dsOutline;
    float dsOutlineTarget;
    float uInterfere;
    float uInterfereTarget;
    SMP smpInterfere;
    float uUnwind;
};

void StartupTvLeft(TV* ptvleft);
void StartupTvRight(TV* ptvright);
void InitTv(TV* ptv, BLOTK blotk);
void ResizeTvForFramebuffer(TV* ptv);
void ResizeTvUiForFramebuffer();
void PostTvLoad(TV* ptv);
void GetTvItvbMinMax(TV* ptv, int* pitvbDrawMin, int* pitvbDrawMax);
void OnTvReset(TV* ptv);
void SetTvBlots(TV* ptv, BLOTS blots);
void SetTvTvgs(TV* ptv, TVGS tvgs);
void SetTvReplace(TV* ptv, ALO* paloReplace);
void AcceptTvSpeaker(TV* ptv);
void UpdateTv(TV* ptv);
void RenderTv(TV* ptv);
void PreTvContext(RPL* prpl);
void PostTvContext(RPL* prpl);
void DrawTv(TV* ptv);
void SetTvTvs(TV* ptv, TVS tvs);
void SetTvSpeaker(TV* ptv, SPEAKER* pspeaker);
void DrawTvArea(TV* ptv);
void DrawTvBands(TV* ptv);
void DrawTvOutline(TV* ptv);

extern TV g_tvRight;
extern TV g_tvLeft;
// Non-null only while DrawSw is executing the ordered
// PreTvContext -> speaker globs -> PostTvContext range for this TV.
extern TV* g_ptvDrawContext;

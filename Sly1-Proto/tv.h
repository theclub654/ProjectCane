#pragma once
#include "blot.h"

enum TVS
{
    TVS_Nil = -1,
    TVS_Listen = 0,
    TVS_Talk = 1,
    TVS_Max = 2
};
enum TVGS {
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
    struct ALO* paloReplace;
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
void PostTvLoad(TV* ptv);
void OnTvReset(TV* ptv);
void SetTvBlots(TV* ptv, BLOTS blots);
void UpdateTv(TV* ptv);
void DrawTv(TV* ptv);

extern TV g_tvRight;
extern TV g_tvLeft;
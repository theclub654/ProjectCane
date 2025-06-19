#include "tv.h"

void StartupTvLeft(TV* ptvleft)
{
    ptvleft->pvttv = &g_vttv;
}

void StartupTvRight(TV* ptvright)
{
    ptvright->pvttv = &g_vttv;
}

void InitTv(TV* ptv, BLOTK blotk)
{
    InitBlot(ptv, blotk);
}

void PostTvLoad(TV* ptv)
{

}

void OnTvReset(TV* ptv)
{

}

void SetTvBlots(TV* ptv, BLOTS blots)
{

}

void UpdateTv(TV* ptv)
{

}

void DrawTv(TV* ptv)
{

}

TV g_tvRight;
TV g_tvLeft;
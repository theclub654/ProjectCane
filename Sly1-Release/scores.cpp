#include "scores.h"

void StartupScores(SCORES* pscores)
{
    pscores->pvtscores = &g_vtscores;
}

void PostScoresLoad(SCORES* pscores)
{
    PostBlotLoad(pscores);

    pscores->rFontScale = 0.75;
    pscores->pfont = g_pfontScreenCounters;
    pscores->achzFormat = "J%d  J%d";
}

void UpdateScores(SCORES* pscores)
{

}

void DrawScores(SCORES* pscores)
{

}

SCORES g_scores;
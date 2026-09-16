#include "scores.h"

void StartupScores(SCORES* pscores)
{
    pscores->pvtscores = &g_vtscores;
}

SCORES g_scores;

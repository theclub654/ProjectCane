#pragma once
#include "ctr.h"

struct SCORES : public CTR
{
    int cCpuCollected;
};

void StartupScores(SCORES* pscores);

extern SCORES g_scores;

#pragma once
#include "blot.h"

struct SCORES : public BLOT
{
    const char* achzFormat;
    int an[8];
};

void StartupScores(SCORES* pscores);
void PostScoresLoad(SCORES* pscores);
void UpdateScores(SCORES* pscores);
void DrawScores(SCORES* pscores);

extern SCORES g_scores;
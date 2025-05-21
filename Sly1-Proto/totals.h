#pragma once
#include "blot.h"

struct TOTALS : public BLOT
{
    int fReshow;
    GRFLS grflsReshow;
};

void StartupTotals(TOTALS* ptotals);
void PostTotalsLoad(TOTALS* ptotals);
void SetTotalsBlots(TOTALS* ptotals, BLOTS blots);
void DrawTotals(TOTALS* ptotals);

extern TOTALS g_totals;
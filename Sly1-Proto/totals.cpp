#include "totals.h"

void StartupTotals(TOTALS* ptotals)
{
    ptotals->pvttotals = &g_vttotals;
}

void PostTotalsLoad(TOTALS* ptotals)
{
    PostBlotLoad(ptotals);
}

void SetTotalsBlots(TOTALS* ptotals, BLOTS blots)
{

}

void DrawTotals(TOTALS* ptotals)
{

}

TOTALS g_totals;
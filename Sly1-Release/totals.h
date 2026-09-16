#pragma once
#include "blot.h"

struct TOTALS : public BLOT
{
    int fReshow;
    char achzTotals[512];
    int fDrawOverLetterbox;
};

void StartupTotals(TOTALS* ptotals);
void PostTotalsLoad(TOTALS* ptotals);
void SetTotalsAchzDraw(TOTALS* ptotal, char* pchz);
void FormatTotalsTasks(char* pchzText, LS* pls, uint32_t grfTasks);
void SetTotalsDestinationText(TOTALS* ptotals, int levelId);
void SetTotalsLevelText(TOTALS* ptotals, int worldId, int levelIndex);
void SetTotalsWorldText(TOTALS* ptotals, int worldId);
void SetTotalsBlots(TOTALS* ptotals, BLOTS blots);
void ShowTotals(TOTALS* ptotals);
void HideTotals(TOTALS* ptotals);
void DrawTotals(TOTALS* ptotals);

extern TOTALS g_totals;
extern CTextEdge g_teTotals;
extern int g_fTotalsAvailable;
static const char* const s_apchzWorldNames[] =
{
    "",
    "Tide of Terror",
    "Sunset Snake Eyes",
    "Vicious Voodoo",
    "Fire in the Sky",
    "The Cold Heart of Hate"
};

static const char* s_pchzTotalsKeysFormat = " &1%d^10~b2a83d/%d~.^.&.&3k&.";
static const char* s_pchzTotalsVaultsFormat = "%s&1%d^10~b2a83d/%d~.^.&.&3v&.";
static const char* s_pchzTotalsTimedFormat = "%s&1%d^10~b2a83d/%d~.^.h&.";
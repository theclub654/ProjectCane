#include "totals.h"

void StartupTotals(TOTALS* ptotals)
{
    ptotals->pvttotals = &g_vttotals;
}

void PostTotalsLoad(TOTALS* ptotals)
{
    PostBlotLoad(ptotals);

    // Clone the screen font at 1:1 scale
    ptotals->pfont = g_afontBrx[2].PfontClone(1.0, 1.0);
;    // Set RGBA color (0x5f, 0x5f, 0x5f, 0xbf)
    ptotals->rgba = glm::vec4(0.3725f, 0.3725f, 0.3725f, 0.749f);

    // Sample text for measuring width
    const char* sampleText = "KJH";
    float dx = ptotals->pfont->DxFromPchz((char*)sampleText);
    float dy = ptotals->pfont->m_dyUnscaled * ptotals->pfont->m_ryScale;

    ResizeBlot(ptotals, dx, dy);
}

void SetTotalsBlots(TOTALS* ptotals, BLOTS blots)
{
    if ((ptotals->fReshow != 0) && (blots == BLOTS_Hidden)) 
    {
        ptotals->fReshow = 0;
        //FormatTotalsGrfls(ptotals, ptotals->grflsReshow);
        blots = BLOTS_Appearing;
    }
    SetBlotBlots(ptotals, blots);
}

void DrawTotals(TOTALS* ptotals)
{
    if (ptotals->achzDraw[0] == '\0')
        return;

    // Create rich text from string and font
    CRichText rt(ptotals->achzDraw, ptotals->pfont);

    // Set up text box
    CTextBox tbx;
    tbx.SetPos(ptotals->x, ptotals->y);
    tbx.SetSize(ptotals->dx, ptotals->dy);
    glm::vec4 textColor = glm::vec4{ 1.0, 1.0, 1.0, 1.0 };
    tbx.SetTextColor(&textColor); // Gray color
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    // Draw the rich text
    rt.Draw(&tbx);
}

TOTALS g_totals;
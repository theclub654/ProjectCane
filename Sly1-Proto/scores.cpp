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
    UpdateBlot(pscores);

    pscores->pfont->PushScaling(pscores->rFontScale, pscores->rFontScale);
    float dx = pscores->pfont->DxFromPchz(pscores->achzDraw);
    ResizeBlot(pscores, dx, pscores->pfont->m_dyUnscaled * pscores->pfont->m_ryScale);
    pscores->pfont->PopScaling();
}

void DrawScores(SCORES* pscores)
{
    if (pscores == nullptr) return;
    if (pscores->achzDraw[0] == '\0') return;

    CTextBox tbx;
    float dx = pscores->dx;
    float dy = pscores->dy;

    tbx.SetPos(pscores->x, pscores->y);
    tbx.SetSize(dx, dy);


    tbx.SetTextColor(&pscores->rgba);

    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    ;
    float rFontScale = pscores->rFontScale;

    if (pscores->pte != nullptr) {
        pscores->pte->m_pfont->EdgeRect(pscores->pte, &tbx);
    }

    pscores->pfont->PushScaling(rFontScale, rFontScale);
    pscores->pfont->DrawPchz(pscores->achzDraw, &tbx);
    pscores->pfont->PopScaling();
}

SCORES g_scores;
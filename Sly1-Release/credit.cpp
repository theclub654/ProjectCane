#include "credit.h"

void StartupNote(NOTE* pnote)
{
    g_teNote.m_dyExtra = 1.0;
    g_teNote.m_rgba = glm::vec4(0.0f, 0.294, 0.490, 1.0);
    g_teNote.m_ch = '-';
    g_teNote.m_dxExtra = 2.0;
    g_teNote.m_rxScaling = 0.3;
    g_teNote.m_ryScaling = 0.3;

    pnote->pvtnote = &g_vtnote;
}

void PostNoteLoad(NOTE* pnote)
{
    // Step 1: Run shared UI element initialization
    PostBlotLoad(pnote);

    // Step 2: Clone the font at 1.25x scale if available
    if (pnote->pfont) {
        // Assumes CFontBrx has a Clone() method that takes scaleX, scaleY
        pnote->pfont = pnote->pfont->PfontClone(1.25f, 1.25f);
    }

    // Step 3: Apply shared CTextEdge if the counter font exists
    if (g_pfontScreenCounters) {
        pnote->pte = &g_teNote;
        g_teNote.m_pfont = &g_afontBrx[2];
    }
}

void DrawNote(NOTE* pnote)
{
    if (pnote->achzDraw[0] == '\0') return;

    // Fade color based on pnote->uOn
    glm::vec4 fadedColor = pnote->rgba;
    fadedColor.a *= pnote->uOn;

    // Setup text box
    CTextBox tbx;
    tbx.SetPos(pnote->x, pnote->y);
    tbx.SetSize(pnote->dx, pnote->dy);
    tbx.SetTextColor(&fadedColor);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    // Handle optional edge rendering
    float fontScale = pnote->rFontScale;
    if (pnote->pte)
    {
        // Backup edge RGBA
        glm::vec4 edgeColor = pnote->pte->m_rgba;

        // Apply fade to edge alpha
        pnote->pte->m_rgba.a *= pnote->uOn;

        if (pnote->pte->m_pfont)
        {
            pnote->pte->m_pfont->EdgeRect(pnote->pte, &tbx);
        }

        // Restore original edge color
        pnote->pte->m_rgba = edgeColor;

        fontScale = pnote->rFontScale;
    }

    // Push font scaling
    pnote->pfont->PushScaling(fontScale, fontScale);

    // Draw rich text
    CRichText rt(pnote->achzDraw, pnote->pfont);
    rt.Draw(&tbx);

    // Pop font scaling
    pnote->pfont->PopScaling();
}

void StartupCredit(CREDIT* pcredit)
{
    pcredit->pvtcredit = &g_vtcredit;
}

void InitCredit(CREDIT* pcredit, BLOTK blotk)
{
    InitBlot(pcredit, BLOTK_Credit);

    for (int i = 0; i < 4; i++)
    {
        g_vtnote.pfnInitBlot(&pcredit->anote[i], BLOTK_Note);
        pcredit->anote[i].pvtnote = &g_vtnote;
    }
}

void PostCreditLoad(CREDIT* pcredit)
{
    PostBlotLoad(pcredit);

    for (int i = 0; i < 4; i++)
    {
        pcredit->anote[i].pvtblot->pfnPostBlotLoad(&pcredit->anote[i]);

        pcredit->anote[i].pte = nullptr;
        pcredit->pfont->m_rxScale = 0.8;
        pcredit->pfont->m_ryScale = 0.8;

        SetBlotDtAppear(&pcredit->anote[i], 1.0);
        SetBlotDtDisappear(&pcredit->anote[i], 1.0);
        SetBlotDtVisible(&pcredit->anote[i], 0.0);
    }
}

void SetCreditClock(CREDIT* pcredit, float* pt)
{
    // Update the clock for the CREDIT's base BLOT
    SetBlotClock(pcredit, pt);

    // Update clocks for each of the 4 NOTE elements in CREDIT
    for (int i = 0; i < 4; ++i)
    {
        NOTE* pnote = &pcredit->anote[i];
        if (pnote->pvtblot && pnote->pvtblot->pfnSetBlotClock)
        {
            pnote->pvtblot->pfnSetBlotClock(pnote, pt);
        }
    }
}

void UpdateCredit(CREDIT* pcredit)
{
    // Update the base CREDIT BLOT
    UpdateBlot(pcredit);

    // Update all active NOTE entries
    for (int i = 0; i < pcredit->cnote; ++i)
    {
        NOTE* pnote = &pcredit->anote[i];
        if (pnote->pvtblot && pnote->pvtblot->pfnUpdateBlot)
        {
            pnote->pvtblot->pfnUpdateBlot(pnote);
        }
    }
}

void DrawCredit(CREDIT* pcredit)
{
    // Do not draw if binoculars are visible
    if (GetBlot((BLOT*)&g_binoc) != BLOTS_Hidden)
        return;
    
    float yCurrent = pcredit->y;

    for (int i = 0; i < pcredit->cnote; ++i)
    {
        NOTE* pnote = &pcredit->anote[i];

        // Cache original uOn value
        float originalUOn = pnote->uOn;

        // Set position
        pnote->x = pcredit->x;
        pnote->y = yCurrent;

        // Temporarily adjust uOn for rendering (scaled by g_rtClock)
        pnote->uOn = originalUOn * g_rtClock;

        // Draw the note via virtual call
        if (pnote->pvtblot && pnote->pvtblot->pfnDrawBlot)
        {
            pnote->pvtblot->pfnDrawBlot(pnote);
        }

        // Restore original uOn
        pnote->uOn = originalUOn;

        // Advance y-position for next NOTE
        yCurrent += pnote->dy;
    }
}

CREDIT g_credit;
NOTE g_note;
CTextEdge g_teNote;
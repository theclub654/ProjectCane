#include "title.h"

void StartupTitle(TITLE* ptitle)
{
    g_teTitle.m_rgba = glm::vec4(0.0f, 75.0f / 255.0f, 125.0f / 255.0f, 1.0f);
    g_teTitle.m_ch = '-';
    g_teTitle.m_dxExtra = 2.0;
    g_teTitle.m_rxScaling = 0.3;
    g_teTitle.m_ryScaling = 0.3;
    g_teTitle.m_dyExtra = -4.0;

    ptitle->pvttitle = &g_vttitle;
}

void PostTitleLoad(TITLE* ptitle)
{
    // Initialize base BLOT layout
    PostBlotLoad(ptitle);

    // Clone and rescale font (0.95f x 0.95f)
    if (ptitle->pfont)
    {
        ptitle->pfont = ptitle->pfont->PfontClone(0.95f, 0.95f);
    }

    ptitle->rgba = glm::vec4(
        127.0f / 255.0f,
        127.0f / 255.0f,
        127.0f / 255.0f,
        223.0f / 255.0f
    );

    // Assign edge font if available
    ptitle->pte = &g_teTitle;
    g_teTitle.m_pfont = &g_afontBrx[2];

    // Used for testing
    ptitle->pvtblot->pfnSetBlotAchzDraw(ptitle, (char*)"A Sucker Punch Production");
}

int FIncludeTitleForPeg(TITLE* ptitle, BLOT* pblotOther)
{
    int iVar1;

    iVar1 = FIncludeBlotForPeg((BLOT*)ptitle, pblotOther);

    if ((iVar1 != 0) || (iVar1 = 0, ptitle->fReshow != 0)) {
        iVar1 = 1;
    }
    return iVar1;
}

void SetTitleAchzDraw(TITLE* ptitle, char* pchz)
{
    if (ptitle->blots == BLOTS_Hidden) {
        SetBlotAchzDraw((BLOT*)ptitle, pchz);
    }
    else if ((BLOTS_Nil < ptitle->blots) && (ptitle->blots < BLOTS_Max)) {
        ptitle->pchzReshow = pchz;
        ptitle->fReshow = 1;
        ptitle->pvtblot->pfnSetBlotBlots(ptitle, BLOTS_Disappearing);
    }
}

void SetTitleBlots(TITLE* ptitle, BLOTS blots)
{
    if ((ptitle->fReshow != 0) && (blots == BLOTS_Hidden)) {
        ptitle->fReshow = 0;
        SetBlotAchzDraw(ptitle, ptitle->pchzReshow);
        blots = BLOTS_Appearing;
    }
    SetBlotBlots((BLOT*)ptitle, blots);
}

void DrawTitle(TITLE* ptitle)
{
    // Skip if not in a drawable state
    if (ptitle->blots != BLOTS_Visible &&
        ptitle->blots != BLOTS_Appearing &&
        ptitle->blots != BLOTS_Disappearing)
        return;

    // Draw base BLOT if visible
    if (ptitle->blots == BLOTS_Visible) {
        DrawBlot(ptitle);
        return;
    }

    const std::string& text = ptitle->achzDraw;
    const size_t length = text.length();
    if (length == 0) return;

    float uOn = ptitle->uOn;
    float xOn = ptitle->xOn;
    float xOff = ptitle->xOff;
    float y = ptitle->yOn;

    // Interpolate overall draw position based on progress
    float xInterp = std::clamp(uOn / (1.0f / static_cast<float>(length)), 0.0f, 1.0f);
    float xStart = xOff * (1.0f - xInterp) + xOn * xInterp;

    // Edge text rendering
    if (ptitle->pte && ptitle->pte->m_pfont)
    {
        CTextBox tbx;
        tbx.SetPos(xStart, ptitle->y);
        tbx.SetSize(ptitle->dx, ptitle->dy);
        glm::vec4 textColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        tbx.SetTextColor(&textColor);

        tbx.SetHorizontalJust(JH_Left);
        tbx.SetVerticalJust(JV_Top);

        ptitle->pte->m_pfont->EdgeRect(ptitle->pte, &tbx);
    }

    // Draw animated text using DxDrawCh
    CFontBrx* font = ptitle->pfont;
    if (!font) return;

    font->SetupDraw();

    float xCursorOn = ptitle->xOn;
    float xCursorOff = ptitle->xOff;

    for (size_t i = 0; i < length; ++i)
    {
        float ratio = uOn / (static_cast<float>(i + 1) / static_cast<float>(length));
        ratio = std::clamp(ratio, 0.0f, 1.0f);

        float xChar = xCursorOff * (1.0f - ratio) + xCursorOn * ratio;
        char ch = text[i];

        // Draw the glyph and advance
        float dx = font->DxDrawCh(ch, xChar, y, ptitle->rgba);
        xCursorOn += dx;
        xCursorOff += dx;
    }

    font->CleanUpDraw();
}

TITLE g_title;
CTextEdge g_teTitle;
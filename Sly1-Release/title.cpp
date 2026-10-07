#include "title.h"
#include "gui_layout.h"
#include "gl.h"

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
    ptitle->pfontOwned = ptitle->pfont->PfontClone(0.9f, 0.9f);
    ptitle->pfont = ptitle->pfontOwned.get();
    // Retail stores PS2 GS colors, where 0x80 is full RGB intensity.
    ptitle->rgba = glm::vec4(127.0f / 128.0f, 127.0f / 128.0f,
        127.0f / 128.0f, 223.0f / 255.0f);

    if (FFontLoaded(2))
    {
        ptitle->pte = &g_teTitle;
        ptitle->pte->m_pfont = PfontFromFont(2);
    }

    // Used for testing
    //ptitle->pvtblot->pfnSetBlotAchzDraw(ptitle, (char*)"A Sucker Punch Production");
}

int FIncludeTitleForPeg(TITLE* ptitle, BLOT* pblotOther)
{
    return FIncludeBlotForPeg(ptitle, pblotOther) || ptitle->fReshow;
}

void SetTitleAchzDraw(TITLE* ptitle, char* pchz)
{
    if (ptitle->blots == BLOTS_Hidden) {
        SetBlotAchzDraw(ptitle, pchz);
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

void ShowTitle(TITLE* ptitle)
{
    if ((ptitle->blots != BLOTS_Disappearing) || (ptitle->fReshow == 0))
        ShowBlot(ptitle);
}

void HideTitle(TITLE* ptitle)
{
    if ((ptitle->blots == BLOTS_Disappearing) && (ptitle->fReshow != 0)) 
        ptitle->fReshow = 0;
    else
        HideBlot(ptitle);
}

void DrawTitle(TITLE* ptitle)
{
    // Skip if not in a drawable state
    if (ptitle->blots != BLOTS_Visible &&
        ptitle->blots != BLOTS_Appearing &&
        ptitle->blots != BLOTS_Disappearing)
        return;

    const GuiScale guiScale = GetGuiScale();
    const float scaleX = guiScale.x;
    const float scaleY = guiScale.y;
    const auto ScaleX = [scaleX](float x) { return x * scaleX; };
    const auto ScaleY = [scaleY](float y)
    {
        return g_gl.height - (g_gl.height - y) * scaleY;
    };

    CFontBrx* font = ptitle->pfont;
    if (!font)
        return;

    if (ptitle->blots == BLOTS_Visible)
    {
        CTextBox tbx;
        tbx.SetPos(ScaleX(ptitle->x), ScaleY(ptitle->y));
        tbx.SetSize(ptitle->dx * scaleX, ptitle->dy * scaleY);
        tbx.SetTextColor(&ptitle->rgba);
        tbx.SetHorizontalJust(JH_Left);
        tbx.SetVerticalJust(JV_Top);
        if (ptitle->pte && ptitle->pte->m_pfont)
        {
            const float edgeScaleX = ptitle->pte->m_rxScaling;
            const float edgeScaleY = ptitle->pte->m_ryScaling;
            ptitle->pte->m_rxScaling *= scaleX;
            ptitle->pte->m_ryScaling *= scaleY;
            ptitle->pte->m_pfont->EdgeRect(ptitle->pte, &tbx);
            ptitle->pte->m_rxScaling = edgeScaleX;
            ptitle->pte->m_ryScaling = edgeScaleY;
        }
        font->PushScaling(ptitle->rFontScale * scaleX, ptitle->rFontScale * scaleY);
        font->DrawPchz(ptitle->achzDraw, &tbx);
        font->PopScaling();
        return;
    }

    const std::string& text = ptitle->achzDraw;
    const size_t length = text.length();
    if (length == 0) return;

    float uOn = ptitle->uOn;
    float xOn = ScaleX(ptitle->xOn);
    float xOff = ScaleX(ptitle->xOff);
    float y = ScaleY(ptitle->yOn);
    float xInterp = std::clamp(uOn / (1.0f / static_cast<float>(length)), 0.0f, 1.0f);
    float xStart = xOff * (1.0f - xInterp) + xOn * xInterp;

    if (ptitle->pte && ptitle->pte->m_pfont)
    {
        CTextBox tbx;
        tbx.SetPos(xStart, ScaleY(ptitle->y));
        const float edgeWidth = xOn * uOn + xOff * (1.0f - uOn) + ptitle->dx * scaleX;
        tbx.SetSize(edgeWidth, ptitle->dy * scaleY);
        glm::vec4 textColor(1.0f);
        tbx.SetTextColor(&textColor);
        tbx.SetHorizontalJust(JH_Left);
        tbx.SetVerticalJust(JV_Top);
        const float edgeScaleX = ptitle->pte->m_rxScaling;
        const float edgeScaleY = ptitle->pte->m_ryScaling;
        ptitle->pte->m_rxScaling *= scaleX;
        ptitle->pte->m_ryScaling *= scaleY;
        ptitle->pte->m_pfont->EdgeRect(ptitle->pte, &tbx);
        ptitle->pte->m_rxScaling = edgeScaleX;
        ptitle->pte->m_ryScaling = edgeScaleY;
    }

    font->PushScaling(ptitle->rFontScale * scaleX, ptitle->rFontScale * scaleY);

    float xCursorOn = xOn;
    float xCursorOff = xOff;
    std::vector<float> xPositions(length);

    for (size_t i = 0; i < length; ++i)
    {
        float ratio = uOn / (static_cast<float>(i + 1) / static_cast<float>(length));
        ratio = std::clamp(ratio, 0.0f, 1.0f);

        float xChar = xCursorOff * (1.0f - ratio) + xCursorOn * ratio;
        char ch = text[i];

        xPositions[i] = xChar;
        float dx = font->DxFromCh(ch);
        xCursorOn += dx;
        xCursorOff += dx;
    }

    font->DrawGlyphRun(text.data(), xPositions.data(), length, y, ptitle->rgba);
    font->PopScaling();
}

TITLE g_title;
CTextEdge g_teTitle;

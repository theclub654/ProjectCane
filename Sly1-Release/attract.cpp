#include "attract.h"
#include "gui_layout.h"
#include "logo.h"
#include "save.h"
#include "ui.h"

void StartupAttract(ATTRACT* pattract)
{
    pattract->pvtattract = &g_vtattract;

    g_teAttract.m_rgba = glm::vec4(0.0f, 75.0f / 128.0f, 125.0f / 128.0f, 1.0f);
    g_teAttract.m_ch = '-';
    g_teAttract.m_dxExtra = 2.0;
    g_teAttract.m_ryScaling = 0.3;
    g_teAttract.m_rxScaling = 0.3;
    g_teAttract.m_dyExtra = -2.0f;
}

void PostAttractLoad(ATTRACT* pattract)
{
    PostBlotLoad(pattract);

    pattract->pfontOwned = pattract->pfont->PfontClone(RX_Attract, RY_Attract);
    pattract->pfont = pattract->pfontOwned.get();

    if (CFontBrx* pfontEdge = PfontFromFont(2))
    {
        pattract->pte = &g_teAttract;
        pattract->pte->m_pfont = pfontEdge;
    }

    pattract->fJoyValid = g_joy.joys == JOYS_Ready;
    pattract->fReshow = g_saveData.pgsAttractSave && g_saveData.pgsAttractSave->dt != 0.0f;

    UpdateAttractText(pattract);
}

void UpdateAttractText(ATTRACT* pattract)
{
    if (!pattract->fJoyValid)
    {
        pattract->rgba = glm::vec4(111.0f / 128.0f, 31.0f / 128.0f, 31.0f / 128.0f, 1.0f);
        std::strcpy(pattract->achzDraw, "No Controller");
    }
    else
    {
        pattract->rgba = glm::vec4(111.0f / 128.0f, 111.0f / 128.0f, 111.0f / 128.0f, 1.0f);
        const int ichz = pattract->fReshow ? 1 : 0;
        std::snprintf(pattract->achzDraw, sizeof(pattract->achzDraw), g_aachzAttract[ichz], "Press SELECT button for Menu");
    }

    pattract->pfont->PushScaling(pattract->rFontScale, pattract->rFontScale);
    CRichText sizingText(pattract->achzDraw, pattract->pfont);
    ResizeBlot(pattract, sizingText.DxMaxLine(), sizingText.DyWrap(0.0f));
    pattract->pfont->PopScaling();
}

void SetAttractAchzDraw(ATTRACT* pattract, char* pchz)
{
    BLOTS blots = pattract->blots;

    if (blots == BLOTS_Hidden)
    {
        SetBlotAchzDraw(pattract, pchz);

        // Choose color based on controller validity
        glm::vec4 color;
        color = glm::vec4(1.0, 1.0f, 1.0, 1.0f);

        pattract->rgba = color;
    }
    else if (blots > BLOTS_Nil && blots < BLOTS_Max)
    {
        pattract->pchzReshow = pchz;
        pattract->fReshow = 1;

        // Manually invoke the SetBlotBlots virtual function
        pattract->pvtblot->pfnSetBlotBlots(pattract, BLOTS_Appearing);
    }
}

void SetAttractBlots(ATTRACT* pattract, BLOTS blots)
{
    if (pattract->pchzReshow && blots == BLOTS_Hidden)
    {
        pattract->pchzReshow = nullptr;
        UpdateAttractText(pattract);
        blots = BLOTS_Appearing;
    }

    SetBlotBlots(pattract, blots);
}

void UpdateAttract(ATTRACT* pattract)
{
    if (g_ui.uisPlaying == UIS_Attract)
    {
        const int fJoyValid = g_joy.joys == JOYS_Ready;
        const int fReshow = g_saveData.pgsAttractSave && g_saveData.pgsAttractSave->dt != 0.0f;

        if (pattract->fJoyValid != fJoyValid || pattract->fReshow != fReshow)
        {
            pattract->fJoyValid = fJoyValid;
            pattract->fReshow = fReshow;
            UpdateAttractText(pattract);
        }
    }

    UpdateBlot(pattract);
}

void DrawAttract(ATTRACT* pattract)
{
    const float pulse = std::sin(g_clock.tReal * 3.0f) * 0.5f + 0.5f;
    const float u = std::min(pattract->uOn, g_logo.uOn);

    const GuiScale guiScale = GetGuiScale();
    const float scaleX = guiScale.x;
    const float scaleY = guiScale.y;
    const float dx = pattract->dx * scaleX;
    const float dy = pattract->dy * scaleY;

    float xOn = pattract->xOn;
    float yOn = pattract->yOn;

    if (pattract->pbloti != nullptr)
    {
        if (pattract->pbloti->x < 0.0f)
            xOn -= dx - pattract->dx;
        else if (pattract->pbloti->x == 0.0f)
            xOn -= (dx - pattract->dx) * 0.5f;

        if (pattract->pbloti->y < 0.0f)
            yOn -= dy - pattract->dy;
        else if (pattract->pbloti->y == 0.0f)
            yOn -= (dy - pattract->dy) * 0.5f;

        xOn += pattract->pbloti->x * (scaleX - 1.0f);
        yOn += pattract->pbloti->y * (scaleY - 1.0f);
    }

    float xOff = xOn;
    float yOff = yOn;

    if (pattract->pbloti != nullptr)
    {
        switch (pattract->pbloti->blote)
        {
        case BLOTE_Left:
            xOff = -dx;
            break;
        case BLOTE_Right:
            xOff = g_gl.width;
            break;
        case BLOTE_Top:
            yOff = -dy;
            break;
        case BLOTE_Bottom:
            yOff = g_gl.height;
            break;
        default:
            break;
        }
    }

    const float x = glm::mix(xOff, xOn, u);
    const float y = glm::mix(yOff, yOn, u);

    glm::vec4 color = pattract->rgba;
    color.a = pulse;

    CTextBox tbx;
    tbx.SetPos(x, y);
    tbx.SetSize(dx, dy);
    tbx.SetTextColor(&color);
    tbx.SetHorizontalJust(JH_Right);
    tbx.SetVerticalJust(JV_Top);

    if (pattract->pte && pattract->pte->m_pfont)
    {
        const float edgeScaleX = pattract->pte->m_rxScaling;
        const float edgeScaleY = pattract->pte->m_ryScaling;
        pattract->pte->m_rxScaling *= scaleX;
        pattract->pte->m_ryScaling *= scaleY;
        pattract->pte->m_pfont->EdgeRect(pattract->pte, &tbx);
        pattract->pte->m_rxScaling = edgeScaleX;
        pattract->pte->m_ryScaling = edgeScaleY;
    }

    pattract->pfont->PushScaling(
        pattract->rFontScale * scaleX,
        pattract->rFontScale * scaleY);

    CRichText richText(pattract->achzDraw, pattract->pfont);
    richText.Draw(&tbx, nullptr);

    pattract->pfont->PopScaling();
}

ATTRACT g_attract;
CTextEdge g_teAttract;
const char* g_aachzAttract[2]
{
    "Press START button to Play\n%s",
    "Press START button to Resume Game\n%s"
};
float RX_Attract = 0.69999999;
float RY_Attract = 0.6;

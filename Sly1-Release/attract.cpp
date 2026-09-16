#include "attract.h"
#include "logo.h"
#include "save.h"
#include "ui.h"

void StartupAttract(ATTRACT* pattract)
{
    pattract->pvtattract = &g_vtattract;

    g_teAttract.m_rgba = glm::vec4(0.0f, 75.0f / 255.0f, 125.0f / 255.0f, 1.0f);
    g_teAttract.m_ch = '-';
    g_teAttract.m_dxExtra = 2.0;
    g_teAttract.m_ryScaling = 0.3;
    g_teAttract.m_rxScaling = 0.3;
    g_teAttract.m_dyExtra = -4.0;
}

void PostAttractLoad(ATTRACT* pattract)
{
    PostBlotLoad(pattract);

    pattract->pfont = pattract->pfont->PfontClone(RX_Attract, RY_Attract);

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
        pattract->rgba = glm::vec4(111.0f / 255.0f, 31.0f / 255.0f, 31.0f / 255.0f, 1.0f);
        std::strcpy(pattract->achzDraw, "No Controller");
    }
    else
    {
        pattract->rgba = glm::vec4(111.0f / 255.0f, 111.0f / 255.0f, 111.0f / 255.0f, 1.0f);
        const int ichz = pattract->fReshow ? 1 : 0;
        std::snprintf(pattract->achzDraw, sizeof(pattract->achzDraw), g_aachzAttract[ichz], "Press SELECT button for Menu");
    }

    pattract->pfont->PushScaling(pattract->rFontScale, pattract->rFontScale);

    CRichText richText(pattract->achzDraw, pattract->pfont);

    const float width  = richText.DxMaxLine();
    const float height = richText.DyWrap(0.0);

    ResizeBlot(pattract, width, height);

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

    const float x = glm::mix(pattract->xOff, pattract->xOn, u);
    const float y = glm::mix(pattract->yOff, pattract->yOn, u);

    glm::vec4 color = pattract->rgba;
    color.a = pulse;

    CTextBox tbx;
    tbx.SetPos(x, y);
    tbx.SetSize(pattract->dx, pattract->dy);
    tbx.SetTextColor(&color);
    tbx.SetHorizontalJust(JH_Right);
    tbx.SetVerticalJust(JV_Top);

    if (pattract->pte && pattract->pte->m_pfont)
        pattract->pte->m_pfont->EdgeRect(pattract->pte, &tbx);

    pattract->pfont->PushScaling(pattract->rFontScale, pattract->rFontScale);

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

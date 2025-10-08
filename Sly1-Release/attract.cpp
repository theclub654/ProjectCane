#include "attract.h"

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
    // Initialize the base BLOT
    PostBlotLoad(pattract);

    // Clone the font with RX_Attract and RY_Attract scaling
    if (pattract->pfont)
    {
        pattract->pfont = pattract->pfont->PfontClone(RX_Attract, RY_Attract);
    }

    // Assign text edge font if screen counter font exists
    if (g_pfontScreenCounters)
    {
        pattract->pte = &g_teAttract;
        g_teAttract.m_pfont = &g_afontBrx[2];
    }

    // Check if joypad is valid
    //pattract->fJoyValid = (g_joy.joys == JOYS_Ready);

    // Select default attract string based on whether the joypad is ready
    //int index = pattract->fJoyValid ? 0 : 4;

    // Set the draw string using virtual function
    if (pattract->pvtblot && pattract->pvtblot->pfnSetBlotAchzDraw)
    {
        const char* text = g_aachzAttract[0];
        pattract->pvtblot->pfnSetBlotAchzDraw(pattract, (char*)text);
    }
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
    if (pattract->fReshow && blots == BLOTS_Hidden)
    {
        pattract->fReshow = 0;

        // Restore previously hidden string
        SetBlotAchzDraw(pattract, pattract->pchzReshow);

        // Choose text color based on joypad presence
        glm::vec4 color;
        color = glm::vec4(1.0, 1.0f, 1.0, 1.0f);

        pattract->rgba = color;

        // Mark as reappearing
        blots = BLOTS_Appearing;
    }

    SetBlotBlots(pattract, blots);
}

void UpdateAttract(ATTRACT* pattract)
{
    //bool joyNowValid = (g_joy.joys == JOYS_Ready);

    //// Only update text if fJoyValid has changed
    //if (joyNowValid != pattract->fJoyValid)
    //{
    //    pattract->fJoyValid = joyNowValid;

    //    // Choose string index based on joypad state
    //    int index = joyNowValid ? 0 : 4;
    //    const char* text = g_aachzAttract[index];

    //    if (pattract->pvtblot &&
    //        pattract->pvtblot->pfnSetBlotAchzDraw)
    //    {
    //        pattract->pvtblot->pfnSetBlotAchzDraw(pattract, text);
    //    }
    //}

    UpdateBlot(pattract);
}

void DrawAttract(ATTRACT* pattract)
{
    // 1. Pulsing alpha using sine wave
    float t = g_clock.tReal * 3.0f;
    float alpha = (sinf(t) * 0.5f + 0.5f);  // 0.0 to 1.0

    // 2. Apply pulsing alpha to the color
    glm::vec4 color = glm::vec4(
        pattract->rgba.r,
        pattract->rgba.g,
        pattract->rgba.b,
        alpha
    );

    // Setup text box
    CTextBox tbx;
    tbx.SetPos(pattract->x, pattract->y);
    tbx.SetSize(pattract->dx, pattract->dy);
    tbx.SetTextColor(&color);
    tbx.SetHorizontalJust(JH_Right);
    tbx.SetVerticalJust(JV_Top);

    // Optionally draw edge box
    if (pattract->pte && pattract->pte->m_pfont)
        pattract->pte->m_pfont->EdgeRect(pattract->pte, &tbx);

    // Draw the attract text string
    if (pattract->pfont)
        pattract->pfont->DrawPchz(pattract->achzDraw, &tbx);
}

ATTRACT g_attract;
CTextEdge g_teAttract;
const char* g_aachzAttract[2]
{
    "Press Start to Play",
    "No Controller"
};
float RX_Attract = 0.69999999;
float RY_Attract = 0.6;
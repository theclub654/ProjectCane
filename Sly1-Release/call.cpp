#include "call.h"

void StartupCall(CALL* pcall)
{
    pcall->pvtcall = &g_vtcall;
}

void PostCallLoad(CALL* pcall)
{
    // Initialize base UI object
    PostBlotLoad(pcall);

    // Clone font at 1:1 scale
    pcall->pfont = &g_afontBrx[2];

    // Set default string (e.g. "B")
    SetBlotAchzDraw(pcall, (char*)"B");
}

void DrawCall(CALL* pcall)
{
    // 1. Draw background
    DrawBlot(pcall);
    
    // 2. Pulsing scale
    float pulse = cosf(g_clock.tReal * 8.0f);
    float scale = (pulse * 0.5f + 0.5f) * 0.1f + 0.5f;
    
    float textWidth  = g_pfontJoy->DxFromPchz((char*)"L");
    float textHeight = g_pfontJoy->m_dyUnscaled * g_pfontJoy->m_ryScale;

    // 5. Center it in the CALL box
    float cx = pcall->x + pcall->dx * 0.5f;
    float cy = pcall->y + pcall->dy * 0.5f;

    float x = cx - textWidth  + 55.0;
    float y = cy - textHeight + 65.0;

    // 6. Setup textbox
    CTextBox tbx;
    tbx.SetPos(x, y);
    tbx.SetSize(textWidth, textHeight);
    glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    tbx.SetTextColor(&color);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    // 7. Apply scale and draw
    g_pfontJoy->PushScaling(scale, scale);
    g_pfontJoy->DrawPchz((char*)"L", &tbx);
    g_pfontJoy->PopScaling();
}

CALL g_call;
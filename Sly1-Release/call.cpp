#include "call.h"
#include "gui_layout.h"
#include "jt.h"

void StartupCall(CALL* pcall)
{
    pcall->pvtcall = &g_vtcall;
}

void PostCallLoad(CALL* pcall)
{
    PostBlotLoad(pcall);

    CFontBrx* pfontBase = PfontFromFont(2);
    pcall->pfontOwned = pfontBase->PfontClone(1.0f, 1.0f);
    pcall->pfont = pcall->pfontOwned.get();

    pcall->pvtblot->pfnSetBlotAchzDraw(pcall, (char*)"B");
    pcall->pdialogTriggered = nullptr;
}

void UpdateCall(CALL* pcall)
{
    UpdateBlot(pcall);

    const bool fDialogAvailable = pcall->pdialogTriggered != nullptr;
    const bool fPlayerCanCall = g_pjt == nullptr || g_pjt->jts != 6;
    const bool fUiCanShowCall = g_ui.uis == UIS_Playing && g_ui.cpblotActive < 2;

    if (fDialogAvailable && fPlayerCanCall && fUiCanShowCall)
        pcall->pvtblot->pfnShowBlot(pcall);
    else
        pcall->pvtblot->pfnHideBlot(pcall);
}

void DrawCall(CALL* pcall)
{
    DrawBlot(pcall);

    const float scale = 0.55f + std::cos(RadNormalize(g_clock.tReal * 8.0f)) * 0.05f;

    CFontBrx* pfont = PfontFromFont(1);
    if (pfont == nullptr)
        return;

    // DrawBlot converts the CALL's main glyph from the retail 640 x 492.8
    // canvas into framebuffer pixels. The separately drawn L1 glyph must use
    // that same canvas conversion; on PS2 this happened globally in the GS
    // projection rather than in DrawCall itself.
    const GuiScale guiScale = GetGuiScale();
    float xCall = 0.0f;
    float yCall = 0.0f;
    float dxCall = 0.0f;
    float dyCall = 0.0f;
    GetGuiScaledBlotRect(pcall, &xCall, &yCall, &dxCall, &dyCall);

    pfont->PushScaling(scale * guiScale.x, scale * guiScale.y);

    const float dxText = pfont->DxFromPchz((char*)"L");
    const float dyText = static_cast<float>(pfont->m_dyUnscaled) * pfont->m_ryScale;

    CTextBox tbx;
    tbx.SetPos(
        xCall + 2.0f * guiScale.x + dxCall * 0.5f - dxText * 0.5f,
        yCall + 14.0f * guiScale.y + dyCall * 0.5f - dyText * 0.5f);
    tbx.SetSize(dxText, dyText);

    // Retail passes 0xFF808080. PS2 GS RGB channels use 0x80 as full
    // intensity, so this maps to white in the conventional 0..1 shader range.
    glm::vec4 color = glm::vec4(1.0f);

    tbx.SetTextColor(&color);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    pfont->DrawPchz((char*)"L", &tbx);
    pfont->PopScaling();
}

CALL g_call;

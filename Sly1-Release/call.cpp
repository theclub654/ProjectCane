#include "call.h"
#include "jt.h"

void StartupCall(CALL* pcall)
{
    pcall->pvtcall = &g_vtcall;
}

void PostCallLoad(CALL* pcall)
{
    PostBlotLoad(pcall);

    CFontBrx* pfontBase = PfontFromFont(2);
    pcall->pfont = pfontBase->PfontClone(1.0f, 1.0f);

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
    pfont->PushScaling(scale, scale);

    const float dxText = pfont->DxFromPchz((char*)"L");
    const float dyText = static_cast<float>(pfont->m_dyUnscaled) * pfont->m_ryScale;

    CTextBox tbx;
    tbx.SetPos(pcall->x + 2.0f + pcall->dx * 0.5f - dxText * 0.5f, pcall->y + 14.0f + pcall->dy * 0.5f - dyText * 0.5f);
    tbx.SetSize(dxText, dyText);

    glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    tbx.SetTextColor(&color);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    pfont->DrawPchz((char*)"L", &tbx);
    pfont->PopScaling();
}

CALL g_call;

#include "ctr.h"
#include "scores.h"
#include "van.h"
#include "suv.h"

void PostCtrLoad(CTR* pctr)
{
    PostBlotLoad(pctr);

    pctr->pfont = PfontFromFont(2);
    pctr->nDisplay = -1;
    pctr->dgDisplayMax = 6000.0f;

    switch (static_cast<int>(pctr->blotk))
    {
        case 3:
        pctr->pnActual = &g_pgsCur->clife;
        break;

        case 4:
        pctr->pnActual = &g_plsCur->abitClue.m_cbitSet;
        pctr->pnTotal = &g_psw->cclueAll;
        break;

        case 5:
        pctr->pnActual = &g_pwsCur->ckey;
        break;

        case 6:
        pctr->pnActual = &g_pgsCur->ccoin;
        break;

        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        pctr->pnActual = &pctr->nDisplay;
        break;

        case BLOTK_VanComputer:
        pctr->pnActual = &g_cCpuCollected;
        break;

        case BLOTK_Scores:
        pctr->pnActual = &g_scores.cCpuCollected;
        break;

        case BLOTK_Percent:
        pctr->pnActual = &pctr->nDisplay;
        break;

        default:
        pctr->pnActual = nullptr;
        break;
    }
}

float DtVisibleCtr(CTR* pctr)
{
    float dt = 0.0;
    if (g_clock.fEnabled != 0)
        dt = 2.5;

    return dt;
}

void UpdateCtr(CTR* pctr)
{
    UpdateBlot(pctr);

    if (pctr->pnActual == nullptr)
        return;

    const int display = pctr->nDisplay;
    const int actual = *pctr->pnActual;

    if (display == actual)
        return;

    if (display < 0)
        pctr->nDisplay = actual;
    else
    {
        const float updateAmount = g_clock.dt * pctr->dgDisplayMax + pctr->uRemain;
        const int delta = static_cast<int>(updateAmount);

        pctr->uRemain = updateAmount - static_cast<float>(delta);

        const int target = *pctr->pnActual;

        if (target < display)
            pctr->nDisplay = std::max(display - delta, target);
        else
            pctr->nDisplay = std::min(display + delta, target);
    }

    if (pctr->nDisplay == *pctr->pnActual && pctr->pfnsmack != nullptr)
    {
        pctr->pfnsmack(pctr->pv);
        pctr->pfnsmack = nullptr;
        pctr->pv = nullptr;
    }

    RebuildCtrAchzDraw(pctr);

    if (pctr->achzDraw[0] == '\0')
    {
        ResizeBlot(pctr, 0.0f, 0.0f);
        return;
    }

    pctr->pfont->PushScaling(pctr->rFontScale, pctr->rFontScale);

    CRichText rtxt(pctr->achzDraw, pctr->pfont);

    const float dx = rtxt.Dx();
    const float dy = pctr->pfont->m_dyUnscaled * pctr->pfont->m_ryScale;

    ResizeBlot(pctr, dx, dy);
    pctr->pfont->PopScaling();
}

void RebuildCtrAchzDraw(CTR* pctr)
{
    switch (static_cast<int>(pctr->blotk))
    {
        case 3:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%c%d", 'L', pctr->nDisplay);
        break;

        case 4:
        {
            const int total = *pctr->pnTotal;

            if (total == 0)
            {
                pctr->achzDraw[0] = '\0';
                break;
            }

            const char suffix = pctr->nDisplay >= total ? '!' : '?';
            std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%d/%d%c", pctr->nDisplay, total, suffix);
            break;
        }

        case 5:
        if (static_cast<unsigned int>(g_pgsCur->gameWorldCur + GAMEWORLD_Nil) < 4)
            std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%d%c", pctr->nDisplay, 'K');
        else
            pctr->achzDraw[0] = '\0';
        break;

        case 6:
        if (g_psw->fDprizeEnabled)
            std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%c%d", 'C', pctr->nDisplay);
        else
            pctr->achzDraw[0] = '\0';
        break;

        case 7:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%d/%d%c", pctr->nDisplay, *pctr->pnTotal, 'V');
        break;

        case 8:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%d/%d%c", pctr->nDisplay, *pctr->pnTotal, 'N');
        break;

        case 9:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%c%d/%d", 'F', pctr->nDisplay, *pctr->pnTotal);
        break;

        case 10:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%c%d", 'N', pctr->nDisplay);
        break;

        case 11:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%d/%d%c", pctr->nDisplay, *pctr->pnTotal, 'V');
        break;

        case 12:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%d/%d%c", pctr->nDisplay, *pctr->pnTotal, 'd');
        break;

        case 13:
        {
            const int count = pctr->nDisplay;
            const char slot0 = count >= 1 ? 'b' : 'm';
            const char slot1 = count >= 2 ? 'b' : 'm';
            const char slot2 = count >= 3 ? 'b' : 'm';
            const char slot3 = count >= 4 ? 'b' : 'm';
            const char slot4 = count >= 5 ? 'b' : 'm';

            std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%c%c%c%c%c%c",
                's', slot0, slot1, slot2, slot3, slot4);
            break;
        }

        case 14:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "rstuv");
        break;

        case 15:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%c%d/5", 'F', pctr->nDisplay);
        break;

        case BLOTK_VanComputer:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%d/%d%c", pctr->nDisplay, N_SuvChaseWin, 'V');
        break;

        case BLOTK_Scores:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%c%d/%d", 'N', pctr->nDisplay, N_SuvChaseWin);
        break;

        case BLOTK_Percent:
        std::snprintf(pctr->achzDraw, sizeof(pctr->achzDraw), "%d%%", pctr->nDisplay);
        break;
    }

    FormatBlotRichText(pctr);
}

void DrawCtr(CTR* pctr)
{
    if (pctr->achzDraw[0] == '\0')
        return;

    float alpha = 1.0f;

    if (g_prompt.blots != BLOTS_Hidden)
        alpha = g_prompt.alpha;

    glm::vec4 rgba = pctr->rgba;
    rgba.a = pctr->rgba.a * alpha;

    CTextBox tbx;

    tbx.SetPos(pctr->x, pctr->y);
    tbx.SetSize(pctr->dx, pctr->dy);
    tbx.SetTextColor(&rgba);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    pctr->pfont->PushScaling(pctr->rFontScale, pctr->rFontScale);

    CRichText rtxt(pctr->achzDraw, pctr->pfont);
    rtxt.Draw(&tbx, nullptr);

    pctr->pfont->PopScaling();
}

void StartupPercentCtr(PERCENTCTR* ppercentctr)
{
    ppercentctr->pvtctr = &g_vtctr;
}

PERCENTCTR g_percentctr;

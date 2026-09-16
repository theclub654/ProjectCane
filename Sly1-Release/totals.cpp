#include "totals.h"
#include "binoc.h"
#include "wm.h"
#include "render.h"

void StartupTotals(TOTALS* ptotals)
{
    ptotals->pvttotals = &g_vttotals;
}

void PostTotalsLoad(TOTALS* ptotals)
{
    PostBlotLoad(ptotals);

    CFontBrx* pfontBase = PfontFromFont(0);
    ptotals->pfont = pfontBase->PfontClone(1.0f, 1.0f);

    ptotals->rFontScale = 0.8f;
    ptotals->dtDisappear = 0.5f;

    if (FFontLoaded(2))
    {
        ptotals->pte = &g_teTotals;
        ptotals->pte->m_pfont = PfontFromFont(2);
    }
}

void SetTotalsAchzDraw(TOTALS* ptotals, char* pchz)
{
    if (ptotals->blots == BLOTS_Hidden)
    {
        SetBlotAchzDraw(ptotals, pchz);
        return;
    }

    if (ptotals->blots > BLOTS_Nil && ptotals->blots < BLOTS_Max &&
        (ptotals->fReshow || pchz == nullptr || std::strcmp(ptotals->achzDraw, pchz) != 0))
    {
        ptotals->fReshow = 1;

        if (pchz != nullptr)
            std::strcpy(ptotals->achzTotals, pchz);
        else
            ptotals->achzTotals[0] = '\0';

        ptotals->pvtblot->pfnSetBlotBlots(ptotals, BLOTS_Disappearing);
    }
}

void FormatTotalsTasks(char* pchzText, LS* pls, uint32_t grfTasks)
{
    char* pchzWrite = pchzText;
    pchzText[0] = '\0';

    if ((grfTasks & FLS_KeyCollected) != 0)
    {
        const char* pchzStatus = (pls->grfls & FLS_KeyCollected) != 0
            ? "k"
            : "l";

        pchzWrite += std::sprintf(pchzWrite, "&3%s&.", pchzStatus);
    }

    if ((grfTasks & FLS_Secondary) != 0)
    {
        if ((grfTasks & FLS_KeyCollected) != 0)
            *pchzWrite++ = ' ';

        const char* pchzStatus = (pls->grfls & FLS_Secondary) != 0
            ? "v"
            : "w";

        pchzWrite += std::sprintf(pchzWrite, "&3%s&.", pchzStatus);
    }

    if ((grfTasks & FLS_Tertiary) != 0)
    {
        if ((grfTasks & (FLS_KeyCollected | FLS_Secondary)) != 0)
            *pchzWrite++ = ' ';

        const bool fComplete = (pls->grfls & FLS_Tertiary) != 0;
        const char* pchzStatus = fComplete
            ? "h"
            : "i";

        pchzWrite += std::sprintf(pchzWrite, "&1%s", pchzStatus);

        if (fComplete)
        {
            const float dtBest = pls->dtTimedBest;

            if (dtBest >= 1.0f)
                std::sprintf(pchzWrite, "^10~b2a83d%d:%02d~.^.", static_cast<int>(dtBest) / 60, static_cast<int>(dtBest) % 60);
            else
                std::sprintf(pchzWrite, "^10~b2a83d0.%d~.^.", static_cast<int>(dtBest * 10.0f));
        }

        std::strcat(pchzWrite, "&.");
    }
}

void SetTotalsDestinationText(TOTALS* ptotals, int levelId)
{
    if (levelId == -1)
        ptotals->pvtblot->pfnSetBlotAchzDraw(ptotals, (char*)"< unknown > ");
    else
        SetTotalsLevelText(ptotals, levelId >> 8, levelId & 0xFF);
}

void SetTotalsLevelText(TOTALS* ptotals, int worldId, int levelIndex)
{
    const int levelId = (worldId << 8) | levelIndex;
    const int tasks = TasksFromLevelID(levelId);
    LS* pls = &g_pgsCur->aws[worldId].als[levelIndex];

    char achzTasks[64] = {};
    char achzText[64] = {};

    FormatTotalsTasks(achzTasks, pls, tasks);

    const char* pchzLevelName = PchzFriendlyFromLevelId(levelId);

    if (achzTasks[0] == '\0')
        std::strcpy(achzText, pchzLevelName);
    else
        std::snprintf(achzText, sizeof(achzText), "%s%s%s", pchzLevelName, " ", achzTasks);

    ptotals->pvtblot->pfnSetBlotAchzDraw(ptotals, achzText);
}

void SetTotalsWorldText(TOTALS* ptotals, int worldId)
{
    int ckeyTotal = 0;
    int cvaultTotal = 0;
    int ctimedTotal = 0;

    TallyWorldTaskTotals(static_cast<GAMEWORLD>(worldId), &ckeyTotal, &cvaultTotal, &ctimedTotal);

    char achzText[512]{};

    std::snprintf(achzText, sizeof(achzText), "%s", s_apchzWorldNames[worldId]);

    char* pchzWrite = achzText + std::strlen(achzText);
    const WS& ws = g_pgsCur->aws[worldId];

    if (ckeyTotal != 0)
        pchzWrite += std::sprintf(pchzWrite, s_pchzTotalsKeysFormat, ws.ckey, ckeyTotal);

    if (cvaultTotal != 0)
    {
        const char* separator = ckeyTotal == 0 ? "" : " ";
        pchzWrite += std::sprintf(pchzWrite, s_pchzTotalsVaultsFormat, separator, ws.cvault, cvaultTotal);
    }

    if (ctimedTotal != 0)
    {
        const char* separator = ckeyTotal == 0 && cvaultTotal == 0 ? "" : " ";
        std::sprintf(pchzWrite, s_pchzTotalsTimedFormat, separator, ws.ctimed, ctimedTotal);
    }

    //std::printf("TOTALS TEXT: \"%s\"\n", achzText);
    ptotals->pvtblot->pfnSetBlotAchzDraw(ptotals, achzText);
}

void SetTotalsBlots(TOTALS* ptotals, BLOTS blots)
{
    if (ptotals->fReshow && blots == BLOTS_Hidden)
    {
        ptotals->fReshow = 0;
        SetBlotAchzDraw(ptotals, ptotals->achzTotals);
        blots = BLOTS_Appearing;
    }

    if (blots == BLOTS_Hidden)
        ptotals->fDrawOverLetterbox = 0;

    SetBlotBlots(ptotals, blots);
}

void ShowTotals(TOTALS* ptotals)
{
    if ((ptotals->blots != BLOTS_Disappearing) || (ptotals->fReshow == 0))
        ShowBlot(ptotals);
}

void HideTotals(TOTALS* ptotals)
{
    if ((ptotals->blots == BLOTS_Disappearing) && (ptotals->fReshow != 0))
        ptotals->fReshow = 0;
    else
        HideBlot(ptotals);
}

void DrawTotals(TOTALS* ptotals)
{
    if (ptotals->achzDraw[0] == '\0')
        return;

    if (g_binoc.blots != BLOTS_Hidden)
    {
        const int binocs = static_cast<int>(g_binoc.binocs);
        const int binocsNil = static_cast<int>(BINOCS_Nil);

        if (static_cast<uint32_t>(binocs + binocsNil) < 2)
            return;
    }

    float opacity;
    float yOffset;

    if (ptotals->fDrawOverLetterbox)
    {
        opacity = 1.0f;
        yOffset = (1.0f - g_letterbox.uOn) * 66.40001f;
    }
    else
    {
        opacity = ptotals->uOn;
        yOffset = 0.0f;
    }

    glm::vec4 textColor = ptotals->rgba;
    textColor.a *= opacity;

    CTextBox tbx;
    tbx.SetPos(ptotals->xOn, ptotals->yOn + yOffset);
    tbx.SetSize(ptotals->dx, ptotals->dy);
    tbx.SetTextColor(&textColor);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    glDisable(GL_DEPTH_TEST);
    if (!ptotals->fDrawOverLetterbox)
    {
        float backgroundOpacity = opacity;

        if (g_wmc.fActive && g_wmc.pwmCurrent != nullptr && g_wmc.pwmCurrent->wms == WMS_Manual)
            backgroundOpacity = 1.0f;

        const float yTopLogical = ((yOffset + 492.80002f) - 66.40001f) * 0.45454547f;
        const float yTopPixels = (yTopLogical / 224.0f) * static_cast<float>(g_gl.height);
        const int alpha = static_cast<int>(backgroundOpacity * 64.0f);

        FillScreenRect(0, 0, 0, alpha, 0.0f, yTopPixels, static_cast<float>(g_gl.width), static_cast<float>(g_gl.height));
    }

    ptotals->pfont->PushScaling(ptotals->rFontScale, ptotals->rFontScale);

    CRichText rt(ptotals->achzDraw, ptotals->pfont);
    rt.Draw(&tbx, nullptr);

    ptotals->pfont->PopScaling();
    glEnable(GL_DEPTH_TEST);
}

TOTALS g_totals;
CTextEdge g_teTotals;
int g_fTotalsAvailable = 0;

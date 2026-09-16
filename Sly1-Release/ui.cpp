#include "ui.h"
#include "binoc.h"
#include "wm.h"
#include "logo.h"
#include "attract.h"
#include "coin.h"
#include "pzo.h"
#include "totals.h"

void StartupUi()
{
	InitUi(&g_ui);
}

void InitUi(UI *pui)
{
	pui->uis = UIS_Nil;
	pui->uisPlaying = UIS_Nil;
	SetUiUis(pui, UIS_Playing);
	pui->apblotActive[0] = nullptr;
	pui->cpblotActive = 1;
}

void PostUiLoad(UI* pui)
{
	SetUiUis(pui, UIS_Playing);
}

void SetUiUis(UI* pui, int uis)
{
    if (!pui)
        return;

    int uisResolved = uis;
    if (uis == UIS_Playing && pui->uisPlaying != UIS_Nil)
        uisResolved = pui->uisPlaying;

    if (pui->uis == uisResolved)
        return;

    // Raw value 1 is UIS_Attract.
    if (pui->uis == 1)
    {
        if (g_logo.pvtblot->pfnHideBlot)
            g_logo.pvtblot->pfnHideBlot(&g_logo);

        if (g_attract.pvtblot->pfnHideBlot)
            g_attract.pvtblot->pfnHideBlot(&g_attract);

        DecrementSwHandsOff(g_psw);
    }

    pui->uis = static_cast<UIS>(uisResolved);
    pui->tUis = g_clock.tReal;

    switch (uisResolved)
    {
        case 0: // UIS_Splash
        if (g_saveData.state == SAVE_MENU_STATE_Nil)
            SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Selecting);

        SetWipeWipes(&g_wipe, WIPES_Idle);
        SetUiUPause(pui, 1.0f);
        break;

        case 1: // UIS_Attract
        SetUiUPause(pui, 1.0f);

        if (g_logo.pvtblot->pfnShowBlot)
            g_logo.pvtblot->pfnShowBlot(&g_logo);

        if (g_attract.pvtblot->pfnShowBlot)
            g_attract.pvtblot->pfnShowBlot(&g_attract);

        IncrementSwHandsOff(g_psw);
        EnableVibration();
        break;

        case 2:
        SetUiUPause(pui, 1.0f);
        break;

        case 3: // UIS_Playing
        SetUiUPause(pui, 1.0f);
        g_iexcHyst = -100;
        break;

        case 5:
        case 7:
        SetUiUPause(pui, 0.0f);
        break;

        case 8:
        if (g_lifectr.pvtblot->pfnHideBlot)
            g_lifectr.pvtblot->pfnHideBlot(&g_lifectr);

        if (g_cluectr.pvtblot->pfnHideBlot)
            g_cluectr.pvtblot->pfnHideBlot(&g_cluectr);

        if (g_keyctr.pvtblot->pfnHideBlot)
            g_keyctr.pvtblot->pfnHideBlot(&g_keyctr);

        if (g_coinctr.pvtblot->pfnHideBlot)
            g_coinctr.pvtblot->pfnHideBlot(&g_coinctr);

        break;

        default:
        break;
    }
}

void SetUiUPause(UI* pui, float uPause)
{
    uPause = glm::clamp(uPause, 0.0f, 1.0f);

    if (g_clock.t > 0.0f)
    {
        const float volume = glm::mix(0.6f, 1.0f, uPause);

        SetMvgkRvol(volume, 4, 1);
        SetMvgkRvol(volume, 4, 0);
    }

    SetClockRate(uPause);
}

bool FCanShowGameplayBlot()
{
    if (g_wipe.wipes != WIPES_Idle)
        return false;

    if (FSwHandsOff(g_psw))
        return false;

    if (g_prompt.fActive && g_prompt.prk != 0)
        return false;

    if (g_wmc.fActive || g_wmc.blots != BLOTS_Hidden)
        return false;

    if (g_binoc.binocs == BINOCS_Dialog)
        return false;

    if (g_binoc.binocs == BINOCS_Peek)
        return false;

    if (g_binoc.binocs == BINOCS_Confront)
        return false;

    return true;
}

bool FCanDrawBinoc()
{
    return g_wipe.wipes == 0 &&
        !g_prompt.fActive &&
        g_prompt.blots == BLOTS_Hidden &&
        !g_wmc.fActive &&
        g_wmc.blots == BLOTS_Hidden;
}

bool FCanDrawTv()
{
    return g_wipe.wipes == WIPES_Idle &&
        !g_prompt.fActive &&
        g_prompt.blots == BLOTS_Hidden &&
        !g_wmc.fActive &&
        g_wmc.blots == BLOTS_Hidden &&
        g_binoc.binocs != BINOCS_Confront &&
        g_binoc.binocs != BINOCS_Peek;
}

bool FCanRenderTv()
{
    if ((((g_wipe.wipes == 0) && (g_prompt.fActive == 0)) && (g_prompt.blots == BLOTS_Hidden)) &&
        ((g_wmc.fActive == 0 && (g_wmc.blots == 0)))) {
        return 1;
    }

    return 0;
}

void PushUiActiveBlot(UI* pui, BLOT* pblot)
{
    if (!pui || !pblot)
        return;

    if (FPlayingCommentaryAudio() && pui->cpblotActive == 1)
    {
        PauseVag();
    }

    if (pui->cpblotActive > 1)
    {
        BLOT* pblotCurrent = pui->apblotActive[pui->cpblotActive - 1];

        if (pblotCurrent && pblotCurrent->pvtblot->pfnOnBlotActive)
            pblotCurrent->pvtblot->pfnOnBlotActive(pblotCurrent, false);
    }

    if (pblot->pvtblot->pfnOnBlotActive)
        pblot->pvtblot->pfnOnBlotActive(pblot, true);

    pui->apblotActive[pui->cpblotActive++] = pblot;

    if (pblot->pvtblot->pfnOnBlotPush)
        pblot->pvtblot->pfnOnBlotPush(pblot);

}

void PopUiActiveBlot(UI* pui)
{
    if (!pui || pui->cpblotActive <= 1)
        return;

    BLOT* pblot = pui->apblotActive[pui->cpblotActive - 1];
    --pui->cpblotActive;

    if (!pblot)
        return;

    if (pblot->pvtblot->pfnOnBlotActive)
        pblot->pvtblot->pfnOnBlotActive(pblot, false);

    if (pblot->pvtblot->pfnOnBlotPop)
        pblot->pvtblot->pfnOnBlotPop(pblot);

    if (pui->cpblotActive > 1)
    {
        BLOT* pblotPrevious = pui->apblotActive[pui->cpblotActive - 1];

        if (pblotPrevious && pblotPrevious->pvtblot->pfnOnBlotActive)
            pblotPrevious->pvtblot->pfnOnBlotActive(pblotPrevious, true);
    }

    if (pui->cpblotActive == 1 && FPlayingCommentaryAudio())
    {
        ContinueVag();
    }
}

int FDebugmenuActive(UI* pui)
{
    return (int)((DEBUGMENU *)pui->apblotActive[pui->cpblotActive + -1] == &g_debugmenu);
}

void UpdateUi(UI* pui)
{
    if (pui == nullptr)
        return;

    UIS uis = pui->uis;
    JOY* pjoy = (g_grfjoyt & 4U) != 0 ? &g_joy : &g_joyZero;

    const float dtUis = g_clock.tReal - pui->tUis;
    bool fChangeStateWhenBlotStackEmpty = false;

    if ((g_grfusr & 128U) != 0 && g_clock.t > 0.1f)
        RemoveGrfusr(128);

    switch (pui->uis)
    {
    case UIS_Splash:
    {
        const bool fSaveManagerFinished =
            g_saveData.state == SAVE_MENU_STATE_Closing ||
            g_saveData.state == SAVE_MENU_STATE_Selecting;

        if (g_prompt.blots == BLOTS_Hidden && fSaveManagerFinished)
            TriggerExit(PexitDefault());

        break;
    }

    case UIS_Attract:
    {
        if (pjoy->IsPressed(BTN_START))
        {
            pjoy->SetHandled(BTN_START);
            g_saveData.pgsCurrentSave = g_saveData.pgsAttractSave;

            if (g_saveData.pgsAttractSave != nullptr)
                LoadCurrentSave(&g_saveData);
            else
                InitGameState(g_pgsCur);

            ReloadCurrentLevel();
            uis = pui->uis;
        }
        else if (pjoy->IsPressed(BTN_SELECT))
        {
            pjoy->SetHandled(BTN_SELECT);
            SetPrompt(&g_prompt, PRP_Basic, PRK_OptionsMenu);
            uis = pui->uis;
        }

        break;
    }

    case UIS_Hub:
    case UIS_Paused:
    case UIS_WorldMap:
        fChangeStateWhenBlotStackEmpty = true;
        break;

    case UIS_Playing:
    {
        if ((g_grfusr & 2U) != 0 || g_wipe.wipes != WIPES_Idle)
            break;

        const bool fPausePressed = pjoy->IsPressed(BTN_START);
        const bool fControllerMissing = pjoy->joys == JOYS_Searching && g_clock.t >= 5.0f;

        if (fPausePressed || fControllerMissing)
        {
            SetPrompt(&g_prompt, PRP_Basic, PRK_PauseMenu);
            uis = pui->uis;

            if (fPausePressed)
                pjoy->SetHandled(BTN_START);

            break;
        }

        if (pjoy->IsPressed(BTN_SELECT) && g_wmc.pwmCurrent != nullptr && g_wmc.pwmCurrent->wms == WMS_Hidden)
        {
            PushUiActiveBlot(pui, &g_wmc);
            uis = pui->uis;
            pjoy->SetHandled(BTN_SELECT);
        }

        break;
    }

    case UIS_Pausing:
        if (dtUis <= 0.2f)
            SetUiUPause(pui, (0.2f - dtUis) * 5.0f);
        else
            uis = UIS_Paused;
        break;

    case UIS_Unpausing:
        if (dtUis <= 0.5f)
            SetUiUPause(pui, dtUis * 2.0f);
        else
            uis = UIS_Playing;
        break;

    default:
        break;
    }

    SetUiUis(pui, uis);

    BLOT* pblotActive = nullptr;

    if (pui->cpblotActive > 0)
        pblotActive = pui->apblotActive[pui->cpblotActive - 1];

    if (pblotActive != nullptr)
    {
        if (pblotActive->pvtblot->pfnUpdateBlotActive != nullptr)
            pblotActive->pvtblot->pfnUpdateBlotActive(pblotActive, pjoy);
    }
    else if (fChangeStateWhenBlotStackEmpty)
    {
        SetUiUis(pui, UIS_Playing);
    }

    UpdateBlots();
}

void DrawUiSelf(UI* pui)
{
    if (pui->uis == UIS_Splash) {
        DrawUiSplash(pui);
    }
}

void DrawUiSplash(UI* pui)
{
    constexpr float kVirtualWidth = 640.0f;
    constexpr float kVirtualHeight = 480.0f;

    float scaleX = g_gl.width / kVirtualWidth;
    float scaleY = g_gl.height / kVirtualHeight;

    float timeSinceSplash = g_clock.tReal - pui->tUis;

    BLOT* ptitle = (BLOT*)&g_title;
    if ((timeSinceSplash >= 6.0f) && (timeSinceSplash - g_clock.dtReal < 6.0f)) {
        ptitle->pvtblot->pfnSetBlotAchzDraw(ptitle, (char*)suckerPunchProd);
        SetBlotDtAppear(ptitle, 1.0f);
        SetBlotDtVisible(ptitle, 0.0f);
        ptitle->pvtblot->pfnShowBlot(ptitle);
    }

    CFontBrx* font = g_pfont;
    if ((timeSinceSplash <= 4.0f) && font) {
        float fadeAlpha = 0.0f;

        if (timeSinceSplash < 0.5f)
            fadeAlpha = timeSinceSplash * 2.0f;
        else if (timeSinceSplash < 3.5f)
            fadeAlpha = 1.0f;
        else if (timeSinceSplash < 4.0f)
            fadeAlpha = 1.0f - ((timeSinceSplash - 3.5f) * 2.0f);

        glm::vec4 fadeColor(0.5f, 0.5f, 0.5f, fadeAlpha);

        if (((int)(fadeAlpha * 255.0f)) & 0xFF) {
            CTextBox tbx;

            // LEGAL TEXT
            font->PushScaling(RX_UiLegal * scaleX, RY_UiLegal * scaleY);

            float lineHeight = font->m_dyUnscaled * font->m_ryScale * scaleY;
            float legalStartY = 22.0f * scaleY;

            for (int i = 0; i < 2; ++i) {
                const char* textLine = g_aachzLegal[i];

                tbx.SetPos(0.0f, legalStartY);
                tbx.SetSize(g_gl.width, lineHeight);
                tbx.SetTextColor(&fadeColor);
                tbx.SetHorizontalJust(JH_Center);
                tbx.SetVerticalJust(JV_Top);

                font->DrawPchz((char*)textLine, &tbx);
                legalStartY += lineHeight;
            }

            font->PopScaling();

            // PRESENTS TEXT
            font->PushScaling(R_UiPresents * scaleX, R_UiPresents * scaleY);

            float presentsLineHeight = font->m_dyUnscaled * font->m_ryScale * scaleY;

            float totalTextHeight = lineHeight + presentsLineHeight * 2.0f + 44.0f * scaleY;
            float presentsStartY = (g_gl.height - totalTextHeight) * 0.5f + 22.0f * scaleY;

            for (int i = 0; i < 2; ++i) {
                const char* textLine = g_aachzPresents[i];

                tbx.SetPos(0.0f, presentsStartY);
                tbx.SetSize(g_gl.width, presentsLineHeight);
                tbx.SetTextColor(&fadeColor);
                tbx.SetHorizontalJust(JH_Center);
                tbx.SetVerticalJust(JV_Top);

                font->DrawPchz((char*)textLine, &tbx);
                presentsStartY += presentsLineHeight;
            }

            font->PopScaling();
        }
    }
}

void DrawUi(UI* pui)
{
    glBlotShader.Use();
    glClearStencil(0);
    glStencilMask(0xFF);

    //DrawUiSelf(pui);
    DrawBlots();
}

void RenderUi(UI* pui)
{
    RenderBlots();
}

void ResetUi(UI* pui)
{
    int iVar1;

    iVar1 = pui->cpblotActive;
    while (1 < iVar1) {
        PopUiActiveBlot(pui);
        iVar1 = pui->cpblotActive;
    }
    pui->apblotActive[0] = nullptr;
    pui->uisPlaying = UIS_Nil;
    pui->cpblotActive = 1;

    ResetBlots();

    glDeleteVertexArrays(1, &g_binoc.backGroundBinocVAO);
    glDeleteBuffers(1, &g_binoc.backGroundBinocVBO);
    glDeleteBuffers(1, &g_binoc.backGroundBinocEBO);

    glDeleteVertexArrays(1, &g_binoc.triangleBinocVAO);
    glDeleteBuffers(1, &g_binoc.triangleBinocVBO);
    glDeleteBuffers(1, &g_binoc.triangleBinocEBO);

    glDeleteVertexArrays(1, &g_binoc.outlineVAO);
    glDeleteBuffers(1, &g_binoc.outlineVBO);
    glDeleteBuffers(1, &g_binoc.outlineEBO);
}

UI g_ui;
const char *suckerPunchProd = "A Sucker Punch Production";
float RX_UiLegal = 0.6;
float RY_UiLegal = 0.5;
float R_UiPresents = 1.0;
const char* g_aachzLegal[] = {"Developed by Sucker Punch Productions, LLC.", " 2002 Sony Computer Entertainment America Inc."};
const char* g_aachzPresents[] = {"Sony Computer Entertainment America", "Presents"};

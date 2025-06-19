#include "ui.h"
#include "binoc.h"

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

void SetUiUis(UI* pui, UIS uis)
{
    UIS uis_00 = uis;
    if ((uis == UIS_Playing) && (uis_00 = pui->uisPlaying, pui->uisPlaying == UIS_Nil)) {
        uis_00 = uis;
    }

    if (pui->uis != uis_00) {
        if (pui->uis == UIS_Attract) {
            //(*(code*)(g_logo.field0_0x0.pvtblot)->pfnHideBlot)(0x287c98);
            //(*(code*)(g_attract.field0_0x0.pvtblot)->pfnHideBlot)(0x287f00);
            //DecrementSwHandsOff(g_psw);
            pui->uis = uis_00;
        }
        else {
            pui->uis = uis_00;
        }
        pui->tUis = g_clock.tReal;
        //OnGameEnterUis(&g_game, uis_00);
        switch (pui->uis) 
        {
            case UIS_Splash:
            g_ui.uisPlaying = UIS_Splash;
            break;
            case UIS_Attract:
            g_ui.uisPlaying = UIS_Attract;
            SetUiUPause(pui, 1.0);
            //(*(code*)(g_logo.field0_0x0.pvtblot)->pfnShowBlot)(0x287c98);
            //(*(code*)(g_attract.field0_0x0.pvtblot)->pfnShowBlot)(0x287f00);
            //IncrementSwHandsOff(g_psw);
            break;
            case UIS_Playing:
            SetUiUPause(pui, 1.0);
            //pui->rvolMusicPrepause = RvolGetMvgk(MVGK_Music);
            //pui->rvolEffectsPrepause = RvolGetMvgk(MVGK_Effects);
            
            //g_iexcHyst = -100;
            break;
            case UIS_Paused:
            case UIS_WorldMap:
            SetUiUPause(pui, 0.0);
        }
    }
    return;
}

void SetUiUPause(UI* pui, float uPause)
{
	SetClockRate(uPause);
}

void PushUiActiveBlot(UI* pui, BLOT* pblot)
{
    // Deactivate currently active blot, if applicable
    if (pui->cpblotActive > 1) {
        BLOT* currentActive = pui->apblotActive[pui->cpblotActive - 1];
        if (auto* vt = currentActive->pvtblot) {
            if (vt->pfnOnBlotActive != nullptr) {
                vt->pfnOnBlotActive(currentActive, 0);
            }
        }
    }

    // Activate the new blot
    if (auto* vt = pblot->pvtblot) {
        if (vt->pfnOnBlotActive != nullptr) {
            vt->pfnOnBlotActive(pblot, 1);
        }
    }

    // Push the new blot to the active stack
    int i = pui->cpblotActive;
    pui->apblotActive[i] = pblot;
    pui->cpblotActive = i + 1;

    // Trigger OnBlotPush if defined
    if (auto* vt = pblot->pvtblot) {
        if (vt->pfnOnBlotPush) {
            //vt->pfnOnBlotPush(pblot);
        }
    }
}

void PopUiActiveBlot(UI* pui)
{
    int index = pui->cpblotActive - 1;
    pui->cpblotActive = index;

    BLOT* pblot = pui->apblotActive[index];
    auto* pvtblot = pblot->pvtblot;

    // Call OnBlotActive(0) for the blot being popped
    if (pvtblot->pfnOnBlotActive) {
        pvtblot->pfnOnBlotActive(pblot, 0);
    }

    // Call OnBlotPop if exists
    /*if (pvtblot->pfnOnBlotPop) {
        pvtblot->pfnOnBlotPop(pblot);
    }*/

    int nextIndex = pui->cpblotActive;

    // Call OnBlotActive(1) for the next active blot if there is one
    if (nextIndex > 1) {
        auto* nextBlot = pui->apblotActive[nextIndex - 1];
        auto* nextPvtblot = nextBlot->pvtblot;
        if (nextPvtblot->pfnOnBlotActive) {
            nextPvtblot->pfnOnBlotActive(nextBlot, 1);
        }
    }
}

int FDebugmenuActive(UI* pui)
{
    return (int)((DEBUGMENU *)pui->apblotActive[pui->cpblotActive + -1] == &g_debugmenu);
}

void UpdateUi(UI *pui)
{
    float deltaTime = g_clock.tReal - pui->tUis;

    // Choose joy instance based on g_grfjoyt flag (simulate original behavior)
    JOY* pjoy = &g_joy;

    UIS currentUis = pui->uis;
    bool uiBlocked = false;

    // Toggle Debug Menu on "button 0x100" pressed (map this to a key/button you assigned, e.g. BTN_A)
    if (currentUis != UIS_WorldMap && pjoy->IsPressed(BTN_SELECT)) {
        if (!FDebugmenuActive(pui)) {
            PushUiActiveBlot(pui, (BLOT*)&g_debugmenu);
        }
        else {
            PopUiActiveBlot(pui);
        }
        // No explicit SetJoyBtnHandled needed, IsPressed resets every frame automatically
        currentUis = pui->uis; // refresh state after menu toggle
    }

    switch (currentUis) {
    case UIS_Splash:
    case UIS_Attract:
        // No action for these states currently
        break;

    case UIS_Playing:
        if (g_letterbox.blots == BLOTS_Hidden && g_binoc.blots == BLOTS_Hidden) {
            if (pjoy->IsPressed(BTN_START)) { // e.g. map 0x800 to BTN_START
                SetPrompt(&g_prompt, PRP_Basic, PRK_PauseMenu);
                currentUis = pui->uis;
            }

            //else if (pjoy->IsPressed(BTN_A)) { // map 0x100 to BTN_A
            //    //g_lifectr.pvtblot->pfnShowBlot(0x2828b8, currentUis);
            //    //g_cluectr.pvtblot->pfnShowBlot(0x282b38);
            //    //g_keyctr.pvtblot->pfnShowBlot(0x282db8);
            //    //g_coinctr.pvtblot->pfnShowBlot(0x2832b8);
            //}
        }
        break;

    case UIS_Pausing:
        if (deltaTime <= 0.2f) {
            SetUiUPause(pui, (0.2f - deltaTime) * 5.0f);
        }
        else {
            currentUis = UIS_Paused;
        }
        break;

    case UIS_Paused:
    case UIS_WorldMap:
        uiBlocked = true;
        SetUiUPause(pui, 0.0);
        break;

    case UIS_Unpausing:
        if (deltaTime > 0.5f) {
            currentUis = UIS_Playing;
        }
        else {
            SetUiUPause(pui, deltaTime * 2.0f);
        }
        break;

    default:
        break;
    }

    SetUiUis(pui, currentUis);

    // Update active blot
    BLOT* pActiveBlot = pui->apblotActive[pui->cpblotActive - 1];
    if (pActiveBlot != nullptr) {
        if (pActiveBlot->pvtblot->pfnUpdateBlotActive) {
            pActiveBlot->pvtblot->pfnUpdateBlotActive(pActiveBlot, pjoy);
        }
    }
    else if (uiBlocked) {
        SetUiUis(pui, UIS_Playing);
    }

    // Debug time toggle, e.g. map 0x200 to BTN_B or another key
    //if (!g_clock.fEnabled && pjoy->IsPressed(BTN_B)) {
    //    // No need to call SetJoyBtnHandled anymore
    //    // You can call the debug toggle function or your logic here
    //}

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
    DrawUiSelf(pui);
    DrawBlots();
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
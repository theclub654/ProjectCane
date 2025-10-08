#include "prompt.h"
#include "ui.h"
#include "dialog.h"
#include "wm.h"

void StartupPrompt(PROMPT* pprompt)
{
    g_tePrompt.m_ch = 45;
    g_tePrompt.m_rgba = { 0.0f, 0.294117659, 0.490196079, 1.0f };

    g_tePrompt.m_dxExtra = 15.0;
    g_tePrompt.m_ryScaling = 0.6;
    g_tePrompt.m_rxScaling = 0.6;

    pprompt->pvtprompt = &g_vtprompt;
}

void PostPromptLoad(PROMPT* pprompt)
{
    PostBlotLoad(pprompt);

    if (g_pfontScreenCounters != nullptr)
    {
        pprompt->pte = &g_tePrompt;
        g_tePrompt.m_pfont = g_pfontScreenCounters;
    }

    pprompt->prk = PRK_Nil;

    for (int i = 0; i < 3; i++)
        pprompt->mpprpprk[i] = PRK_Nil;
}

void SetPrompt(PROMPT* pprompt, PRP prp, PRK prk)
{
    // Set the PRK for the given priority
    pprompt->mpprpprk[prp] = prk;

    // Find the highest-priority non-Nil PRK (last one wins)
    PRK resolvedPrk = PRK_Nil;
    for (int i = 0; i < 3; ++i) {
        if (pprompt->mpprpprk[i] != PRK_Nil) {
            resolvedPrk = pprompt->mpprpprk[i];
        }
    }

    // Handle based on blot state
    BLOTS state = pprompt->blots;

    if (state == BLOTS_Hidden)
    {
        SetPromptPrk(pprompt, resolvedPrk);
        if (resolvedPrk != PRK_Nil && !pprompt->fActive) {
            PushUiActiveBlot(&g_ui, pprompt);
        }
    }
    else if (state > BLOTS_Nil && state < BLOTS_Max && pprompt->prk != resolvedPrk)
    {
        if (resolvedPrk == PRK_Nil) {
            if (pprompt->fActive) {
                PopUiActiveBlot(&g_ui);
            }
            pprompt->fReshow = 0;
        }
        else {
            pprompt->prkReshow = resolvedPrk;
            pprompt->fReshow = 1;

            // Call virtual function safely
            if (pprompt->pvtblot && pprompt->pvtblot->pfnSetBlotBlots) {
                pprompt->pvtblot->pfnSetBlotBlots(pprompt, BLOTS_Disappearing);
            }
        }
    }
}

void SetPromptPrk(PROMPT* pprompt, PRK prk)
{
    if (pprompt->prk == prk || prk == PRK_Nil)
        return;

    pprompt->prk = prk;
    pprompt->irespk = 0;

    float widthText = 0.0f;
    float heightText = 0.0f;

    if (prk == PRK_PauseMenu)
    {
        s_mpprkprd[0].crespk = 0;
        for (uint32_t i = 0; i < 6; ++i) {
            RESPK rspk = s_arespkPauseMenuAll[i];
            bool include = true;

            if (rspk == RESPK_Options) {
                include = true;
            }
            else if (rspk == RESPK_Map) {
                include = g_wmc.pwm != nullptr;
            }
            else if (rspk == RESPK_Save) {
                include = false;
            }
            else if (rspk == RESPK_Exit) {
                /*if (g_pgsCur->gameworldCur == GAMEWORLD_Intro || g_pgsCur->worldlevelCur < WORLDLEVEL_1)
                    include = false;*/
                include = false;
            }

            if (include == true) {
                s_mpprkprd[0].arespk[s_mpprkprd[0].crespk++] = rspk;
            }
        }
    }

    CFontBrx* font = pprompt->pfont;

    float dxPrompt = 0.0f;
    float dyPrompt = 0.0f;

    const char* text = s_mpprkprd[prk].pchz;
    if (text) {
        pprompt->pfont->PushScaling(s_mpprkprd[prk].rScale, s_mpprkprd[prk].rScale);
        pprompt->pfont->GetExtents(const_cast<char*>(text), &dxPrompt, &dyPrompt, 500.0f);
        pprompt->pfont->PopScaling();
        heightText += dyPrompt;
    }

    // Measure options
    float maxOptionWidth = 0.0f;
    float totalOptionWidth = 0.0f;
    float totalOptionHeight = 0.0f;

    char tempBuffer[512];  // Temporary buffer for line wrapping

    for (int i = 0; i < s_mpprkprd[prk].crespk; ++i) {
        const char* option = AchzFromRespk(s_mpprkprd[prk].arespk[i]);
        strncpy(tempBuffer, option, sizeof(tempBuffer));
        tempBuffer[sizeof(tempBuffer) - 1] = '\0';

        float dx = 0.0f, dy = 0.0f;

        pprompt->pfont->PushScaling(s_mpprkprd[prk].rScale, s_mpprkprd[prk].rScale);
        pprompt->pfont->GetExtents(tempBuffer, &dx, &dy, 500.0f);
        pprompt->pfont->PopScaling();

        if (s_mpprkprd[prk].fVertical) {
            totalOptionHeight += dy;
            maxOptionWidth = std::max(maxOptionWidth, dx);
        }
        else {
            if (i > 0)
                totalOptionWidth += pprompt->pfont->m_dxSpaceUnscaled * pprompt->pfont->m_rxScale;
            totalOptionWidth += dx;
            totalOptionHeight = std::max(totalOptionHeight, dy);
        }
    }

    float finalWidth = s_mpprkprd[prk].fVertical ? maxOptionWidth : totalOptionWidth;
    float finalHeight = dyPrompt + totalOptionHeight;

    float kPaddingX = 10.0f;
    float kPaddingY = 6.0f;
    // Add padding to final dimensions
    finalWidth += kPaddingX * 5.0f;
    //finalHeight += kPaddingY * 2.0f;

    ResizeBlot(pprompt, finalWidth, finalHeight);
}

void ExecutePrompt(PROMPT* pprompt)
{
    // Retrieve the PRK (prompt type) from the prompt structure.
    const PRK prk = pprompt->prk;

    // Fetch the corresponding PRD (prompt data) from the s_mpprkprd array using the prk.
    const PRD& prd = s_mpprkprd[prk];

    // Get the specific RESPK (response) for the current prompt based on the response index.
    const RESPK respk = prd.arespk[pprompt->irespk];

    // Switch case for different PRK values (prompt types).
    switch (prk)
    {
        // If the prompt type is PRK_ExitLevel, handle the exit level logic.
    case PRK_ExitLevel:
    {
        // Clear the prompt by setting it to PRK_Nil (no prompt).
        SetPrompt(pprompt, PRP_Basic, PRK_Nil);

        // If the response is "Yes", trigger a default exit (game exit).
        if (respk == RESPK_Yes)
        {
            //TriggerDefaultExit(0, WIPEK_Keyhole);
        }
        else
        {
            // Otherwise, prepare a transition (wipe effect).
            //TRANS trans{
            //    .grftrans = 0x12,  // Set the wipe transition type (e.g., Keyhole wipe).
            //    .fSet = true,       // Set the flag indicating the transition should be triggered.
            //    .pchzWorld = g_transition.m_achzWorldCur,  // World to transition to.
            //    .oidWarpContext = OID_Nil,  // No warp context.
            //    .oidWarp = OID_Nil  // No specific warp object.
            //};

            // Trigger the wipe effect using the defined transition.
            //ActivateWipe(&g_wipe, &trans, WIPEK_Keyhole);
        }
        break;
    }

    // Case for PRK_PauseMenu prompt type.
    case PRK_PauseMenu:
    {
        // Switch based on the response selected (respk).
        switch (respk)
        {
            // If the response is "Continue", reset the prompt (exit the pause menu).
        case RESPK_Continue:
            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            break;

            // If the response is "Map", handle the map option logic.
        case RESPK_Map:
            // If no map is available, play an unavailable sound.
            //if (g_wmc.pwm == nullptr)
            //{
            //    //StartSound(SFXID_MenuUnavail, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            //}
            //else
            //{
            //    // Otherwise, swap to the map UI.
            //    PopUiActiveBlot(&g_ui);
            //    PushUiActiveBlot(&g_ui, &g_wmc);
            //}
            break;

            // If the response is "Exit", exit the game and trigger the wipe effect.
        case RESPK_Exit:
            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            //TriggerDefaultExit(1, WIPEK_Fade);
            break;

            // If the response is "Quit", show the quit confirmation prompt.
        case RESPK_Quit:
            SetPrompt(pprompt, PRP_Basic, PRK_QuitConfirm);
            break;

            // Default case (no action needed).
        default:
            break;
        }
        break;
    }

    // Case for PRK_GameOver prompt type.
    case PRK_GameOver:
    {
        // Reset the prompt (no prompt displayed).
        SetPrompt(pprompt, PRP_Basic, PRK_Nil);

        // Prepare a transition for a fade wipe effect.
        //TRANS trans
        //{
        //    .grftrans = 0x04,  // Fade transition type.
        //    .fSet = true,       // Set the flag to indicate the transition.
        //    .pchzWorld = "unknown",  // World name for the transition (could be any name).
        //    .oidWarpContext = OID_Nil,  // No warp context.
        //    .oidWarp = OID_Nil  // No specific warp object.
        //};

        // Trigger the wipe effect with the defined transition parameters.
        //ActivateWipe(&g_wipe, &trans, WIPEK_Fade);
        break;
    }

    // Case for PRK_QuitConfirm prompt type.
    case PRK_QuitConfirm:
    {
        // Reset the prompt to PRK_Nil (no prompt) after handling the quit confirmation.
        SetPrompt(pprompt, PRP_Basic, PRK_Nil);

        // If the response is "Yes", quit the game and set the demo end reason.
        if (respk == RESPK_Yes)
            fQuitGame = 1;

        break;
    }

    // Default case: If no known prompt type, do nothing.
    default:
        break;
    }
}

void SetPromptBlots(PROMPT* pprompt, BLOTS blots)
{
    if ((blots == BLOTS_Hidden) && (pprompt->fReshow != 0)) {
        pprompt->fReshow = 0;
        SetPromptPrk(pprompt, pprompt->prkReshow);
        blots = BLOTS_Appearing;
    }
    SetBlotBlots(pprompt, blots);
}

const char* AchzFromRespk(RESPK respk)
{
    return s_mprespkachz[respk];
}

void OnPromptActive(PROMPT* pprompt, int fActive)
{
    if (pprompt->fActive == fActive)
        return;

    if (fActive == 0) // Deactivate prompt
    {
        if (g_pdialogPlaying == nullptr && pprompt->prk == PRK_PauseMenu)
        {
            SetUiUis(&g_ui, UIS_Unpausing);
        }
        else
        {
            SetUiUis(&g_ui, UIS_Playing);
        }

        if (pprompt->prk == PRK_PauseMenu)
        {
            HideBlot((BLOT*)&g_lifectr);
            HideBlot((BLOT*)&g_cluectr);
            HideBlot((BLOT*)&g_keyctr);
            HideBlot((BLOT*)&g_coinctr);

            /*if (g_game.fTimedChallenge != 0)
            {
                g_goldctr.field0_0x0.pvtblot->pfnHideBlot(0x283038);
                g_timer.field0_0x0.pvtblot->pfnHideBlot(0x2846b8);
            }*/
        }

        pprompt->pvtblot->pfnHideBlot(pprompt);
        pprompt->fActive = 0;
    }
    else // Activate prompt
    {
        if (g_pdialogPlaying == nullptr)
        {
            if (g_ui.uis == UIS_Playing)
            {
                SetUiUis(&g_ui, UIS_Pausing);
            }
            else
            {
                SetUiUis(&g_ui, UIS_Paused);
            }
        }
        else
        {
            SetUiUis(&g_ui, UIS_Paused);
        }

        if (pprompt->prk == PRK_PauseMenu)
        {
            ShowBlot((BLOT*)&g_lifectr);
            ShowBlot((BLOT*)&g_cluectr);
            ShowBlot((BLOT*)&g_keyctr);
            ShowBlot((BLOT*)&g_coinctr);

            /*if (g_game.fTimedChallenge != 0)
            {
                g_goldctr.pvtblot->pfnShowBlot(0x283038);
                g_timer.field0_0x0.pvtblot->pfnShowBlot(0x2846b8);
            }*/
        }

        pprompt->irespk = 0;
        pprompt->pvtblot->pfnShowBlot(pprompt);
        g_joy.StartJoySelection();

        pprompt->fActive = fActive;
    }
}

void UpdatePromptActive(PROMPT* pprompt, JOY* pjoy)
{
    PRK prk = pprompt->prk;
    const PRD& prd = s_mpprkprd[prk];

    // Navigation delta
    int delta = prd.fVertical ? pjoy->DySelectionJoy(g_clock.tReal)
        : pjoy->DxSelectionJoy(g_clock.tReal);

    if (delta != 0) {
        int count = prd.crespk;
        if (count == 0) {
            assert(false && "No selectable prompt entries.");
            return;
        }

        pprompt->irespk = (pprompt->irespk + delta + count) % count;
        //StartSound(SFXID_MenuSelect, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        return;
    }

    // Cancel (BTN_START or BTN_B/Circle)
    if (pjoy->IsPressed(BTN_START) || pjoy->IsPressed(BTN_CIRCLE)) {
        CancelPrompt(pprompt);
        return;
    }

    // Special case: Attract menu using BTN_A (Cross)
    if (pjoy->IsPressed(BTN_CROSS) && prk == PRK_AttractMenu) {

        SetPrompt(pprompt, PRP_Basic, PRK_MemcardEraseConfirm);
        return;
    }

    if (pjoy->IsPressed(BTN_CROSS)) {
        ExecutePrompt(pprompt);
        return;
    }
}

void CancelPrompt(PROMPT* pprompt)
{
    // Only allow cancellation if prompt is visible and has been on-screen long enough
    bool allowCancel = false;
    if (pprompt->blots == BLOTS_Visible && (g_clock.tReal - pprompt->tBlots) > 0.3f) {
        allowCancel = true;
    }

    switch (pprompt->prk) {
    case PRK_PauseMenu:
    case PRK_MemcardEraseConfirm:
    case PRK_AttractMenu:
        // These prompts can only be cancelled if the timer condition is met
        if (!allowCancel) {
            return;
        }
        break;

    case PRK_QuitConfirm:
        // Quit confirm always returns to Pause menu
        SetPrompt(pprompt, PRP_Basic, PRK_PauseMenu);
        return;

    case PRK_OptionsMenu:
        // Options menu can only be cancelled if the timer allows
        if (!allowCancel) {
            return;
        }

        // If not in attract mode, return to pause menu
        /*if (g_ui.uisPlaying != UIS_Attract) {
            SetPrompt(pprompt, PRP_Basic, PRK_PauseMenu);
            return;
        }*/
        SetPrompt(pprompt, PRP_Basic, PRK_PauseMenu);
        return;
        break;

    default:
        // For all other prompt kinds, fall through to generic cancel
        break;
    }

    // Default cancellation: set to nil prompt
    SetPrompt(pprompt, PRP_Basic, PRK_Nil);
}

void DrawPrompt(PROMPT* pprompt)
{
    if (!pprompt || pprompt->blots == BLOTS_Hidden || pprompt->uOn == 0.0f)
        return;

    float scale = pprompt->uOn;
    float dx = pprompt->dx * scale;
    float dy = pprompt->dy * scale;

    float centerX = pprompt->xOn + pprompt->dx * 0.5f;
    float centerY = pprompt->yOn + pprompt->dy * 0.5f;

    float x = pprompt->xOn * scale + centerX * (1.0f - scale);
    float y = pprompt->yOn * scale + centerY * (1.0f - scale);

    float t = sinf(g_clock.tReal * 10.0f);
    float pulse = t * 0.5f + 0.5f;
    PRK prk = pprompt->prk;
    PRD* prd = &s_mpprkprd[prk];

    CTextBox textBox;
    textBox.SetPos(x, y);
    textBox.SetSize(dx, dy);

    glm::vec4 baseColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    textBox.SetTextColor(&baseColor);
    textBox.SetHorizontalJust(JH_Left);
    textBox.SetVerticalJust(JV_Top);

    // Edge rendering with color animation if no responses
    if (pprompt->pte) {
        if (prd->crespk == 0) {
            glm::vec4 yellow = glm::vec4(0.44f, 0.56f, 0.0f, 1.0f);
            glm::vec4 edgeColor = glm::mix(baseColor, yellow, pulse * pulse);
            pprompt->pte->m_rgba = edgeColor;
        }
        if (pprompt->pte->m_pfont)
            pprompt->pte->m_pfont->EdgeRect(pprompt->pte, &textBox);
    }

    // Draw prompt text if available
    if (prd->pchz != nullptr) {
        float scaled = pprompt->uOn * prd->rScale;
        pprompt->pfont->PushScaling(scaled, scaled);

        textBox.SetPos(x, y);
        textBox.SetSize(dx, dy);

        glm::vec4 white = glm::vec4(0.37f, 0.37f, 0.37f, 1.0f);
        textBox.SetTextColor(&white);
        textBox.SetHorizontalJust(JH_Center);
        textBox.SetVerticalJust(JV_Top);

        pprompt->pfont->DrawPchz((char*)prd->pchz, &textBox);

        y += pprompt->pfont->DyWrapPchz((char*)prd->pchz, 500.0f);
        pprompt->pfont->PopScaling();
    }

    // Draw selectable response options
    pprompt->pfont->PushScaling(scale, scale);

    float lineHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    float textX = x;
    float textY = y;

    if (!prd->fVertical) {
        float totalWidth = 0.0f;
        for (int i = 0; i < prd->crespk; ++i) {
            const char* option = AchzFromRespk(prd->arespk[i]);
            totalWidth += pprompt->pfont->DxFromPchz(const_cast<char*>(option));
            if (i > 0)
                totalWidth += pprompt->pfont->m_dxSpaceUnscaled * pprompt->pfont->m_rxScale;
        }
        textX += (dx - totalWidth) * 0.5f;
    }

    for (int i = 0; i < prd->crespk; ++i) {
        const char* option = AchzFromRespk(prd->arespk[i]);

        glm::vec4 color = (i == pprompt->irespk)
            ? glm::mix(glm::vec4(0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), pulse * pulse)
            : glm::vec4(0.37f, 0.37f, 0.37f, 1.0f);

        float textWidth = pprompt->pfont->DxFromPchz(const_cast<char*>(option));
        float xDraw = textX;
        float yDraw = textY;

        if (prd->fVertical) {
            xDraw += (dx - textWidth) * 0.5f;
        }

        textBox.SetPos(xDraw, yDraw);
        textBox.SetSize(dx, lineHeight);
        textBox.SetTextColor(&color);
        textBox.SetHorizontalJust(JH_Left);
        textBox.SetVerticalJust(JV_Top);

        pprompt->pfont->DrawPchz((char*)option, &textBox);

        if (!prd->fVertical) {
            textX += textWidth + pprompt->pfont->m_dxSpaceUnscaled * pprompt->pfont->m_rxScale;
        }
        else {
            textY += lineHeight;
        }
    }

    pprompt->pfont->PopScaling();
}

PROMPT g_prompt;
CTextEdge g_tePrompt;
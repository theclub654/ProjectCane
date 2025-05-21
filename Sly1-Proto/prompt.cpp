#include "prompt.h"
#include "ui.h"
#include "dialog.h"

void StartupPrompt(PROMPT* pprompt)
{
    g_tePrompt.m_ch = (char)"-";
    g_tePrompt.m_rgba.r = 0.0;
    g_tePrompt.m_rgba.g = 75.0;
    g_tePrompt.m_rgba.b = 125.0;
    g_tePrompt.m_rgba.a = 1.0;
    
    g_tePrompt.m_dxExtra = 1.0;
    g_tePrompt.m_ryScaling = 1.0;
    g_tePrompt.m_rxScaling = 1.0;

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
    // Early exit: if prompt already has this PRK or PRK is invalid, do nothing.
    if (pprompt->prk == prk || prk == PRK_Nil)
        return;

    // Set the new PRK and reset the response count.
    pprompt->prk = prk;
    pprompt->irespk = 0;

    float dx = 0.0f; // Accumulated width of the prompt area
    float dy = 0.0f; // Accumulated height of the prompt area

    // Special case: if PRK is the pause menu, dynamically construct its response list
    if (prk == PRK_PauseMenu) {
        s_mpprkprd[0].crespk = 0; // Clear count of responses

        // Loop through predefined responses for pause menu
        for (int i = 0; i < 6; ++i) {
            RESPK respk = s_arespkPauseMenuAll[i];
            bool add = true;

            // Filter which responses should be included based on game state
            if (respk == RESPK_Options) {
                add = false; // Always skip Options
            }
            else if (respk == RESPK_Map) {
                //add = (g_wmc.pwm != nullptr); // Only show Map if it's available
                add == true;
            }
            else if (respk == RESPK_Save) {
                add = false; // Save is always skipped here
            }
            else if (respk == RESPK_Exit) {
                // Exit only shown if not in intro or early world level
                /*if (g_pgsCur->gameworldCur == GAMEWORLD_Intro || g_pgsCur->worldlevelCur < WORLDLEVEL_1) {
                    add = false;
                }*/

                add == true;
            }

            // Add the response to the prompt's response list
            if (add) {
                auto& prd = s_mpprkprd[0];
                prd.arespk[prd.crespk++] = respk;
            }
        }
    }

    // Get the PRD (Prompt Definition) for this PRK
    auto& prd = s_mpprkprd[prk];

    // If there's a static label for this prompt (e.g. "Press to continue")
    if (prd.pchz != 0) 
    {
        // Temporarily apply font scaling
        pprompt->pfont->PushScaling(prd.rScale, prd.rScale);

        // Query the width/height of the text (within 500px wrapping)
        float dxQuery, dyQuery;
        pprompt->pfont->GetExtents((char*)prd.pchz, &dxQuery, &dyQuery, 500.0f);

        // Restore previous font scaling
        pprompt->pfont->PopScaling();

        // Update maximum dimensions
        dx = std::max(dx, dxQuery);
        dy += dyQuery;
    }

    float height = 0.0f;
    const int crespk = prd.crespk;

    // Compute height of all dynamic response entries
    if (prd.fVertical == 0) {
        // Horizontal layout: use one line height
        height = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    }
    else {
        // Vertical layout: multiple lines
        height = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale * crespk;
    }

    float totalWidth = 0.0f;

    // Measure width of each response entry
    for (int i = 0; i < crespk; ++i) {
        RESPK respk = prd.arespk[i];
        if (respk == RESPK_Nil)
            break;

        // Get text label for this response type
        const char* text = AchzFromRespk(respk);
        float textWidth = pprompt->pfont->DxFromPchz((char*)text);

        if (prd.fVertical == 0) {
            // Horizontal layout: sum up all widths + spacing
            if (i == 0) {
                totalWidth += textWidth;
            }
            else {
                // Add space between entries
                totalWidth += static_cast<float>(pprompt->pfont->m_dxSpaceUnscaled) * pprompt->pfont->m_rxScale;
                totalWidth += textWidth;
            }
        }
        else {
            // Vertical layout: track widest entry
            totalWidth = std::max(totalWidth, textWidth);
        }
    }

    // Total prompt width is max(static label, dynamic responses) + padding
    float finalWidth = std::max(dx, totalWidth) + 10.0f;

    // Resize the prompt to fit computed width/height
    ResizeBlot(pprompt, finalWidth, dy + height);
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
            //    PushUiActiveBlot(&g_ui, reinterpret_cast<BLOT*>(&g_wmc));
            //}
            break;

            // If the response is "Exit", exit the game and trigger the wipe effect.
        case RESPK_Exit:
            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            //TriggerDefaultExit(1, WIPEK_Fade);
            break;

            // If the response is "Quit", show the quit confirmation prompt.
        case RESPK_Quit:
            SetPrompt(&g_prompt, PRP_Basic, PRK_QuitConfirm);
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
        {
            //g_sampler.m_ser = SCE_DEMO_ENDREASON_PLAYABLE_QUIT;
        }
        break;
    }

    // Default case: If no known prompt type, do nothing.
    default:
        break;
    }
}

void SetPromptBlots(PROMPT* pprompt, BLOTS blots)
{
    //std::cout << pprompt->blots<<"\n";

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
            /*g_lifectr.field0_0x0.pvtblot->pfnHideBlot(0x2828b8);
            g_cluectr.field0_0x0.pvtblot->pfnHideBlot(0x282b38);
            g_keyctr.field0_0x0.pvtblot->pfnHideBlot(0x282db8);
            g_coinctr.field0_0x0.pvtblot->pfnHideBlot(0x2832b8);

            if (g_game.fTimedChallenge != 0)
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
            /*g_lifectr->pvtblot->pfnShowBlot(0x2828b8);
            g_cluectr->pvtblot->pfnShowBlot(0x282b38);
            g_keyctr->pvtblot->pfnShowBlot(0x282db8);
            g_coinctr->pvtblot->pfnShowBlot(0x2832b8);

            if (g_game.fTimedChallenge != 0)
            {
                g_goldctr.field0_0x0.pvtblot->pfnShowBlot(0x283038);
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

    // Determine navigation direction (horizontal or vertical)
    int selectionDelta = prd.fVertical == 0
        ? pjoy->DxSelectionJoy(g_clock.tReal)
        : pjoy->DySelectionJoy(g_clock.tReal);

    // Handle selection change
    if (selectionDelta != 0)
    {
        int count = prd.crespk;
        if (count == 0)
        {
            assert(false && "Prompt has no selectable entries.");
            return;
        }

        pprompt->irespk = (pprompt->irespk + selectionDelta + count) % count;
        //StartSound(SFXID_MenuSelect, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        return;
    }

    // Cancel prompt: BTN_B (e.g., Circle) or BTN_START
    if (pjoy->IsPressed(BTN_B))
    {
        CancelPrompt(pprompt);
        return;
    }

    // Confirm special: BTN_X (e.g., Square in PS2 context)
    if (pjoy->IsPressed(BTN_X))
    {
        if (pprompt->prk == PRK_AttractMenu)
        {
            SetPrompt(pprompt, PRP_Basic, PRK_MemcardEraseConfirm);
        }
        return;
    }

    // Confirm: BTN_A (Cross)
    if (pjoy->IsPressed(BTN_A))
    {
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
    if (!pprompt || pprompt->blots == BLOTS_Hidden)
        return;

    float interp = pprompt->uOn;

    if (interp == 0.0f)
        return;

    // Interpolated position and size
    float dx = pprompt->dx * interp;
    float dy = pprompt->dy * interp;

    float xMid = pprompt->xOn + pprompt->dx * 0.5f;
    float yMid = pprompt->yOn + pprompt->dy * 0.5f;

    float x = pprompt->xOn * interp + xMid * (1.0f - interp);
    float y = pprompt->yOn * interp + yMid * (1.0f - interp);

    // Text edge animation
    float wave = std::sinf(g_clock.tReal * 10.0f);
    float fVar23 = wave * 0.5f + 0.5f;

    PRK PVar3 = pprompt->prk;
    PRD* pPVar16 = &s_mpprkprd[PVar3];

    CTextEdge* pte = pprompt->pte;

    if (pte != nullptr) 
    {
        // Setup text box
        CTextBox tbx;
        tbx.SetPos(x, y);
        tbx.SetSize(dx, dy);

        glm::vec4 rgba;
        rgba.r = 128;
        rgba.g = 128;
        rgba.b = 128;
        rgba.a = 0;

        tbx.SetTextColor(&rgba);
        tbx.SetHorizontalJust(JH_Left);
        tbx.SetVerticalJust(JV_Top);

        // Blend edge color toward yellow if not blocked by crespk
        if (pPVar16->crespk == 0) 
        {
            CTextEdge* pte = pprompt->pte;

            glm::vec4 baseColor = glm::vec4(pte->m_rgba.r, pte->m_rgba.g, pte->m_rgba.b, pte->m_rgba.a);

            glm::vec4 targetColor = glm::vec4(111, 111, 111, 255) / 255.0f; // Yellowish gray

            float blendFactor = fVar23 * fVar23;

            glm::vec4 blendedColor = glm::mix(baseColor, targetColor, blendFactor);
            blendedColor = glm::clamp(blendedColor, 0.0f, 1.0f);

            // Convert to 8-bit color for setting
            pte->m_rgba.r = blendedColor.r;
            pte->m_rgba.g = blendedColor.g;
            pte->m_rgba.b = blendedColor.b;
            pte->m_rgba.a = blendedColor.a;
        }

        pte->m_pfont->EdgeRect(pte, &tbx);
    }

    pprompt->pfont->PopScaling();
}

PROMPT g_prompt;
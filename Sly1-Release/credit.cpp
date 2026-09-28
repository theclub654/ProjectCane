#include "credit.h"
#include "gui_layout.h"
#include "timer.h"
#include "coin.h"
#include <cstdio>
#include <cstring>

COMMENTARY* PcommentaryFromLevelId(int levelId)
{
    for (COMMENTARY& commentary : g_aCommentaryLoadData)
    {
        if (commentary.levelId == levelId)
            return &commentary;
    }

    return nullptr;
}

void StartupNote(NOTE* pnote)
{
    pnote->pvtnote = &g_vtnote;

    // Retail initializes the shared NOTE edge here (FUN_001e0848).
    // The font itself is assigned after font 2 has loaded in PostNoteLoad.
    g_teNote.m_pfont = nullptr;
    g_teNote.m_ch = '-';
    g_teNote.m_dxExtra = 2.0f;
    g_teNote.m_dyExtra = 1.0f;
    g_teNote.m_rxScaling = 0.3f;
    g_teNote.m_ryScaling = 0.3f;
    // GS color channels use 128 as full intensity.
    g_teNote.m_rgba = glm::vec4(0.0f, 75.0f / 128.0f,
        125.0f / 128.0f, 1.0f);
}

void PostNoteLoad(NOTE* pnote)
{
    // Step 1: Run shared UI element initialization
    PostBlotLoad(pnote);

    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | g_pgsCur->worldLevelCur;
    COMMENTARY* pcommentary = PcommentaryFromLevelId(levelId);
	const auto* commentaryDescriptor = reinterpret_cast<const char*>(
		g_psw->commentaryAudioDescriptor.data());
	const bool fHasBrxCommentaryDescriptor =
		g_psw->audioLocation == commentaryDescriptor;

    if (pcommentary != nullptr)
    {
        if (pcommentary->audioDuration != 0.0f)
            g_psw->dtCommentaryAudio = pcommentary->audioDuration;

        if (!fHasBrxCommentaryDescriptor &&
			(g_psw->audioLocation == nullptr || g_psw->audioLocation[0] == '\0'))
            g_psw->audioLocation = pcommentary->pchzAudio;

        if (g_psw->pchzSpeakers == nullptr || g_psw->pchzSpeakers[0] == '\0')
            g_psw->pchzSpeakers = pcommentary->pchzSpeakerNames;
    }

    if (FFontLoaded(2))
    {
        pnote->pte = &g_teNote;
        pnote->pte->m_pfont = PfontFromFont(2);
    }

	SetBlotFontScale(pnote, 0.6f);
    // PS2 GS RGB uses 128 as full intensity. Retail's (127,127,127,223)
    // therefore renders nearly white and fully opaque, not 50% gray.
    pnote->rgba = glm::vec4(127.0f / 128.0f,
        127.0f / 128.0f,
        127.0f / 128.0f,
        1.0f);

    const bool fCommentaryUnlocked = FCommentaryUnlockedForLevel(g_psw, levelId) != 0;
    SetNoteMtsState(pnote, fCommentaryUnlocked ? MTSSTATE_Eligible : MTSSTATE_None);

	const bool fHasCommentaryAudio = g_psw->audioLocation != nullptr &&
		(g_psw->audioLocation == commentaryDescriptor || g_psw->audioLocation[0] != '\0');
    pnote->fCommentaryPromptActive = 0;

    // Both NOTE (the sprint/commentary display) and PNOTE (the general
    // power/status message) use PostNoteLoad. Only NOTE owns the commentary
    // state machine; showing this prompt on PNOTE creates a second box.
    if (pnote == &g_note &&
        (g_plsCur->grfls & FLS_Tertiary) != 0 && fHasCommentaryAudio)
    {
        pnote->pvtblot->pfnSetBlotAchzDraw(pnote, (char*)s_pchzBonusCommentaryPrompt);
        SetBlotDtVisible(pnote, 5.0f);
        pnote->pvtblot->pfnShowBlot(pnote);
        pnote->fCommentaryPromptActive = 1;
    }

    pnote->pgold = nullptr;
}

void UpdateNote(NOTE* pnote)
{
    UpdateBlot(pnote);

    if (pnote->mtsState >= MTSSTATE_None && pnote->mtsState < MTSSTATE_Active)
    {
        if (!pnote->fCommentaryPromptActive)
            return;

        if (!g_joy.IsPressed(BTN_L1))
        {
            if (g_clock.t >= 5.0f)
            {
                pnote->pvtblot->pfnHideBlot(pnote);
                pnote->fCommentaryPromptActive = 0;
            }

            return;
        }

		g_joy.SetHandled(BTN_L1);
        PlayCommentaryAudio((char*)g_psw->audioLocation);

        if (g_psw->pchzSpeakers == nullptr || g_psw->pchzSpeakers[0] == '\0')
            pnote->pvtblot->pfnHideBlot(pnote);
        else
        {
            SetBlotDtVisible(pnote, 15.0f);
            pnote->pvtblot->pfnSetBlotAchzDraw(pnote, (char*)g_psw->pchzSpeakers);
        }

        pnote->fCommentaryPromptActive = 0;
        return;
    }

    if (pnote->mtsState != MTSSTATE_Active)
        return;

    if (g_timer.timers == TIMERS_Expired)
    {
        SetNoteMtsState(pnote, MTSSTATE_Finished);
        return;
    }

    const float timerSpeed = -std::pow(2.0f, static_cast<float>(g_game.cAlarmsTriggered));
    SetTimerSpeed(&g_timer, timerSpeed);

    const bool fCanShowGameplayBlot = FCanShowGameplayBlot() != 0;

    if (fCanShowGameplayBlot)
        g_timer.pvttimer->pfnShowBlot(&g_timer);
    else
        g_timer.pvttimer->pfnHideBlot(&g_timer);

    if ((g_plsCur->grfls & FLS_Tertiary) == 0)
        return;

    const bool fShowNote = fCanShowGameplayBlot && !g_prompt.fActive && g_plsCur->dtTimedBest < g_timer.dt;

    if (fShowNote)
        g_note.pvtblot->pfnShowBlot(&g_note);
    else
        g_note.pvtblot->pfnHideBlot(&g_note);
}

void SetNoteMtsState(NOTE* pnote, MTSSTATE mtsState)
{
    const MTSSTATE mtsStatePrev = pnote->mtsState;

    if (mtsStatePrev == mtsState)
        return;

    if (mtsStatePrev == MTSSTATE_Eligible)
    {
        if (pnote->fCommentaryPromptActive)
        {
            pnote->pvtblot->pfnHideBlot(pnote);
            pnote->fCommentaryPromptActive = 0;
        }
    }
    else if (mtsStatePrev == MTSSTATE_Active)
    {
        StopTimer(&g_timer);
        g_timer.pvttimer->pfnHideBlot(&g_timer);
        pnote->pvtblot->pfnHideBlot(pnote);
    }

    if (mtsState == MTSSTATE_Active)
    {
        StartSound(static_cast<SFXID>(109), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        SetCcharm(0);
        CancelSwDialogPlaying(g_psw);

        const float dtSprint = g_psw->dtCommentaryAudio + 0.99f;
        SetTimer(&g_timer, dtSprint);

        const float timerSpeed = -std::pow(2.0f, static_cast<float>(g_game.cAlarmsTriggered));
        SetTimerSpeed(&g_timer, timerSpeed);
        SetTimerExpire(&g_timer, 0.0f, nullptr);
        StartTimer(&g_timer);

        if ((g_plsCur->grfls & FLS_Tertiary) == 0)
        {
            SetBlotDtVisible(pnote, 5.0f);
            pnote->pvtblot->pfnSetBlotAchzDraw(pnote, (char*)s_pchzCommentaryTimer);
        }
        else
        {
            const float dtBest = g_plsCur->dtTimedBest;

            if (dtBest < 1.0f)
                std::snprintf(s_achzCommentaryTime, sizeof(s_achzCommentaryTime), s_achzCommentaryTimeTenths, static_cast<int>(dtBest * 10.0f));
            else
                std::snprintf(s_achzCommentaryTime, sizeof(s_achzCommentaryTime), s_achzCommentaryTimeMinutes, static_cast<int>(dtBest) / 60, static_cast<int>(dtBest) % 60);

            SetBlotDtVisible(pnote, 0.0f);
            pnote->pvtblot->pfnSetBlotAchzDraw(pnote, s_achzCommentaryTime);
        }

        pnote->pvtblot->pfnShowBlot(pnote);
        pnote->mtsState = MTSSTATE_Active;
    }
    else if (mtsState == MTSSTATE_Finished)
    {
        PRK prk;

        if (g_timer.dt > 0.0f)
        {
            if ((g_plsCur->grfls & FLS_Tertiary) == 0)
            {
                prk = PRK_MtsComplete;

                g_plsCur->grfls |= FLS_Tertiary;
                g_plsCur->dtTimedBest = g_timer.dt;
                ++g_pwsCur->ctimed;

                if ((GetGameProgress() & FLS_Tertiary) != 0)
                {
                    UnlockProgressRewards(FLS_Tertiary);
                    AutosaveCurrentGame(&g_saveData);
                }
            }
            else if (g_timer.dt <= g_plsCur->dtTimedBest)
                prk = PRK_MtsFailedBestTime;
            else
            {
                prk = PRK_MtsBestTime;

                g_plsCur->dtTimedBest = g_timer.dt;
                AutosaveCurrentGame(&g_saveData);
            }
        }
        else
        {
            prk = (g_plsCur->grfls & FLS_Tertiary) != 0 ? PRK_MtsFailedBestTime : PRK_MtsExpired;
        }

        SetPrompt(&g_prompt, PRP_Basic, prk);
        pnote->mtsState = MTSSTATE_Finished;
    }
    else
        pnote->mtsState = mtsState;

    pnote->tMtsState = *pnote->ptNow;

    if (mtsState == MTSSTATE_Active)
        RemoveSwExtraneousCharms(g_psw);

    pnote->fGoldEnabled = pnote->mtsState != MTSSTATE_None;
}

void DrawNote(NOTE* pnote)
{
    if (pnote->achzDraw[0] == '\0')
        return;

    glm::vec4 fadedColor = pnote->rgba;
    fadedColor.a *= pnote->uOn;

    // Credit events and NOTE dimensions are authored in the PS2's
    // 640 x 492.8 virtual canvas. Credits fill the active output canvas rather
    // than the centered 4:3 HUD canvas, so scale each axis independently.
    // Embedded CREDIT notes remain at Modern 125%; the global gameplay NOTE
    // continues to honor the selected HUD scale.
    const GuiScale guiScale = FModernGui() && pnote->blotk == BLOTK_CreditNote
        ? GetGuiScale(1.25f)
        : GetGuiScale();
    const float scaleX = guiScale.x;
    const float scaleY = guiScale.y;

    float x = pnote->xOn;
    float y = pnote->yOn;
    float dx = pnote->dx * scaleX;
    float dy = pnote->dy * scaleY;

    // PNOTE is used for bottom-edge status prompts such as "Erase Game".
    // Its dimensions were scaled for Modern GUI while its authored bottom
    // anchor was not, leaving the enlarged box partly below the framebuffer.
    // Use the same scaled edge-anchor calculation as ordinary BLOT drawing.
    if (pnote == &g_pnote)
        GetGuiScaledBlotRect(pnote, &x, &y, &dx, &dy);

    // The global NOTE is conditionally pegged above TIMER. Sprint messages use
    // that peg while the timer is active; power-selection and developer-
    // commentary notes must fall back to their normal bottom-left anchor when
    // the timer is hidden.
    if (pnote->blotk == BLOTK_Note && pnote->pbloti != nullptr &&
        pnote->pbloti->blotkPeg == BLOTK_Timer)
    {
        x = pnote->pbloti->x * scaleX;

        if (FIncludeBlotForPeg(&g_timer, pnote))
        {
            const float timerY = g_gl.height -
                (g_gl.height - g_timer.y) * scaleY;
            y = timerY + pnote->pbloti->y * scaleY -
                pnote->dy * scaleY;
        }
        else
        {
            y = g_gl.height + pnote->pbloti->y * scaleY -
                pnote->dy * scaleY;
        }
    }

    // Letterbox geometry always uses the full PS2-canvas-to-framebuffer
    // height conversion, even when Modern GUI content is drawn smaller.
    // Using Modern's density here moves the note by too little and lets its
    // lower edge overlap the black bar.
    constexpr float kPs2CanvasHeight = 492.80002f;
    const float letterboxScale = g_gl.height / kPs2CanvasHeight;

    CTextBox tbx;
    tbx.SetPos(x, y - g_letterbox.uOn * 66.40001f * letterboxScale);
    tbx.SetSize(dx, dy);
    tbx.SetTextColor(&fadedColor);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    CRichText rt(pnote->achzDraw, pnote->pfont);

    // The commentary prompt's authored NOTE width is larger than its actual
    // retail string in this data set. Retail's rectangle ends with the text,
    // so measure this prompt using the same effective font scale and trim only
    // its edge box. Keep the text box itself unchanged for wrapping/layout.
    CTextBox edgeBox = tbx;
    const bool fDrawingCommentaryPrompt =
        pnote->blotk == BLOTK_Note &&
        std::strcmp(pnote->achzDraw, s_pchzBonusCommentaryPrompt) == 0;
    if (fDrawingCommentaryPrompt)
    {
        pnote->pfont->PushScaling(pnote->rFontScale * scaleX,
            pnote->rFontScale * scaleY);
        edgeBox.m_dx = std::min(edgeBox.m_dx, rt.DxMaxLine());
        const float textLineHeight =
            static_cast<float>(pnote->pfont->m_dyUnscaled) *
            pnote->pfont->m_ryScale;
        edgeBox.m_dy = std::min(edgeBox.m_dy, textLineHeight);
        pnote->pfont->PopScaling();
    }

    if (pnote->pte != nullptr)
    {
        const glm::vec4 edgeColorPrev = pnote->pte->m_rgba;
        pnote->pte->m_rgba.a *= pnote->uOn;

        if (pnote->pte->m_pfont != nullptr)
        {
            const float edgeScaleX = pnote->pte->m_rxScaling;
            const float edgeScaleY = pnote->pte->m_ryScaling;
            pnote->pte->m_rxScaling *= scaleX;
            pnote->pte->m_ryScaling *= scaleY;
            pnote->pte->m_pfont->EdgeRect(pnote->pte, &edgeBox);
            pnote->pte->m_rxScaling = edgeScaleX;
            pnote->pte->m_ryScaling = edgeScaleY;
        }

        pnote->pte->m_rgba = edgeColorPrev;
    }

    pnote->pfont->PushScaling(pnote->rFontScale * scaleX,
        pnote->rFontScale * scaleY);

    rt.Draw(&tbx, nullptr);

    pnote->pfont->PopScaling();
}

void FinishNoteSprint(NOTE* pnote)
{
    SetNoteMtsState(pnote, MTSSTATE_Finished);
}

void StartupCredit(CREDIT* pcredit)
{
    pcredit->pvtcredit = &g_vtcredit;
}

void InitCredit(CREDIT* pcredit, BLOTK blotk)
{
    InitBlot(pcredit, BLOTK_Credit);

    for (int i = 0; i < 4; i++)
    {
        // Retail initializes CREDIT's embedded notes as kind 18. Unlike the
        // global NOTE (kind 17), this placement is not pegged to the timer.
        g_vtnote.pfnInitBlot(&pcredit->anote[i], BLOTK_CreditNote);
        pcredit->anote[i].pvtnote = &g_vtnote;
    }
}

void PostCreditLoad(CREDIT* pcredit)
{
    PostBlotLoad(pcredit);

    for (int i = 0; i < 4; i++)
    {
        NOTE* const pnote = &pcredit->anote[i];
        pnote->pvtblot->pfnPostBlotLoad(pnote);

        pnote->pte = nullptr;
        pnote->pfont->m_rxScale = 0.8f;
        pnote->pfont->m_ryScale = 0.8f;

        SetBlotDtAppear(pnote, 1.0f);
        SetBlotDtDisappear(pnote, 1.0f);
        SetBlotDtVisible(pnote, 0.0f);
    }
}

void SetCreditClock(CREDIT* pcredit, float* pt)
{
    // Update the clock for the CREDIT's base BLOT
    SetBlotClock(pcredit, pt);

    // Update clocks for each of the 4 NOTE elements in CREDIT
    for (int i = 0; i < 4; ++i)
    {
        NOTE* pnote = &pcredit->anote[i];
        if (pnote->pvtblot && pnote->pvtblot->pfnSetBlotClock)
        {
            pnote->pvtblot->pfnSetBlotClock(pnote, pt);
        }
    }
}

void UpdateCredit(CREDIT* pcredit)
{
    // Update the base CREDIT BLOT
    UpdateBlot(pcredit);

    // Update all active NOTE entries
    for (int i = 0; i < pcredit->cnote; ++i)
    {
        NOTE* pnote = &pcredit->anote[i];
        if (pnote->pvtblot && pnote->pvtblot->pfnUpdateBlot)
        {
            pnote->pvtblot->pfnUpdateBlot(pnote);
        }
    }
}

void DrawCredit(CREDIT* pcredit)
{
    // Retail draws credits only while the normal gameplay-blot set is active.
    if (g_ui.cpblotActive != 1)
        return;

    // Credit events store their origin in the PS2's 640 x 492.8 canvas.
    // PlaceCredit has already scaled the CREDIT bounds used by RepositionBlot,
    // but the authored x/y margin still needs conversion to output pixels.
    const GuiScale guiScale = FModernGui() ? GetGuiScale(1.25f) : GetGuiScale();
    const float scaleX = guiScale.x;
    const float scaleY = guiScale.y;
    float x = pcredit->x;
    float yCurrent = pcredit->y;

    if (pcredit->pbloti != nullptr)
    {
        x += pcredit->pbloti->x * (scaleX - 1.0f);
        yCurrent += pcredit->pbloti->y * (scaleY - 1.0f);
    }

    for (int i = 0; i < pcredit->cnote; ++i)
    {
        NOTE* pnote = &pcredit->anote[i];

        // Cache original uOn value
        float originalUOn = pnote->uOn;

        // DrawNote uses the settled target coordinates, matching retail.
        pnote->xOn = x;
        pnote->yOn = yCurrent;

        // Retail multiplies this by _DAT_00261850, which is the constant 1.0f.
        pnote->uOn = originalUOn;

        // Draw the note via virtual call
        if (pnote->pvtblot && pnote->pvtblot->pfnDrawBlot)
            pnote->pvtblot->pfnDrawBlot(pnote);

        // Restore original uOn
        pnote->uOn = originalUOn;

        // Advance y-position for next NOTE
        yCurrent += pnote->dy * scaleY;
    }
}

void PlaceCredit(CREDIT* pcredit, float x, float y, int cline)
{
    pcredit->pbloti->x = x;
    pcredit->pbloti->y = y;
    pcredit->cnote = cline;

    float dxMax = 0.0f;
    for (int i = 0; i < cline; ++i)
        dxMax = std::max(dxMax, pcredit->anote[i].dx);

    const GuiScale guiScale = FModernGui() ? GetGuiScale(1.25f) : GetGuiScale();
    const float scaleX = guiScale.x;
    const float scaleY = guiScale.y;
    const float dyLine = static_cast<float>(pcredit->pfont->m_dyUnscaled) * pcredit->pfont->m_ryScale;
    ResizeBlot(pcredit, dxMax * scaleX,
        dyLine * static_cast<float>(cline) * scaleY);

    SetBlotBlots(pcredit, BLOTS_Visible);
}

void VacateCredit(CREDIT* pcredit)
{
    SetBlotBlots(pcredit, BLOTS_Hidden);
}

void SetCreditLine(CREDIT* pcredit, int iline, char* pchz, float dtVisible)
{
    char* pchzLine = pchz;

    for (; *pchz; ++pchz)
    {
        if (*pchz == '_')
            *pchz = ' ';
    }

    BLOT* pnote = &pcredit->anote[iline];
    SetBlotAchzDraw(pnote, pchzLine);

    float dxMax = 0.0f;

    for (int i = 0; i < pcredit->cnote; ++i)
        dxMax = std::max(dxMax, pcredit->anote[i].dx);

    const GuiScale guiScale = FModernGui() ? GetGuiScale(1.25f) : GetGuiScale();
    const float scaleX = guiScale.x;
    const float scaleY = guiScale.y;
    const float dyLine = static_cast<float>(pcredit->pfont->m_dyUnscaled) * pcredit->pfont->m_ryScale;
    ResizeBlot(pcredit, dxMax * scaleX,
        dyLine * static_cast<float>(pcredit->cnote) * scaleY);

    if (dtVisible != 0.0f)
    {
        float dtHold = dtVisible - (DtAppearBlot(pnote) + DtDisappearBlot(pnote));

        if (dtHold <= 0.0f)
            dtHold = 1.0f / 60.0f;

        SetBlotDtVisible(pnote, dtHold);
        ShowBlot(pnote);
    }
}

CREDIT g_credit;
NOTE g_note;
CTextEdge g_teNote{};
NOTE g_pnote;
COMMENTARY g_aCommentaryLoadData[33] =
{
    { 0x100,  77.0f, "Commentary_0100.vag", "Nate Fox\nHokyo Lim" },
    { 0x102,  60.0f, "Commentary_0102.vag", "Rob McDaniel\nSuzanne Kaufmann" },
    { 0x103, 139.0f, "Commentary_0103.vag", "Rob McDaniel\nDev Madan\nAugie Pagan\nKarin Young" },
    { 0x104,  90.0f, "Commentary_0104.vag", "Bruce Oberg\nRob McDaniel\nHokyo Lim" },
    { 0x105,  92.0f, "Commentary_0105.vag", "Rob McDaniel\nChris Heidorn" },
    { 0x106,   0.0f, "Commentary_0106.vag", "Brian Fleming\nChris Bentzel" },
    { 0x107,  70.0f, "Commentary_0107.vag", "Brian Fleming\nDJ Stiner\nRob McDaniel" },

    { 0x200,  90.0f, "Commentary_0200.vag", "Brian Fleming\nNate Fox\nTravis Kotzesbue" },
    { 0x202, 135.0f, "Commentary_0202.vag", "Bruce Oberg\nNate Fox\nMatt Olsen" },
    { 0x203,  80.0f, "Commentary_0203.vag", "Brian Fleming\nChris Bentzel" },
    { 0x204,   0.0f, "Commentary_0204.vag", "Chris Zimmerman\nMatt Olsen\nBruce Oberg" },
    { 0x205, 118.0f, "Commentary_0205.vag", "Nate Fox\nChris Bentzel" },
    { 0x206,  77.0f, "Commentary_0206.vag", "Rob McDaniel\nSuzanne Kaufmann" },
    { 0x207,  83.0f, "Commentary_0207.vag", "Rob McDaniel\nBrian Fleming" },

    { 0x300,  95.0f, "Commentary_0300.vag", "Nate Fox\nMatt Olsen\nAndrew Woods" },
    { 0x302, 122.0f, "Commentary_0302.vag", "Nate Fox\nBrian Fleming\nReid Johnson" },
    { 0x303, 138.0f, "Commentary_0303.vag", "Suzanne Kaufmann\nKelle DeForrest\nRob McDaniel" },
    { 0x304,   0.0f, "Commentary_0304.vag", "Chris Bentzel\nKelle DeForrest" },
    { 0x305, 128.0f, "Commentary_0305.vag", "Rob McDaniel\nSuzanne Kaufmann\nAndrew Woods" },
    { 0x306,   0.0f, "Commentary_0306.vag", "Nate Fox\nMatthew Morgaine\nChris Heidorn" },
    { 0x307,   0.0f, "Commentary_0307.vag", "Bruce Oberg\nGary Burd\nChris Bentzel" },

    { 0x400, 129.0f, "Commentary_0400.vag", "Nate Fox\nMatt Olsen" },
    { 0x402, 135.0f, "Commentary_0402.vag", "Rob McDaniel\nDarrell Plank" },
    { 0x403, 145.0f, "Commentary_0403.vag", "Nate Fox\nAndrew Woods" },
    { 0x404, 165.0f, "Commentary_0404.vag", "Chris Bentzel\nIan Stout\nKelle DeForrest" },
    { 0x405,   0.0f, "Commentary_0405.vag", "Chris Bentzel\nRob McDaniel" },
    { 0x406, 162.0f, "Commentary_0406.vag", "Rob McDaniel\nAugie Pagan\nAndrew Woods" },
    { 0x407,   0.0f, "Commentary_0407.vag", "Matthew Morgaine\nChris Zimmerman" },

    { 0x502,   0.0f, "Commentary_0502.vag", "Nate Fox\nChris Zimmerman" },
    { 0x503,   0.0f, "Commentary_0503.vag", "Nate Fox\nMatthew Morgaine" },
    { 0x504,   0.0f, "Commentary_0504.vag", "Matthew Morgaine\nMatt Olsen\nChris Bentzel" },
    { 0x505,   0.0f, "Commentary_0505.vag", "Suzanne Kaufmann\nChris Bentzel" },
    { 0x506,   0.0f, "Commentary_0506.vag", "Rob McDaniel\nDev Madan\nTravis Kotzesbue" }
};

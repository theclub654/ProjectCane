#include "prompt.h"
#include "ui.h"
#include "dialog.h"
#include "wm.h"
#include "sm.h"
#include "credit.h"
#include "coin.h"
#include "pzo.h"
#include "binoc.h"
#include "rog.h"
#include "ctr.h"
#include "screen.h"
#include "gl.h"
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <vector>

namespace
{
GLFWmonitor* ActiveMonitor()
{
    if (GLFWmonitor* monitor = glfwGetWindowMonitor(g_gl.window))
        return monitor;

    int windowX = 0;
    int windowY = 0;
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowPos(g_gl.window, &windowX, &windowY);
    glfwGetWindowSize(g_gl.window, &windowWidth, &windowHeight);

    const int centerX = windowX + windowWidth / 2;
    const int centerY = windowY + windowHeight / 2;
    int monitorCount = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

    for (int i = 0; i < monitorCount; ++i)
    {
        int monitorX = 0;
        int monitorY = 0;
        glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

        if (mode != nullptr && centerX >= monitorX && centerX < monitorX + mode->width &&
            centerY >= monitorY && centerY < monitorY + mode->height)
        {
            return monitors[i];
        }
    }

    return glfwGetPrimaryMonitor();
}

std::vector<int> SupportedFrameRates()
{
    std::vector<int> frameRates;
    GLFWmonitor* monitor = ActiveMonitor();

    if (monitor != nullptr)
    {
        int modeCount = 0;
        const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modeCount);

        for (int i = 0; i < modeCount; ++i)
        {
            if (modes[i].refreshRate >= 30)
                frameRates.push_back(modes[i].refreshRate);
        }
    }

    std::sort(frameRates.begin(), frameRates.end());
    frameRates.erase(std::unique(frameRates.begin(), frameRates.end()), frameRates.end());

    if (frameRates.empty())
        frameRates.push_back(60);

    return frameRates;
}

void SelectNextSupportedFrameRate()
{
    const std::vector<int> frameRates = SupportedFrameRates();
    auto next = std::upper_bound(frameRates.begin(), frameRates.end(), g_targetFrameRate);

    if (next == frameRates.end())
        next = frameRates.begin();

    g_targetFrameRate = *next;
}

void SelectNextInternalResolution()
{
    constexpr int resolutions[] = { 0, 720, 1080, 1440, 2160 };
    auto current = std::find(std::begin(resolutions), std::end(resolutions),
        g_internalResolutionHeight);

    if (current == std::end(resolutions) || ++current == std::end(resolutions))
        current = std::begin(resolutions);

    g_internalResolutionHeight = *current;
    ApplyInternalResolutionSettings();
}

void SelectNextDrawDistance()
{
    if (g_pcm == nullptr)
        return;

    constexpr float multipliers[] = { 0.75f, 1.0f, 1.5f, 2.0f, 3.0f };
    auto next = std::upper_bound(std::begin(multipliers), std::end(multipliers),
        g_pcm->rMRD + 0.001f);

    if (next == std::end(multipliers))
        next = std::begin(multipliers);

    SetCmMrdRatio(g_pcm, *next);
    g_drawDistanceMultiplier = *next;
}

void SelectNextWindowMode()
{
    const int next = (static_cast<int>(g_windowMode) + 1) % 3;
    ApplyWindowModeSettings(static_cast<WindowMode>(next));
}
}

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

    if (FFontLoaded(2))
    {
        pprompt->pte = &g_tePrompt; 
        pprompt->pte->m_pfont = PfontFromFont(2);
    }

    pprompt->prkSaveContext = PRK_Nil;
    pprompt->prk = PRK_Nil;

    for (int i = 0; i < 3; ++i)
        pprompt->mpprpprk[i] = PRK_Nil;

    pprompt->alpha = 1.0f;
}

void HandlePromptPrkTransition(PROMPT* pprompt, PRK prkOld, PRK prkNew)
{
    if (pprompt == nullptr || prkOld == prkNew)
        return;

    if (pprompt->fVagPlaying != 0)
    {
        StopVag();
        pprompt->fVagPlaying = 0;
    }

    switch (prkOld)
    {
        case PRK_MemcardChooseLoadSlot:
        g_note.pvtblot->pfnSetBlotBlots(&g_note, BLOTS_Hidden);
        g_note.pvtblot->pfnSetBlotAchzDraw(&g_note, nullptr);
        break;

        default:
        break;
    }

    switch (prkNew)
    {
        case PRK_Nil:
        g_lifectr.pvtlifectr->pfnHideBlot(&g_lifectr);
        g_cluectr.pvtblot->pfnHideBlot(&g_cluectr);
        g_keyctr.pvtkeyctr->pfnHideBlot(&g_keyctr);
        g_coinctr.pvtcoinctr->pfnHideBlot(&g_coinctr);
        g_percentctr.pvtctr->pfnHideBlot(&g_percentctr);
        break;

        case PRK_MemcardChooseLoadSlot:
        g_note.pvtblot->pfnSetBlotAchzDraw(&g_note, (char*)"&2S&.: Erase Game");
        SetBlotDtVisible(&g_note, 0.0f);
        SetBlotFontScale(&g_note, 0.6f);
        g_note.pvtblot->pfnShowBlot(&g_note);
        break;

        default:
        break;
    }
}

void SetPrompt(PROMPT* pprompt, int prp, PRK prk)
{
    pprompt->mpprpprk[prp] = prk;

    // The last non-nil prompt slot has priority.
    PRK prkNew = PRK_Nil;

    for (int i = 0; i < 3; ++i)
    {
        if (pprompt->mpprpprk[i] != PRK_Nil)
            prkNew = static_cast<PRK>(pprompt->mpprpprk[i]);
    }

    const PRK prkCurrent =
        pprompt->fReshow
        ? pprompt->prkReshow
        : pprompt->prk;

    HandlePromptPrkTransition(pprompt, prkCurrent, prkNew);

    if (prkCurrent != prkNew)
    {
        if (pprompt->blots == BLOTS_Hidden)
            ChangePromptPrk(pprompt, prkNew);
        else if (pprompt->blots > BLOTS_Nil && pprompt->blots < BLOTS_Max)
        {
            if (prkNew == PRK_Nil)
            {
                pprompt->fReshow = 0;
                pprompt->prkReshow = PRK_Nil;
            }
            else
            {
                pprompt->fReshow = 1;
                pprompt->prkReshow = prkNew;

                if (pprompt->pvtblot->pfnSetBlotBlots != nullptr)
                    pprompt->pvtblot->pfnSetBlotBlots(pprompt, BLOTS_Disappearing);
            }
        }
    }

    if (prkNew == PRK_Nil)
    {
        if (pprompt->fActive != 0)
            PopUiActiveBlot(&g_ui);
    }
    else if (pprompt->fActive == 0)
        PushUiActiveBlot(&g_ui, pprompt);
}

SMA* PsmaFindSpecialPromptStateMachine()
{
    SM* psm = (SM*)PloFindSwObject(g_psw, 5, (OID)1068, nullptr);

    if (psm == nullptr)
        return nullptr;

    return psm->dlSma.psmaFirst;
}

void SetPromptPrk(PROMPT* pprompt)
{
    if (pprompt == nullptr || pprompt->pfont == nullptr)
        return;

    const PRK prk = pprompt->prk;

    if (prk < PRK_PauseMenu || prk >= PRK_Max)
        return;

    // -------------------------------------------------------------------------
    // Build the dynamic pause menu
    // -------------------------------------------------------------------------

    if (prk == PRK_PauseMenu)
    {
        PRD& prd = s_mpprkprd[PRK_PauseMenu];
        prd.arespk = s_arespkPauseMenu;
        prd.crespk = 0;

        for (RESPK response : s_arespkPauseMenuAll)
        {
            bool include = true;

            switch (response)
            {
                case RESPK_Map:
                include = g_wmc.pwmCurrent != nullptr;
                break;

                case RESPK_RestartRace:
                {
                    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);
                    include = false;

                    if (levelId == 516 || levelId == 1031)
                    {
                        SMA* psma = PsmaFindSpecialPromptStateMachine();

                        if (psma != nullptr)
                        {
                            int smaState[4]{};
                            GetSmaCur(psma, (OID*)smaState);
                            include = smaState[0] > 1070 && smaState[0] < 1073;
                        }
                    }

                    break;
                }

                case RESPK_RestartSprint:
                include = g_note.mtsState == MTSSTATE_Active;
                break;

                case RESPK_RestartLevel:
                {
                    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);
                    include = levelId == 772 || levelId == 775 || levelId == 1282;
                    break;
                }

                case RESPK_ExitLevel:
                if (g_pgsCur->gameWorldCur == GAMEWORLD_Intro)
                {
                    include = false;
                }
                else if (g_pgsCur->gameWorldCur == GAMEWORLD_Clockwerk ||
                    g_pgsCur->worldLevelCur == WORLDLEVEL_Hub ||
                    (g_pgsCur->worldLevelCur == WORLDLEVEL_Approach && (g_pwsCur->als[1].grfls & 1U) == 0))
                {
                    response = RESPK_ExitToHideout;
                }
                break;

                default:
                break;
            }

            if (include && prd.crespk < 8)
                prd.arespk[prd.crespk++] = response;
        }
    }

    // -------------------------------------------------------------------------
    // Build the dynamic options menu
    // -------------------------------------------------------------------------

    else if (prk == PRK_OptionsMenu)
    {
        PRD& prd = s_mpprkprd[PRK_OptionsMenu];
        prd.arespk = s_arespkOptionsMenu;
        prd.crespk = 0;

        for (RESPK response : s_arespkOptionsMenuAll)
        {
            bool include = true;

            if (response == RESPK_NewGame)
                include = g_ui.uisPlaying == UIS_Attract;
            else if (response == RESPK_SaveGame)
                include = g_ui.uisPlaying != UIS_Attract;

            if (include &&prd.crespk < static_cast<int>(std::size(s_arespkOptionsMenu)))
                prd.arespk[prd.crespk++] = response;
        }
    }

    // -------------------------------------------------------------------------
    // Rebuild dynamic response strings
    // -------------------------------------------------------------------------

    std::snprintf(g_achzRespk9, sizeof(g_achzRespk9), "%s", s_apchzRespk9[g_saveData.pgsCurrentSave != nullptr ? 1 : 0]);

    char* const slotStrings[SAVE_SLOT_COUNT] =
    {
        g_achzRespk10,
        g_achzRespk11,
        g_achzRespk12,
        g_achzSaveSlot4,
        g_achzSaveSlot5,
        g_achzSaveSlot6
    };

    for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
    {
        GS* pgs = &g_saveData.saveData[slot];

        if (pgs->dt != 0.0f)
            BuildSaveSlotText(pgs, slotStrings[slot]);
        else
        {
            const bool choosingSlot = prk == PRK_MemcardChooseNewSlot || prk == PRK_MemcardChooseLoadSlot;
            std::snprintf(slotStrings[slot], 64, "%s", choosingSlot ? "New Game" : "Empty");
        }
    }

    std::snprintf(g_achzRespk13, sizeof(g_achzRespk13), "%s", s_apchzRespk13[FVibrationEnabled(g_pgsCur) ? 0 : 1]);
    std::snprintf(g_achzRespk15, sizeof(g_achzRespk15), "%s", s_apchzRespk15[(g_pgsCur->grfgs & 0x80U) != 0 ? 1 : 0]);
    std::snprintf(g_achzRespk16, sizeof(g_achzRespk16), "%s", s_apchzRespk16[(g_pgsCur->grfgs & 0x40U) != 0 ? 1 : 0]);
    std::snprintf(g_achzRespk24, sizeof(g_achzRespk24), "%s", s_apchzRespk24[(g_pgsCur->grfgs & 0x200U) != 0 ? 1 : 0]);
    std::snprintf(g_achzRespk25, sizeof(g_achzRespk25), "%s", s_apchzRespk25[(g_pgsCur->grfgs & 0x400U) != 0 ? 1 : 0]);
    std::snprintf(g_achzRespk26, sizeof(g_achzRespk26), "%s", s_apchzRespk26[(g_pgsCur->grfgs & 0x800U) != 0 ? 1 : 0]);
    if (g_fMsaa)
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: %dx", g_msaaSamples);
    else
        std::snprintf(g_achzRespk27, sizeof(g_achzRespk27), "MSAA: Off");
    std::snprintf(g_achzRespk28, sizeof(g_achzRespk28), "Frame Rate: %d FPS", g_targetFrameRate);
    if (g_internalResolutionHeight == 0)
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30), "Internal Resolution: Native");
    else if (g_internalResolutionHeight == 2160)
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30), "Internal Resolution: 4K");
    else
        std::snprintf(g_achzRespk30, sizeof(g_achzRespk30), "Internal Resolution: %dp", g_internalResolutionHeight);

    const char* fogName = g_fogType == 2 ? "PS3 Style" : "PS2 Style";
    std::snprintf(g_achzRespk31, sizeof(g_achzRespk31), "Fog: %s", fogName);

    const float drawDistance = g_drawDistanceMultiplier;
    const char* drawDistanceName = nullptr;
    if (drawDistance > 0.749f && drawDistance < 0.751f)
        drawDistanceName = "Low";
    else if (drawDistance > 0.999f && drawDistance < 1.001f)
        drawDistanceName = "Original";
    else if (drawDistance > 1.499f && drawDistance < 1.501f)
        drawDistanceName = "High";
    else if (drawDistance > 1.999f && drawDistance < 2.001f)
        drawDistanceName = "Very High";
    else if (drawDistance > 2.999f && drawDistance < 3.001f)
        drawDistanceName = "Extreme";

    if (drawDistanceName != nullptr)
        std::snprintf(g_achzRespk32, sizeof(g_achzRespk32), "Draw Distance: %s", drawDistanceName);
    else
        std::snprintf(g_achzRespk32, sizeof(g_achzRespk32), "Draw Distance: Custom (%.2fx)", drawDistance);

    constexpr const char* windowModeNames[] = { "Windowed", "Borderless", "Fullscreen" };
    const int windowModeIndex = (std::clamp)(static_cast<int>(g_windowMode), 0, 2);
    std::snprintf(g_achzRespk33, sizeof(g_achzRespk33), "Window Mode: %s",
        windowModeNames[windowModeIndex]);
    std::snprintf(g_achzRespk34, sizeof(g_achzRespk34), "VSync: %s",
        g_fVsync ? "On" : "Off");
    const char* aspectRatioName = "Fit to Screen";
    if (g_gl.aspectMode == Fixed_16_9)
        aspectRatioName = "16:9";
    else if (g_gl.aspectMode == Fixed_16_10)
        aspectRatioName = "16:10";
    else if (g_gl.aspectMode == Fixed_4_3)
        aspectRatioName = "4:3";
    std::snprintf(g_achzRespk35, sizeof(g_achzRespk35), "Aspect Ratio: %s", aspectRatioName);

    // -------------------------------------------------------------------------
    // Choose initial response
    // -------------------------------------------------------------------------

    pprompt->irespk = 0;

    switch (prk)
    {
        case PRK_QuitConfirm:
        case PRK_MemcardFormatConfirm:
        case PRK_MemcardCreateConfirm:
        case PRK_MemcardOverwriteConfirm:
        case PRK_MemcardEraseConfirm:
        pprompt->irespk = 1;
        break;

        case PRK_MemcardChooseSaveSlot:
        case PRK_MemcardChooseLoadSlot:
        for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
        {
            if (g_saveData.pgsCurrentSave == &g_saveData.saveData[slot])
            {
                pprompt->irespk = slot;
                break;
            }
        }
        break;

        case PRK_MemcardChooseNewSlot:
        while (pprompt->irespk < SAVE_SLOT_COUNT &&
            g_saveData.saveData[pprompt->irespk].dt != 0.0f)
            ++pprompt->irespk;
        break;

        case PRK_MemcardAutoNotify:
        if (!FVagPlaying())
        {
            // Extracted equivalent of the original VAG descriptor at 0x0026A118.
            PreloadVag("PromptPrk18.wav");
            ContinueVag();
            pprompt->fVagPlaying = FVagPlaying();
        }
        break;

        default:
        break;
    }

    // -------------------------------------------------------------------------
    // Measure prompt
    // -------------------------------------------------------------------------

    PRD& prd = s_mpprkprd[prk];

    float titleWidth = 0.0f;
    float titleHeight = 0.0f;
    float responseWidth = 0.0f;

    if (prd.pchz != nullptr)
    {
        pprompt->pfont->PushScaling(prd.rScaleTitle, prd.rScaleTitle);
        pprompt->pfont->GetExtents(const_cast<char*>(prd.pchz), &titleWidth, &titleHeight, 500.0f);
        pprompt->pfont->PopScaling();

        titleWidth = std::max(titleWidth, 0.0f);
    }

    pprompt->pfont->PushScaling(prd.rScaleRespk, prd.rScaleRespk);

    const float lineHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    float responseHeight = prd.fVertical != 0 ? lineHeight * static_cast<float>(prd.crespk) : lineHeight;

    if (prd.crespk == 0)
        responseHeight = prd.fVertical != 0 ? 0.0f : lineHeight;

    for (int i = 0; i < prd.crespk; ++i)
    {
        const RESPK response = prd.arespk[i];

        if (response == RESPK_Nil)
            break;

        if (response < RESPK_Yes || response >= RESPK_Max)
            continue;

        const RESPD& responseData = s_arespd[response];
        float itemWidth = 0.0f;

        if (responseData.cpchz <= 0 || responseData.apchz == nullptr)
        {
            CRichText text(const_cast<char*>(AchzFromRespk(response)), pprompt->pfont);
            itemWidth = text.Dx();
        }
        else
        {
            for (int variant = 0; variant < responseData.cpchz; ++variant)
            {
                if (responseData.apchz[variant] == nullptr)
                    continue;

                CRichText text(const_cast<char*>(responseData.apchz[variant]), pprompt->pfont);
                itemWidth = std::max(itemWidth, text.Dx());
            }
        }

        if (prd.fVertical != 0)
            responseWidth = std::max(responseWidth, itemWidth);
        else
        {
            if (i != 0)
            {
                const float spaceWidth = static_cast<float>(pprompt->pfont->m_dxSpaceUnscaled) * pprompt->pfont->m_rxScale;
                responseWidth += spaceWidth * 2.0f;
            }

            responseWidth += itemWidth;
        }
    }

    pprompt->pfont->PopScaling();

    ResizeBlot(pprompt, std::max(titleWidth, responseWidth) + 10.0f, titleHeight + responseHeight);
}

void ChangePromptPrk(PROMPT* pprompt, PRK prk)
{
    if ((pprompt->prk != prk) && (pprompt->prk = prk, prk != PRK_Nil))
        SetPromptPrk(pprompt);
}

void ExecutePrompt(PROMPT* pprompt)
{
    if (pprompt == nullptr || pprompt->prk < PRK_PauseMenu || pprompt->prk >= PRK_Max)
        return;

    const PRD& promptData = s_mpprkprd[pprompt->prk];

    RESPK response = RESPK_Nil;

    if (promptData.arespk != nullptr && pprompt->irespk >= 0 && pprompt->irespk < promptData.crespk)
        response = promptData.arespk[pprompt->irespk];

    if (response == RESPK_Back)
    {
        CancelPrompt(pprompt);
        return;
    }

    const auto openOptionsMenu = [&]()
    {
        SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
    };

    const auto playInvalidSelectionSound = [&]()
    {
        StartSound(static_cast<SFXID>(123), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
    };

    const auto writeSelectedSave = [&]()
    {
        SetPrompt(pprompt, PRP_Memcard,
            SaveCurrentGameToDisk(&g_saveData)
                ? PRK_MemcardSlotSaved
                : PRK_Unknown15);
    };

    const auto getSaveSlotIndex = [](RESPK slotResponse) -> int
    {
        for (int slot = 0; slot < SAVE_SLOT_COUNT; ++slot)
        {
            if (s_arespkSlots[slot] == slotResponse)
                return slot;
        }

        return -1;
    };

    const auto closeSaveManagerPrompt = [&]()
    {
        SetPrompt(pprompt, PRP_Memcard, PRK_Nil);
        SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Closing);

        const PRK basicPrompt = pprompt->mpprpprk[PRP_Basic];

        if (basicPrompt == PRK_MemcardChooseSaveSlot ||
            basicPrompt == PRK_MemcardChooseNewSlot ||
            basicPrompt == PRK_MemcardChooseLoadSlot)
        {
            openOptionsMenu();
        }
    };

    const auto rebuildPromptKeepingSelection = [&]()
    {
        const int selectedResponse = pprompt->irespk;
        SetPromptPrk(pprompt);
        pprompt->irespk = selectedResponse;
    };

    PRP promptGroupToClear = PRP_Basic;

    switch (pprompt->prk)
    {
        case PRK_PauseMenu:
        {
            switch (response)
            {
                case RESPK_ReturnToGame:
                break;

                case RESPK_Map:
                if (g_wmc.pwmCurrent == nullptr)
                {
                    playInvalidSelectionSound();
                    return;
                }

                SetPrompt(pprompt, PRP_Basic, PRK_Nil);
                PushUiActiveBlot(&g_ui, &g_wmc);
                return;

                case RESPK_Options:
                openOptionsMenu();
                return;

                case RESPK_Quit:
                SetPrompt(pprompt, PRP_Basic, PRK_QuitConfirm);
                return;

                case RESPK_RestartRace:
                {
                    SMA* psma = PsmaFindSpecialPromptStateMachine();
                    PO* ppo = PpoCur();

                    if (psma != nullptr && ppo != nullptr && FIsBasicDerivedFrom(ppo, CID_SUV))
                    {
                        int state[4]{};
                        GetSmaCur(psma, (OID*)state);

                        if (state[0] > 1070 && state[0] < 1073)
                        {
                            HandleSuvRaceLoss(reinterpret_cast<SUV*>(ppo));
                            SeekSma(psma, (OID)1070);
                        }
                    }

                    break;
                }

                case RESPK_RestartSprint:
                case RESPK_RestartLevel: 
                ExitOrRestartLevelFromPrompt(pprompt, true, WIPEK_Keyhole);
                return;

                case RESPK_ExitLevel:
                SetPrompt(pprompt, PRP_Basic, PRK_Nil);
                TriggerDefaultExit(true, WIPEK_WorldMap);
                return;

                case RESPK_ExitToHideout:
                {
                    LEVELINFO* plevelHideout = PlevelinfoFromSearchKey(9);

                    if (plevelHideout == nullptr)
                    {
                        std::printf("PROMPT ERROR: Hideout search key 9 not found\n");
                        playInvalidSelectionSound();
                        return;
                    }

                    SetPrompt(pprompt, PRP_Basic, PRK_Nil);
                    WipeToWorldWarp(plevelHideout, OID_Nil, WIPEK_WorldMap);
                    return;
                }

                case RESPK_Controls:
                SetPrompt(pprompt, PRP_Basic, PRK_ControlsMenu);
                return;

                default:
                return;
            }

            break;
        }

        case PRK_VideoMenu:
        switch (response)
        {
            case RESPK_Msaa:
            if (!g_fMsaa)
            {
                g_fMsaa = true;
                g_msaaSamples = 2;
            }
            else if (g_msaaSamples < 4)
                g_msaaSamples = 4;
            else if (g_msaaSamples < 8)
                g_msaaSamples = 8;
            else if (g_msaaSamples < 16)
                g_msaaSamples = 16;
            else
                g_fMsaa = false;

            ApplyMsaaSettings();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_FrameRate:
            SelectNextSupportedFrameRate();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_InternalResolution:
            SelectNextInternalResolution();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_DrawDistance:
            SelectNextDrawDistance();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_WindowMode:
            SelectNextWindowMode();
            rebuildPromptKeepingSelection();
            return;

            case RESPK_Vsync:
            g_fVsync = !g_fVsync;
            glfwSwapInterval(g_fVsync ? 1 : 0);
            rebuildPromptKeepingSelection();
            return;

            case RESPK_AspectRatio:
            switch (g_gl.aspectMode)
            {
                case FitToScreen:
                ApplyAspectRatioSettings(Fixed_16_9);
                break;
                case Fixed_16_9:
                ApplyAspectRatioSettings(Fixed_16_10);
                break;
                case Fixed_16_10:
                ApplyAspectRatioSettings(Fixed_4_3);
                break;
                case Fixed_4_3:
                default:
                ApplyAspectRatioSettings(FitToScreen);
                break;
            }
            rebuildPromptKeepingSelection();
            return;

            case RESPK_Back:
            SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
            return;

            default:
            return;
        }

        case PRK_GameOver:
        case PRK_TryAgain:
        case PRK_MtsExpired:
        case PRK_MtsBestTime:
        case PRK_MtsFailedBestTime:
        ExitOrRestartLevelFromPrompt(pprompt, response == RESPK_Yes, WIPEK_Keyhole);
        return;

        case PRK_QuitConfirm:
        {
            if (response == RESPK_Yes)
            {
                if (g_saveData.pgsCurrentSave == nullptr)
                {
                    WipeToTitleScreen();
                    return;
                }

                pprompt->prkSaveContext = PRK_QuitConfirm;

                if ((g_pgsCur->grfgs & 0x100) == 0)
                    SetPrompt(pprompt, PRP_Memcard, PRK_MemcardAutoNotify);
                else
                {
                    writeSelectedSave();
                }
            }

            break;
        }

        case PRK_MemcardMissing:
        case PRK_MemcardFormatError:
        case PRK_MemcardCreateError:
        case PRK_MemcardCardFull:
        case PRK_Unknown21:
        case PRK_Unknown32:
        closeSaveManagerPrompt();
        return;

        case PRK_MemcardFormatConfirm:
        if (response == RESPK_Yes)
        {
            //StartMemoryCardOperation(2529680, 1622440, 2509376);
            SetPrompt(pprompt, PRP_Memcard, PRK_MemcardFormatting);
            return;
        }

        closeSaveManagerPrompt();
        return;

        case PRK_MemcardCreateConfirm:
        if (response == RESPK_Yes)
        {
            //StartMemoryCardOperation(2529680, 1622608, 2509376);
            SetPrompt(pprompt, PRP_Memcard, PRK_MemcardCreateError);
            return;
        }

        closeSaveManagerPrompt();
        return;

        case PRK_MemcardChooseSaveSlot:
        {
            const int slot = getSaveSlotIndex(response);

            if (slot < 0)
                return;

            GS* selectedSave = &g_saveData.saveData[slot];
            g_saveData.pgsSelectedSave = selectedSave;

            if (selectedSave->dt != 0.0f)
            {
                SetPrompt(pprompt, PRP_Memcard, PRK_MemcardOverwriteConfirm);
                return;
            }

            g_saveData.pgsCurrentSave = selectedSave;
            pprompt->prkSaveContext = PRK_MemcardChooseSaveSlot;

            if ((g_pgsCur->grfgs & 0x100) == 0)
                SetPrompt(pprompt, PRP_Memcard, PRK_MemcardAutoNotify);
            else
            {
                writeSelectedSave();
            }

            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            return;
        }

        case PRK_MemcardSlotSaved:
        promptGroupToClear = PRP_Memcard;
        break;

        case PRK_MemcardOverwriteConfirm:
        if (response != RESPK_Yes)
        {
            promptGroupToClear = PRP_Memcard;
            break;
        }

        g_saveData.pgsCurrentSave = g_saveData.pgsSelectedSave;
        pprompt->prkSaveContext = PRK_MemcardChooseSaveSlot;

        if ((g_pgsCur->grfgs & 0x100) == 0)
            SetPrompt(pprompt, PRP_Memcard, PRK_MemcardAutoNotify);
        else
        {
            writeSelectedSave();
        }

        SetPrompt(pprompt, PRP_Basic, PRK_Nil);
        return;

        case PRK_MemcardChooseNewSlot:
        {
            const int slot = getSaveSlotIndex(response);

            if (slot < 0)
                return;

            if (g_saveData.saveData[slot].dt != 0.0f)
            {
                playInvalidSelectionSound();
                return;
            }

            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            g_saveData.pgsCurrentSave = &g_saveData.saveData[slot];
            InitGameState(g_pgsCur);
            SaveCurrentGameToDisk(&g_saveData);
            ReloadCurrentLevel();
            return;
        }

        case PRK_MemcardChooseLoadSlot:
        {
            const int slot = getSaveSlotIndex(response);

            if (slot < 0)
                return;

            g_saveData.pgsCurrentSave = &g_saveData.saveData[slot];
            if (!LoadCurrentSave(&g_saveData))
            {
                playInvalidSelectionSound();
                return;
            }

            SetPrompt(pprompt, PRP_Basic, PRK_Nil);
            ReloadCurrentLevel();
            return;
        }

        case PRK_MemcardEraseConfirm:
        if (response == RESPK_Yes)
        {
            int slot = -1;
            if (g_saveData.pgsSelectedSave != nullptr)
                slot = static_cast<int>(g_saveData.pgsSelectedSave - g_saveData.saveData);

            if (!DeleteSaveSlotFromDisk(&g_saveData, slot))
            {
                playInvalidSelectionSound();
                return;
            }

            SetPrompt(pprompt, PRP_Memcard, PRK_Nil);
            return;
        }

        promptGroupToClear = PRP_Memcard;
        break;

        case PRK_MemcardAutoNotify:
        {
            g_pgsCur->grfgs |= 0x100;

            switch (pprompt->prkSaveContext)
            {
                case PRK_Nil:
                SetPrompt(pprompt, PRP_Memcard, PRK_Nil);
                g_autosave.pvtblot->pfnShowBlot(&g_autosave);
                break;

                case PRK_QuitConfirm:
                case PRK_MemcardChooseSaveSlot:
                writeSelectedSave();
                break;

                default:
                break;
            }
            return;
        }

        case PRK_OptionsMenu:
        {
            switch (response)
            {
                case RESPK_NewGame:
                SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Selecting);
                SetPrompt(pprompt, PRP_Basic, PRK_MemcardChooseNewSlot);
                return;

                case RESPK_LoadGame:
                SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Selecting);
                SetPrompt(pprompt, PRP_Basic, PRK_MemcardChooseLoadSlot);
                return;

                case RESPK_SaveGame:
                SetSaveManagerState(&g_saveData, SAVE_MENU_STATE_Selecting);
                SetPrompt(pprompt, PRP_Basic, PRK_MemcardChooseSaveSlot);
                return;

                case RESPK_Vibration:
                ToggleVibration(g_pgsCur);
                rebuildPromptKeepingSelection();
                return;

                case RESPK_Music:
                g_pgsCur->grfgs ^= 0x80;
                SetAttractVolume((g_pgsCur->grfgs & 0x80) != 0);
                rebuildPromptKeepingSelection();
                return;
    
                case RESPK_Speakers:
                g_pgsCur->grfgs ^= 0x40;
                SetAttractSoundOption((g_pgsCur->grfgs & 0x40) != 0);
                rebuildPromptKeepingSelection();
                return;

                case RESPK_Fog:
                g_fogType = g_fogType == 1 ? 2 : 1;
                glGlobShader.Use();
                glUniform1i(glslFogType, g_fogType);
                rebuildPromptKeepingSelection();
                return;

                case RESPK_Controls:
                SetPrompt(pprompt, PRP_Basic, PRK_ControlsMenu);
                return;

                case RESPK_Video:
                SetPrompt(pprompt, PRP_Basic, PRK_VideoMenu);
                return;

                default:
                return;
            }
        }

        case PRK_SpecialRetry:
        {
            SMA* psma = pprompt->psma;
            pprompt->psma = nullptr;

            if (psma != nullptr)
            {
                const OID oidGoal = response == RESPK_Yes ? static_cast<OID>(1076) : static_cast<OID>(1077);
                SetSmaGoal(psma, oidGoal);
            }

            break;
        }

        case PRK_MtsComplete:
        if ((GetGameProgress() & 0x08) == 0)
            ExitOrRestartLevelFromPrompt(pprompt, false, WIPEK_WorldMap);
        else
            PlayEndingFromCompletionFlag(8);

        return;

        case PRK_ControlsMenu:
        switch (response)
        {
            case RESPK_BinocInvert:
            g_pgsCur->grfgs ^= 0x200;
            break;

            case RESPK_TurretInvert:
            g_pgsCur->grfgs ^= 0x400;
            break;

            case RESPK_JetpackInvert:
            g_pgsCur->grfgs ^= 0x800;
            break;

            default:
            return;
        }

        rebuildPromptKeepingSelection();
        return;

        default:
        return;
    }

    SetPrompt(pprompt, promptGroupToClear, PRK_Nil);
}

void SetPromptBlots(PROMPT* pprompt, BLOTS blots)
{
    if (pprompt->blots != blots) {
        if ((pprompt->blots == BLOTS_Hidden) && (pprompt->prk == PRK_PauseMenu)) {
            SetPromptPrk(pprompt);
        }
        if (blots == BLOTS_Hidden) {
            if (pprompt->fReshow == 0) {
                if (pprompt->prk == PRK_PauseMenu) {
                    SetPromptPrk(pprompt);
                }
            }
            else {
                blots = BLOTS_Appearing;
                ChangePromptPrk(pprompt, pprompt->prkReshow);
                pprompt->fReshow = 0;
                pprompt->prkReshow = PRK_Nil;
            }
        }
        SetBlotBlots(pprompt, blots);
    }
}

void ShowPrompt(PROMPT* pprompt)
{
    if ((pprompt->blots != BLOTS_Disappearing) || (pprompt->fReshow == 0))
        ShowBlot(pprompt);
}

void HidePrompt(PROMPT* pprompt)
{
    if ((pprompt->blots == BLOTS_Disappearing) && (pprompt->fReshow != 0)) {
        pprompt->fReshow = 0;
        pprompt->prkReshow = (PRK)-1;
    }
    else
        HideBlot(pprompt);
}

const char* AchzFromRespk(RESPK respk)
{
    return s_mprespkachz[respk];
}

void BuildSaveSlotText(GS* pgs, char* pchzDst)
{
    int percentComplete = CalculatePercentCompletion(pgs);
    const char* pchzLevel = PchzFriendlyFromLevelId((pgs->gameWorldCur << 8) | pgs->worldLevelCur);
    int totalSeconds = std::max(static_cast<int>(pgs->dt / 60.0f), 1);

    std::snprintf(pchzDst, 64, "%d%% - %s - %d:%02d", percentComplete, pchzLevel, totalSeconds / 60, totalSeconds % 60);
}

void OnPromptActive(PROMPT* pprompt, int fActive)
{
    if (pprompt->fActive == fActive)
        return;

    const int levelId = (g_pgsCur->gameWorldCur << 8) | g_pgsCur->worldLevelCur;

    if (fActive == 0)
    {
        if (levelId == 0x0308)
        {
            ResumeVag();
            ContinueMusicSequencer();
        }

        if (levelId != 0x0308 &&
            g_binoc.pdialogPlaying == nullptr &&
            pprompt->prk == PRK_PauseMenu)
        {
            SetUiUis(&g_ui, UIS_Unpausing);
        }
        else
            SetUiUis(&g_ui, UIS_Playing);

        HandlePromptPrkTransition(pprompt, pprompt->prk, PRK_Nil);
        pprompt->pvtblot->pfnHideBlot(pprompt);

        RemoveGrfusr(1);

        pprompt->fActive = 0;
        return;
    }

    if (levelId == 0x0308)
    {
        PauseVag();
        PauseMusicSequencer();
        SetUiUis(&g_ui, UIS_Paused);
    }
    else if (g_binoc.pdialogPlaying == nullptr)
    {
        if (g_ui.uis == UIS_Playing)
            SetUiUis(&g_ui, UIS_Pausing);
        else
            SetUiUis(&g_ui, UIS_Paused);
    }
    else
        SetUiUis(&g_ui, UIS_Paused);

    HandlePromptPrkTransition(pprompt, PRK_Nil, pprompt->prk);
    pprompt->pvtblot->pfnShowBlot(pprompt);

    AddGrfusr(1);
    g_joy.StartJoySelection();

    pprompt->fActive = 1;
}

void ExitOrRestartLevelFromPrompt(PROMPT* pprompt, int fRestartLevel, WIPEK wipek)
{
    if (pprompt == nullptr || g_pgsCur == nullptr)
        return;

    SetPrompt(pprompt, PRP_Basic, PRK_Nil);

    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);

    if (levelId == 0x0106)
    {
        if (fRestartLevel == 0)
        {
            ROB* prob = static_cast<ROB*>(PloFindSwObjectByClass(g_psw, 5, CID_ROB, nullptr));

            if (prob != nullptr)
                SetRobRobs(prob, ROBS_Max);
        }
        else
        {
            LEVELINFO* plevelCurrent = g_transition.m_plevelCurrent;

            if (plevelCurrent == nullptr && !g_transition.m_worldCurrent.empty())
                plevelCurrent = PlevelinfoFromLevelName(g_transition.m_worldCurrent);

            if (plevelCurrent == nullptr)
            {
                std::printf("RESTART LEVEL ERROR: no current level\n");
                return;
            }

            TRANS trans{};
            trans.fSet = 1;
            trans.plevel = plevelCurrent;
            trans.oidWarp = OID_Nil;
            trans.oidWarpContext = OID_Nil;
            trans.grftrans = static_cast<GRFTRANS>(0);

            ActivateWipe(&g_wipe, &trans, wipek);
        }

        return;
    }

    if (fRestartLevel == 0)
    {
        TriggerDefaultExit(0, wipek);
        return;
    }

    LEVELINFO* plevelCurrent = g_transition.m_plevelCurrent;

    if (plevelCurrent == nullptr && !g_transition.m_worldCurrent.empty())
        plevelCurrent = PlevelinfoFromLevelName(g_transition.m_worldCurrent);

    if (plevelCurrent == nullptr)
    {
        std::printf("RESTART LEVEL ERROR: no current level\n");
        return;
    }

    TRANS trans{};
    trans.fSet = 1;
    trans.plevel = plevelCurrent;
    trans.oidWarp = OID_Nil;
    trans.oidWarpContext = OID_Nil;
    trans.grftrans = static_cast<GRFTRANS>(0x10);

    ActivateWipe(&g_wipe, &trans, wipek);
}

void WipeToTitleScreen()
{
    LEVELINFO* plevelSplash = PlevelinfoFromSearchKey(21);

    if (plevelSplash == nullptr)
    {
        std::printf("TITLE SCREEN ERROR: Splash level not found\n");
        return;
    }

    TRANS trans{};
    trans.fSet = 1;
    trans.plevel = plevelSplash;
    trans.oidWarp = OID_Nil;
    trans.oidWarpContext = OID_Nil;
    trans.grftrans = static_cast<GRFTRANS>(0x04);

    ActivateWipe(&g_wipe, &trans, WIPEK_Fade);
}

void UpdatePromptActive(PROMPT* pprompt, JOY* pjoy)
{
    constexpr uint16_t kBtnConfirm = 0x0040;
    constexpr uint16_t kBtnErase = 0x0080;
    constexpr uint16_t kBtnCancel = 0x0810;

    if (pprompt == nullptr || pjoy == nullptr || g_pgsCur == nullptr)
        return;

    const PRK prk = pprompt->prk;

    if (prk < PRK_PauseMenu || prk >= PRK_Max)
        return;

    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);
    const PRD& promptData = s_mpprkprd[prk];

    const bool isHudExcludedLevel =
        levelId == 262 ||
        levelId == 516 ||
        levelId == 772 ||
        levelId == 775 ||
        levelId == 1031 ||
        levelId == 1282 ||
        levelId == 1284;

    const bool showGameplayCounters = prk == PRK_PauseMenu && !isHudExcludedLevel;

    if (showGameplayCounters)
    {
        g_lifectr.pvtlifectr->pfnShowBlot(&g_lifectr);
        g_cluectr.pvtblot->pfnShowBlot(&g_cluectr);
        g_keyctr.pvtkeyctr->pfnShowBlot(&g_keyctr);
        g_coinctr.pvtcoinctr->pfnShowBlot(&g_coinctr);
    }
    else
    {
        g_lifectr.pvtlifectr->pfnHideBlot(&g_lifectr);
        g_cluectr.pvtblot->pfnHideBlot(&g_cluectr);
        g_keyctr.pvtkeyctr->pfnHideBlot(&g_keyctr);
        g_coinctr.pvtcoinctr->pfnHideBlot(&g_coinctr);
    }

    const bool showPercentCompletion =
        prk == PRK_PauseMenu ||
        prk == PRK_MemcardChooseSaveSlot ||
        prk == PRK_MemcardChooseLoadSlot;

    if (showPercentCompletion)
    {
        g_percentCompletion = CalculatePercentCompletion(g_pgsCur);
        g_percentctr.pnActual = &g_percentCompletion;
        g_percentctr.pvtctr->pfnShowBlot(&g_percentctr);
    }
    else
        g_percentctr.pvtctr->pfnHideBlot(&g_percentctr);

    if (pprompt->fVagPlaying != 0)
        pprompt->fVagPlaying = FVagPlaying();

    const bool canAcceptInput =
        (pprompt->fVagPlaying == 0 || prk == PRK_MemcardAutoNotify) &&
        pprompt->fReshow == 0 &&
        pprompt->blots == BLOTS_Visible &&
        g_clock.tReal - pprompt->tBlots > 0.1f;

    if (promptData.crespk > 0)
    {
        const int selectionDelta = promptData.fVertical != 0 ? pjoy->DySelectionJoy(g_clock.tReal) : pjoy->DxSelectionJoy(g_clock.tReal);

        if (selectionDelta > 0)
        {
            pprompt->irespk = (pprompt->irespk + 1) % promptData.crespk;
            StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            return;
        }

        if (selectionDelta < 0)
        {
            pprompt->irespk = (pprompt->irespk + promptData.crespk - 1) % promptData.crespk;
            StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            return;
        }
    }

    if (!canAcceptInput)
        return;

    if (pjoy->IsPressed(BTN_START) || pjoy->IsPressed(BTN_TRIANGLE))
    {
        pjoy->SetHandled(BTN_START);
        pjoy->SetHandled(BTN_TRIANGLE);
        CancelPrompt(pprompt);
        return;
    }

    if (pjoy->IsPressed(BTN_SQUARE))
    {
        pjoy->SetHandled(BTN_SQUARE);

        if (prk == PRK_MemcardChooseLoadSlot &&
            promptData.arespk != nullptr &&
            pprompt->irespk >= 0 && pprompt->irespk < SAVE_SLOT_COUNT)
        {
            const RESPK response = promptData.arespk[pprompt->irespk];
            int slot = -1;

            for (int i = 0; i < SAVE_SLOT_COUNT; ++i)
            {
                if (s_arespkSlots[i] == response)
                {
                    slot = i;
                    break;
                }
            }

            if (slot >= 0 && g_saveData.saveData[slot].dt != 0.0f)
            {
                g_saveData.pgsSelectedSave = &g_saveData.saveData[slot];
                SetPrompt(pprompt, PRP_Memcard, PRK_MemcardEraseConfirm);
            }
        }

        return;
    }

    if (pjoy->IsPressed(BTN_CROSS))
    {
        // Consume the confirmation before ExecutePrompt can close the menu or
        // reload a level. Otherwise JT sees this same press as a jump.
        pjoy->SetHandled(BTN_CROSS);
        ExecutePrompt(pprompt);
        return;
    }
}

void CancelPrompt(PROMPT* pprompt)
{
    if (pprompt == nullptr)
        return;

    switch (pprompt->prk)
    {
        case PRK_PauseMenu:
        SetPrompt(pprompt, PRP_Basic, PRK_Nil);
        break;

        case PRK_QuitConfirm:
        SetPrompt(pprompt, PRP_Basic, PRK_PauseMenu);
        break;

        case PRK_MemcardChooseSaveSlot:
        case PRK_MemcardChooseNewSlot:
        case PRK_MemcardChooseLoadSlot:
        case PRK_ControlsMenu:
        case PRK_VideoMenu:
        SetPrompt(pprompt, PRP_Basic, PRK_OptionsMenu);
        break;

        case PRK_MemcardSlotSaved:
        case PRK_MemcardOverwriteConfirm:
        case PRK_MemcardEraseConfirm:
        SetPrompt(pprompt, PRP_Memcard, PRK_Nil);
        break;

        case PRK_OptionsMenu:
        {
            const unsigned playingState = static_cast<unsigned>(static_cast<int>(g_ui.uisPlaying) + static_cast<int>(UIS_Nil));

            if (playingState > 1U)
                SetPrompt(pprompt, PRP_Basic, PRK_PauseMenu);
            else
                SetPrompt(pprompt, PRP_Basic, PRK_Nil);

            break;
        }

        default:
        break;
    }
}

void DrawPrompt(PROMPT* pprompt)
{
    if (pprompt == nullptr || pprompt->pfont == nullptr || pprompt->blots == BLOTS_Hidden || pprompt->uOn <= 0.0f)
        return;

    if (pprompt->prk < PRK_PauseMenu || pprompt->prk >= PRK_Max)
        return;

    PRD& prd = s_mpprkprd[pprompt->prk];

    const float uOn = pprompt->uOn;
    const float dx = pprompt->dx * uOn;
    const float dy = pprompt->dy * uOn;

    const float centerX = pprompt->xOn + pprompt->dx * 0.5f;
    const float centerY = pprompt->yOn + pprompt->dy * 0.5f;

    const float x = pprompt->xOn * uOn + centerX * (1.0f - uOn);
    float y = pprompt->yOn * uOn + centerY * (1.0f - uOn);

    const float pulse = std::sin(g_clock.tReal * 10.0f) * 0.5f + 0.5f;
    const float pulseSquared = pulse * pulse;

    float targetAlpha = 1.0f;

    if (pprompt->prk == PRK_PauseMenu && g_ui.cpblotActive == 2)
        targetAlpha = 1.0f - std::clamp(g_joy.uDeflect2, 0.0f, 1.0f);

    const float alphaBlend = 1.0f - std::exp(-12.0f * g_clock.dtReal);
    pprompt->alpha = glm::mix(pprompt->alpha, targetAlpha, std::clamp(alphaBlend, 0.0f, 1.0f));

    const float alpha = std::clamp(pprompt->alpha, 0.0f, 1.0f);

    glm::vec4 edgeBoxColor(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, alpha);
    glm::vec4 edgePulseTarget(111.0f / 255.0f, 111.0f / 255.0f, 111.0f / 255.0f, alpha);
    glm::vec4 titleColor(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, alpha);
    glm::vec4 responseColor(95.0f / 255.0f, 95.0f / 255.0f, 95.0f / 255.0f, alpha);

    const auto GetSelectedColor = [&](float pulseValue)
        {
            const float blue = (47.0f + pulseValue * 64.0f) / 255.0f;
            return glm::vec4(111.0f / 255.0f, 111.0f / 255.0f, blue, alpha);
        };

    CTextBox textBox;

    // -------------------------------------------------------------------------
    // Background/edge rectangle
    // -------------------------------------------------------------------------

    if (pprompt->pte != nullptr)
    {
        textBox.SetPos(x, y);
        textBox.SetSize(dx, dy);
        textBox.SetTextColor(&edgeBoxColor);
        textBox.SetHorizontalJust(JH_Left);
        textBox.SetVerticalJust(JV_Top);

        const glm::vec4 savedEdgeColor = pprompt->pte->m_rgba;

        glm::vec4 edgeColor = savedEdgeColor;
        edgeColor.a = alpha;

        if (prd.crespk == 0)
            edgeColor = glm::mix(edgeColor, edgePulseTarget, pulseSquared);

        pprompt->pte->m_rgba = edgeColor;

        if (pprompt->pte->m_pfont != nullptr)
            pprompt->pte->m_pfont->EdgeRect(pprompt->pte, &textBox);

        pprompt->pte->m_rgba = savedEdgeColor;
    }

    // -------------------------------------------------------------------------
    // Prompt title
    // -------------------------------------------------------------------------

    if (prd.pchz != nullptr)
    {
        const float titleScale = prd.rScaleTitle * uOn;

        pprompt->pfont->PushScaling(titleScale, titleScale);

        textBox.SetPos(x, y);
        textBox.SetSize(dx, dy);
        textBox.SetTextColor(&titleColor);
        textBox.SetHorizontalJust(JH_Center);
        textBox.SetVerticalJust(JV_Top);

        CRichText titleText(const_cast<char*>(prd.pchz), pprompt->pfont);

        titleText.Draw(&textBox, nullptr);
        y += titleText.DyWrap(500.0f);

        pprompt->pfont->PopScaling();
    }

    const float dividerY = y - 1.0f;

    // -------------------------------------------------------------------------
    // Responses
    // -------------------------------------------------------------------------

    const float responseScale = prd.rScaleRespk * uOn;

    pprompt->pfont->PushScaling(responseScale, responseScale);

    const float baseLineHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;
    const float spaceWidth = static_cast<float>(pprompt->pfont->m_dxSpaceUnscaled) * pprompt->pfont->m_rxScale;

    float cursorX = x;
    float cursorY = y;

    // Measure the complete horizontal response row.
    if (prd.fVertical == 0)
    {
        float totalWidth = 0.0f;

        for (int i = 0; i < prd.crespk; ++i)
        {
            if (i > 0)
                totalWidth += spaceWidth * 2.0f;

            const RESPK response = prd.arespk[i];

            if (response == RESPK_Nil)
                continue;

            const char* responseText = AchzFromRespk(response);

            if (responseText == nullptr)
                continue;

            CRichText richText(const_cast<char*>(responseText), pprompt->pfont);

            totalWidth += richText.Dx();
        }

        cursorX = x + (dx - totalWidth) * 0.5f;
    }

    for (int i = 0; i < prd.crespk; ++i)
    {
        const RESPK response = prd.arespk[i];

        if (response == RESPK_Nil)
            continue;

        const char* responseText = AchzFromRespk(response);

        if (responseText == nullptr)
            continue;

        // Measure the item at its normal response scale.
        CRichText unselectedText(const_cast<char*>(responseText), pprompt->pfont);

        const float unselectedWidth = unselectedText.Dx();
        const bool selected = i == pprompt->irespk;

        glm::vec4 color = selected ? GetSelectedColor(pulse) : responseColor;
        const float selectionScale = selected ? 1.0f + (1.0f - pulse) * 0.1f : 1.0f;
        const float itemScale = responseScale * selectionScale;

        pprompt->pfont->PushScaling(itemScale, itemScale);

        // Reconstruct after changing the scale so switched fonts inherit it.
        CRichText selectedText(const_cast<char*>(responseText), pprompt->pfont);

        const float selectedWidth = selectedText.Dx();
        const float selectedHeight = static_cast<float>(pprompt->pfont->m_dyUnscaled) * pprompt->pfont->m_ryScale;

        float drawX = cursorX + (unselectedWidth - selectedWidth) * 0.5f;
        const float drawY = cursorY + (baseLineHeight - selectedHeight) * 0.5f;

        if (prd.fVertical != 0)
            drawX = x + (dx - unselectedWidth) * 0.5f + (unselectedWidth - selectedWidth) * 0.5f;

        textBox.SetPos(drawX, drawY);
        textBox.SetSize(dx, selectedHeight);
        textBox.SetTextColor(&color);
        textBox.SetHorizontalJust(JH_Left);
        textBox.SetVerticalJust(JV_Top);

        selectedText.Draw(&textBox, nullptr);

        pprompt->pfont->PopScaling();

        if (prd.fVertical == 0)
            cursorX += unselectedWidth + spaceWidth * 2.0f;
        else
            cursorY += baseLineHeight;
    }

    pprompt->pfont->PopScaling();

    // -------------------------------------------------------------------------
    // Title/response divider
    // -------------------------------------------------------------------------

    if (prd.pchz != nullptr && prd.crespk > 0)
    {
        const glm::vec4 dividerColor(95.0f / 255.0f, 95.0f / 255.0f, 95.0f / 255.0f, alpha);

        DrawLineScreen(x, dividerY, 0.0f, x + dx, dividerY, 0.0f, dividerColor, false);
    }
}

char GetAnimatedPromptCharacter()
{
    constexpr int frameCount = static_cast<int>(sizeof(s_achzAnimatedPromptFrames) - 1);

    const int frame =
        static_cast<int>(g_clock.tReal * 5.0f) % frameCount;

    return s_achzAnimatedPromptFrames[frame];
}

PROMPT g_prompt;
CTextEdge g_tePrompt;
float g_promptFade;

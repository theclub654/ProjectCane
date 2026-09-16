#include "hubsel.h"
#include "hubblot.h"
#include "dialog.h"
#include "sm.h"
#include "totals.h"

HUBSEL* NewHubSel()
{
    return new HUBSEL{};
}

void CloneHubSel(HUBSEL* phubsel, HUBSEL* phubselBase)
{
    CloneAlo(phubsel, phubselBase);
}

void PostHubSelLoad(HUBSEL* phubsel)
{
    PostAloLoad(phubsel);
    phubsel->fNoFreeze = true;
    g_ui.uisPlaying = UIS_Hub;
    g_hubblot.phubsel = phubsel;
    SetUiUis(&g_ui, UIS_Hub);
    SnipAloObjects(phubsel, 18, s_asnipHubsel);
    phubsel->gameWorldSelected = GAMEWORLD_Nil;
    PostSwCallback(phubsel->psw, (PFNMQ)SetupHubsel, phubsel, MSGID_callback, 0);
}

void UpdateHubselUi(HUBSEL* phubsel)
{
    bool fShowUnavailableHint = false;
    const char* achzTotals = nullptr;

    if (phubsel->psmaWorld->ismsCur == phubsel->psmaWorld->ismsGoal)
    {
        const GAMEWORLD gameWorld = phubsel->gameWorldSelected;

        if (gameWorld == GAMEWORLD_Intro)
            achzTotals = "Bentley";
        else if ((g_pgsCur->aws[gameWorld].fws & FWS_Visited) != 0)
            SetTotalsWorldText(&g_totals, gameWorld);
        else if (gameWorld != phubsel->gameWorldDisplay)
        {
            fShowUnavailableHint = true;
            achzTotals = "";
        }
        else
            achzTotals = "Murray";
    }
    else
        achzTotals = "";

    if (achzTotals != nullptr)
        g_totals.pvtblot->pfnSetBlotAchzDraw(&g_totals, (char*)achzTotals);

    SetBlotDtVisible(&g_totals, 0.0);
    g_totals.fDrawOverLetterbox = 0;
    g_totals.pvtblot->pfnShowBlot(&g_totals);

    if (fShowUnavailableHint)
    {
        g_hubblot.pvtblot->pfnSetBlotAchzDraw(&g_hubblot, (char*)"Press &2X&. to Start Mission!");
        g_hubblot.pvtblot->pfnShowBlot(&g_hubblot);
    }
    else
        g_hubblot.pvtblot->pfnHideBlot(&g_hubblot);
}

void UpdateHubselSelection(HUBSEL* phubsel, JOY* pjoy)
{
    GAMEWORLD gameWorld = phubsel->gameWorldSelected;
    const GAMEWORLD gameWorldMax = phubsel->gameWorldMax;

    const int dxSelection = pjoy->DxSelectionJoy(g_clock.tReal);
    const int dySelection = pjoy->DySelectionJoy(g_clock.tReal);

    const int selection =
        dxSelection != 0
        ? dxSelection
        : dySelection;

    if (selection < 0)
    {
        if (gameWorld >= GAMEWORLD_Underwater)
            gameWorld = static_cast<GAMEWORLD>(gameWorld - 1);
        else if (gameWorldMax == GAMEWORLD_Clockwerk)
            gameWorld = GAMEWORLD_Clockwerk;
    }
    else if (selection > 0)
    {
        if (gameWorld < gameWorldMax)
            gameWorld = static_cast<GAMEWORLD>(gameWorld + 1);
        else if (gameWorldMax == GAMEWORLD_Clockwerk)
            gameWorld = GAMEWORLD_Intro;
    }

    SetHubselSelection(phubsel, gameWorld, 0);
}

void ConfirmHubselSelection(HUBSEL* phubsel)
{
    if (phubsel == nullptr || g_pgsCur == nullptr)
        return;

    const GAMEWORLD gameWorld = phubsel->gameWorldSelected;

	if (gameWorld == GAMEWORLD_Intro || (g_pgsCur->aws[gameWorld].fws & FWS_Visited) != 0)
    {
        SetHubselSelection(phubsel, gameWorld, 1);
        return;
    }

    if (gameWorld == phubsel->gameWorldDisplay)
    {
        if (phubsel->psmaDisplay1 != nullptr)
            SetSmaGoal(phubsel->psmaDisplay1, (OID)g_aoidHubselState[gameWorld][1]);

        return;
    }

    UnlockWorldCutscene(gameWorld);

    bool fCloseHub = true;
    ASEG* pasegWorld = phubsel->apasegWorld[gameWorld];

    if (pasegWorld != nullptr)
    {
        ApplyAseg(pasegWorld, phubsel, 0.0f, 1.0f, 0, nullptr);

        if (FWipingAseg(pasegWorld) != 0)
        {
            PopUiActiveBlot(&g_ui);
            return;
        }
    }

    LEVELINFO* plevel = GetLevelInfo(static_cast<int>(gameWorld) << 8);

    if (plevel != nullptr)
        WipeToWorldWarp(plevel, OID_Nil, WIPEK_Fade);
    else
        fCloseHub = false;

    if (fCloseHub)
        PopUiActiveBlot(&g_ui);
}
bool FHubselWorldAvailable(GAMEWORLD gameWorld)
{
    if (gameWorld == GAMEWORLD_Intro)
        return true;

    if (gameWorld < GAMEWORLD_Underwater || gameWorld > GAMEWORLD_Clockwerk)
        return false;

    return g_fUnlockAllHubWorlds || (g_pgsCur->aws[gameWorld].fws & FWS_Visited) != 0;
}

void SetupHubsel(HUBSEL* phubsel)
{
    if (phubsel == nullptr || g_pgsCur == nullptr)
        return;

    if (phubsel->psmWorld != nullptr)
        phubsel->psmaWorld = PsmaApplySm(phubsel->psmWorld, nullptr, (OID)-1, 0);

    if (phubsel->psmSecondary != nullptr)
        phubsel->psmaSecondary = PsmaApplySm(phubsel->psmSecondary, nullptr, (OID)-1, 0);

    if (phubsel->psmDisplay0 != nullptr)
        phubsel->psmaDisplay0 = PsmaApplySm(phubsel->psmDisplay0, nullptr, (OID)-1, 0);

    if (phubsel->psmDisplay1 != nullptr)
        phubsel->psmaDisplay1 = PsmaApplySm(phubsel->psmDisplay1, nullptr, (OID)-1, 1);

    phubsel->gameWorldMax = GAMEWORLD_Clockwerk;

    // Preserve the original progression calculation when the override is off.
    if (!g_fUnlockAllHubWorlds)
    {
        while (phubsel->gameWorldMax > GAMEWORLD_Muggshot)
        {
            const GAMEWORLD previousWorld = static_cast<GAMEWORLD>(phubsel->gameWorldMax - 1);
            const GAMEWORLD worldBeforePrevious = static_cast<GAMEWORLD>(phubsel->gameWorldMax - 2);
            const uint32_t previousWorldFlags = g_pgsCur->aws[previousWorld].fws;
            const uint32_t worldBeforePreviousFlags = g_pgsCur->aws[worldBeforePrevious].fws;

            if ((worldBeforePreviousFlags & FWS_Complete) != 0 ||
                (previousWorldFlags & FWS_Visited) != 0)
                break;

            phubsel->gameWorldMax = previousWorld;
        }
    }

    for (GAMEWORLD gameWorld = GAMEWORLD_Underwater; gameWorld <= phubsel->gameWorldMax; gameWorld = static_cast<GAMEWORLD>(gameWorld + 1))
    {
        const int worldIndex = static_cast<int>(gameWorld) - static_cast<int>(GAMEWORLD_Underwater);
        ALO* paloWorld = phubsel->apaloWorld[worldIndex];

        if (paloWorld != nullptr && FHubselWorldAvailable(gameWorld))
            paloWorld->pvtlo->pfnAddLo((LO*)paloWorld);
    }

    phubsel->pdialogPending = nullptr;

    const uint32_t gameProgress = GetGameProgress();

    for (int i = 0; i < 4; ++i)
    {
        DIALOG* pdialog = phubsel->apdialogProgress[i];

        if (pdialog == nullptr || pdialog->pfPlayed == nullptr || *pdialog->pfPlayed != 0)
            continue;

        if (i == 0 || (gameProgress & (1U << i)) != 0)
        {
            phubsel->pdialogPending = pdialog;
            break;
        }
    }

    GAMEWORLD gameWorldSelected = GAMEWORLD_Intro;

    if (phubsel->pdialogPending == nullptr)
    {
        if (g_gameWorldPrev != GAMEWORLD_Nil && FHubselWorldAvailable(g_gameWorldPrev))
            gameWorldSelected = g_gameWorldPrev;
        else
        {
            gameWorldSelected = phubsel->gameWorldMax;

            if (!g_fUnlockAllHubWorlds)
            {
                const GAMEWORLD previousWorld = static_cast<GAMEWORLD>(gameWorldSelected - 1);

                if (!FHubselWorldAvailable(previousWorld))
                    gameWorldSelected = static_cast<GAMEWORLD>(gameWorldSelected - 2);
                else if (!FHubselWorldAvailable(gameWorldSelected))
                    gameWorldSelected = previousWorld;
            }
        }
    }

    SetHubselSelection(phubsel, gameWorldSelected, 0);

    if (phubsel->psmaDisplay0 != nullptr || phubsel->psmaDisplay1 != nullptr)
    {
        phubsel->gameWorldDisplay = phubsel->gameWorldMax;

        if (phubsel->gameWorldMax == GAMEWORLD_Clockwerk)
        {
            const bool clockwerkPrerequisiteComplete = (g_pgsCur->aws[GAMEWORLD_Snow].fws & 32U) != 0;
            const bool clockwerkStarted = (g_pgsCur->aws[GAMEWORLD_Clockwerk].fws & 1U) != 0;

            if (g_fUnlockAllHubWorlds || clockwerkPrerequisiteComplete || clockwerkStarted)
                phubsel->gameWorldDisplay = GAMEWORLD_Intro;
        }

        if (phubsel->psmaDisplay0 != nullptr)
            SeekSma(phubsel->psmaDisplay0, (OID)g_aoidHubselState[phubsel->gameWorldDisplay][0]);

        if (phubsel->psmaDisplay1 != nullptr)
            SeekSma(phubsel->psmaDisplay1, (OID)g_aoidHubselState[phubsel->gameWorldDisplay][0]);
    }

    PushUiActiveBlot(&g_ui, (BLOT*)&g_hubblot);
}

void SetHubselSelection(HUBSEL* phubsel, GAMEWORLD gameworld, int fAccepted)
{
    if (gameworld == phubsel->gameWorldSelected && fAccepted == phubsel->fSelectionAccepted)
        return;

    //VAGDESC* pvag = nullptr;

    if (gameworld != phubsel->gameWorldDisplay && gameworld >= GAMEWORLD_Underwater)
    {
        const bool fWorldAvailable = (g_pgsCur->aws[gameworld].fws & FWS_Visited) != 0;

        if (!fWorldAvailable)
        {
            if (g_clock.t - g_hubblot.tHintLast >= g_dtHubHint)
            {
                g_hubblot.tHintLast = g_clock.t;
                //pvag = &g_avagHubHint[gameworld];
            }
        }
        else if ((g_pgsCur->grfgs & 2U) == 0)
        {
            g_pgsCur->grfgs |= 2U;
            //pvag = &g_vagHubUnlocked;
        }

        /*if (pvag != nullptr)
        {
            PreloadVag(pvag);
            g_hubblot.fWaitVag = FVagPlaying();
        }*/
    }

    SMA* psma = phubsel->psmaWorld;

    if (psma != nullptr)
    {
        if (phubsel->gameWorldSelected == GAMEWORLD_Nil)
        {
            if (gameworld != GAMEWORLD_Intro)
            {
                SeekSma(psma, (OID)g_aoidHubselState[gameworld][1]);
                SetSmaGoal(psma, (OID)g_aoidHubselState[gameworld][0]);
            }
        }
        else
        {
            const OID oidGoal = static_cast<OID>(g_aoidHubselState[gameworld][fAccepted]);
            SetSmaGoal(psma, oidGoal);
        }
    }

    phubsel->gameWorldSelected = gameworld;
    phubsel->fSelectionAccepted = fAccepted;
    phubsel->tSelectionChange = g_clock.t;
}

void UnlockAllHubWorlds()
{
    if (g_pgsCur == nullptr)
        return;

    for (int iWorld = 0; iWorld < 6; ++iWorld)
        g_pgsCur->aws[iWorld].fws = FWS_Visited;
}

void DeleteHubSel(HUBSEL* phubsel)
{
    delete phubsel;
}

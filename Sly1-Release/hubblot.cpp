#include "hubblot.h"
#include "hubsel.h"
#include "sm.h"
#include "binoc.h"
#include "wm.h"
#include "fmv.h"
#include "totals.h"

void StartupHubBlot(HUBBLOT* phublot)
{
	g_teHubBlot = {};
	g_teHubBlot.m_rgba = glm::vec4(0.0f, 75.0f / 255.0f, 125.0f / 255.0f, 1.0f);
	g_teHubBlot.m_ch = '-';
	g_teHubBlot.m_dxExtra = 2.0f;
	g_teHubBlot.m_dyExtra = 1.0f;
	g_teHubBlot.m_rxScaling = 0.3f;
	g_teHubBlot.m_ryScaling = 0.3f;

	phublot->pvthubblot = &g_vthubblot;
}

void PostHubBlotLoad(HUBBLOT* phubblot)
{
	PostBlotLoad(phubblot);

	if (FFontLoaded(2))
	{
		phubblot->pte = &g_teHubBlot;
		phubblot->pte->m_pfont = PfontFromFont(2);
	}

	SetBlotDtVisible(phubblot, 0.0f);

	phubblot->phubsel = nullptr;

	phubblot->rgba = glm::vec4(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 223.0f / 255.0f);

	phubblot->fWaitVag = 0;
	phubblot->tHintLast = -g_dtHubHint;
}

void EnableHubCamera(HUBBLOT* phubblot)
{
	if (!phubblot->fActive)
		return;

	CAMERA* pcamera = (CAMERA*)PloFindSwNearest(g_psw, (OID)1277, nullptr);

	if (pcamera != nullptr)
		EnableCamera(pcamera);
}

void UpdateHubBlotActive(HUBBLOT* phubblot, JOY* pjoy)
{
    if (phubblot->phubsel == nullptr || phubblot->phubsel->psmaWorld == nullptr)
        return;

    /*if (phubblot->fWaitVag != 0)
    {
        if (FPauseForVag() != 0)
            snd_FlushSoundCommands();

        phubblot->fWaitVag = FVagPlaying();
    }*/

    if (g_letterbox.blots != BLOTS_Hidden || g_binoc.pdialogPlaying != nullptr)
        return;

    HUBSEL* phubsel = phubblot->phubsel;
    SMA* psmaWorld = phubsel->psmaWorld;

    if (phubsel->pdialogPending != nullptr)
    {
        if (phubsel->pdialogPending == phubsel->apdialogProgress[0])
            AutosaveCurrentGame(&g_saveData);

        TriggerDialog(phubsel->pdialogPending);
        phubsel->pdialogPending = nullptr;
    }

    UpdateHubselUi(phubsel);

    if (phubsel->fSelectionAccepted == 1)
    {
        const bool fSelectionSettled = psmaWorld->ismsCur == psmaWorld->ismsGoal;
        const bool fSelectionTimedOut = g_clock.t - phubsel->tSelectionChange >= 0.3f;

        if (fSelectionSettled || fSelectionTimedOut)
        {
            if (phubsel->gameWorldSelected != GAMEWORLD_Intro)
            {
                if (g_wmc.blots != BLOTS_Hidden)
                    return;

                g_wmc.pwmCurrent = g_wmc.apwm[phubsel->gameWorldSelected];
                PushUiActiveBlot(&g_ui, &g_wmc);
                return;
            }

            if (g_fmvmenu.blots != BLOTS_Hidden)
                return;

            PushUiActiveBlot(&g_ui, &g_fmvmenu);
            return;
        }
    }

    if (psmaWorld->ismsCur != psmaWorld->ismsGoal)
        return;

    if (pjoy->IsPressed(BTN_START) || (pjoy->joys == JOYS_Searching && g_clock.t >= 5.0f))
        SetPrompt(&g_prompt, PRP_Basic, PRK_OptionsMenu);
    else if (pjoy->IsPressed(BTN_CROSS) || pjoy->IsPressed(BTN_SELECT))
    {
        pjoy->SetHandled(BTN_CROSS);
        pjoy->SetHandled(BTN_SELECT);
        ConfirmHubselSelection(phubsel);
    }
    else
    {
        UpdateHubselSelection(phubsel, pjoy);
    }
}

void OnHubBlotActive(HUBBLOT* phubblot, int fActive)
{
    const bool fEnable = fActive != 0;
    const bool fCurrentlyActive = phubblot->fActive != 0;

    if (fEnable == fCurrentlyActive)
        return;

    if (!fEnable)
    {
        /*if (phubblot->fWaitVag != 0)
        {
            StopVag();
            phubblot->fWaitVag = 0;
        }*/

        RemoveGrfusr(1);
        phubblot->pvtblot->pfnHideBlot(phubblot);
        g_totals.pvtblot->pfnHideBlot(&g_totals);
        phubblot->fActive = 0;
    }
    else
    {
        //UnlockAllHubWorlds();
        SetHubselSelection(phubblot->phubsel, phubblot->phubsel->gameWorldSelected, 0);
        AddGrfusr(1);
        g_joy.StartJoySelection();
        phubblot->fActive = 1;
    }
}

void DrawHubBlot(HUBBLOT* phubblot)
{
	if (phubblot->phubsel == nullptr || phubblot->achzDraw[0] == '\0')
		return;

	glm::vec4 color = phubblot->rgba;
	color.a *= phubblot->uOn;

	CTextBox textBox;
	textBox.SetPos(phubblot->x, phubblot->y);
	textBox.SetSize(phubblot->dx, phubblot->dy);
	textBox.SetTextColor(&color);
	textBox.SetHorizontalJust(JH_Center);
	textBox.SetVerticalJust(JV_Center);

	if (phubblot->pte != nullptr)
	{
		const glm::vec4 edgeColor = phubblot->pte->m_rgba;
		phubblot->pte->m_rgba.a *= phubblot->uOn;

		if (phubblot->pte->m_pfont != nullptr)
			phubblot->pte->m_pfont->EdgeRect(phubblot->pte, &textBox);

		phubblot->pte->m_rgba = edgeColor;
	}

	const float pulse = std::sin(g_clock.tReal * 10.0f) * 0.5f + 0.5f;
	const float scale = phubblot->rFontScale * (0.9f + pulse * 0.1f);

	phubblot->pfont->PushScaling(scale, scale);
	CRichText richText(phubblot->achzDraw, phubblot->pfont);
	richText.Draw(&textBox, nullptr);
	phubblot->pfont->PopScaling();
}

VTHUBBLOT g_vthubblot;
CTextEdge g_teHubBlot;
HUBBLOT g_hubblot;
float g_dtHubHint = 20.0f;

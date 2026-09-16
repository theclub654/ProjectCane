#include "dialog.h"
#include "jt.h"
#include "tv.h"
#include "binoc.h"
#include "credit.h"
#include "wipe.h"
#include "call.h"
#include "speaker.h"
#include "actseg.h"
#include "po.h"
#include "suv.h"
#include "fmv.h"
#include <array>
#include <cstdio>
#include <cstring>

DIALOG* NewDialog()
{
	return new DIALOG{};
}

void InitDialog(DIALOG* pdialog)
{
	InitAlo(pdialog);

	pdialog->oidPntAnchor = OID_Nil;
	pdialog->fHidden = 1;
	pdialog->oidPntAnchorDefault = OID_Nil;
}

void* GetDialogDialogs(DIALOG* pdialog)
{
	return &pdialog->dialogs;
}

void* GetDialogOidPntAnchorDefault(DIALOG* pdialog)
{
	return &pdialog->oidPntAnchorDefault;
}

void SetDialogOidPntAnchorDefault(DIALOG* pdialog, int oidPntAnchorDefault)
{
	pdialog->oidPntAnchorDefault = oidPntAnchorDefault;
}

void* GetDialogFDefault(DIALOG* pdialog)
{
	return &pdialog->fDefault;
}

void SetDialogFDefault(DIALOG* pdialog, int fDefault)
{
	pdialog->fDefault = fDefault;
}

void* GetDialogFCrucial(DIALOG* pdialog)
{
	return &pdialog->fCrucial;
}

void SetDialogFCrucial(DIALOG* pdialog, int fCrucial)
{
	pdialog->fCrucial = fCrucial;
}

void* GetDialogFNoSkip(DIALOG* pdialog)
{
	return &pdialog->fNoSkip;
}

void SetDialogFNoSkip(DIALOG* pdialog, int fNoSkip)
{
	pdialog->fNoSkip = fNoSkip;
}

void* GetDialogOidPntAnchor(DIALOG* pdialog)
{
	return &pdialog->oidPntAnchor;
}

void SetDialogOidPntAnchor(DIALOG* pdialog, int oidPntAnchor)
{
	pdialog->oidPntAnchor = (OID)oidPntAnchor;
}

void* GetDialogFConfrontAudio(DIALOG* pdialog)
{
	return &pdialog->fConfrontAudio;
}

void SetDialogFConfrontAudio(DIALOG* pdialog, int fConfrontAudio)
{
	pdialog->fConfrontAudio = fConfrontAudio;
}

int GetDialogSize()
{
	return sizeof(DIALOG);
}

void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pdialog, pbis);
	LoadDialogEventsFromBrx(pdialog, pbis, &pdialog->cde, pdialog->ade);
}

void CloneDialog(DIALOG* pdialog, DIALOG* pdialogBase)
{
    CloneAlo(pdialog, pdialogBase);

    pdialog->dialogk = pdialogBase->dialogk;
    pdialog->dialogs = pdialogBase->dialogs;
    pdialog->tDialogs = pdialogBase->tDialogs;

    pdialog->cde = pdialogBase->cde;
    pdialog->ade = pdialogBase->ade;

    pdialog->ideCur = pdialogBase->ideCur;
    pdialog->dp = pdialogBase->dp;
    pdialog->oidPntAnchor = pdialogBase->oidPntAnchor;
    pdialog->pfPlayed = pdialogBase->pfPlayed;
    pdialog->fPlayed = pdialogBase->fPlayed;

    pdialog->cpfEquivalence = pdialogBase->cpfEquivalence;
    for (int i = 0; i < 4; ++i)
		pdialog->apfEquivalence[i] = pdialogBase->apfEquivalence[i];

    pdialog->fDefault = pdialogBase->fDefault;
    pdialog->fCrucial = pdialogBase->fCrucial;
}

void LoadDialogEventsFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis, int* pcde, std::vector<DE>& ade)
{
	*pcde = pbis->S8Read();

	ade.clear();
	ade.resize(*pcde);

	for (int ide = 0; ide < *pcde; ide++) 
	{
		DE* pde = &ade[ide];

		pde->dek = static_cast<DEK>(pbis->S8Read());

		switch (pde->dek) 
		{
			case DEK_PreloadVag:
			case DEK_ShowMovie:
			{
				std::array<uint32_t, 8> descriptor{};
				bool fDescriptorPresent = false;
				for (uint32_t& word : descriptor)
				{
					word = pbis->U32Read();
					fDescriptorPresent |= word != 0;
				}

				if (fDescriptorPresent)
				{
					pde->string.pchz = std::shared_ptr<char[]>(new char[33]());
					std::memcpy(pde->string.pchz.get(), descriptor.data(), 32);
				}
				else
				{
					pde->string.pchz.reset();
				}

				break;
			}

			case DEK_SpeakerLeft:
			case DEK_SpeakerRight:
			pde->speaker.oid = static_cast<OID>(pbis->S16Read());
			pde->speaker.oidReplace = static_cast<OID>(pbis->S16Read());
			break;

			case DEK_Talk:
			pde->talk.uZoom = pbis->F32Read();
			pde->talk.dtPause = pbis->F32Read();
			pde->talk.fUser = pbis->U8Read();
			pde->talk.dpk = static_cast<DPK>(pbis->U8Read());
			pde->talk.oidSpeaker = static_cast<OID>(pbis->S16Read());
			pde->talk.oidLipsAseg = static_cast<OID>(pbis->S16Read());
			pde->talk.oidFocus = static_cast<OID>(pbis->S16Read());
			pde->talk.oidPntAnchor = static_cast<OID>(pbis->S16Read());
			pde->talk.pchzText = pbis->ReadStringAlloc();
			break;

			case DEK_CallSplice:
			pde->splice.oidLabel = static_cast<OID>(pbis->S16Read());
			break;

			case DEK_ShowLetterbox:
			case DEK_HideLetterbox:
			break;
		}
	}
}

void PostDialog(DIALOG* pdialog)
{
	PostAloLoad(pdialog);

	if (pdialog->oidPntAnchor == OID_Nil)
		pdialog->oidPntAnchor = pdialog->oid;

	pdialog->pfPlayed = PfLookupDialog(g_plsCur, pdialog->oidPntAnchor);

	if (pdialog->pfPlayed == nullptr)
		pdialog->pfPlayed = &pdialog->fPlayed;

}

void SetDialogInstruct(DIALOG* pdialog)
{
	pdialog->dialogk = DIALOGK_Instruct;
}

void SetDialogConfront(DIALOG* dialog)
{
	dialog->dialogk = DIALOGK_Confront;
}

void GetDialogPlayed(DIALOG* pdialog, int* pfPlayed)
{
	*pfPlayed = *pdialog->pfPlayed;
}

void SetDialogPlayed(DIALOG* pdialog, int fPlayed)
{
	*pdialog->pfPlayed = fPlayed;
}

void SetDialogDialogs(DIALOG* pdialog, DIALOGS dialogs)
{
	DIALOGS dialogsPrev = pdialog->dialogs;

	if (dialogs == dialogsPrev)
		return;

	// Clean up the previous state.
	if (dialogsPrev == DIALOGS_Playing)
	{
		*pdialog->pfPlayed = 1;

		StopVag();
		HandleLoSpliceEvent(pdialog, 21, 0, nullptr);

		if (pdialog->dialogk == DIALOGK_Confront)
		{
			RevokeCmPolicy(g_pcm, 15, CPP_LookAround, &g_pcm->cplook, nullptr, nullptr);
			DecrementSwHandsOff(g_psw);
		}

		LookkPopCplook(&g_pcm->cplook);

		if ((pdialog->dp.grfdp & 16u) != 0 && pdialog->dp.pasegaLipsync != nullptr)
			RetractAsega(pdialog->dp.pasegaLipsync);

		if (pdialog->dialogk == DIALOGK_Confront && pdialog->fConfrontAudio)
		{
			SetMvgkRvol(1.0f, 9, 0);
			SetMvgkRvol(1.0f, 9, 1);
		}
	}
	else if (dialogsPrev == DIALOGS_Triggered)
	{
		if (pdialog->dialogk != DIALOGK_Instruct)
			--g_cdialogTriggered;
	}
	else if (dialogsPrev == DIALOGS_Calling)
	{
		g_binoc.pdialogPlaying = nullptr;
		SetBinocBinocs(&g_binoc, BINOCS_None);
	}
	else if (dialogsPrev == DIALOGS_UnableToCall)
	{
		if (g_call.pdialogTriggered == pdialog)
			g_call.pdialogTriggered = nullptr;
	}

	// Initialize the new state.
	if (dialogs == DIALOGS_Playing)
	{
		pdialog->ideCur = -1;
		pdialog->dp.grfdp = 0;

		if (pdialog->dialogk == DIALOGK_Confront)
		{
			PushCplookLookk(&g_pcm->cplook, LOOKK_Confront);
			SetCmPolicy(g_pcm, CPP_LookAround, &g_pcm->cplook, nullptr, nullptr);
			IncrementSwHandsOff(g_psw);
			SetBinocLookat(&g_binoc, nullptr);

			if (pdialog->fConfrontAudio)
			{
				SetMvgkRvol(0.6f, 9, 0);
				SetMvgkRvol(0.6f, 9, 1);
			}
		}
		else
		{
			PushCplookLookk(&g_pcm->cplook, LOOKK_Dialog);

			if (pdialog->dialogk == DIALOGK_Binoc)
			{
				if (pdialog->oidPntAnchorDefault == OID_Nil)
					g_pcm->cplook.ppntAnchor = nullptr;
				else
					g_pcm->cplook.ppntAnchor = static_cast<PNT*>(PloFindSwNearest(pdialog->psw, (OID)pdialog->oidPntAnchorDefault, nullptr));
			}
		}
	}
	else if (dialogs == DIALOGS_Triggered)
	{
		if (pdialog->dialogk != DIALOGK_Instruct)
			++g_cdialogTriggered;
	}
	else if (dialogs == DIALOGS_Calling)
	{
		SetTvSpeaker(&g_tvLeft, nullptr);
		SetTvReplace(&g_tvLeft, nullptr);
		SetTvSpeaker(&g_tvRight, nullptr);
		SetTvReplace(&g_tvRight, nullptr);
	}
	else if (dialogs == DIALOGS_UnableToCall)
	{
		if (g_call.pdialogTriggered != nullptr)
			SetDialogDialogs(g_call.pdialogTriggered, DIALOGS_Enabled);

		g_call.pdialogTriggered = pdialog;
	}

	pdialog->dialogs = dialogs;
	pdialog->tDialogs = g_clock.t;
}

void HandleDialogMessage(DIALOG* pdialog, MSGID msgid, void* pv)
{
    HandleAloMessage(pdialog, msgid, pv);

    if ((msgid == MSGID_asega_retracted) && (pv == (pdialog->dp).pasegaLipsync)) 
        (pdialog->dp).pasegaLipsync = nullptr;
}

bool FPauseDialog(DIALOG* pdialog)
{
	constexpr uint32_t DPF_Ambient = 1u << 0;
	constexpr uint32_t DPF_SpeakerLeft = 1u << 1;
	constexpr uint32_t DPF_SpeakerRight = 1u << 2;
	constexpr uint32_t DPF_Vag = 1u << 3;
	constexpr uint32_t DPF_Lipsync = 1u << 4;
	constexpr uint32_t DPF_BinocAnimation = 1u << 5;
	constexpr uint32_t DPF_CameraZoom = 1u << 6;
	constexpr uint32_t DPF_MinimumTime = 1u << 7;
	constexpr uint32_t DPF_PlayerInput = 1u << 8;

	uint32_t grfdp = pdialog->dp.grfdp;

	DE* pdeNext = nullptr;

	if (pdialog->ideCur + 1 < pdialog->cde)
		pdeNext = &pdialog->ade[pdialog->ideCur + 1];

	if ((grfdp & DPF_SpeakerLeft) != 0)
	{
		if (pdeNext != nullptr && pdeNext->dek == DEK_SpeakerRight && grfdp == DPF_SpeakerLeft)
			return false;

		if (g_tvLeft.pspeaker == pdialog->dp.pspeakerLeft && g_tvLeft.tvgs == TVGS_Open)
			grfdp &= ~DPF_SpeakerLeft;
	}

	if ((grfdp & DPF_SpeakerRight) != 0)
	{
		if (pdeNext != nullptr && pdeNext->dek == DEK_SpeakerLeft && grfdp == DPF_SpeakerRight)
			return false;

		if (g_tvRight.pspeaker == pdialog->dp.pspeakerRight && g_tvRight.tvgs == TVGS_Open)
			grfdp &= ~DPF_SpeakerRight;
	}

	if ((grfdp & DPF_Ambient) != 0 && !FPauseForVag())
		grfdp &= ~DPF_Ambient;

	if ((grfdp & DPF_Vag) != 0 && !FVagPlaying())
		grfdp &= ~DPF_Vag;

	if ((grfdp & DPF_Lipsync) != 0 && pdialog->dp.pasegaLipsync == nullptr)
		grfdp &= ~DPF_Lipsync;

	if ((grfdp & DPF_BinocAnimation) != 0 && FDoneBinocAchz(&g_binoc))
		grfdp &= ~DPF_BinocAnimation;

	if ((grfdp & DPF_CameraZoom) != 0 && g_pcm->cplook.uZoom == g_binoc.uZoom)
		grfdp &= ~DPF_CameraZoom;

	if ((grfdp & DPF_MinimumTime) != 0 && g_clock.t >= pdialog->dp.tPauseMin)
		grfdp &= ~DPF_MinimumTime;

	g_binoc.chPause = '\0';

	if (grfdp == DPF_PlayerInput)
	{
		JOY* pjoy = (g_grfjoyt & 4u) != 0 ? &g_joy : &g_joyZero;
		DPK dpk = pdialog->dp.dpk;
		bool fInputReceived = false;

		if (grfdp == DPF_PlayerInput)
		{
			JOY* pjoy = (g_grfjoyt & 4u) != 0 ? &g_joy : &g_joyZero;
			DPK dpk = pdialog->dp.dpk;
			bool fInputReceived = false;

			switch (dpk)
			{
			case DPK_None:
			case DPK_X:
				fInputReceived = pjoy->IsPressed(BTN_CROSS);
				break;

			case DPK_Square:
				fInputReceived = pjoy->IsPressed(BTN_SQUARE);
				break;

			case DPK_Circle:
				fInputReceived = pjoy->IsPressed(BTN_CIRCLE);
				break;

			case DPK_Triangle:
				fInputReceived = pjoy->IsPressed(BTN_TRIANGLE);
				break;

			case DPK_L1:
				fInputReceived = pjoy->IsPressed(BTN_L1);
				break;

			case DPK_R1:
				fInputReceived = pjoy->IsPressed(BTN_R1);
				break;

			case DPK_JoyLeft:
				fInputReceived = pjoy->uDeflect > 0.8f;
				break;

			case DPK_JoyRight:
				fInputReceived = pjoy->uDeflect2 > 0.8f;
				break;

			case DPK_JoyLeftX:
				fInputReceived = std::abs(pjoy->x) > 0.8f;
				break;

			case DPK_JoyLeftY:
				fInputReceived = std::abs(pjoy->y) > 0.8f;
				break;

			case DPK_DoubleJump:
				fInputReceived = g_pjt != nullptr && g_pjt->jts == JTS_Jump && g_pjt->jtbs == JTBS_Jump_Boost;
				break;

			case DPK_VaultOpen:
				fInputReceived = g_plsCur != nullptr && (g_plsCur->grfls & 4u) != 0;
				break;

			default:
				fInputReceived = false;
				break;
			}

			if (fInputReceived)
				grfdp = 0;
			else if (dpk >= DPK_None && dpk < DPK_Max)
				g_binoc.chPause = s_mpdpkchPause[dpk];
		}

		if (fInputReceived)
			grfdp = 0;
		else
			g_binoc.chPause = s_mpdpkchPause[dpk];
	}

	pdialog->dp.grfdp = grfdp;
	return grfdp != 0;
}

void FinishDialogEvents(DIALOG* pdialog)
{
	if (pdialog->dialogs == DIALOGS_Calling)
		return;

	++pdialog->ideCur;

	while (pdialog->ideCur < pdialog->cde)
	{
		DE& de = pdialog->ade[pdialog->ideCur];

		switch (de.dek)
		{
			case DEK_CallSplice:
			{
				void* apv[4]{};
				apv[0] = &de.splice.oidLabel;
				HandleLoSpliceEvent(pdialog, 16, 1, apv);
				break;
			}

			case DEK_ShowLetterbox:
				AcquireSwLetterbox(pdialog->psw);
				break;

			case DEK_HideLetterbox:
				ReleaseSwLetterbox(pdialog->psw);
				break;

			default:
				break;
		}

		++pdialog->ideCur;
	}

	if (pdialog->dialogk == DIALOGK_Confront)
		PopUiActiveBlot(&g_ui);
}
void HandleDialogEvents(DIALOG* pdialog)
{
	if (pdialog->ideCur >= pdialog->cde)
		return;

	while (!FPauseDialog(pdialog))
	{
		if (pdialog->dp.pasegaLipsync != nullptr)
		{
			RetractAsega(pdialog->dp.pasegaLipsync);
			pdialog->dp.pasegaLipsync = nullptr;
		}

		++pdialog->ideCur;

		if (pdialog->ideCur >= pdialog->cde)
		{
			FinishDialogEvents(pdialog);
			return;
		}

		HandleDialogEvent(pdialog, &pdialog->ade[pdialog->ideCur]);
	}
}

void StartDialog(DIALOG* pdialog, PO* ppo)
{
	if (ppo == nullptr)
	{
		g_binoc.pdialogPlaying = pdialog;
		PushUiActiveBlot(&g_ui, &g_binoc);
	}
	else
		ppo->pvtpo->pfnPlayPoDialog(ppo, pdialog);
}

void TriggerDialog(DIALOG* pdialog)
{
	if (g_note.mtsState >= MTSSTATE_Active ||
		g_note.fCommentaryPromptActive ||
		FPlayingCommentaryAudio() ||
		g_wipe.wipes == WIPES_WipingOut ||
		pdialog == g_binoc.pdialogPlaying ||
		(*pdialog->pfPlayed && pdialog->dialogk == DIALOGK_Confront))
		return;

	DIALOGS dialogs = DIALOGS_Triggered;

	if (*pdialog->pfPlayed)
	{
		if (pdialog->dialogk == DIALOGK_Confront)
			return;

		dialogs = DIALOGS_UnableToCall;
	}
	else
	{
		// The dialog is unavailable if any equivalent dialog was already played.
		for (int i = 0; i < pdialog->cpfEquivalence; ++i)
		{
			if (*pdialog->apfEquivalence[i])
			{
				dialogs = DIALOGS_UnableToCall;
				break;
			}
		}
	}

	SetDialogDialogs(pdialog, dialogs);

}

void UntriggerDialog(DIALOG* pdialog)
{
	if ((pdialog->dialogs != DIALOGS_Triggered) && (pdialog->dialogs != DIALOGS_Playing))
		SetDialogDialogs(pdialog, DIALOGS_Enabled);
}

void AddDialogEquivalence(DIALOG* pdialog, int wid, int oid)
{
	if (pdialog->cpfEquivalence >= 4)
		return;

	LS* pls = PlsFromWorldLevelKey(wid);

	if (pls == nullptr)
		return;

	int* pfEquivalent = PfLookupDialog(pls, oid);

	if (pfEquivalent == nullptr)
		return;

	pdialog->apfEquivalence[pdialog->cpfEquivalence] = pfEquivalent;
	++pdialog->cpfEquivalence;
}

void UpdateDialog(DIALOG* pdialog, float dt)
{
	UpdateAlo(pdialog, dt);

	DIALOGS dialogs = pdialog->dialogs;

	const bool fDialogBlocked =
		g_note.mtsState >= MTSSTATE_Active ||
		g_note.fCommentaryPromptActive ||
		FPlayingCommentaryAudio() ||
		g_wipe.wipes == WIPES_WipingOut;

	if (fDialogBlocked)
	{
		const bool fResetDialog =
			dialogs == DIALOGS_Triggered ||
			dialogs == DIALOGS_UnableToCall ||
			dialogs == DIALOGS_Unavailable;

		if (fResetDialog)
		{
			SetDialogDialogs(pdialog, DIALOGS_Enabled);
			return;
		}
	}

	DIAPI diapi{};
	PO* ppo = PpoCur();

	const bool fCameraPolicyActive = FActiveCplcy(&g_pcm->cpman) != 0;
	if (fCameraPolicyActive)
	{
		diapi.fCallable = 0;
		diapi.fPlayable = 0;
	}
	else if (!ppo)
		diapi.fPlayable = 1;
	else
		ppo->pvtpo->pfnGetPoDiapi(ppo, pdialog, &diapi);


	dialogs = pdialog->dialogs;

	switch (dialogs)
	{
		case DIALOGS_Enabled:
		{
			if (pdialog->fDefault && g_wipe.wipes == WIPES_Idle && diapi.fPlayable)
			{
				pdialog->fDefault = 0;
				TriggerDialog(pdialog);
				dialogs = pdialog->dialogs;
			}
			break;
		}

		case DIALOGS_Triggered:
		{
			if (!diapi.fPlayable)
			{
				if (g_clock.t - pdialog->tDialogs >= 5.0f)
					dialogs = DIALOGS_Enabled;

				break;
			}

			if (g_binoc.pdialogPlaying)
			{
				DIALOG* pdialogPlaying = g_binoc.pdialogPlaying;

				if (pdialogPlaying != pdialog && !pdialogPlaying->fCrucial && pdialogPlaying->dialogs == DIALOGS_Playing)
					FinishDialogEvents(pdialogPlaying);

				break;
			}

			StartDialog(pdialog, ppo);
			dialogs = pdialog->dialogs;
			break;
		}

		case DIALOGS_Playing:
		{
			const bool fEventsRemaining = pdialog->ideCur < pdialog->cde;
			const bool fWipingOut = g_wipe.wipes == WIPES_WipingOut;

			if (pdialog->dialogk != DIALOGK_Confront && (!fEventsRemaining || fWipingOut))
			{
				PopUiActiveBlot(&g_ui);
				dialogs = pdialog->dialogs;
			}

			break;
		}

		case DIALOGS_Calling:
		{
			if (g_tvLeft.blots == BLOTS_Hidden && g_tvRight.blots == BLOTS_Hidden)
				dialogs = DIALOGS_Enabled;

			break;
		}

		case DIALOGS_UnableToCall:
		{
			if (diapi.fCallable)
			{
				if (g_clock.t - pdialog->tDialogs >= 5.0f)
					dialogs = DIALOGS_Enabled;
			}
			else
				dialogs = DIALOGS_Unavailable;

			break;
		}

		case DIALOGS_Unavailable:
		{
			if (!diapi.fCallable)
			{
				if (g_clock.t - pdialog->tDialogs >= 5.0f)
					dialogs = DIALOGS_Enabled;
			}
			else
				dialogs = DIALOGS_UnableToCall;

			break;
		}

		default:
		break;
	}

	SetDialogDialogs(pdialog, dialogs);

	if (pdialog->dialogs == DIALOGS_Playing)
		HandleDialogEvents(pdialog);
}

void HandleDialogEvent(DIALOG* pdialog, DE* pde)
{
	DP& dp = pdialog->dp;
	LO* ploRoot = pdialog->paloRoot ? static_cast<LO*>(pdialog->paloRoot) : static_cast<LO*>(pdialog);

	switch (pde->dek)
	{
		case DEK_PreloadVag:
		{
			PreloadVag(pde->string.pchz.get());
			dp.grfdp |= GRFDP_VagPreloaded;
			break;
		}

		case DEK_SpeakerLeft:
		case DEK_SpeakerRight:
		{
			SPEAKER* pspeaker = nullptr;
			ALO* paloReplace = nullptr;

			if (pde->speaker.oid != OID_Nil)
				pspeaker = static_cast<SPEAKER*>(PloFindSwObject(pdialog->psw, 260, pde->speaker.oid, ploRoot));

			if (pde->speaker.oidReplace != OID_Nil)
				paloReplace = static_cast<ALO*>(PloFindSwObject(pdialog->psw, 260, pde->speaker.oidReplace, ploRoot));

			if (pspeaker && pspeaker->psmaIdle)
				SetSmaGoal(pspeaker->psmaIdle, static_cast<OID>(844));

			if (pde->dek == DEK_SpeakerLeft)
			{
				SetTvSpeaker(&g_tvLeft, pspeaker);
				SetTvReplace(&g_tvLeft, paloReplace);

				dp.pspeakerLeft = pspeaker;

				if (pspeaker)
					dp.grfdp |= GRFDP_SpeakerLeft;

			}
			else
			{
				SetTvSpeaker(&g_tvRight, pspeaker);
				SetTvReplace(&g_tvRight, paloReplace);

				dp.pspeakerRight = pspeaker;

				if (pspeaker)
					dp.grfdp |= GRFDP_SpeakerRight;

			}

			break;
		}

		case DEK_Talk:
		{
			auto& talk = pde->talk;
			SPEAKER* pspeaker = nullptr;

			if (dp.pspeakerLeft && FMatchesLoName(static_cast<LO*>(dp.pspeakerLeft), talk.oidSpeaker))
			{
				pspeaker = dp.pspeakerLeft;
				SetTvTvs(&g_tvLeft, TVS_Talk);
				SetTvTvs(&g_tvRight, TVS_Listen);
			}
			else if (dp.pspeakerRight && FMatchesLoName(static_cast<LO*>(dp.pspeakerRight), talk.oidSpeaker))
			{
				pspeaker = dp.pspeakerRight;
				SetTvTvs(&g_tvRight, TVS_Talk);
				SetTvTvs(&g_tvLeft, TVS_Listen);
			}

			ASEG* pasegLipsync = nullptr;

			if (talk.oidLipsAseg == OID_Nil)
			{
				if (pspeaker)
					pasegLipsync = static_cast<ASEG*>(PloFindSwObject(pdialog->psw, 1, static_cast<OID>(1354), static_cast<LO*>(pspeaker)));
			}
			else
			{
				pasegLipsync = static_cast<ASEG*>(PloFindSwObject(pdialog->psw, 2, talk.oidLipsAseg, static_cast<LO*>(pdialog)));
				dp.grfdp |= GRFDP_ExplicitLipsync;
			}

			if (pasegLipsync)
			{
				ApplyAseg(pasegLipsync, pspeaker, 0.0f, 1.0f, 1, &dp.pasegaLipsync);
				SubscribeAsegaObject(dp.pasegaLipsync, static_cast<LO*>(pdialog));
			}

			char* pchzText = talk.pchzText.get();

			if (pchzText && pchzText[0] != '\0')
			{
				g_binoc.pvtblot->pfnSetBlotAchzDraw(&g_binoc, pchzText);
				g_binoc.rgbaText = pspeaker ? pspeaker->rgbaText : glm::vec4(128.0f / 255.0f);

				float dtText = talk.dtPause;

				if (dp.pasegaLipsync && talk.oidLipsAseg != OID_Nil)
					dtText = dp.pasegaLipsync->paseg->tMax;

				g_binoc.svch = dtText > 0.0f ? static_cast<float>(std::strlen(pchzText) + 10) / dtText : 15.0f;
				dp.grfdp |= GRFDP_Text;
			}

			if (talk.uZoom >= 0.0f)
			{
				SetBinocZoom(&g_binoc, talk.uZoom + 1.0f);
				dp.grfdp |= GRFDP_Zoom;
			}

			if (talk.dtPause >= 0.0f)
			{
				dp.tPauseMin = g_clock.t + talk.dtPause;
				dp.grfdp |= GRFDP_Pause;
			}

			if (talk.oidFocus != OID_Nil)
			{
				ALO* paloFocus = static_cast<ALO*>(PloFindSwObject(pdialog->psw, 260, talk.oidFocus, ploRoot));
				SetBinocLookat(&g_binoc, paloFocus);
			}

			OID oidAnchor = talk.oidPntAnchor;

			if (oidAnchor == OID_Nil)
				oidAnchor = static_cast<OID>(pdialog->oidPntAnchorDefault);

			if (oidAnchor != OID_Nil)
				g_pcm->cplook.ppntAnchor = static_cast<PNT*>(PloFindSwNearest(pdialog->psw, oidAnchor, nullptr));
			else
				g_pcm->cplook.ppntAnchor = nullptr;

			if (talk.fUser)
			{
				dp.dpk = talk.dpk;
				dp.grfdp |= GRFDP_UserInput;
			}

			dp.grfdp |= GRFDP_Talk;
			break;
		}

		case DEK_CallSplice:
		{
			void* pvArg = &pde->splice;
			HandleLoSpliceEvent(pdialog, 16, 1, &pvArg);
			break;
		}

		case DEK_ShowLetterbox:
		{
			AcquireSwLetterbox(pdialog->psw);
			break;
		}

		case DEK_HideLetterbox:
		{
			ReleaseSwLetterbox(pdialog->psw);
			break;
		}

		case DEK_ShowMovie:
		{
			QueueCutsceneDescriptor(pde->string.pchz.get());
			break;
		}

		default:
		break;
	}
}

void DeleteDialog(DIALOG* pdialog)
{
	delete pdialog;
}

int g_cdialogTriggered = 0;

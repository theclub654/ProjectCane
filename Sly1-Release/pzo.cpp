#include "pzo.h"
#include "chkpnt.h"
#include "sm.h"
#include "button.h"
#include "dialog.h"
#include "jt.h"
#include "binoc.h"
#include "emitter.h"
#include "expl.h"
#include "smack.h"
#include "steppower.h"
#include "xform.h"

VAULT* NewVault()
{
	return new VAULT{};
}

void InitVault(VAULT* pvault)
{
	InitPo(pvault);
	pvault->oidDialogCombo = OID_Nil;
	pvault->oidVolbtnGoad  = OID_Nil;
}

void SetVaultnCombination(VAULT* pvault, int nCombination)
{
	pvault->nCombination = nCombination;
}

void* GetVaultnCombination(VAULT* pvault)
{
	return &pvault->nCombination;
}

void SetVaultoidVolbtnGoad(VAULT* pvault, OID oidVolbtnGoad)
{
	pvault->oidVolbtnGoad = oidVolbtnGoad;
}

void* GetVaultoidVolbtnGoad(VAULT* pvault)
{
	return &pvault->oidVolbtnGoad;
}

void SetVaultoidDialogCombo(VAULT* pvault, OID oidDialogCombo)
{
	pvault->oidDialogCombo = oidDialogCombo;
}

void* GetVaultoidDialogCombo(VAULT* pvault)
{
	return &pvault->oidDialogCombo;
}

void SetVaultfGoadStart(VAULT* pvault, int fGoadStart)
{
	pvault->fGoadStart = fGoadStart;
}

void* GetVaultfGoadStart(VAULT* pvault)
{
	return &pvault->fGoadStart;
}

int GetVaultSize()
{
	return sizeof(VAULT);
}

void CloneVault(VAULT* pvault, VAULT* pvaultBase)
{
	ClonePo(pvault, pvaultBase);

	// Shallow copy of the value members
	pvault->nCombination = pvaultBase->nCombination;
	pvault->oidDialogCombo = pvaultBase->oidDialogCombo;
	pvault->oidVolbtnGoad = pvaultBase->oidVolbtnGoad;
	pvault->fGoadStart = pvaultBase->fGoadStart;
	pvault->fVault = pvaultBase->fVault;
	pvault->cpdialogPending = pvaultBase->cpdialogPending;

	// Shallow copy of the pointer members
	pvault->psmVault = pvaultBase->psmVault;
	pvault->psmaVault = pvaultBase->psmaVault;
	pvault->pxfmJt = pvaultBase->pxfmJt;
	std::memcpy(pvault->atmbl, pvaultBase->atmbl, sizeof(pvault->atmbl)); // Copy the array
	pvault->pdialogCombo = pvaultBase->pdialogCombo;
	pvault->pvolbtnGoad = pvaultBase->pvolbtnGoad;

	// Shallow copy the array of GOAD structures
	std::memcpy(pvault->mpgoadkgoad, pvaultBase->mpgoadkgoad, sizeof(pvault->mpgoadkgoad));

	// Shallow copy the arrays of DIALOG pointers
	std::memcpy(pvault->apdialogInstruct, pvaultBase->apdialogInstruct, sizeof(pvault->apdialogInstruct));
	std::memcpy(pvault->apdialogPending, pvaultBase->apdialogPending, sizeof(pvault->apdialogPending));
}

void PostTmblLoad(TMBL* ptmbl, OID oidInitialState)
{
	ptmbl->psmDial = (SM*)PloFindSwObject(g_psw, 0x101, (OID)890, ptmbl->palo);

	SnipLo(ptmbl->psmDial);

	ptmbl->psmaDial = PsmaApplySm(ptmbl->psmDial, ptmbl->palo, oidInitialState, 0);
}

void PostVaultLoad(VAULT* pvault)
{
	const int grfVaultValid = 0xF000FFFF;

	PostAloLoad(pvault);
	SnipAloObjects(pvault, 26, s_asnipVault);

	for (int i = 0; i < 3; i++)
		PostTmblLoad(&pvault->atmbl[i], (OID)912);

	pvault->psmaVault = PsmaApplySm(pvault->psmVault, pvault, (OID)900, 1);
	pvault->idialogInstruct = -1;

	if ((g_plsCur->grfls & 4) != 0) {
		SetSmaGoal(pvault->psmaVault, (OID)911);
		return;
	}

	pvault->psw->pvault = pvault;

	for (int i = 0; i < 6; i++) {
		GOAD* pgoad = &pvault->mpgoadkgoad[i];

		pgoad->cpdialog = 0;

		for (int j = 0; j < pgoad->coidDialog; j++) {
			DIALOG* pdialog = (DIALOG*)PloFindSwNearest(pvault->psw, (OID)pgoad->aoidDialog[j], pvault);

			if (pdialog != nullptr) 
			{
				pgoad->apdialog[pgoad->cpdialog] = pdialog;
				pgoad->cpdialog++;
			}
		}

		if (pgoad->cpdialog > 0)
			pgoad->ipdialog = NRandInRange(0, pgoad->cpdialog - 1);
	}

	pvault->pdialogCombo = (DIALOG*)PloFindSwNearest(pvault->psw, (OID)pvault->oidDialogCombo, pvault);
	pvault->pvolbtnGoad = (VOLBTN*)PloFindSwNearest(pvault->psw, (OID)pvault->oidVolbtnGoad, pvault);

	if (pvault->pvolbtnGoad != nullptr)
		pvault->pvolbtnGoad->pvtlo->pfnSubscribeLoObject(pvault->pvolbtnGoad, pvault);

	if (((g_pgsCur->gameWorldCur << 8) | g_pgsCur->worldLevelCur) == 3) {
		pvault->fVault = 1;
	}
	else 
	{
		GetBlueprintInfo(&pvault->fVault, 3);

		if ((pvault->fVault & grfVaultValid) != 0) {
			int cwsVault = 0;

			// Retail scans each LS in the current world's als array. The previous
			// reconstruction advanced a WS pointer through LS storage, producing
			// the wrong stride and potentially reading beyond the level array.
			for (const LS& level : g_pwsCur->als) {
				if ((level.grfls & FLS_Secondary) != 0)
					cwsVault++;
			}

			if (cwsVault != 2)
				pvault->fVault = 0;
		}

		if ((pvault->fVault & grfVaultValid) == 0) {
			pvault->fVault = 2;

			if ((g_pgsCur->grfvault & 2) != 0) {
				while (true) {
					pvault->fVault <<= 1;

					if ((pvault->fVault & grfVaultValid) == 0 || (g_pgsCur->grfvault & pvault->fVault) == 0)
						break;
				}
			}

			pvault->fVault &= grfVaultValid;
		}
	}

	if ((pvault->fVault & grfVaultValid) == 0)
		return;

	pvault->idialogInstruct = 0;

	while (pvault->idialogInstruct <= 27 && s_agrfvaultInstruct[pvault->idialogInstruct] != pvault->fVault)
		pvault->idialogInstruct++;

	if (pvault->idialogInstruct > 27)
		pvault->idialogInstruct = -1;
}

void OnVaultActive(VAULT* pvault, int fActive)
{
	OID oidState;

	GetSmaGoal(pvault->psmaVault, &oidState);

	if (oidState == OID_Nil)
		GetSmaCur(pvault->psmaVault, &oidState);

	if (!fActive)
	{
		if (oidState != 911)
			SetSmaGoal(pvault->psmaVault, (OID)900);

		pvault->cpdialogPending = 0;

		if ((GetGameProgress() & 4U) != 0)
			PlayEndingFromCompletionFlag(4);

		return;
	}

	if (oidState != 911)
		SetSmaGoal(pvault->psmaVault, (OID)901);

	g_joy.StartJoySelection();

	OID oidDialog = OID_Nil;

	if ((g_pgsCur->grfgs & 8U) == 0)
		oidDialog = (OID)922;
	else if ((g_pgsCur->grfgs & 16U) == 0)
		oidDialog = (OID)923;

	if (oidDialog != OID_Nil)
	{
		DIALOG* pdialog = static_cast<DIALOG*>(PloFindSwNearest(g_psw, oidDialog, pvault));

		if (pdialog)
			pvault->apdialogPending[pvault->cpdialogPending++] = pdialog;
	}

	if (pvault->pdialogCombo)
		pvault->apdialogPending[pvault->cpdialogPending++] = pvault->pdialogCombo;

	if (g_pjt)
	{
		pvault->pzi = g_pjt->pzi;
		pvault->paloCharmEffect = g_pjt->paloUseCharm;
	}
}

void UpdateVaultActive(VAULT* pvault, JOY* pjoy, float dt)
{
	OID oidCur;
	OID oidGoal;

	GetSmaCur(pvault->psmaVault, &oidCur);
	GetSmaGoal(pvault->psmaVault, &oidGoal);

	OID oidState = oidGoal;

	if (oidState == OID_Nil)
		oidState = oidCur;

	if (oidCur == 911)
	{
		SwitchToPo(g_pjt);
		return;
	}

	if (oidCur == 910 && oidGoal == 910)
	{
		if (pvault->cpdialogPending == 0 && g_binoc.pdialogPlaying == nullptr)
		{
			SetSmaGoal(pvault->psmaVault, (OID)911);

			switch (pvault->fVault)
			{
				case 2:
				case 16:
				SetFsp(FSP_Dive);
				break;

				case 4:
				case 1024:
				SetFsp(FSP_Ball);
				break;

				case 8:
				case 4096:
				SetFsp(FSP_SlowDownClock);
				break;

				case 64:
				SetFsp(FSP_Mine);
				break;

				case 128:
				SetFsp(FSP_SpeedUpClock);
				break;

				case 512:
				SetFsp(FSP_Decoy);
				break;

				case 32768:
				SetFsp(FSP_Stun);
				break;
			}
		}

		goto ProcessPendingDialog;
	}

	if (oidGoal == 911)
		goto ProcessPendingDialog;

	if (oidGoal == 910)
		return;

	{
		int selection = -1;

		const int dx = pjoy->DxSelectionJoy(g_clock.tReal);
		const int dy = pjoy->DySelectionJoy(g_clock.tReal);

		if (dx < 0)
			selection = 1;
		else if (dx > 0)
			selection = 0;
		else if (dy < 0)
			selection = 2;
		else if (dy > 0)
			selection = 3;
		else if (pjoy->IsPressed(BTN_CIRCLE))
			selection = 0;
		else if (pjoy->IsPressed(BTN_SQUARE))
			selection = 1;
		else if (pjoy->IsPressed(BTN_TRIANGLE))
			selection = 2;
		else if (pjoy->IsPressed(BTN_CROSS))
			selection = 3;

		// Retail consumes all four face buttons while the vault UI is active
		// (SetJoyBtnHandled(pjoy, 0xF0)).
		pjoy->SetHandled(BTN_SQUARE);
		pjoy->SetHandled(BTN_CIRCLE);
		pjoy->SetHandled(BTN_CROSS);
		pjoy->SetHandled(BTN_TRIANGLE);

		switch (selection)
		{
			case 0:
			if (oidState >= 901 && oidState < 904)
				SetSmaGoal(pvault->psmaVault, (OID)904);
			else if (oidState >= 904 && oidState < 907)
				SetSmaGoal(pvault->psmaVault, (OID)907);
			break;

			case 1:
			if (oidState >= 904 && oidState < 907)
				SetSmaGoal(pvault->psmaVault, (OID)901);
			else if (oidState >= 907 && oidState < 910)
				SetSmaGoal(pvault->psmaVault, (OID)904);
			break;

			case 2:
			if (oidState >= 901 && oidState < 904)
				SetSmaGoal(pvault->psmaVault, (OID)902);
			else if (oidState >= 904 && oidState < 907)
				SetSmaGoal(pvault->psmaVault, (OID)905);
			else if (oidState >= 907 && oidState < 910)
				SetSmaGoal(pvault->psmaVault, (OID)908);
			break;

			case 3:
			if (oidState >= 901 && oidState < 904)
				SetSmaGoal(pvault->psmaVault, (OID)903);
			else if (oidState >= 904 && oidState < 907)
				SetSmaGoal(pvault->psmaVault, (OID)906);
			else if (oidState >= 907 && oidState < 910)
				SetSmaGoal(pvault->psmaVault, (OID)909);
			break;
		}
	}

	{
		int combination = pvault->nCombination;

		for (int i = 0; i < 3; ++i)
		{
			OID oidDialGoal;
			OID oidDialCur;

			GetSmaGoal(pvault->atmbl[i].psmaDial, &oidDialGoal);

			if (oidDialGoal != OID_Nil)
				goto ProcessPendingDialog;

			GetSmaCur(pvault->atmbl[i].psmaDial, &oidDialCur);

			if (oidDialCur - 912 != (combination / 100) % 10)
				goto ProcessPendingDialog;

			combination *= 10;
		}
	}

	if ((g_plsCur->grfls & 4U) == 0)
	{
		OpenVault(pvault);
		return;
	}

	ProcessPendingDialog:

	if (pvault->cpdialogPending > 0 && g_binoc.pdialogPlaying == nullptr)
	{
		TriggerDialog(pvault->apdialogPending[0]);

		--pvault->cpdialogPending;

		std::memmove(
			pvault->apdialogPending,
			pvault->apdialogPending + 1,
			pvault->cpdialogPending * sizeof(pvault->apdialogPending[0]));
	}
}

void HandleVaultMessage(VAULT* pvault, MSGID msgid, void* pv)
{
	HandlePoMessage((PO*)pvault, msgid, pv);

	if (msgid != MSGID_button_trigger)
		return;

	BTN* pbtn = static_cast<BTN*>(pv);
	ALO* powner = pbtn != nullptr ? pbtn->paloOwner : nullptr;

	if (powner != pvault->pvolbtnGoad)
		return;

	JT* pjt = g_pjt;

	if (pvault->fGoadStart != 0 && pjt != nullptr)
	{
		pjt->pvaultCur = pvault;
		pjt->jthk = JTHK_Vault;
		SetJtJts(pjt, JTS_Hide, JTBS_Hide_PeekLeft);

		return;
	}
	g_cluectr.pvtcluectr->pfnShowBlot(&g_cluectr);

	int cclue = g_plsCur->abitClue.m_cbitSet;
	int cclueAll = g_psw->cclueAll;
	int igoad = -1;

	if (cclue == cclueAll)
		igoad = FWouldCompleteVaultSet(pvault->fVault) ? 5 : 4;
	else if ((g_pgsCur->grfvault & 0xF0000000) == 0) {
		if (cclue == cclueAll - 1) {
			igoad = 3;
		}
		else {
			float uClue = (float)cclue / (float)cclueAll;

			if (uClue < 0.5f)
				igoad = 0;
			else if (uClue < 0.8f)
				igoad = 1;
			else
				igoad = 2;
		}
	}

	if (igoad == -1)
		return;

	GOAD* pgoad = &pvault->mpgoadkgoad[igoad];
	DIALOG* pdialog = pgoad->apdialog[pgoad->ipdialog];

	*pdialog->pfPlayed = 0;
	TriggerDialog(pdialog);

	/*if (pgoad->cpdialog == 0)
		trap(7);*/

	pgoad->ipdialog = (pgoad->ipdialog + 1) % pgoad->cpdialog;
}

void AddVaultGoadDialog(VAULT* pvault, int goadk, OID oidDialog)
{
	GOAD* pgoad = &pvault->mpgoadkgoad[goadk];

	if (pgoad->coidDialog < 4) {
		pgoad->aoidDialog[pgoad->coidDialog] = oidDialog;
		pgoad->coidDialog++;
	}
}

int FCanOpenVault(VAULT* pvault)
{
	if (pvault == nullptr || g_pgsCur == nullptr)
		return 0;

	// Duel by the Dragon (world 4, level 6) is the post-game vault. Retail
	// keeps it unavailable until the Clockwerk world-complete flag is set.
	const int levelKey =
		(static_cast<int>(g_pgsCur->gameWorldCur) << 8) |
		static_cast<int>(g_pgsCur->worldLevelCur);

	if (levelKey == 0x406 &&
		(g_pgsCur->aws[GAMEWORLD_Clockwerk].fws & FWS_Complete) == 0)
	{
		return 0;
	}

	return 1;
}

void OpenVault(VAULT* pvault)
{
	if (!FCanOpenVault(pvault))
		return;

	SetSmaGoal(pvault->psmaVault, (OID)910);

	if (pvault->pvolbtnGoad != nullptr) {
		pvault->pvolbtnGoad->pvtlo->pfnUnsubscribeLoObject(pvault->pvolbtnGoad, pvault);
		pvault->pvolbtnGoad = nullptr;
	}

	pvault->cpdialogPending = 0;

	if (g_binoc.fActive)
		PopUiActiveBlot(&g_ui);

	if ((pvault->fVault & 0xf000ffff) == 0)
		return;

	g_pwsCur->cvault++;
	g_pgsCur->grfvault |= pvault->fVault;
	pvault->psw->pvault = nullptr;
	g_plsCur->grfls |= FLS_Secondary;

	if ((g_pgsCur->grfgs & 8) == 0)
		g_pgsCur->grfgs |= 8;
	else if ((g_pgsCur->grfgs & 16) == 0)
		g_pgsCur->grfgs |= 16;

	if ((GetGameProgress() & FGS_AllClues) != 0)
		UnlockProgressRewards(FGS_AllClues);

	OID oid = (OID)894;

	if (pvault->fVault != 1) {
		oid = (OID)895;
		AutosaveCurrentGame(&g_saveData);
	}

	LO* ploReward = PloFindSwObject(pvault->psw, 257, oid, (LO*)pvault);

	if (ploReward != nullptr)
		ploReward->pvtlo->pfnAddLo(ploReward);

	if (pvault->idialogInstruct >= 0 && pvault->apdialogInstruct[pvault->idialogInstruct] != nullptr)
		pvault->apdialogPending[pvault->cpdialogPending++] = pvault->apdialogInstruct[pvault->idialogInstruct];
}

void DeleteVault(VAULT* pvault)
{
	delete pvault;
}

void GetBlueprintInfo(GRFVAULT* pgrfvault, int ipdialog)
{
	GRFVAULT grfvault = 0;

	switch (g_pgsCur->gameWorldCur) 
	{
		case 1:
		grfvault = 0x20000000;
		break;

		case 2:
		grfvault = 0x40000000;
		break;

		case 3:
		grfvault = 0x80000000;
		break;

		case 4:
		grfvault = 0x10000000;
		break;
	}

	if (pgrfvault != nullptr)
		*pgrfvault = grfvault;
}

SPRIZE* NewSprize()
{
	return new SPRIZE{};
}

void InitSprize(SPRIZE* psprize)
{
	InitSo(psprize);
	psprize->fCollect = 1;
}

void* GetSprizeSCollect(SPRIZE* psprize)
{
	return &psprize->sCollect;
}

void SetSprizeSCollect(SPRIZE* psprize, float sCollect)
{
	psprize->sCollect = sCollect;
}

void* GetSprizeFNoExit(SPRIZE* psprize)
{
	return &psprize->fNoExit;
}

void SetSprizeFNoExit(SPRIZE* psprize, int fNoExit)
{
	psprize->fNoExit = fNoExit;
}

int GetSprizeSize()
{
	return sizeof(SPRIZE);
}

void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* stream)
{
	LoadSoFromBrx(psprize, stream);
	InferExpl(&psprize->pexpl, psprize);

	DLI iterator;
	iterator.m_pdl = &psprize->dlChild;
	iterator.m_ibDle = psprize->dlChild.ibDle;
	iterator.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &iterator;

	ASEG* aseg = (ASEG*)psprize->dlChild.ploFirst;

	iterator.m_ppv = aseg != nullptr
		? reinterpret_cast<void**>(reinterpret_cast<char*>(aseg) + iterator.m_ibDle)
		: nullptr;

	while (aseg != nullptr)
	{
		const auto* basic = static_cast<const BASIC*>(aseg);

		if (basic->pvtlo != nullptr && (basic->pvtlo->grfcid & 0x40U) != 0)
		{
			if (psprize->cpaseg < static_cast<int>(std::size(psprize->apaseg)))
				psprize->apaseg[psprize->cpaseg++] = aseg;

			SnipLo(aseg);
		}

		// Fetch through the iterator after SnipLo(), since removing the
		// current object may update the link stored at m_ppv.
		aseg = iterator.m_ppv != nullptr ? static_cast<ASEG*>(*iterator.m_ppv) : nullptr;

		iterator.m_ppv = aseg != nullptr
			? reinterpret_cast<void**>(reinterpret_cast<char*>(aseg) + iterator.m_ibDle)
			: nullptr;
	}

	s_pdliFirst = iterator.m_pdliNext;
}

void CloneSprize(SPRIZE* psprize, SPRIZE* psprizeBase)
{
	CloneSo(psprize, psprizeBase);

	psprize->sCollect = psprizeBase->sCollect;
	psprize->fCollect = psprizeBase->fCollect;
	psprize->fNoExit = psprizeBase->fNoExit;
	psprize->coidAseg = psprizeBase->coidAseg;

	// Shallow copy of the arrays
	std::memcpy(psprize->aoidAseg, psprizeBase->aoidAseg, sizeof(psprize->aoidAseg));
	std::memcpy(psprize->mpipasegfDone, psprizeBase->mpipasegfDone, sizeof(psprize->mpipasegfDone));

	// Shallow copy of the pointer members
	psprize->cpaseg = psprizeBase->cpaseg;
	std::memcpy(psprize->apaseg, psprizeBase->apaseg, sizeof(psprize->apaseg));

	// CloneSo can produce a clone-local explosion hierarchy. Resolve that
	// hierarchy before falling back to the shared/template explosion used by
	// shallow-cloned objects in the original game.
	InferExpl(&psprize->pexpl, psprize);

	if (psprize->pexpl == nullptr)
		psprize->pexpl = psprizeBase->pexpl;

	// Copy the flag for Jt done
	psprize->fJtDone = psprizeBase->fJtDone;
}

void BindSprize(SPRIZE* psprize)
{
	BindAlo(psprize);

	for (int i = 0; i < psprize->coidAseg; ++i) {
		ASEG* paseg = (ASEG*)PloFindSwNearest(psprize->psw, (OID)psprize->aoidAseg[i], psprize);

		if (paseg != nullptr && psprize->cpaseg < static_cast<int>(std::size(psprize->apaseg))) {
			psprize->apaseg[psprize->cpaseg++] = paseg;
		}
	}
}

void PostSprizeLoad(SPRIZE* psprize)
{
	PostAloLoad(psprize);

	for (int i = 0; i < psprize->cpaseg; ++i) {
		ASEG* paseg = psprize->apaseg[i];

		if (FWipingAseg(paseg) != 0) {
			psprize->fNoExit = 1;
		}

		SnipLo(paseg);
	}

	if (psprize->pvtsprize->pfnPcsFromSprize(psprize) == 0) {
		psprize->pvtlo->pfnRemoveLo(psprize);
	}
}

void UpdateSprize(SPRIZE* psprize, float dt)
{
	UpdateSo(psprize, dt);

	int fForceCollect = psprize->fForceCollect;
	psprize->fForceCollect = 0;

	PO* ppo = PpoCur();

	if (ppo == nullptr) {
		return;
	}

	if (psprize->pvtsprize->pfnPcsFromSprize(psprize) != 1) {
		return;
	}

	if (ppo->pzi.fCollectEnabled == 0) {
		return;
	}

	ALO* paloCollect = ppo->pzi.paloCollect;

	if (paloCollect == nullptr) {
		return;
	}

	glm::vec3 delta = glm::vec3(psprize->xf.posWorld) - glm::vec3(paloCollect->xf.posWorld);

	if (fForceCollect == 0) {
		float sCollect = ppo->pzi.sAutoCollect + psprize->sCollect;

		if (glm::dot(delta, delta) >= sCollect * sCollect) {
			return;
		}
	}

	psprize->pvtlo->pfnSetLoParent(psprize, nullptr);

	delta.z = 0.0f;

	float length = glm::length(delta);
	glm::vec3 direction = length < 0.0001f ? glm::vec3(0.0f) : delta / length;
	glm::vec3 velocity = glm::vec3(paloCollect->xf.v) + direction * VXY_SprizeBreakDefault + glm::vec3(g_normalZ) * VZ_SprizeBreakDefault;
	glm::vec3 v = velocity;

	psprize->pvtalo->pfnSetAloVelocityVec(psprize, &v);
	psprize->pvtsprize->pfnCollectSprize(psprize);
}

void CollectSprize(SPRIZE* psprize)
{
	psprize->pvtlo->pfnRemoveLo(psprize);

	for (int i = psprize->cpaseg - 1; i >= 0; --i) 
	{
		ASEG* paseg = psprize->apaseg[i];
		PasegaEnsureAseg(paseg, nullptr, SEEK_Current, 0.0f, 1.0f, 0);
	}

	psprize->pvtsprize->pfnEmitSprizeExplosion(psprize);
	HandleLoSpliceEvent(psprize, 2, 0, nullptr);
}

void EmitSprizeExplosion(SPRIZE* psprize)
{
	EXPL* pexpl = psprize->pexpl;
	if (pexpl == nullptr) {
		return;
	}

	EXPLSO explso{};
	explso.grfexplso = 1;
	explso.paloReference = psprize;

	if (pexpl->pvtexpl != nullptr && pexpl->pvtexpl->pfnExplodeExplExplso != nullptr)
		pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
}

PCS PcsFromSprize(SPRIZE* psprize)
{
	if (FIsLoInWorld(psprize) == 0)
		return PCS_Collected;

	if (psprize->fCollect == 0) 
		return PCS_NotCollectible;

	return PCS_Collectible;
}

void AddSprizeAseg(SPRIZE* psprize, OID oidAseg)
{
	psprize->aoidAseg[psprize->coidAseg++] = oidAseg;
}

void HandleSprizeMessage(SPRIZE* psprize, MSGID msgid, void* pv)
{
	HandleAloMessage(psprize, msgid, pv);

	if (msgid != MSGID_asega_limit) {
		return;
	}

	ASEGA* pasega = static_cast<ASEGA*>(pv);
	ASEG* paseg = pasega->paseg;

	for (int i = 0; i < psprize->cpaseg; ++i) {
		if (psprize->apaseg[i] == paseg) {
			psprize->mpipasegfDone[i] = 1;
		}
	}

	if (g_pjt != nullptr && pv == g_pjt->pasegaCur) {
		g_pjt->pvtlo->pfnUnsubscribeLoObject(g_pjt, psprize);
		psprize->fJtDone = 1;
	}
}

int FIgnoreSprizeIntersection(SPRIZE* psprize, SO* psoOther)
{
	return FIsBasicDerivedFrom(psoOther, CID_STEPGUARD) ||
		FIsBasicDerivedFrom(psoOther, CID_BOMB);
}

void AdjustSprizeNewXp(SPRIZE* psprize, XP* pxp, int ixpd)
{
	PZI* ppzi = PpziCur();
	SO* psoLeaf = pxp->axpd[1 - ixpd].psoLeaf;

	if (ppzi->paloCollect != nullptr && FFindLoParent(psoLeaf, ppzi->paloCollect) != 0) {
		psprize->fForceCollect = 1;
	}
}

void DeleteSprize(SPRIZE *psprize)
{
	delete psprize;
}

SCPRIZE* NewScprize()
{
	return new SCPRIZE{};
}

void InitScprize(SCPRIZE* pscprize)
{
	InitSprize(pscprize);
	pscprize->ichkCollected = IchkAllocChkmgr(&g_chkmgr);
}

int GetScprizeSize()
{
	return sizeof(SCPRIZE);
}

void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase)
{
	int ichkCollected = pscprizeBase->ichkCollected;

	CloneSprize(pscprize, pscprizeBase);

	pscprize->ichkCollected = ichkCollected;
}

PCS PcsFromScprize(SCPRIZE* pscprize)
{
	PCS pcs = PcsFromSprize(pscprize);

	if (pcs == PCS_Collectible && FGetChkmgrIchk(&g_chkmgr, pscprize->ichkCollected) != 0) {
		return PCS_Collected;
	}

	return pcs;
}

void CollectScprize(SCPRIZE* pscprize)
{
	SetChkmgrIchk(&g_chkmgr, pscprize->ichkCollected);
	CollectSprize(pscprize);
}

void DeleteScprize(SCPRIZE *pscprize)
{
	delete pscprize;
}

void StartupClueCtr(CLUECTR* pcluectr)
{
	pcluectr->pvtcluectr = &g_vtcluectr;
}

CLUE* NewClue()
{
	return new CLUE{};
}

void InitClue(CLUE* pclue)
{
	InitSprize(pclue);
	pclue->ibit = pclue->psw->cclueAll;
	pclue->psw->cclueAll++;
}

void SetClueDtFrameMax(CLUE* pclue, float dtFrameMax)
{
	pclue->dtFrameMax = dtFrameMax;
}

void* GetClueDtFrameMax(CLUE* pclue)
{
	return &pclue->dtFrameMax;
}

int GetClueSize()
{
	return sizeof(CLUE);
}

void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis)
{
	LoadSprizeFromBrx(pclue, pbis);
	SnipAloObjects(pclue, 1, asnipClue);

	pclue->apaloRender.clear();
	pclue->apaloRender.reserve(32);

	for (int oid = 792; oid <= 823; ++oid) {
		XFM* paloRender = static_cast<XFM*>(PloFindSwObject(pclue->psw, 1, (OID)oid, pclue));

		if (paloRender == nullptr) {
			break;
		}

		pclue->apaloRender.push_back(paloRender);
		SnipLo(paloRender);
	}

	pclue->cpaloRender = static_cast<int>(pclue->apaloRender.size());
}

void CloneClue(CLUE* pclue, CLUE* pclueBase)
{
	int ibit = pclue->ibit;

	CloneSprize(pclue, pclueBase);

	pclue->ibit = pclueBase->ibit;
	pclue->dtFrame = pclueBase->dtFrame;
	pclue->dtFrameMax = pclueBase->dtFrameMax;
	pclue->cpaloRender = pclueBase->cpaloRender;
	pclue->swSpin = pclueBase->swSpin;
	pclue->radSpin = pclueBase->radSpin;
	pclue->apaloRender = pclueBase->apaloRender;
	pclue->paloSmack = pclueBase->paloSmack;

	pclue->ibit = ibit;
}

void PostClueLoad(CLUE* pclue)
{
	PostSprizeLoad(pclue);

	pclue->radSpin = GRandInRange(-3.1415927f, 3.1415927f);
	pclue->swSpin = GRandInRange(s_lmClueSpin.gMin, s_lmClueSpin.gMax);

	if (NRandInRange(0, 1) != 0) {
		pclue->swSpin = -pclue->swSpin;
	}

	pclue->dtFrameMax *= GRandInRange(s_lmClueBounce.gMin, s_lmClueBounce.gMax);
	pclue->dtFrame = GRandInRange(0.0f, pclue->dtFrameMax * 2.0f);

	int ibit = static_cast<int>(pclue->ibit);
	int ibyte = ibit / 8;
	uint8_t mask = static_cast<uint8_t>(1u << (ibit & 7));

	if ((g_plsCur->abitClue.m_ab[ibyte] & mask) != 0) {
		pclue->pvtlo->pfnRemoveLo(pclue);
	}
}

void UpdateClue(CLUE* pclue, float dt)
{
	UpdateSprize(pclue, dt);

	if ((GetAvailableVaultFlags() & 0x20000U) == 0) {
		return;
	}

	if (pclue->pvtsprize->pfnPcsFromSprize(pclue) != PCS_Collectible)
		return;

	PZI* ppzi = PpziCur();

	if (ppzi->fCollectEnabled == 0 || ppzi->paloCollect == nullptr) {
		return;
	}

	ALO* paloCollect = ppzi->paloCollect;
	glm::vec3 toClue = glm::vec3(pclue->xf.posWorld) - glm::vec3(paloCollect->xf.posWorld);

	if (glm::dot(toClue, toClue) > sClueMagnetRadius * sClueMagnetRadius)
		return;

	glm::vec3 vCollect;
	glm::vec3 vClue;

	CalculateAloMovement(paloCollect, 0, paloCollect->xf.posWorld, &vCollect, nullptr, nullptr, nullptr);
	CalculateAloMovement(pclue, 0, pclue->xf.posWorld, &vClue, nullptr, nullptr, nullptr);

	float distance = glm::length(toClue);
	glm::vec3 direction = distance >= 0.0001f ? toClue / distance : glm::vec3(0.0f);
	glm::vec3 relativeMovement = glm::vec3(vClue) - glm::vec3(vCollect);

	if (glm::dot(relativeMovement, direction) > -50.0f) 
		pclue->pvtsprize->pfnCollectSprize(pclue);
}

void OnClueSmack(CLUE* pclue)
{
	int ibit = static_cast<int>(pclue->ibit);
	int ibyte = ibit / 8;
	uint8_t mask = static_cast<uint8_t>(1u << (ibit & 7));
	uint8_t& clueByte = g_plsCur->abitClue.m_ab[ibyte];

	if ((clueByte & mask) == 0) {
		clueByte |= mask;
		++g_plsCur->abitClue.m_cbitSet;
	}

	int cclue = g_plsCur->abitClue.m_cbitSet;
	int oid = -1;
	uint32_t grfgs = 0;

	if ((g_pgsCur->grfgs & 1U) == 0) {
		if (cclue > 0) {
			oid = 840;
			grfgs = 1;
		}
	}
	else if ((g_pgsCur->grfgs & 4U) == 0 && cclue >= g_psw->cclueAll) {
		oid = 842;
		grfgs = 4;
	}

	if (cclue >= g_psw->cclueAll) 
		StartSound((SFXID)106, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

	g_pgsCur->grfgs |= grfgs;

	DIALOG* pdialog = (DIALOG*)PloFindSwObject(g_psw, 1, (OID)oid, nullptr);

	if (pdialog != nullptr) {
		TriggerDialog(pdialog);
	}
}

void CollectClue(CLUE* pclue)
{
	g_cluectr.pvtblot->pfnShowBlot(&g_cluectr);

	SMACK* psmack = static_cast<SMACK*>(PripNewRipg(RIPT_Smack, nullptr));

	if (psmack == nullptr)
		OnClueSmack(pclue);
	else
	{
		if (g_plsCur->abitClue.m_cbitSet + 1 >= g_psw->cclueAll)
		{
			StartSound(static_cast<SFXID>(33), nullptr, nullptr, nullptr,
				3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
		}

		glm::vec3 pos;

		psmack->pv = pclue;
		psmack->psmacki =(SMACKI*)&s_smackiClue;

		ConvertAloPos(pclue, nullptr, &pclue->paloSmack->xf.pos, &pos);
		psmack->pvtrip->pfnInitRip(psmack, &pos, 1.0f, nullptr);

		psmack->paloRender = pclue->paloSmack;
	}

	CollectSprize(pclue);
}

void BreakClue(CLUE* pclue)
{
	pclue->pvtclue->pfnCollectClue(pclue);
}

void CollectClueSilent(CLUE* pclue)
{
	pclue->pvtlo->pfnRemoveLo(pclue);

	int ibit = static_cast<int>(pclue->ibit);
	int ibyte = ibit / 8;
	uint8_t mask = static_cast<uint8_t>(1u << (ibit & 7));
	uint8_t& clueByte = g_plsCur->abitClue.m_ab[ibyte];

	if ((clueByte & mask) == 0) 
	{
		clueByte |= mask;
		++g_plsCur->abitClue.m_cbitSet;
	}

	for (int i = pclue->cpaseg - 1; i >= 0; --i) 
		PasegaEnsureAseg(pclue->apaseg[i], nullptr, SEEK_Current, 0.0f, 1.0f, 0);

	HandleLoSpliceEvent(pclue, 2, 0, nullptr);
}

void GetClueCollected(CLUE* pclue, uint32_t* pfCollected)
{
	int ibit = static_cast<int>(pclue->ibit);
	uint8_t mask = static_cast<uint8_t>(1u << (ibit & 7));

	*pfCollected = (g_plsCur->abitClue.m_ab[ibit / 8] & mask) != 0;
}

void ImpactClue(CLUE* pclue, int fParentDirty)
{
	ImpactSo(pclue, fParentDirty);
}

int FAbsorbClueWkr(CLUE* pclue, WKR* pwkr)
{
	if ((pwkr->grfic & 8U) != 0) {
		pclue->pvtlo->pfnSetLoParent(pclue, nullptr);
		SetSoConstraints(pclue, CT_Free, nullptr, CT_Locked, nullptr);
	}

	FAbsorbSoWkr(pclue, pwkr);
	if ((pwkr->grfic & 16U) != 0) 
		pclue->pvtclue->pfnCollectClue(pclue);

	return 1;
}

void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro)
{
	// Retail tests bit 32 of the packed ALO state beginning at mtlk. With the
	// corrected field order, that bit is fHidden (not fRemerge).
	if (pclue->fHidden != 0)
		return;

	pclue->radSpin += pclue->swSpin * g_clock.dt;

	glm::mat3 matSpin{};
	LoadRotateMatrixRad(pclue->radSpin, &g_normalZ, &matSpin);

	glm::mat3 mat = pclue->xf.matWorld * matSpin;

	if (pclue->paloParent != nullptr)
		TiltMatUpright(&mat, nullptr, &mat);

	RO ro{};
	DupAloRo(pclue, pro, &ro);
	LoadMatrixFromPosRot(&pclue->xf.posWorld, &mat, &ro.model);

	const int frameCount = glm::min(
		pclue->cpaloRender,
		static_cast<int>(pclue->apaloRender.size()));

	if (frameCount > 0 && pclue->dtFrameMax > 0.000001f)
	{
		pclue->dtFrame += g_clock.dt;

		const float cycleDuration = pclue->dtFrameMax * 2.0f;

		while (pclue->dtFrame >= cycleDuration)
			pclue->dtFrame -= cycleDuration;
		while (pclue->dtFrame < 0.0f)
			pclue->dtFrame += cycleDuration;

		int frame = static_cast<int>(
			static_cast<float>(frameCount) *
			pclue->dtFrame /
			pclue->dtFrameMax);

		if (frame >= frameCount)
			frame = frameCount * 2 - (frame + 1);

		frame = glm::clamp(frame, 0, frameCount - 1);

		XFM* paloFrame = pclue->apaloRender[frame];

		if (paloFrame != nullptr)
		{
			glm::mat4 matFrame(1.0f);
			LoadMatrixFromPosRot(&paloFrame->posLocal, &paloFrame->matLocal, &matFrame);
			ro.model = ro.model * matFrame;
		}
	}

	RenderAloAll(pclue, pcm, &ro);
}
void CollectAllClues()
{
	CLUE* apclue[256];
	int cclue = CploFindSwObjectsByClass(g_psw, 261, CID_CLUE, nullptr, 256, (LO**)apclue);

	for (int i = cclue - 1; i >= 0; --i) {
		CLUE* pclue = apclue[i];

		CollectClueSilent(pclue);

		if (pclue->paloParent != nullptr) 
			pclue->paloParent->pvtlo->pfnRemoveLo(pclue->paloParent);
	}

	g_cluectr.pvtblot->pfnShowBlot(&g_cluectr);
}

void DeleteClue(CLUE *pclue)
{
	delete pclue;
}

LOCK* NewLock()
{
	return new LOCK{};
}

int GetLockSize()
{
	return sizeof(LOCK);
}

void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plock, pbis);
	SnipAloObjects(plock, 1, &asnipLockLoadBrx);
}

void CloneLock(LOCK* plock, LOCK* plockBase)
{
	CloneAlo(plock, plockBase);

	plock->psm = plockBase->psm;
	plock->psma = plockBase->psma;
	plock->paloKey = plockBase->paloKey;
}

void PostLockLoad(LOCK* plock)
{
	PostAloLoad(plock);
	SnipAloObjects(plock, 1, &asnipPostLockLoad);
}

void DeleteLock(LOCK* plock)
{
	delete plock;
}

LOCKG* NewLockg()
{
	return new LOCKG{};
}

int GetLockgSize()
{
	return sizeof(LOCKG);
}

void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plockg, pbis);
	SnipAloObjects(plockg, 1, &asnipLockgBrx);
}

void CloneLockg(LOCKG* plockg, LOCKG* plockgBase)
{
	CloneAlo(plockg, plockgBase);

	plockg->grfws = plockgBase->grfws;
	plockg->psm = plockgBase->psm;
	plockg->psma = plockgBase->psma;
	plockg->coidLock = plockgBase->coidLock;

	plockg->cplock = plockgBase->cplock;
	for (int i = 0; i < plockgBase->cplock; ++i)
		plockg->aplock[i] = plockgBase->aplock[i];

	for (int i = 0; i < 8; ++i)
		plockg->aoidLock[i] = plockgBase->aoidLock[i];
}

void PostLockgLoad(LOCKG* plockg)
{
	PostAloLoad(plockg);

	plockg->cplock = CploFindSwObjectsByClass(plockg->psw, 1, CID_LOCK, plockg, 8, reinterpret_cast<LO**>(plockg->aplock));

	for (int i = 0; i < plockg->coidLock && plockg->cplock < 8; ++i)
	{
		LOCK* plock = reinterpret_cast<LOCK*>(PloFindSwObject(plockg->psw, 4, (OID)plockg->aoidLock[i], plockg));

		if (plock != nullptr)
			plockg->aplock[plockg->cplock++] = plock;
	}

	const OID oidGoal = (g_pwsCur->fws & plockg->grfws) != 0 ? static_cast<OID>(559) : static_cast<OID>(558);

	for (int i = 0; i < plockg->cplock; ++i)
	{
		LOCK* plock = plockg->aplock[i];

		plock->psma = PsmaApplySm(plock->psm, plock, OID_Nil, 0);
		SetSmaGoal(plock->psma, oidGoal);
	}

	plockg->psma = PsmaApplySm(plockg->psm, plockg, OID_Nil, 0);
	SetSmaGoal(plockg->psma, oidGoal);
}

void SetLockgIndex(LOCKG* plockg, int ifws)
{
	plockg->grfws = 1 << (ifws + 1U & 31);
}

void AddLockgLock(LOCKG* plockg, int oidLock)
{
	if (plockg->coidLock >= 8)
		return;

	plockg->aoidLock[plockg->coidLock++] = (OID)oidLock;
}

void TriggerLockg(LOCKG* plockg)
{
	if (g_pjt != nullptr)
		TriggerJtLockg(g_pjt, plockg);
}

void DeleteLockg(LOCKG* plockg)
{
	delete plockg;
}

CLUECTR g_cluectr;
GRFVAULT s_agrfvaultInstruct[28] = 
{
	0x2,
	0x4,
	0x8,
	0x10,
	0x10000,
	0x20,
	0x40,
	0x80,
	0x100,
	0x200,
	0x20000,
	0x400,
	0x800,
	0x2000,
	0x1000,
	0x4000,
	0x8000,
	0,
	0,
	0,
	0,
	0,
	0,
	0x10000000,
	0x20000000,
	0x40000000,
	0x80000000,
	0x1,
};

SNIP s_asnipVault[26] = 
{
	{ 0, (OID)0x376, offsetof(VAULT, psmVault) },
	{ 0, (OID)0x37C, offsetof(VAULT, pxfmJt) },
	{ 4, (OID)0x377, offsetof(VAULT, atmbl[0]) },
	{ 4, (OID)0x378, offsetof(VAULT, atmbl[1]) },
	{ 4, (OID)0x379, offsetof(VAULT, atmbl[2]) },
	{ 6, (OID)0x3A2, offsetof(VAULT, apdialogInstruct[0]) },
	{ 6, (OID)0x3A4, offsetof(VAULT, apdialogInstruct[1]) },
	{ 6, (OID)0x3A5, offsetof(VAULT, apdialogInstruct[2]) },
	{ 6, (OID)0x3A6, offsetof(VAULT, apdialogInstruct[3]) },
	{ 6, (OID)0x3A7, offsetof(VAULT, apdialogInstruct[5]) },
	{ 6, (OID)0x3A8, offsetof(VAULT, apdialogInstruct[6]) },
	{ 6, (OID)0x3A9, offsetof(VAULT, apdialogInstruct[7]) },
	{ 6, (OID)0x3AB, offsetof(VAULT, apdialogInstruct[8]) },
	{ 6, (OID)0x3AC, offsetof(VAULT, apdialogInstruct[9]) },
	{ 6, (OID)0x3AD, offsetof(VAULT, apdialogInstruct[10]) },
	{ 6, (OID)0x3AE, offsetof(VAULT, apdialogInstruct[11]) },
	{ 6, (OID)0x3AF, offsetof(VAULT, apdialogInstruct[12]) },
	{ 6, (OID)0x3B0, offsetof(VAULT, apdialogInstruct[13]) },
	{ 6, (OID)0x3B1, offsetof(VAULT, apdialogInstruct[14]) },
	{ 6, (OID)0x3B2, offsetof(VAULT, apdialogInstruct[15]) },
	{ 6, (OID)0x3B3, offsetof(VAULT, apdialogInstruct[16]) },
	{ 6, (OID)0x3B4, offsetof(VAULT, apdialogInstruct[23]) },
	{ 6, (OID)0x3B5, offsetof(VAULT, apdialogInstruct[24]) },
	{ 6, (OID)0x3B6, offsetof(VAULT, apdialogInstruct[25]) },
	{ 6, (OID)0x3B7, offsetof(VAULT, apdialogInstruct[26]) },
	{ 6, (OID)0x3D8, offsetof(VAULT, apdialogInstruct[27]) },
};

float VXY_SprizeBreakDefault = 100.0;
float VZ_SprizeBreakDefault = 0.0;
SNIP asnipClue[1] =
{
	0, (OID)0x339, offsetof(CLUE, paloSmack)
};

LM s_lmClueSpin = {0.8, 1.2};
LM s_lmClueBounce = {0.89999998, 1.1};
float sClueMagnetRadius = 300.0;
SNIP asnipLockLoadBrx  = {0, (OID)0x22B, offsetof(LOCK, psm)};
SNIP asnipPostLockLoad = {0, (OID)0x22D, offsetof(LOCK, paloKey)};
SNIP asnipLockgBrx = {0, (OID)0x22C, offsetof(LOCKG, psm)};
const SMACKI s_smackiClue =
{
	glm::vec3(0.85f, 0.75f, 100.0f), // posTarget
	glm::vec3(1.0f, -3.0f, 0.0f),    // vTarget
	glm::vec3(-1.0f, 2.0f, 0.0f),    // vInitMin
	glm::vec3(1.0f, 3.0f, 0.0f),     // vInitMax
	glm::vec3(1.0f, 0.0f, 0.0f),     // normalSpin
	CLQ{1.0f, -1.2f, 0.25f,},        // clqScale
	(PFNSMACK)OnClueSmack
};

#include "po.h"
#include "binoc.h"
#include "match.h"
#include "expls.h"
#include "jt.h"
#include "call.h"
#include <cstdio>

PO* NewPo()
{
	return new PO{};
}

void InitPo(PO* ppo)
{
	InitSo(ppo);
	ppo->pzi.paloCollect = ppo;
	ppo->pzi.fCollectEnabled = 1;
	SetPoPlayable(ppo, 1);
}

void* GetPofPlayable(PO* ppo)
{
	return &ppo->fPlayable;
}

void ClonePo(PO* ppo, PO* ppoBase)
{
	CloneSo(ppo, ppoBase);

	ppo->fPlayable = ppoBase->fPlayable;
	ppo->fLockBase = ppoBase->fLockBase;
	ppo->posBasePrev = ppoBase->posBasePrev;
	ppo->tWakeRipple = ppoBase->tWakeRipple;
	ppo->pambWake = ppoBase->pambWake;
	ppo->pzi = ppoBase->pzi;
	ppo->paloUseCharm = ppoBase->paloUseCharm;

	if (ppo->fPlayable == 0) 
	{
		ppo->fPlayable = 1;
		SetPoPlayable(ppo, 0);
	}
}

void HandlePoMessage(PO* ppo, MSGID msgid, void* pv)
{
	HandleAloMessage(ppo, msgid, pv);

	if (msgid == MSGID_water_left)
	{
		StopSound(ppo->pambWake, 0);
		return;
	}

	if (msgid != MSGID_rip_removed)
		return;

	MATCH* pmatchRemoved = static_cast<MATCH*>(pv);

	for (int i = 0; i < ppo->cmatchCollect; ++i)
	{
		if (ppo->pmatch[i] == pmatchRemoved)
		{
			ppo->pmatch[i] = nullptr;
			return;
		}
	}
}

void OnPoActive(PO* ppo, int fActive, PO* ppoOther)
{
	if (fActive == 0)
	{
		for (int cpp = 1; cpp < 7; ++cpp)
			RevokeCmPolicy(g_pcm, 5, (CPP)cpp, 0, ppo, nullptr);

		return;
	}

	ppo->posBasePrev = ppo->xf.posWorld;
	SetCmPolicy(g_pcm, CPP_Default, &g_pcm->cptn, ppo, nullptr);
}

void GetPoCpdefi(PO* ppo, float dt, CPDEFI* pcpdefi)
{
	GetSoCpdefi(ppo, dt, pcpdefi);

	if (ppo->fLockBase) {
		pcpdefi->posBase = ppo->posBasePrev;
		return;
	}

	if (!g_pcm->fCut) {
		pcpdefi->posBase.z = GSmooth(ppo->posBasePrev.z, ppo->xf.posWorld.z, dt, &s_smpZ, nullptr);
	}

	ppo->posBasePrev = pcpdefi->posBase;
}

int FIsPoSoundBase(PO* ppo)
{
	return 1;
}

void MakePoActive(PO* ppo)
{
	PO *currentppo = PpoCur();

	if (ppo != currentppo)
		SwitchToPo(ppo);
}

bool FInvulnerablePo(PO* ppo)
{
	return (g_grfcht & 1U) != 0 || FSwHandsOff(ppo->psw);
}

PO* PpoCur()
{
	if (-1 < g_ippoCur) {
		return g_appo[g_ippoCur];
	}
	return nullptr;
}

PO* PpoStart()
{
	PO* ppoStart = PpoCur();

	if (ppoStart == nullptr)
	{
		if (g_pjt != nullptr && g_pjt->fPlayable)
			ppoStart = (PO*)g_pjt;

		if (ppoStart == nullptr && g_cppo > 0)
			ppoStart = g_appo[0];
	}

	return ppoStart;
}

int IppoFindPo(PO* ppo)
{
	for (int i = 0; i < g_cppo; i++) 
	{
		if (g_appo[i] == ppo) 
			return i;
	}

	return -1;
}

void AddPoToList(PO* ppo)
{
    PO* ppoFirst = g_appo[0];
	if (ppo->fPlayable != 0) {
		if (ppo->paloParent != NULL) {
			return;
		}

		if (FIsLoInWorld(ppo) != 0 && IppoFindPo(ppo) < 0) {
			if (g_cppo < static_cast<int>(sizeof(g_appo) / sizeof(g_appo[0])))
			{
				g_appo[g_cppo++] = ppo;
				ppoFirst = g_appo[0];
			}
			else if (ppo->oid == 5) {
				ppoFirst = ppo;

				if (g_pjt != NULL) {
					return;
				}
			}
		}
	}

	g_appo[0] = ppoFirst;
}

void RemovePoFromList(PO* ppo)
{
	int ippo = IppoFindPo(ppo);

	if (ippo < 0)
		return;

	if (ippo == g_ippoCur) 
		SwitchToIppo(-1);

	memmove(&g_appo[ippo], &g_appo[ippo + 1], (g_cppo - ippo - 1) * sizeof(PO*));

	if (ippo < g_ippoCur)
		g_ippoCur--;

	g_cppo--;
}

void OnPoAdd(PO* ppo)
{
	OnSoAdd(ppo);
	AddPoToList(ppo);
}

void OnPoRemove(PO* ppo)
{
	OnSoRemove(ppo);
	RemovePoFromList(ppo);
}

void SwitchToIppo(int ippo)
{
	if (ippo == g_ippoCur)
		return;

	PO *ppoOld = (g_ippoCur >= 0) ? g_appo[g_ippoCur] : nullptr;
	PO *ppoNew = (ippo >= 0 && ippo < g_cppo) ? g_appo[ippo] : nullptr;

	if (ppoOld != nullptr) 
	{
		ppoOld->fActive = 0;
		ppoOld->pvtpo->pfnOnPoActive(ppoOld, 0, ppoNew);
	}

	g_ippoCur = ippo;

	if (ppoNew != nullptr) 
	{
		ppoNew->fActive = 1;
		ppoNew->pvtpo->pfnOnPoActive(ppoNew, 1, ppoOld);
	}
}

void SetPoPlayable(PO* ppo, int fPlayable)
{
	if (ppo->fPlayable != fPlayable) 
	{
		ppo->fPlayable = fPlayable;

		if (fPlayable == 0) 
			RemovePoFromList(ppo);
		else 
			AddPoToList(ppo);
	}
}

void SwitchToPo(PO* ppo)
{
	int ippo = IppoFindPo(ppo);
	SwitchToIppo(ippo);
}

PZI* PpziCur()
{
	PO *ppo = PpoCur();

	if (ppo == nullptr) 
		return &s_pziDefault;

	return &ppo->pzi;
}

static ALO* PaloFindDescendantByOid(ALO* palo, OID oid)
{
	if (palo == nullptr)
		return nullptr;

	for (LO* plo = palo->dlChild.ploFirst; plo != nullptr; plo = plo->dleChild.ploNext)
	{
		if (plo->oid == oid && (plo->pvtlo->grfcid & 1U) != 0)
			return static_cast<ALO*>(plo);

		if ((plo->pvtlo->grfcid & 1U) == 0)
			continue;

		if (ALO* paloFound = PaloFindDescendantByOid(static_cast<ALO*>(plo), oid))
			return paloFound;
	}

	return nullptr;
}

ALO* PaloFindPoBag(PO* ppo)
{
	return PaloFindDescendantByOid(ppo, OID_step_bag);
}

void GetPoDiapi(PO* ppo, DIALOG* pdialog, DIAPI* pdiapi)
{
	pdiapi->fCallable = 1;
	pdiapi->fPlayable = 1;
}

JTHS JthsCurrentPo(PO* ppo)
{
	return JTHS_Normal;
}

void CollectPoPrize(PO* ppo, PCK pck, ALO* paloOther)
{
	if (pck == PCK_Key)
	{
		if (ppo->pasegCollectKey == nullptr || paloOther == nullptr)
			return;

		MATCH* pmatch = static_cast<MATCH*>(PripNewRipg(RIPT_Match, nullptr));

		ppo->pmatch[0] = pmatch;
		ppo->cmatchCollect = 1;

		if (pmatch != nullptr)
		{
			pmatch->pvtrip->pfnInitRip(pmatch, &paloOther->xf.posWorld, 1.0f, nullptr);
			SubscribeRipObject(pmatch, ppo);

			pmatch->dtLifetime = TFindAsegLabel(ppo->pasegCollectKey, (OID)319);
			pmatch->paloRender = ppo->paloCollectTarget[0];
			pmatch->paloLaunch = paloOther;
			pmatch->emitdv.dv = paloOther->psw->dvGravity;
		}

		ppo->pvtpo->pfnSetPoPck(ppo, PCK_Key);
	}
	else if (pck == PCK_Gold)
		ppo->pvtpo->pfnSetPoPck(ppo, PCK_Gold);
}

void SetPoPck(PO* ppo, PCK pck)
{
	if (ppo->pckCollect == pck)
		return;

	ppo->pckCollect = pck;

	if ((static_cast<int>(pck) & static_cast<int>(PCK_Key)) != 0)
		ppo->posCollectPrev = glm::vec3(0.0f);
}

int FTakePoDamage(PO* ppo, ZPR* pzpr)
{
	return 0;
}

void GetJthsCurrentPo(PO* ppo, JTHS* pjths)
{
	*pjths = JthsCurrentPo(ppo);
}

void PlayPoDialog(PO* ppo, DIALOG* pdialog)
{
	g_binoc.pdialogPlaying = pdialog;
	PushUiActiveBlot(&g_ui, (BLOT*)&g_binoc);
}

void HandleDialogButtons(JOY* pjoy)
{
	if (pjoy == nullptr)
		return;

	if (FActiveCplcy(&g_pcm->cpman))
		return;

	JOY* pjoyDialog = (g_grfjoyt & 4U) != 0 ? &g_joy : &g_joyZero;

	const bool fDialogButtonPressed =
		pjoyDialog->IsPressed(BTN_L1) ||
		pjoyDialog->IsPressed(BTN_R1);

	bool fDialogSkipped = false;

	if (g_pdialogCalling != nullptr &&
		g_pdialogCalling->dialogs == DIALOGS_Playing &&
		!g_pdialogCalling->fNoSkip &&
		fDialogButtonPressed &&
		*g_pdialogCalling->pfPlayed != 0)
	{
		FinishDialogEvents(g_pdialogCalling);
		fDialogSkipped = true;
	}

	const bool fTriggerButtonPressed =
		pjoy->IsPressed(BTN_L1) ||
		pjoy->IsPressed(BTN_R1);

	if (!fDialogSkipped && fTriggerButtonPressed && g_call.pdialogTriggered != nullptr)
		SetDialogDialogs(g_call.pdialogTriggered, DIALOGS_Triggered);
}

void UpdatePoCharmVisibility(PO* ppo)
{
	// Slot zero is the overhead collection target. The three backpack states
	// begin at slot one: no charm, silver charm, then gold charm.
	const int ccharm = g_pgsCur != nullptr ? g_pgsCur->ccharm : 0;
	const int icharmVisible = glm::clamp(ccharm, 0, CcharmMost()) + 1;

	for (int icharm = 1; icharm < 4; ++icharm)
	{
		ALO* paloCharm = ppo->pzi.mpccharmpaloCharm[icharm];

		if (paloCharm == nullptr)
			continue;

		if (!ppo->fHidden && icharm == icharmVisible)
			paloCharm->pvtlo->pfnAddLo(paloCharm);
		else
			paloCharm->pvtlo->pfnRemoveLo(paloCharm);
	}
}

void UpdatePo(PO* ppo, float dt)
{
	UpdateSo(ppo, dt);
	UpdatePoCharmVisibility(ppo);

	if ((static_cast<int>(ppo->pckCollect) & static_cast<int>(PCK_Key)) != 0)
		UpdatePoPrizeCollection(ppo);
}

void UsePoCharm(PO* ppo)
{
	if ((g_grfcht & 2U) == 0)
		g_pgsCur->ccharm = std::max(g_pgsCur->ccharm - 1, 0);

	ALO* paloUseCharm = ppo->paloUseCharm;
	// The release uses charm slot zero as the use-charm launch transform.
	ALO* paloCharmOrigin = ppo->pzi.mpccharmpaloCharm[0];

	if (paloUseCharm == nullptr)
	{
		if (paloCharmOrigin == nullptr)
			return;

		g_emitripCharm.paloRender = ppo->pzi.mpccharmpaloCharm[2];
		EmitRipsSphere(&paloCharmOrigin->xf.posWorld, &g_normalZ, 1, &g_emitripCharm, &g_emitvCharm, nullptr, nullptr);
		return;
	}

	if (paloCharmOrigin == nullptr)
		return;
	
	paloUseCharm->pvtlo->pfnAddLo(paloUseCharm);
	paloUseCharm->pvtalo->pfnSetAloParent(paloUseCharm, nullptr);

	glm::mat3 mat;
	TiltMatUpright(&paloCharmOrigin->xf.matWorld, nullptr, &mat);

	paloUseCharm->pvtalo->pfnTranslateAloToPos(paloUseCharm, &paloCharmOrigin->xf.posWorld);
	paloUseCharm->pvtalo->pfnRotateAloToMat(paloUseCharm, &mat);
	paloUseCharm->pvtalo->pfnSetAloVelocityVec(paloUseCharm, &g_vecZero);
	paloUseCharm->pvtalo->pfnSetAloAngularVelocityVec(paloUseCharm, &g_vecZero);

	ASEG* pasegUseCharm = static_cast<ASEG*>(PloFindSwObject(ppo->psw, 1, (OID)396, paloUseCharm));

	if (pasegUseCharm != nullptr)
		ApplyAseg(pasegUseCharm, paloUseCharm, 0.0f, 1.0f, 0, nullptr);
}

void UpdatePoPrizeCollection(PO* ppo)
{
	constexpr int kMaxCollectMatches = 9;
	constexpr float kEffectSpacing = 5.0f;
	constexpr float kEpsilon = 0.0001f;

	if (ppo == nullptr || ppo->psw == nullptr)
		return;

	EXPLS* pexpls = static_cast<EXPLS*>(ppo->psw->aploStock[29]);

	if (pexpls == nullptr)
		return;

	ALO* paloCollectTarget = nullptr;

	for (int i = 0; i < 4; ++i)
	{
		ALO* palo = ppo->paloCollectTarget[i];

		if (palo != nullptr && FIsLoInWorld(palo))
		{
			paloCollectTarget = palo;
			break;
		}
	}

	std::array <glm::vec3, kMaxCollectMatches> aposStart{};
	std::array <glm::vec3, kMaxCollectMatches> aposEnd{};
	int cSegment = 0;

	if (paloCollectTarget != nullptr)
	{
		glm::vec3 posTarget = paloCollectTarget->xf.posWorld;

		if (glm::dot(ppo->posCollectPrev, ppo->posCollectPrev) < kEpsilon * kEpsilon)
			ppo->posCollectPrev = posTarget;

		aposStart[0] = ppo->posCollectPrev;
		aposEnd[0] = posTarget;
		cSegment = 1;
	}
	else
	{
		int cmatch = std::min(ppo->cmatchCollect, kMaxCollectMatches);

		for (int i = 0; i < cmatch; ++i)
		{
			MATCH* pmatch = ppo->pmatch[i];

			if (pmatch == nullptr || pmatch->tCreated > g_clock.t)
				continue;

			aposStart[cSegment] = pmatch->posPrev;
			aposEnd[cSegment] = pmatch->pos;
			++cSegment;
		}
	}

	EXPLSO explso{};
	explso.grfexplso = 4;

	for (int i = 0; i < cSegment; ++i)
	{
		const glm::vec3& posStart = aposStart[i];
		const glm::vec3& posEnd = aposEnd[i];

		glm::vec3 dpos = posEnd - posStart;
		float distance = glm::length(dpos);

		if (distance >= kEpsilon)
		{
			float du = kEffectSpacing / distance;

			for (float u = du; u < 1.0f; u += du)
			{
				explso.posOrigin = glm::mix(posStart, posEnd, u);
				pexpls->pvtexpls->pfnExplodeExplsExplso(pexpls, &explso);
			}
		}

		explso.posOrigin = posEnd;
		pexpls->pvtexpls->pfnExplodeExplsExplso(pexpls, &explso);
		ppo->posCollectPrev = posEnd;
	}
}

int GetPoSize()
{
	return sizeof(PO);
}

void DeletePo(PO* ppo)
{
	delete ppo;
}

int g_ippoCur = -1;
int g_cppo = 0;
PO* g_appo[16];
PZI s_pziDefault{};
SMP s_smpZ = {1000.0, 10.0, 1.0};
CLQ s_clqDtDamageToUSpring = { 0.0, 2.0, 0.0 };
CLQ s_clqUZapToUCel = { 1.0, 0.0, -0.75 };
CLQ s_clqUFlashToUCel = { 0.69999999, 0.3, 0.0 };
EMITRIP g_emitripCharm;
EMITV g_emitvCharm;

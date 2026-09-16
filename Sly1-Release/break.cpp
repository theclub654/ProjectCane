#include "break.h"
#include "chkpnt.h"
#include "expl.h"
#include "jt.h"
#include "lgn.h"

BRK* NewBrk()
{
	return new BRK{};
}

void InitBrk(BRK* pbrk)
{
	InitSo(pbrk);
	pbrk->lmBreakPosToV.gMin = 0.8;
	pbrk->lmBreakPosToV.gMax = 1.2;
	pbrk->cHitsToBreak = 1;
	pbrk->rvAbsorbWhack = 1.0;
	pbrk->ficg.grficSweep = '\x10';
	pbrk->ficg.grficShock = '\x10';
	pbrk->ficg.grficBomb = '\x10';
	pbrk->ficg.grficSmash = '\x10';
	pbrk->ficg.grficRush = '\x10';
	pbrk->ichkBroken = IchkAllocChkmgr(&g_chkmgr);
	pbrk->oidExplBreak = OID_Nil;
}

int GetBrkSize()
{
	return sizeof(BRK);
}

void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbrk, pbis);

	if (pbrk->oidExplBreak == OID_Nil) 
		InferExpl(&pbrk->pexplBreak, pbrk);
	else 
	{
		EXPL* plo = (EXPL*)PloFindSwObject(pbrk->psw, 0x101, (OID)pbrk->oidExplBreak, pbrk);
		pbrk->pexplBreak = plo;

		if (plo != nullptr) 
			SnipLo(plo);
	}
}

void SetBrkFRemain(BRK* pbrk, int fRemain)
{
	pbrk->fRemain = fRemain;
}

void* GetBrkFRemain(BRK* pbrk)
{
	return &pbrk->fRemain;
}

void SetBrkRvAbsorbWhack(BRK* pbrk, float rvAbsorbWhack)
{
	pbrk->rvAbsorbWhack = rvAbsorbWhack;
}

void* GetBrkRvAbsorbWhack(BRK* pbrk)
{
	return &pbrk->rvAbsorbWhack;
}

void SetBrkOidHiPri(BRK* pbrk, OID oidHiPri)
{
	pbrk->oidHiPri = oidHiPri;
}

void* GetBrkOidHiPri(BRK* pbrk)
{
	return &pbrk->oidHiPri;
}

void SetBrkLmBreakPosToV(BRK* pbrk, const LM* plmBreakPosToV)
{
	if (plmBreakPosToV != nullptr)
		pbrk->lmBreakPosToV = *plmBreakPosToV;
}

void* GetBrkLmBreakPosToV(BRK* pbrk)
{
	return &pbrk->lmBreakPosToV;
}

void SetBrkSvBreak(BRK* pbrk, float svBreak)
{
	pbrk->svBreak = svBreak;
}

void* GetBrkSvBreak(BRK* pbrk)
{
	return &pbrk->svBreak;
}

void SetBrkSwBreak(BRK* pbrk, float swBreak)
{
	pbrk->swBreak = swBreak;
}

void* GetBrkSwBreak(BRK* pbrk)
{
	return &pbrk->swBreak;
}

void SetBrkCHitsToBreak(BRK* pbrk, int cHitsToBreak)
{
	pbrk->cHitsToBreak = cHitsToBreak;
}

void* GetBrkCHitsToBreak(BRK* pbrk)
{
	return &pbrk->cHitsToBreak;
}

void* GetBrkBroken(BRK* pbrk)
{
	return &pbrk->fBroken;
}

void SetBrkFIgnoreBrkp(BRK* pbrk, int fIgnoreBrkp)
{
	pbrk->fIgnoreBrkp = fIgnoreBrkp;
}

void* GetBrkFIgnoreBrkp(BRK* pbrk)
{
	return &pbrk->fIgnoreBrkp;
}

void SetBrkExrk(BRK* pbrk, int exrk)
{
	pbrk->exrk = static_cast<EXRK>(exrk);
}

void* GetBrkExrk(BRK* pbrk)
{
	return &pbrk->exrk;
}

void SetBrkRExplodeScale(BRK* pbrk, float rExplodeScale)
{
	pbrk->rExplodeScale = rExplodeScale;
}

void* GetBrkRExplodeScale(BRK* pbrk)
{
	return &pbrk->rExplodeScale;
}

void SetBrkCcoin(BRK* pbrk, int ccoin)
{
	pbrk->ccoin = ccoin;
}

void* GetBrkCcoin(BRK* pbrk)
{
	return &pbrk->ccoin;
}

void SetBrkDposCoin(BRK* pbrk, glm::vec3 dposCoin)
{
	pbrk->dposCoin = dposCoin;
}

void* GetBrkDposCoin(BRK* pbrk)
{
	return &pbrk->dposCoin;
}

void SetBrkFSuppressDifficultyUpdate(BRK* pbrk, int fSuppressDifficultyUpdate)
{
	pbrk->fSuppressDifficultyUpdate = fSuppressDifficultyUpdate;
}

void* GetBrkFSuppressDifficultyUpdate(BRK* pbrk)
{
	return &pbrk->fSuppressDifficultyUpdate;
}

SFXID* PsfxEnsureBrksfxid(BRK* pbrk)
{
	return &PsfxEnsureBrk(pbrk, ENSK_Set)->sfxid;
}

float* PsfxEnsureBrksStart(BRK* pbrk)
{
	return &PsfxEnsureBrk(pbrk, ENSK_Set)->sStart;
}

float* PsfxEnsureBrksFull(BRK* pbrk)
{
	return &PsfxEnsureBrk(pbrk, ENSK_Set)->sFull;
}

float* PsfxEnsureBrkuVol(BRK* pbrk)
{
	return &PsfxEnsureBrk(pbrk, ENSK_Set)->uVol;
}

void SetBrkOidExplBreak(BRK* pbrk, OID oidExplBreak)
{
	pbrk->oidExplBreak = oidExplBreak;
}

void* GetBrkOidExplBreak(BRK* pbrk)
{
	return &pbrk->oidExplBreak;
}

void SetBrkFSkipBreakEffects(BRK* pbrk, int fSkipBreakEffects)
{
	pbrk->fSkipBreakEffects = fSkipBreakEffects;
}

void* GetBrkFSkipBreakEffects(BRK* pbrk)
{
	return &pbrk->fSkipBreakEffects;
}

void CloneBrk(BRK* pbrk, BRK* pbrkBase)
{
	int ichkBroken = pbrk->ichkBroken;
	CloneSo(pbrk, pbrkBase);

	// Retail CloneSo reaches CloneLo, whose raw class-sized copy also copies
	// every BRK field.  Our CloneSo copies only the SO portion, so reproduce
	// the derived part explicitly or cloned breakables lose their BRP table.
	pbrk->cbrp = pbrkBase->cbrp;
	std::memcpy(pbrk->abrp, pbrkBase->abrp, sizeof(pbrk->abrp));
	pbrk->oidExplBreak = pbrkBase->oidExplBreak;
	pbrk->pexplBreak = pbrkBase->pexplBreak;
	pbrk->exrk = pbrkBase->exrk;
	pbrk->rExplodeScale = pbrkBase->rExplodeScale;
	pbrk->rvAbsorbWhack = pbrkBase->rvAbsorbWhack;
	pbrk->lmBreakPosToV = pbrkBase->lmBreakPosToV;
	pbrk->svBreak = pbrkBase->svBreak;
	pbrk->swBreak = pbrkBase->swBreak;
	pbrk->oidHiPri = pbrkBase->oidHiPri;
	pbrk->grfbrk = pbrkBase->grfbrk;
	pbrk->coidTouch = pbrkBase->coidTouch;
	std::memcpy(pbrk->aoidTouch, pbrkBase->aoidTouch, sizeof(pbrk->aoidTouch));
	pbrk->ccidTouch = pbrkBase->ccidTouch;
	std::memcpy(pbrk->acidTouch, pbrkBase->acidTouch, sizeof(pbrk->acidTouch));
	pbrk->ccidAbsorb = pbrkBase->ccidAbsorb;
	std::memcpy(pbrk->acidAbsorb, pbrkBase->acidAbsorb, sizeof(pbrk->acidAbsorb));
	pbrk->fBreakOnUpdate = pbrkBase->fBreakOnUpdate;
	pbrk->fRemain = pbrkBase->fRemain;
	pbrk->fBroken = pbrkBase->fBroken;
	pbrk->cHitsToBreak = pbrkBase->cHitsToBreak;
	pbrk->cHits = pbrkBase->cHits;
	pbrk->fIgnoreBrkp = pbrkBase->fIgnoreBrkp;
	pbrk->ccoin = pbrkBase->ccoin;
	pbrk->dposCoin = pbrkBase->dposCoin;
	pbrk->fSkipBreakEffects = pbrkBase->fSkipBreakEffects;
	pbrk->fSuppressDifficultyUpdate = pbrkBase->fSuppressDifficultyUpdate;

	pbrk->ichkBroken = ichkBroken;

	if (pbrkBase->psfxBreak != nullptr) 
		pbrk->psfxBreak = std::make_shared<SFX>(*pbrkBase->psfxBreak);
	else
		pbrk->psfxBreak.reset();
}

void PostBrkLoad(BRK* pbrk)
{
	PostAloLoad(pbrk);
	PostSwCallback(pbrk->psw, (PFNMQ)PostBrkLoadCallbackHookup, pbrk, MSGID_callback, nullptr);
}

void PostBrkLoadCallbackHookup(BRK* pbrk, MSGID msgid, void* pv)
{
	if (FGetChkmgrIchk(&g_chkmgr, pbrk->ichkBroken) != 0) {
		pbrk->fBreakOnUpdate = 1;
	}

	DLI dli;
	dli.m_pdl = &pbrk->dlChild;
	dli.m_ibDle = pbrk->dlChild.ibDle;
	dli.m_pdliNext = s_pdliFirst;

	BRKP* plo = (BRKP*)pbrk->dlChild.ploFirst;

	if (plo != nullptr) {
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(plo) + dli.m_ibDle);
	}

	s_pdliFirst = &dli;

	while (plo != nullptr) {
		bool fProcess = pbrk->fBreakOnUpdate == 0;

		if (pbrk->fBreakOnUpdate != 0 && FIsBasicDerivedFrom(plo, (CID)CID_CLUE) != 0) {
			uint32_t fCollected = 0;
			GetClueCollected((CLUE*)plo, &fCollected);

			if (fCollected == 0) {
				fProcess = true;
			} 
		}

		if (fProcess) {
			bool fIgnored =
				FIsBasicDerivedFrom(plo, CID_TARGET) != 0 ||
				FIsBasicDerivedFrom(plo, CID_EXPLS) != 0 ||
				FIsBasicDerivedFrom(plo, CID_PROXY) != 0;

			if (!fIgnored) {
				if (FIsBasicDerivedFrom(plo, CID_BRKP) != 0) {
					plo->pbrk = pbrk;
				}

				BRP* pbrpMatch = nullptr;

				for (int i = 0; i < pbrk->cbrp; ++i) {
					BRP* pbrp = &pbrk->abrp[i];

					if (FMatchesLoName((LO*)plo, pbrp->oid) != 0) {
						pbrp->plo = (LO*)plo;
						pbrpMatch = pbrp;
						break;
					}
				}

				bool fKeep = false;

				if (pbrpMatch != nullptr) {
					fKeep =
						pbrpMatch->brpt == BRPT_Disappear ||
						pbrpMatch->brpt == BRPT_Remain ||
						pbrpMatch->brpt == BRPT_RemainFixed;
				}
				else {
					BRP* pbrp = &pbrk->abrp[pbrk->cbrp++];
					pbrp->brpt = BRPT_Appear;
					pbrp->plo = (LO*)plo;
				}

				if (!fKeep) {
					plo->pvtlo->pfnRemoveLo(plo);
				}
			}
		}

		plo = (BRKP*)*dli.m_ppv;

		if (plo != nullptr) {
			dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(plo) + dli.m_ibDle);
		}
	}

	s_pdliFirst = dli.m_pdliNext;
}

void UpdateBrk(BRK* pbrk, float dt)
{
	UpdateSo(pbrk, dt);

	if (pbrk->fBreakOnUpdate != 0) 
	{
		pbrk->fBreakOnUpdate = 0;
		pbrk->pvtbrk->pfnBreakBrk(pbrk);
	}
}

int FAbsorbBrkWkr(BRK* pbrk, WKR* pwkr)
{
	if (pbrk->fBroken)
		return 1;

	if (pwkr->grfic == 0 || pwkr->ploSource == nullptr)
		return 0;

	SO* psoSource = reinterpret_cast<SO*>(pwkr->ploSource);

	if ((psoSource->pvtlo->grfcid & 2) == 0)
		return 0;

	bool fAbsorb = false;

	for (int icid = 0; icid < pbrk->ccidAbsorb; ++icid) {
		if (FIsBasicDerivedFrom(pwkr->ploSource, pbrk->acidAbsorb[icid])) {
			fAbsorb = true;
			break;
		}
	}

	if (!fAbsorb)
		fAbsorb = FCheckBrkTouchObject(pbrk, psoSource) != 0;

	if (!fAbsorb)
		return 0;

	if ((pwkr->grfic & 4) != 0)
		return 1;

	if ((pwkr->grfic & 8) != 0) 
	{
		glm::vec3 v = pwkr->v * pbrk->rvAbsorbWhack;
		SetSoConstraints(pbrk, CT_Free, nullptr, CT_Free, nullptr);
		ApplySoImpulse(pbrk, &pwkr->pos, &v, pwkr->sftMax);
	}

	if ((pwkr->grfic & 16) == 0)
		return 1;

	/*if (pwkr->ploSource == reinterpret_cast<LO*>(PpoCur()) && (pwkr->grftak & 16) == 0)
		TriggerJoyRumbleRumk(&g_joy, RUMK_Break, 0.3f);*/

	++pbrk->cHits;
	void* pvArgs = &pbrk->cHits;
	HandleLoSpliceEvent(pbrk, 20, 1, &pvArgs);

	if (pbrk->cHits < pbrk->cHitsToBreak)
		return 1;

	pbrk->pvtbrk->pfnBreakBrk(pbrk);
	return 1;
}

void BreakBrk(BRK* pbrk)
{
	if (pbrk->fBroken)
		return;

	bool fBreakStartup = FGetChkmgrIchk(&g_chkmgr, pbrk->ichkBroken) == 0;

	if (!pbrk->fSkipBreakEffects) 
	{
		for (int ibrp = 0; ibrp < pbrk->cbrp; ++ibrp) {
			BRP* pbrp = &pbrk->abrp[ibrp];

			if (pbrp->brpt == BRPT_Disappear)
				continue;

			SO* pso = reinterpret_cast<SO*>(pbrp->plo);

			if (pso == nullptr)
				continue;

			bool fHighPriority = FMatchesLoName(pso, (OID)pbrk->oidHiPri) != 0;

			const bool fOverflow = FOverflowSwLo(pbrk->psw, pso, fHighPriority) != 0;

			if (fOverflow)
				continue;

			BRPT brpt = pbrp->brpt;
			bool fAlo = (pso->pvtlo->grfcid & 1) != 0;
			bool fSo = (pso->pvtlo->grfcid & 2) != 0;
			// Retail's `(uint)(brpt + ~BRPT_Remain) < 2` selects enum
			// values 2..3: Fixed and RemainFixed. It does not select Remain.
			bool fFixed = brpt == BRPT_Fixed || brpt == BRPT_RemainFixed;

			if (fAlo && !fFixed) {
				glm::vec3 v(
					GRandInRange(-pbrk->svBreak, pbrk->svBreak),
					GRandInRange(-pbrk->svBreak, pbrk->svBreak),
					GRandInRange(-pbrk->svBreak, pbrk->svBreak)
				);

				float rPosToVelocity = GRandInRange(pbrk->lmBreakPosToV.gMin, pbrk->lmBreakPosToV.gMax);
				v += pso->xf.pos * rPosToVelocity;

				pso->pvtalo->pfnSetAloVelocityVec(pso, &v);

				glm::vec3 w(
					GRandInRange(-pbrk->swBreak, pbrk->swBreak),
					GRandInRange(-pbrk->swBreak, pbrk->swBreak),
					GRandInRange(-pbrk->swBreak, pbrk->swBreak)
				);

				pso->pvtalo->pfnSetAloAngularVelocityVec(pso, &w);

				if (fSo)
					SetSoConstraints(pso, CT_Free, nullptr, CT_Free, nullptr);
			}

			if (brpt == BRPT_Appear || brpt == BRPT_Fixed)
				pso->pvtlo->pfnAddLo(pso);

			if (fFixed || !fSo)
				pso->pvtlo->pfnSetLoParent(pso, pbrk->paloParent);
			else
				pso->pvtlo->pfnSetLoParent(pso, nullptr);

			if (fAlo) {
				if (fFixed) {
					pso->pvtalo->pfnSetAloVelocityVec(pso, &g_vecZero);
					pso->pvtalo->pfnSetAloAngularVelocityVec(pso, &g_vecZero);
				}
				else {
					bool fClue = FIsBasicDerivedFrom(pso, CID_CLUE) != 0;
					bool fDprize = FIsBasicDerivedFrom(pso, CID_DPRIZE) != 0;

					if (fClue || fDprize) {
						glm::mat3 matUpright;
						TiltMatUpright(&pso->xf.mat, nullptr, &matUpright);
						pso->pvtalo->pfnRotateAloToMat(pso, &matUpright);
						pso->pvtalo->pfnSetAloAngularVelocityVec(pso, &g_vecZero);

						if (fClue) {
							SetSoConstraints(pso, CT_Free, nullptr, CT_Locked, nullptr);
						}
						else {
							bool fKey = FIsBasicDerivedFrom(pso, CID_KEY) != 0;
							pso->pvtso->pfnDisplaceSo(pso, fKey ? 1 : 3);
						}
					}
				}
			}

			struct
			{
				BRK* pbrk;
				SO* pso;
			} msgBreakPart = { pbrk, pso };

			pso->pvtlo->pfnSendLoMessage(pso, MSGID_break_piece, &msgBreakPart);

		}

		if (fBreakStartup) {
			float sRadius = 0.0f;

			if (pbrk->exrk == EXRK_Self)
				sRadius = std::min(pbrk->sRadiusRenderSelf, pbrk->sRadiusSelf);
			else if (pbrk->exrk == EXRK_All)
				sRadius = std::min(pbrk->sRadiusRenderAll, pbrk->sRadiusAll);

			if (pbrk->pexplBreak == nullptr) {
				StandardSmokeCloud(&pbrk->xf.posWorld, sRadius);
			}
			else {
				EXPLSO explso{};
				explso.grfexplso = 9;
				explso.paloReference = pbrk;
				explso.sRadius = sRadius;
				explso.rScale = pbrk->rExplodeScale;

				pbrk->pexplBreak->pvtexplo->pfnExplodeExploExplso((EXPLO*)pbrk->pexplBreak, &explso);
			}

			glm::vec3 posCoin;
			ConvertAloPos(pbrk, nullptr, &pbrk->dposCoin, &posCoin);
			OnDifficultyBreak(&g_difficulty, &posCoin, pbrk->ccoin, pbrk->fSuppressDifficultyUpdate);

			if (pbrk->psfxBreak == nullptr) {
				StartSound((SFXID)42, nullptr, nullptr, &pbrk->xf.posWorld, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
			}
			else {
				SFX* psfx = pbrk->psfxBreak.get();
				StartSound(psfx->sfxid, nullptr, nullptr, &pbrk->xf.posWorld, psfx->sStart, psfx->sFull, psfx->uVol, 0.0f, 0.0f, nullptr, nullptr);
			}
		}

		if (!pbrk->fRemain)
			pbrk->pvtlo->pfnRemoveLo(pbrk);
	}

	pbrk->fBroken = 1;

	SetChkmgrIchk(&g_chkmgr, pbrk->ichkBroken);
	pbrk->pvtlo->pfnSendLoMessage(pbrk, MSGID_break_break, pbrk);

	if (fBreakStartup)
	{
		HandleLoSpliceEvent(pbrk, 2, 0, nullptr);
	}
	else
		HandleLoSpliceEvent(pbrk, 22, 0, nullptr);
}

void SetBrkBroken(BRK* pbrk, int fBroken)
{
	pbrk->fBroken = fBroken;
	if (fBroken == 0)
		ClearChkmgrIchk(&g_chkmgr, pbrk->ichkBroken);
	else
		SetChkmgrIchk(&g_chkmgr, pbrk->ichkBroken);
}

void SetBrkExclude(BRK* pbrk, OID oid)
{
	if (pbrk->cbrp < 16) 
	{
		pbrk->abrp[pbrk->cbrp].oid = oid;
		pbrk->abrp[pbrk->cbrp].brpt = BRPT_Disappear;
		pbrk->cbrp = pbrk->cbrp + 1;
	}
}

void SetBrkRemain(BRK* pbrk, OID oid)
{
	if (pbrk->cbrp < 16) 
	{
		pbrk->abrp[pbrk->cbrp].oid = oid;
		pbrk->abrp[pbrk->cbrp].brpt = BRPT_Remain;
		pbrk->cbrp = pbrk->cbrp + 1;
	}
}

void SetBrkFixed(BRK* pbrk, OID oid)
{
	if (pbrk->cbrp < 16) 
	{
		pbrk->abrp[pbrk->cbrp].oid = oid;
		pbrk->abrp[pbrk->cbrp].brpt = BRPT_Fixed;
		pbrk->cbrp = pbrk->cbrp + 1;
	}
}

void SetBrkRemainFixed(BRK* pbrk, OID oid)
{
	if (pbrk->cbrp < 16) 
	{
		pbrk->abrp[pbrk->cbrp].oid = oid;
		pbrk->abrp[pbrk->cbrp].brpt = BRPT_RemainFixed;
		pbrk->cbrp = pbrk->cbrp + 1;
	}
}

void SetBrkOnPlayer(BRK* pbrk, int fBreakOnPlayer)
{
	if (fBreakOnPlayer) {
		pbrk->grfbrk = (pbrk->grfbrk & ~8u) | 1u;
		return;
	}

	pbrk->grfbrk &= ~1u;

	if (pbrk->grfbrk == 0)
		pbrk->grfbrk = 8u;
}

void GetBrkOnPlayer(BRK* pbrk, int* pfBreakOnPlayer)
{
	*pfBreakOnPlayer = pbrk->grfbrk & 1;
}

void SetBrkOnBomb(BRK* pbrk, int fBreakOnBomb)
{
	constexpr uint32_t GRFBRK_OnBomb = 1u << 2;
	constexpr uint32_t GRFBRK_Default = 1u << 3;

	if (fBreakOnBomb) {
		pbrk->grfbrk &= ~GRFBRK_Default;
		pbrk->grfbrk |= GRFBRK_OnBomb;
		return;
	}

	pbrk->grfbrk &= ~GRFBRK_OnBomb;

	if (pbrk->grfbrk == 0)
		pbrk->grfbrk = GRFBRK_Default;
}

void GetBrkOnBomb(BRK* pbrk, int* pfBreakOnBomb)
{
	*pfBreakOnBomb = pbrk->grfbrk >> 2 & 1;
}

void AddBrkTouchObject(BRK* pbrk, OID oid)
{
	if (pbrk->coidTouch >= 4)
		return;

	pbrk->aoidTouch[pbrk->coidTouch++] = oid;
	pbrk->grfbrk |= 2u;
}

void AddBrkTouchClass(BRK* pbrk, CID cid)
{
	if (pbrk->ccidTouch >= 4)
		return;

	pbrk->acidTouch[pbrk->ccidTouch++] = cid;
	pbrk->grfbrk |= 2u;
}

void AddBrkAbsorbClass(BRK* pbrk, CID cid)
{
	if (pbrk->ccidAbsorb >= 3)
		return;

	pbrk->acidAbsorb[pbrk->ccidAbsorb++] = cid;
	pbrk->grfbrk |= 2u;
}

SFX* PsfxEnsureBrk(BRK* pbrk, ENSK ensk)
{
	(void)ensk;

	if (pbrk->psfxBreak == nullptr)
		NewSfx(pbrk->psfxBreak);

	return pbrk->psfxBreak.get();
}

int FCheckBrkTouchObject(BRK* pbrk, SO* psoRoot)
{
	if (pbrk->grfbrk == 0) {
		return 1;
	}

	if ((pbrk->grfbrk & 1) != 0 && (PO*)psoRoot == PpoCur()) {
		return 1;
	}

	if ((pbrk->grfbrk & 4) != 0 && (psoRoot->pvtlo->grfcid & 32) != 0) {
		return 1;
	}

	if ((pbrk->grfbrk & 2) == 0) {
		return 0;
	}

	for (int ioid = 0; ioid < pbrk->coidTouch; ioid++) {
		if (FMatchesLoName(psoRoot, pbrk->aoidTouch[ioid])) {
			return 1;
		}
	}

	for (int icid = 0; icid < pbrk->ccidTouch; icid++) {
		if (FIsBasicDerivedFrom(psoRoot, pbrk->acidTouch[icid])) {
			return 1;
		}
	}

	return 0;
}

void DeleteBrk(BRK *pbrk)
{
	delete pbrk;
}

BRKP* NewBrkp()
{
	return new BRKP{};
}

void SetBrkpDtMaxLifetime(BRKP* pbrkp, float dtMaxLifetime)
{
	pbrkp->dtMaxLifetime = dtMaxLifetime;
}

void* GetBrkpDtMaxLifetime(BRKP* pbrkp)
{
	return &pbrkp->dtMaxLifetime;
}

void SetBrkpDtFade(BRKP* pbrkp, float dtFade)
{
	pbrkp->dtFade = dtFade;
}

void* GetBrkpDtFade(BRKP* pbrkp)
{
	return &pbrkp->dtFade;
}

void SetBrkpFIgnoreBrkp(BRKP* pbrkp, int fIgnoreBrkp)
{
	pbrkp->fIgnoreBrkp = fIgnoreBrkp;
}

void* GetBrkpFIgnoreBrkp(BRKP* pbrkp)
{
	return &pbrkp->fIgnoreBrkp;
}

int GetBrkpSize()
{
	return sizeof(BRKP);
}

void CloneBrkp(BRKP* prkp, BRKP* prkpBase)
{
	CloneSo(prkp, prkpBase);

	// Retail's class-sized clone carries the BRKP tail along with SO. Our
	// CloneSo stops at SO, so copy the derived state explicitly.
	prkp->brkps = prkpBase->brkps;
	prkp->tBrkps = prkpBase->tBrkps;
	prkp->dtMaxLifetime = prkpBase->dtMaxLifetime;
	prkp->dtFade = prkpBase->dtFade;
	prkp->pbrk = prkpBase->pbrk;
	prkp->fIgnoreBrkp = prkpBase->fIgnoreBrkp;
}

void HandleBrkpMessage(BRKP* pbrkp, MSGID msgid, void* pv)
{
	HandleAloMessage((ALO*)pbrkp, msgid, pv);

	if (msgid != MSGID_break_piece || pv == nullptr)
		return;

	MSGBRKP* pmsgbrkp = (MSGBRKP*)pv;

	if (pmsgbrkp->pbrkp == pbrkp)
		SetBrkpBrkps(pbrkp, BRKPS_Pending);
}

int FIgnoreBrkpIntersection(BRKP* pbrkp, SO* psoOther)
{
	if (psoOther->fLockedAll) 
		return 0;

	// Broken pieces are spawned in an overlapping cluster.  Letting every
	// fragment collide with every other fragment creates an O(n^2) contact
	// workload in the PC solver for the entire lifetime of the debris.  Static
	// level geometry is handled above, so fragments still land normally.
	if (FIsBasicDerivedFrom(psoOther, CID_BRKP))
		return 1;

	if (pbrkp->fIgnoreBrkp)
		return 1;

	if (pbrkp->pbrk != nullptr && pbrkp->pbrk->fIgnoreBrkp)
		return 1;

	return 1;
}

void UpdateBrkp(BRKP* pbrkp, float dt)
{
	UpdateSo(pbrkp, dt);

	BRKPS brkps = pbrkp->brkps;

	if (brkps == BRKPS_Pending && (pbrkp->cframeStatic > 1 || g_clock.t - pbrkp->tBrkps > pbrkp->dtMaxLifetime))
	{
		brkps = BRKPS_Fading;
	}

	SetBrkpBrkps(pbrkp, brkps);
}

void SetBrkpBrkps(BRKP* pbrkp, BRKPS brkps)
{
	if (brkps == pbrkp->brkps)
		return;

	pbrkp->brkps = brkps;
	pbrkp->tBrkps = g_clock.t;

	if (brkps == BRKPS_Fading)
		FadeAloOut(pbrkp, pbrkp->dtFade);
}

void* GetBrkpBrkps(BRKP* pbrkp)
{
	return &pbrkp->brkps;
}

void DeleteBrkp(BRKP* pbrkp)
{
	delete pbrkp;
}

BREAK* NewBreak()
{
	return new BREAK{};
}

void InitBreak(BREAK* pbreak)
{
	InitBrk(pbreak);

	pbreak->ccoin = -1;
	pbreak->grfbrk = 5;
}

int GetBreakSize()
{
	return sizeof(BREAK);
}

void CloneBreak(BREAK* pbreak, BREAK* pbreakBase)
{
	CloneBrk(pbreak, pbreakBase);
}

void DeleteBreak(BREAK* pbreak)
{
	delete pbreak;
}

FRAGILE* NewFragile()
{
	return new FRAGILE{};
}

void InitFragile(FRAGILE* pfragile)
{
	InitBrk(pfragile);

	pfragile->cnstrImpactTorque = CNSTR_Nil;
	pfragile->grfbrk = 1;
	pfragile->rImpactMass = 1.0;
	pfragile->cnstrImpactForce = CNSTR_Nil;
}

void SetFragileRImpactMass(FRAGILE* pfragile, float rImpactMass)
{
	pfragile->rImpactMass = rImpactMass;
}

void* GetFragileRImpactMass(FRAGILE* pfragile)
{
	return &pfragile->rImpactMass;
}

void SetFragileCnstrImpactForce(FRAGILE* pfragile, CNSTR cnstrImpactForce)
{
	pfragile->cnstrImpactForce = cnstrImpactForce;
}

void* GetFragileCnstrImpactForce(FRAGILE* pfragile)
{
	return &pfragile->cnstrImpactForce;
}

void SetFragileCnstrImpactTorque(FRAGILE* pfragile, CNSTR cnstrImpactTorque)
{
	pfragile->cnstrImpactTorque = cnstrImpactTorque;
}

void* GetFragileCnstrImpactTorque(FRAGILE* pfragile)
{
	return &pfragile->cnstrImpactTorque;
}

void SetFragilePsoImpacting(FRAGILE* pfragile, SO* psoImpacting)
{
	pfragile->psoImpacting = psoImpacting;
}

void* GetFragilePsoImpacting(FRAGILE* pfragile)
{
	return &pfragile->psoImpacting;
}

void AdjustFragileNewXp(FRAGILE* pfragile, XP* pxp, int ixpd)
{
	if (pfragile->fBroken)
		return;

	SO* psoRoot = pxp->axpd[1 - ixpd].psoRoot;
	const int fTouch = FCheckBrkTouchObject(pfragile, psoRoot);

	if (pfragile->fBreakOnUpdate)
		return;

	if (!fTouch)
		return;

	if (pfragile->cnstrImpactForce != CNSTR_Nil)
		SetSoCnstrForce(pfragile, pfragile->cnstrImpactForce);

	if (pfragile->cnstrImpactTorque != CNSTR_Nil)
		SetSoCnstrTorque(pfragile, pfragile->cnstrImpactTorque);

	pfragile->fBreakOnUpdate = 1;
	pfragile->psoImpacting = psoRoot;
	pfragile->m *= pfragile->rImpactMass;
}

int GetFragileSize()
{
	return sizeof(FRAGILE);
}

void CloneFragile(FRAGILE* pfragile, FRAGILE* pfragileBase)
{
	CloneBrk(pfragile, pfragileBase);

	pfragile->rImpactMass = pfragileBase->rImpactMass;
	pfragile->cnstrImpactForce = pfragileBase->cnstrImpactForce;
	pfragile->cnstrImpactTorque = pfragileBase->cnstrImpactTorque;
	pfragile->psoImpacting = pfragileBase->psoImpacting;
}

void DeleteFragile(FRAGILE* pfragile)
{
	delete pfragile;
}

ZAPBREAK* NewZapbreak()
{
	return new ZAPBREAK{};
}

int GetZapbreakSize()
{
	return sizeof(ZAPBREAK);
}

void* GetZapbreakZpk(ZAPBREAK* pzapbreak)
{
	return &pzapbreak->zpk;
}

void SetZapbreakZpk(ZAPBREAK* pzapbreak, ZPK zpk)
{
	pzapbreak->zpk = zpk;
}

void CloneZapbreak(ZAPBREAK* pzapbreak, ZAPBREAK* pzapbreakBase)
{
	CloneFragile(pzapbreak, pzapbreakBase);

	pzapbreak->zpk = pzapbreakBase->zpk;
	pzapbreak->ppoZap = pzapbreakBase->ppoZap;
}

void AdjustZapbreakNewXp(ZAPBREAK* pzapbreak, XP* pxp, int ixpd)
{
	AdjustFragileNewXp(pzapbreak, pxp, ixpd);

	if (pzapbreak->fBroken)
		return;

	SO* psoRoot = pxp->axpd[1 - ixpd].psoRoot;

	if (FIsBasicDerivedFrom(psoRoot, CID_PO) && FCheckBrkTouchObject(pzapbreak, psoRoot))
		pzapbreak->ppoZap = (PO*)psoRoot;
}

void UpdateZapbreak(ZAPBREAK* pzapbreak, float dt)
{
	UpdateBrk(pzapbreak, dt);

	if (pzapbreak->ppoZap == nullptr)
		return;

	PO* ppoZap = pzapbreak->ppoZap;
	pzapbreak->ppoZap = nullptr;

	if (ppoZap != PpoCur())
		return;

	ZPR zpr;
	InitZpr(&zpr, pzapbreak->zpk, pzapbreak);
	ppoZap->pvtpo->pfnFTakePoDamage(ppoZap, &zpr);
}

void DeleteZapbreak(ZAPBREAK* pzapbreak)
{
	delete pzapbreak;
}

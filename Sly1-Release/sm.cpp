#include "sm.h"
#include "aseg.h"

SM* NewSm()
{
	return new SM{};
}

int GetSmSize()
{
	return sizeof(SM);
}

void LoadSmFromBrx(SM* psm, CBinaryInputStream* pbis)
{
	InitDl(&psm->dlSma, offsetof(SMA, dleSm));

	psm->csms = static_cast<int>(pbis->U8Read());
	psm->asms.clear();
	psm->asms.resize(psm->csms);

	for (SMS& sms : psm->asms)
	{
		sms.oid = static_cast<OID>(pbis->S16Read());
		sms.oidNext = static_cast<OID>(pbis->S16Read());
	}

	for (SMS& sms : psm->asms)
	{
		if (sms.oidNext != OID_Nil)
			sms.ismsNext = IsmsFindSmRequired(psm, sms.oidNext);
	}

	psm->csmt = static_cast<int>(pbis->U8Read());
	psm->asmt.clear();
	psm->asmt.resize(psm->csmt);

	for (SMT& smt : psm->asmt)
	{
		smt.fAseg = static_cast<int>(pbis->S8Read());

		const OID oidFrom = static_cast<OID>(pbis->S16Read());
		const OID oidTo = static_cast<OID>(pbis->S16Read());

		smt.ismsFrom = IsmsFindSmRequired(psm, oidFrom);
		smt.ismsTo = IsmsFindSmRequired(psm, oidTo);
		smt.grfsmt = pbis->S32Read();
		smt.gProbability = pbis->F32Read();
	}

	LoadOptionsFromBrx(psm, pbis);

	pbis->S16Read();
	int ismtLoad = 0;
	for (SMT& smt : psm->asmt)
	{
		if (!smt.fAseg)
		{
			++ismtLoad;
			continue;
		}

		const CID cid = static_cast<CID>(pbis->S16Read());
		const OID oid = static_cast<OID>(pbis->S16Read());
		const OID oidContext = static_cast<OID>(pbis->S16Read());

		LO* plo = PloNew(cid, psm->psw, psm->paloParent, oid, oidContext);
		smt.paseg = static_cast<ASEG*>(plo);

		if (plo != nullptr)
		{
			if (plo->pvtlo->pfnLoadLoFromBrx != nullptr)
				plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);

			SnipLo(plo);
		}

		++ismtLoad;
	}

}

void CloneSm(SM* psm, SM* psmBase)
{
	CloneLo(psm, psmBase);

	psm->csms = psmBase->csms;
	psm->asms = psmBase->asms;
	psm->csmt = psmBase->csmt;
	psm->asmt = psmBase->asmt;
	psm->fDefault = psmBase->fDefault;
	psm->dlSma = psmBase->dlSma;
}

void SetSmFDefault(SM* psm, int fDefault)
{
	psm->fDefault = fDefault;
}

void* GetSmFDefault(SM* psm)
{
	return &psm->fDefault;
}

void* GetSmaPaloRoot(SMA* psma)
{
	return &psma->paloRoot;
}

void* GetSmaSvt(SMA* psma)
{
	return &psma->svtLocal;
}

void* GetSmaAsegaCur(SMA* psma)
{
	return &psma->pasegaCur;
}

void* GetSmaPsm(SMA* psma)
{
	return &psma->psm;
}

void PostSmLoad(SM* psm)
{
	PostLoLoad(psm);
	if (psm->fDefault != 0)
		PostSwCallback(psm->psw, (PFNMQ)PostSmLoadCallback, psm, MSGID_callback, nullptr);
}

void PostSmLoadCallback(SM* psm, MSGID msgid, void* pvData)
{
	ASEG* paseg = nullptr;

	for (int i = 0; i < psm->csmt; ++i) {
		paseg = psm->asmt[i].paseg;

		if (paseg != nullptr) {
			break;
		}
	}

	ALO* paloRoot = nullptr;

	if (paseg != nullptr && paseg->oidRoot != OID_Nil)
		paloRoot = reinterpret_cast<ALO*>(PloFindSwNearest(psm->psw, (OID)paseg->oidRoot, psm));

	PsmaApplySm(psm, paloRoot, OID_Nil, 0);
}

SMA* PsmaApplySm(SM* psm, ALO* paloRoot, OID oidInitialState, GRFAPL grfapl)
{
	auto psma = std::make_shared<SMA>();

	psma->pvtsma = &g_vtsma;
	psma->paloRoot = paloRoot;
	psma->grfapl = grfapl;
	psma->svtLocal = 1.0f;
	psma->psm = psm;
	psma->ismsNext = -1;

	// Retain ownership before publishing the raw pointer to lists or callbacks.
	psm->psw->smaOwners.push_back(psma);

	AppendDlEntry(&psm->psw->dlSma, psma.get());
	AppendDlEntry(&psm->dlSma, psma.get());

	if (oidInitialState != OID_Nil)
	{
		const int ismsInitial = IsmsFindSmRequired(psm, oidInitialState);

		psma->ismsCur = ismsInitial;
		psma->ismsGoal = ismsInitial;

		NotifySmaSpliceOnEnterState(psma.get(), -1, ismsInitial);
	}


	ChooseSmaTransition(psma.get());

	SMA* psmaRaw = psma.get();
	void* apv[1] =
	{
		&psmaRaw
	};

	HandleLoSpliceEvent(psm, 17, 1, apv);

	return psma.get();
}

SMA* PsmaFindSm(SM* psm, ALO* paloRoot)
{
	for (SMA* psma = psm->dlSma.psmaFirst; psma != nullptr; psma = psma->dleSm.psmaNext)
	{
		if (psma->paloRoot == paloRoot)
			return psma;
	}

	return nullptr;
}

int IsmsFindSmOptional(SM* psm, OID oid)
{
	for (int isms = 0; isms < psm->csms; isms++)
	{
		if (psm->asms[isms].oid == oid)
			return isms;
	}

	return -1;
}

int IsmsFindSmRequired(SM* psm, OID oid)
{
	int isms = IsmsFindSmOptional(psm, oid);

	if (isms < 0)
		isms = 0;

	return isms;
}

int  OidFromSmIsms(SM* psm, int isms)
{
	return psm->asms[isms].oid;
}

void RetractSma(SMA* psma)
{
	if (psma == nullptr)
		return;

	SM* psm = psma->psm;
	SW* psw = psm->psw;

	// Keep it alive until this function finishes.
	auto it = std::find_if(psw->smaOwners.begin(), psw->smaOwners.end(), [psma](const std::shared_ptr<SMA>& owner)
	{
		return owner.get() == psma;
	});

	std::shared_ptr<SMA> owner;

	if (it != psw->smaOwners.end())
		owner = *it;

	if (psma->pasegaCur != nullptr)
	{
		RetractAsega(psma->pasegaCur);
		psma->pasegaCur = nullptr;
	}

	FreeSwMqList(psw, psma->pmqFirst);
	psma->pmqFirst = nullptr;

	RemoveDlEntry(&psm->dlSma, psma);
	RemoveDlEntry(&psw->dlSma, psma);

	if (it != psw->smaOwners.end())
		psw->smaOwners.erase(it);

	HandleLoSpliceEvent(psm, 18, 0, nullptr);
}

void SetSmaGoal(SMA* psma, OID oid)
{
	if (psma == nullptr)
		return;

	const int ismsGoal = oid == OID_Nil ? -1 : IsmsFindSmRequired(psma->psm, oid);
	if (ismsGoal == psma->ismsGoal)
		return;

	psma->ismsGoal = ismsGoal;

	// Bit 1 means the current transition may be interrupted when the goal changes.
	if (psma->psmtCur != nullptr && (psma->psmtCur->grfsmt & 2) != 0)
	{
		EndSmaTransition(psma);

		if (psma->pasegaCur != nullptr)
		{
			RetractAsega(psma->pasegaCur);
			psma->pasegaCur = nullptr;
		}
	}

	if (psma->pasegaCur == nullptr)
		ChooseSmaTransition(psma);

}

void GetSmaGoal(const SMA* psma, OID* poid)
{
	if (poid == nullptr)
		return;

	*poid = OID_Nil;

	if (psma == nullptr || psma->psm == nullptr || psma->ismsGoal < 0)
		return;

	const std::size_t index = static_cast<std::size_t>(psma->ismsGoal);

	if (index < psma->psm->asms.size())
		*poid = psma->psm->asms[index].oid;
}

void GetSmaCur(SMA* psma, OID* poid)
{
	if (poid == nullptr)
		return;

	*poid = OID_Nil;

	if (psma == nullptr || psma->psm == nullptr || psma->ismsCur < 0)
		return;

	const std::size_t index = static_cast<std::size_t>(psma->ismsCur);

	if (index < psma->psm->asms.size())
		*poid = psma->psm->asms[index].oid;
}

void GetSmaNext(SMA* psma, OID* poid)
{
	if (poid == nullptr)
		return;

	*poid = OID_Nil;

	if (psma == nullptr || psma->psm == nullptr || psma->ismsNext < 0)
		return;

	const std::size_t index = static_cast<std::size_t>(psma->ismsNext);

	if (index < psma->psm->asms.size())
		*poid = psma->psm->asms[index].oid;
}

void SetSmaSvt(SMA* psma, float svt)
{
	psma->svtLocal = svt;
	if (psma->pasegaCur != nullptr)
		SetAsegaSpeed(psma->pasegaCur, svt);
}

void SeekSma(SMA* psma, OID oid)
{
	if (psma == nullptr || psma->psm == nullptr)
		return;

	const int ismsTarget = IsmsFindSmRequired(psma->psm, oid);
	const bool fStateChanged = psma->ismsCur != ismsTarget;
	const bool fGoalChanged  = psma->ismsGoal != -1 && psma->ismsGoal != ismsTarget;

	if (!fStateChanged && !fGoalChanged)
		return;

	NotifySmaSpliceOnEnterState(psma, psma->ismsCur, ismsTarget);

	psma->ismsCur = ismsTarget;
	psma->ismsGoal = ismsTarget;
	psma->ismsNext = -1;

	ChooseSmaTransition(psma);
}

void ChooseSmaTransition(SMA* psma)
{
	if (psma == nullptr || psma->psm == nullptr)
		return;

	SM* psm = psma->psm;
	auto FinishWithoutTransition = [psma]()
	{
		psma->psmtCur = nullptr;

		if (psma->pasegaCur != nullptr)
		{
			SnapAsega(psma->pasegaCur, 0);
			RetractAsega(psma->pasegaCur);
			psma->pasegaCur = nullptr;
		}

		SendSmaMessage(psma, MSGID_sma_transition, psma);
	};

	// Reaching the goal advances to that state's automatic next goal.
	if (psma->ismsGoal == psma->ismsCur)
	{
		const int ismsCur = psma->ismsCur;

		if (ismsCur >= 0 && static_cast<std::size_t>(ismsCur) < psm->asms.size())
			psma->ismsGoal = psm->asms[ismsCur].ismsNext;
		else
			psma->ismsGoal = -1;

	}


	if (psma->ismsGoal < 0)
	{
		FinishWithoutTransition();
		return;
	}

	const std::size_t stateCount = psm->asms.size();

	if (psma->ismsCur < 0 || psma->ismsGoal < 0 ||
		static_cast<std::size_t>(psma->ismsCur) >= stateCount ||
		static_cast<std::size_t>(psma->ismsGoal) >= stateCount)
	{
		psma->ismsGoal = -1;
		FinishWithoutTransition();
		return;
	}

	const int ismsGoal = psma->ismsGoal;
	const int ismsStart = psma->ismsCur;
	std::vector<int> route(stateCount, -1);
	route[ismsStart] = 0;

	int depth = 1;
	while (true)
	{
		bool changed = false;

		for (const SMT& smt : psm->asmt)
		{
			if (smt.ismsFrom < 0 || smt.ismsTo < 0 || static_cast<std::size_t>(smt.ismsFrom) >= stateCount || static_cast<std::size_t>(smt.ismsTo) >= stateCount)
				continue;

			if (route[smt.ismsFrom] == depth - 1 && route[smt.ismsTo] < 0)
			{
				route[smt.ismsTo] = depth;
				changed = true;
			}

			if ((smt.grfsmt & 1) != 0 && route[smt.ismsTo] == depth - 1 && route[smt.ismsFrom] < 0)
			{
				route[smt.ismsFrom] = depth;
				changed = true;
			}
		}

		// The original gives an explicit self-transition priority at depth one.
		if (depth == 1)
		{
			for (const SMT& smt : psm->asmt)
			{
				if (smt.ismsFrom == ismsStart && smt.ismsTo == ismsStart)
				{
					route[ismsStart] = 1;
					changed = true;
					break;
				}
			}
		}

		if (!changed)
		{
			if (route[ismsGoal] >= 0)
				break;

			psma->ismsGoal = -1;
			FinishWithoutTransition();
			return;
		}

		if (route[ismsGoal] > 0)
			break;

		++depth;
	}

	// Preserve only the reached goal at the final breadth level.
	for (std::size_t i = 0; i < route.size(); ++i)
	{
		if (static_cast<int>(i) != ismsGoal && route[i] == depth)
			route[i] = -1;
	}

	// Mark only nodes that lie on a shortest route back to the current state.
	int routeMarker = depth;
	while (depth > 1)
	{
		--depth;
		const int nextMarker = routeMarker + 1;

		for (const SMT& smt : psm->asmt)
		{
			if (smt.ismsFrom < 0 || smt.ismsTo < 0 || static_cast<std::size_t>(smt.ismsFrom) >= stateCount || static_cast<std::size_t>(smt.ismsTo) >= stateCount)
				continue;

			if (route[smt.ismsTo] == routeMarker && route[smt.ismsFrom] == depth)
				route[smt.ismsFrom] = nextMarker;

			if ((smt.grfsmt & 1) != 0 && route[smt.ismsFrom] == routeMarker && route[smt.ismsTo] == depth)
				route[smt.ismsTo] = nextMarker;
		}

		routeMarker = nextMarker;
	}

	while (true)
	{
		const int ismsCur = psma->ismsCur;

		std::vector<SMT*> candidates;
		candidates.reserve(psm->asmt.size());

		for (SMT& smt : psm->asmt)
		{
			int ismsNext = -1;

			if (smt.ismsFrom == ismsCur)
				ismsNext = smt.ismsTo;
			else if ((smt.grfsmt & 1) != 0 && smt.ismsTo == ismsCur)
				ismsNext = smt.ismsFrom;

			if (ismsNext < 0 || static_cast<std::size_t>(ismsNext) >= stateCount)
				continue;

			if (route[ismsNext] == routeMarker)
				candidates.push_back(&smt);
		}

		if (candidates.empty())
		{
			psma->ismsGoal = -1;
			FinishWithoutTransition();
			return;
		}

		float totalProbability = 0.0f;

		for (const SMT* psmt : candidates)
		{
			if (psmt->gProbability > 0.0f)
				totalProbability += psmt->gProbability;
		}

		SMT* psmtChosen = candidates.back();

		if (totalProbability > 0.0f)
		{
			float choice = GRandInRange(0.0f, totalProbability);

			for (SMT* psmt : candidates)
			{
				choice -= std::max(psmt->gProbability, 0.0f);

				if (choice < 0.0f)
				{
					psmtChosen = psmt;
					break;
				}
			}
		}

		const bool fForward = psmtChosen->ismsFrom == ismsCur;
		const int ismsNext = fForward ? psmtChosen->ismsTo : psmtChosen->ismsFrom;

		if (psmtChosen->fAseg)
		{
			const float tLocal = fForward || psmtChosen->paseg == nullptr ? 0.0f : psmtChosen->paseg->tMax;
			const float svtLocal = fForward ? psma->svtLocal : -psma->svtLocal;

			ASEGA* pasegaPrevious = psma->pasegaCur;

			psma->psmtCur = psmtChosen;
			psma->ismsNext = ismsNext;
			psma->pasegaCur = nullptr;

			if (psmtChosen->paseg != nullptr)
			{
				ApplyAseg(psmtChosen->paseg, psma->paloRoot, tLocal, svtLocal, psma->grfapl, &psma->pasegaCur);

				if (psma->pasegaCur != nullptr)
					SubscribeAsegaStruct(psma->pasegaCur, (PFNMQ)HandleSmaMessage, psma);
			}

			if (pasegaPrevious != nullptr)
			{
				SnapAsega(pasegaPrevious, 0);
				RetractAsega(pasegaPrevious);
			}

			SendSmaMessage(psma, MSGID_sma_transition, psma);
			return;
		}

		// A transition without an ASEG changes state immediately.
		NotifySmaSpliceOnEnterState(psma, psma->ismsCur, ismsNext);
		psma->ismsCur = ismsNext;
		psma->ismsNext = -1;

		if (ismsNext == ismsGoal)
		{
			ChooseSmaTransition(psma);
			return;
		}

		--routeMarker;
	}
}

void EndSmaTransition(SMA* psma)
{
	if (psma == nullptr || psma->ismsNext < 0)
		return;

	const int ismsPrevious = psma->ismsCur;
	const int ismsNext = psma->ismsNext;
	NotifySmaSpliceOnEnterState(psma, ismsPrevious, ismsNext);

	psma->ismsCur = ismsNext;
	psma->ismsNext = -1;
}

void HandleSmaMessage(SMA* psma, MSGID msgid, void* pv)
{
    if ((msgid == MSGID_asega_limit) && (pv == psma->pasegaCur)) {
        EndSmaTransition(psma);
        ChooseSmaTransition(psma);
    }
}

void SkipSma(SMA* psma, float dtSkip)
{
	if (psma == nullptr || dtSkip <= 0.0f)
		return;

	while (psma->pasegaCur != nullptr)
	{
		ASEGA* pasega = psma->pasegaCur;

		if (pasega->paseg == nullptr)
			return;

		const float dtRemaining = pasega->paseg->tMax - pasega->tLocal;

		if (dtSkip < dtRemaining)
		{
			SeekAsega(pasega, SEEK_Current, dtSkip, 1.0f);
			return;
		}

		dtSkip -= dtRemaining;

		EndSmaTransition(psma);
		ChooseSmaTransition(psma);
	}
}

void SendSmaMessage(SMA* psma, MSGID msgid, void* pv)
{
	if (psma == nullptr)
		return;

	if (psma->paloRoot != nullptr)
	{
		auto pfnSendLoMessage = psma->paloRoot->pvtlo->pfnSendLoMessage;

		if (pfnSendLoMessage != nullptr)
			pfnSendLoMessage(psma->paloRoot, msgid, pv);
	}

	for (MQ* pmq = psma->pmqFirst.get(); pmq != nullptr;)
	{
		// Cache everything before invoking the callback because it may
		// unsubscribe or otherwise modify the message queue.
		MQ* pmqNext = pmq->pmqNext.get();
		PFNMQ pfnmq = pmq->pfnmq;
		void* pvContext = pmq->pvContext;

		if (pfnmq != nullptr)
			pfnmq((LO*)pvContext, msgid, pv);

		pmq = pmqNext;
	}
}

void SubscribeSmaStruct(SMA* psma, LO* plo)
{
	SubscribeSwPpmqStruct(psma->psm->psw, &psma->pmqFirst, (PFNMQ)plo->pvtlo->pfnHandleLoMessage, plo);
}

void NotifySmaSpliceOnEnterState(SMA* psma, int ismsFrom, int ismsTo)
{
	OID oidStateFrom = OID_Nil;
	OID oidStateTo = OID_Nil;

	if (ismsFrom >= 0)
		oidStateFrom = psma->psm->asms[ismsFrom].oid;

	if (ismsTo >= 0)
		oidStateTo = psma->psm->asms[ismsTo].oid;

	SMA* psmaArg = psma;

	void* apvArgs[3];
	apvArgs[0] = &psmaArg;
	apvArgs[1] = &oidStateFrom;
	apvArgs[2] = &oidStateTo;

	HandleLoSpliceEvent(psma->psm, 14, 3, apvArgs);
}

void DeleteSm(SM* psm)
{
	delete psm;
}

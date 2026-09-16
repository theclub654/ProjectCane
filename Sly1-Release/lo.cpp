#include "lo.h"
#include "game.h"
#include "spliceobj.h"
#include "eval.h"
#include "clock.h"
#include <cstdio>

LO* NewLo()
{
	return new LO{};
}

void InitLo(LO* plo)
{
	plo->pvtlo->pfnSetLoDefaults(plo);
	plo->pvtlo->pfnAddLo(plo);
}

void SetLoDefaults(LO* plo)
{
	CID cid = plo->pvtbasic->cid;
	auto& arysplopt = g_mpcidarysplopt[cid];
	
	for (int isplopt = 0; isplopt < arysplopt.csplopt; ++isplopt)
	{
		EOPID* peopid = arysplopt.asplopt[isplopt].peopid;

		if ((peopid->grfeopid & 0x800) == 0)
			continue;

		CBinaryInputStream bis;
		bis.OpenMemory(sizeof(peopid->optdat), &peopid->optdat);
		LoadOptionFromBrx(plo, peopid, &bis);
	}
}

void AddLo(LO* plo)
{
	// Loading objects parent child list
	DL* objectChildList = &plo->paloParent->dlChild;

	// If object doesnt have a parent load up the static world dlChild
	if (plo->paloParent == nullptr)
		objectChildList = &plo->psw->dlChild;

	// Returns if parent LO or SW has a child object or not
	bool isFound = FFindDlEntry(objectChildList, plo);
	if (isFound == 0)
	{
		// Storing object child in parent LO
		AppendDlEntry(objectChildList, plo);
		// Returns if LO is in world or not
		isFound = FIsLoInWorld(plo);

		// Adds object to hierarchy
		if (isFound != 0)
			plo->pvtlo->pfnAddLoHierarchy(plo);
	}

}

void AddLoHierarchy(LO* plo)
{
	plo->pvtlo->pfnOnLoAdd(plo);
	SendLoMessage(plo, MSGID_added, plo);
}

void RemoveLoHierarchy(LO* plo)
{
	plo->pvtlo->pfnOnLoRemove(plo);
	plo->pvtlo->pfnSendLoMessage(plo, MSGID_removed, plo);
}

void SnipLo(LO* plo)
{
	if (FIsLoInWorld(plo) == 0)
		return;

	VTLO* pvtlo = plo->pvtlo;

	if (pvtlo->pfnBindLo != nullptr)
		pvtlo->pfnBindLo(plo);

	// Binding can legally change object state, so reload the vtable just as
	// the retail implementation does before each subsequent dispatch.
	pvtlo = plo->pvtlo;
	if (pvtlo->pfnPostLoLoad != nullptr)
		pvtlo->pfnPostLoLoad(plo);

	pvtlo = plo->pvtlo;
	if (pvtlo->pfnRemoveLo != nullptr)
		pvtlo->pfnRemoveLo(plo);
}

int FFindLoParent(LO* plo, ALO* paloParent)
{
	while (plo != nullptr)
	{
		if (plo == paloParent)
			return true;

		plo = reinterpret_cast<LO*>(reinterpret_cast<ALO*>(plo)->paloParent);
	}

	return paloParent == nullptr;
}

void CloneLoHierarchy(LO* plo, LO* ploBase)
{
	plo->pvtlo->pfnCloneLo(plo, ploBase);
}

void CloneLo(LO* plo, LO* ploBase)
{
	SW* psw = plo->psw;

	if (ploBase->psidebag != nullptr)
	{
		plo->psidebag = PsidebagNew();
		ploBase->psidebag->CloneTo(plo->psidebag.get());
		g_gc.AddRootSidebag(plo->psidebag.get());
	}
	else
		plo->psidebag.reset();

	RemoveDlEntry(PdlFromSwOid(psw, static_cast<OID>(plo->oid)), plo);
	AppendDlEntry(PdlFromSwOid(psw, static_cast<OID>(ploBase->oid)), plo);

	plo->oid = ploBase->oid;
	plo->pchzName = ploBase->pchzName;

	if (ploBase->pframe != nullptr)
	{
		//g_gc.UpdateRecyclable();

		plo->pframe = PframeNew();
		ploBase->pframe->CloneTo(plo->pframe.get());

		g_gc.AddRootFrame(plo->pframe.get());

	}
	else
		plo->pframe.reset();

	plo->ppxr = ploBase->ppxr;
}

LO* PloCloneLo(LO* plo, SW* psw, ALO* paloParent)
{
	// Returns object to be cloned
	LO* localObject = PloNew(plo->pvtbasic->cid, psw, paloParent, (OID)plo->oid, -1);
	// Clones the base object and it's children
	localObject->pvtlo->pfnCloneLoHierarchy(localObject, plo);
	// Returns cloned object
	return localObject;
}

void SendLoMessage(LO* plo, MSGID msgid, void* pv)
{
	if (plo->pvtlo->pfnHandleLoMessage != nullptr)
		plo->pvtlo->pfnHandleLoMessage(plo, msgid, pv);

	// A message handler may unsubscribe itself, which can unlink and destroy
	// the current MQ. Keep both the current node and its successor alive across
	// the callback instead of reading pmqNext from a potentially freed node.
	std::shared_ptr<MQ> pmq = plo->pmqFirst;
	while (pmq != nullptr)
	{
		std::shared_ptr<MQ> pmqNext = pmq->pmqNext;
		PFNMQ pfnmq = pmq->pfnmq;
		void* pvContext = pmq->pvContext;

		if (pfnmq != nullptr)
			pfnmq(static_cast<LO*>(pvContext), msgid, pv);

		pmq = std::move(pmqNext);
	}
}

void LoadLoFromBrx(LO* plo, CBinaryInputStream* pbis)
{
	LoadOptionsFromBrx(plo, pbis);
}

int FMatchesLoName(LO* plo, OID oid)
{
	if (oid == OID_Nil)
		return 0;
	
	if ((plo->oid != oid) && ((plo->ppxr == nullptr || (plo->ppxr->oidProxyRoot != oid))))
		return 0;

	return 1;
}

void RemoveLo(LO* plo)
{
	DL* objectChildList;

	if (plo->paloParent != nullptr)
		objectChildList = &plo->paloParent->dlChild;
	else
		objectChildList = &plo->psw->dlChild;

	bool isFound = FFindDlEntry(objectChildList, plo) != 0;
	if (!isFound)
		return;

	bool inWorld = FIsLoInWorld(plo) != 0;

	RemoveDlEntry(objectChildList, plo);

	if (inWorld)
		plo->pvtlo->pfnRemoveLoHierarchy(plo);

}

void DeferLoRemove(LO* plo)
{
	ALO* paloOwner = nullptr;

	if (plo->pvtlo->grfcid & 1)
		paloOwner = static_cast<ALO*>(plo);
	else
		paloOwner = plo->paloParent;

	if (paloOwner == nullptr)
		return;

	auto pdlr = std::make_unique<DLR>();

	pdlr->oidChild = (OID)plo->oid;
	pdlr->pdlrNext = std::move(paloOwner->pdlrFirst);

	paloOwner->pdlrFirst = std::move(pdlr);
}

void SetLoSuckHideLimits(LO* plo, LM* plmUSuck)
{
	const float uSuck = g_plsCur->uSuck;

	if (plmUSuck->gMin < uSuck && uSuck < plmUSuck->gMax)
		DeferLoRemove(plo);
}

void OnLoAdd(LO* plo)
{

}

void OnLoRemove(LO* plo)
{

}

void* AccessLoSw(LO *plo)
{;
	return &plo->psw;
}

void* GetLoSw(LO* plo)
{
	return &plo->psw;
}

int FIsLoInWorld(LO* plo)
{
	if (!plo) return 1;

	ALO* parent = plo->paloParent;

	while (true) {
		ALO* palo = parent;
		DL* pdl = palo ? &palo->dlChild : &plo->psw->dlChild;

		if (!FFindDlEntry(pdl, plo))
			return 0;

		if (!palo)
			break;

		parent = palo->paloParent;
		plo = palo;
	}

	return 1;
}

void GetLoInWorld(LO* plo, int* pfInWorld)
{
	int fIsLoInWorld = FIsLoInWorld(plo);
	*pfInWorld = fIsLoInWorld;
}

void* AccessLoPaloParent(LO* plo)
{
	return &plo->paloParent;
}

void* GetLoPaloParent(LO* plo)
{
	return &plo->paloParent;
}

void* AccessLoOid(LO* plo)
{
	return &plo->oid;
}

OID GetLoOid(LO* plo)
{
	return (OID)plo->oid;
}

OID OidProxyLo(LO* plo)
{
	if (plo->ppxr == nullptr)
		return OID_Nil;

	return plo->ppxr->oidProxyRoot;
}

OID OidProxyLoPreferred(LO* plo)
{
	if (plo->ppxr == nullptr) 
		return (OID)plo->oid;

	return plo->ppxr->oidProxyRoot;
}

void GetLoOidProxy(LO* plo, OID* poid)
{
	OID prooxyOid = OidProxyLo(plo);
	*poid = prooxyOid;
}

void HandleLoSpliceEvent(LO* plo, SYMID symidEvent, int cargs, void** ppvargs)
{
    if (plo == nullptr || plo->pframe == nullptr)
        return;

	CRef refCallback;
    const bool hasBinding = plo->pframe->FFindBinding(symidEvent, true, &refCallback) != 0;
    if (!hasBinding)
        return;

	const int fFiltered = FFilterSpliceEvent(static_cast<ALO*>(plo), static_cast<SYMEVID>(symidEvent), cargs, ppvargs);
    if (fFiltered)
    {
        return;
    }

	if (cargs == 0)
		refCallback.SetPair(nullptr);
	else
	{
		// The shared LO event-signature table ends at event 26. Rhythm
		// sequences define their own event 27 and pass one integer command value.
		// Indexing s_aiEvtParm[27] read beyond the table, assigned a random
		// argument type, and made the sequence callback's 2160/2161 case miss.
		if (symidEvent == 27 && cargs == 1)
		{
			OTYP otypRythmEvent = OTYP_Int;
			refCallback = RefSetArgListFromPvs(cargs, &otypRythmEvent, ppvargs);
		}
		else if (symidEvent >= 0 && symidEvent < SYMEVID_Max)
		{
			OTYP* aotyp = s_aotypEvtParm + s_aiEvtParm[symidEvent];
			refCallback = RefSetArgListFromPvs(cargs, aotyp, ppvargs);
		}
		else
		{
			return;
		}
	}

    PostSwCallback(plo->psw, reinterpret_cast<PFNMQ>(PostSpliceEventCallback), plo, static_cast<MSGID>(symidEvent), refCallback.m_ppair);
}

int FFilterSpliceEvent(ALO* plo, SYMEVID symevid, int cargs, void** ppvargs)
{
	MSGID msgidCancel = MSGID_Nil;

	switch (symevid)
	{
		case SYMEVID_OnUpdate:
		if (plo->dtUpdatePause <= 0.0f)
			return 0;

		plo->dtUpdatePause -= *static_cast<float*>(ppvargs[0]);
		return plo->dtUpdatePause > 0.0f;

		case SYMEVID_OnAdd:
		msgidCancel = MSGID_asega_limit;
		break;

		case SYMEVID_OnRemove:
		msgidCancel = MSGID_rotated;
		break;

		case SYMEVID_OnTouch:
		case SYMEVID_OnUntouch:
		break;

		default:
		return 0;
	}

	SW* psw = plo->psw;

	std::shared_ptr<MQ> prev;
	std::shared_ptr<MQ>* link = &psw->pmqCallbackFirst;

	while (*link)
	{
		std::shared_ptr <MQ> mq = *link;

		if (mq->pfnmq == reinterpret_cast<PFNMQ>(PostSpliceEventCallback) && mq->pvContext == plo)
		{
			if (mq->msgid == symevid)
			{
				if (symevid < SYMEVID_OnTouch || symevid > SYMEVID_OnUntouch)
					return 1;

				CPair* queuedArgs = mq->pvCallbackData.get();

				if (!queuedArgs || cargs <= 0 || !ppvargs || !ppvargs[0])
					return 0;

				BASIC* current = *static_cast<BASIC**>(ppvargs[0]);

				return queuedArgs->m_ref.m_tagk == TAGK_Basic && queuedArgs->m_ref.m_pbasic == current;
			}

			if (mq->msgid == msgidCancel)
			{
				*link = mq->pmqNext;

				if (!*link)
					psw->pmqCallbackLast = prev;

				mq->pmqNext.reset();
				FreeSwMqList(psw, mq);
				return 1;
			}
		}

		prev = mq;
		link = &mq->pmqNext;
	}

	return 0;
}

void PostSpliceEventCallback(LO* plo, SYMID symidEvent, void* pvarg)
{
	CRef refProc;
	if (plo == nullptr || plo->pframe == nullptr || !plo->pframe->FFindBinding(symidEvent, true, &refProc))
	{
		return;
	}

	if (refProc.m_tagk != TAGK_Proc || refProc.m_pproc == nullptr)
	{
		return;
	}

	CProc* pproc = refProc.m_pproc.get();

	if (pproc->m_pframe == nullptr || pproc->m_ppairFormals == nullptr || pproc->m_ppairCodeExpr == nullptr)
	{
		return;
	}

	auto pframeCall = PframeNew();

	if (pframeCall == nullptr)
	{
		return;
	}

	pframeCall->SetSingleParent(pproc->m_pframe);
	CPair* ppairFormal = pproc->m_ppairFormals;
	CRef refSelf;
	refSelf.SetBasic(static_cast<BASIC*>(plo));
	pframeCall->RefAddBinding(ppairFormal->m_ref.GetSymid(), &refSelf);
	ppairFormal = ppairFormal->m_ppairNext.get();

	CPair* ppairArg = static_cast<CPair*>(pvarg);

	while (ppairFormal != nullptr && ppairArg != nullptr)
	{
		const SYMID formalSymid = ppairFormal->m_ref.GetSymid();
		pframeCall->RefAddBinding(formalSymid, &ppairArg->m_ref);
		ppairFormal = ppairFormal->m_ppairNext.get();
		ppairArg = ppairArg->m_ppairNext.get();
	}

	RefEvalLambdaBody(pproc->m_ppairCodeExpr, pframeCall.get());
}

void EnsureLoSidebagBool(LO* plo, OPTID optid, int f)
{
	CRef ref;
	ref.SetBool(f);

	RefEnsureOption(reinterpret_cast<BASIC*>(plo), static_cast<int>(optid), &ref);
}

void EnsureLoSidebagInt(LO* plo, OPTID optid, int n)
{
	CRef ref;
	ref.SetS32(n);

	RefEnsureOption(reinterpret_cast<BASIC*>(plo), static_cast<int>(optid), &ref);
}

void EnsureLoSidebagFloat(LO* plo, OPTID optid, float g)
{
	CRef ref;
	ref.SetF32(g);

	RefEnsureOption(reinterpret_cast<BASIC*>(plo), static_cast<int>(optid), &ref);
}

void EnsureLoSidebagClq(LO* plo, OPTID optid, CLQ* pclq)
{
	CRef ref;

	if (pclq)
		ref.SetClq(*pclq);
	else
		ref.SetClq(CLQ{});

	RefEnsureOption(reinterpret_cast<BASIC*>(plo), static_cast<int>(optid), &ref);
}

void EnsureLoSidebagLm(LO* plo, OPTID optid, LM* plm)
{
	CRef ref;

	if (plm)
		ref.SetLm(*plm);
	else
		ref.SetLm(LM{});

	RefEnsureOption(reinterpret_cast<BASIC*>(plo), static_cast<int>(optid), &ref);
}

void EnsureLoSidebagOid(LO* plo, OPTID optid, OID oid)
{
	CRef ref;
	ref.SetS32(static_cast<s32>(oid));

	RefEnsureOption(reinterpret_cast<BASIC*>(plo), static_cast<int>(optid), &ref);
}

void EnsureLoSidebagVector(LO* plo, OPTID optid, glm::vec3* pvec)
{
	CRef ref;

	if (pvec)
		ref.SetVector(*pvec);
	else
		ref.SetVector(glm::vec3{});

	RefEnsureOption(reinterpret_cast<BASIC*>(plo), static_cast<int>(optid), &ref);
}

void PostLoLoad(LO* plo)
{
	HandleLoSpliceEvent(plo, 0, 0, nullptr);
}

void SetLoParent(LO* plo, ALO* paloParent)
{
	plo->pvtlo->pfnRemoveLo(plo);
	plo->paloParent = paloParent;
	plo->pvtlo->pfnAddLo(plo);
}

void SubscribeLoObject(LO* plo, LO* ploTarget)
{
	SubscribeSwPpmqStruct(plo->psw, &plo->pmqFirst, (PFNMQ)ploTarget->pvtlo->pfnHandleLoMessage, ploTarget);
}

void SubscribeSwPpmqStruct(SW* psw, std::shared_ptr<MQ>* ppmqFirst, PFNMQ pfnmq, void* pvContext)
{
	std::shared_ptr <MQ> pmq = std::make_shared<MQ>();

	pmq->pfnmq = pfnmq;
	pmq->pvContext = pvContext;
	pmq->pmqNext = *ppmqFirst;
	*ppmqFirst = pmq;
}

void UnsubscribeLoObject(LO* plo, LO* ploTarget)
{
	UnsubscribeSwPpmqStruct(plo->psw, &plo->pmqFirst, (PFNMQ)ploTarget->pvtlo->pfnHandleLoMessage, ploTarget);
}

void UnsubscribeSwPpmqStruct(SW* psw, std::shared_ptr<MQ>* ppmqFirst, PFNMQ pfnmq, void* pvContext)
{
	while (*ppmqFirst != nullptr) 
	{
		std::shared_ptr<MQ> pmq = *ppmqFirst;

		if (pmq->pfnmq == pfnmq && pmq->pvContext == pvContext) {
			*ppmqFirst = pmq->pmqNext;
			pmq->pmqNext.reset();
			FreeSwMqList(psw, pmq);
			return;
		}

		ppmqFirst = &pmq->pmqNext;
	}
}

void SubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext)
{
	SubscribeSwPpmqStruct(plo->psw, &plo->pmqFirst, (PFNMQ)pfnmq, pvContext);
}

void UnsubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext)
{
	UnsubscribeSwPpmqStruct(plo->psw, &plo->pmqFirst, (PFNMQ)pfnmq, pvContext);
}

int GetLoSize()
{
	return sizeof(LO);
}

void DeleteLo(LO* plo)
{
	delete plo;
}

std::vector <LO*> allWorldObjs;

OTYP s_aotypEvtParm[44] =
{
	OTYP_Basic, OTYP_Float, OTYP_Basic, OTYP_Basic,
	OTYP_Basic, OTYP_Basic, OTYP_Basic, OTYP_Basic,
	OTYP_Basic, OTYP_Basic, OTYP_Basic, OTYP_Basic,
	OTYP_Basic, OTYP_Basic, OTYP_Basic, OTYP_Int,
	OTYP_Basic, OTYP_Basic, OTYP_Basic, OTYP_Basic,
	OTYP_Basic, OTYP_Basic, OTYP_Basic, OTYP_Oid,
	OTYP_Oid,   OTYP_Basic, OTYP_Cnstr, OTYP_Cnstr,
	OTYP_Basic, OTYP_Oid,   OTYP_Basic, OTYP_Basic,
	OTYP_Basic, OTYP_Basic, OTYP_Cnstr, OTYP_Basic,
	OTYP_Int,   OTYP_Basic, OTYP_Basic, OTYP_Basic,
	OTYP_Basic, OTYP_Oid,   OTYP_Basic, OTYP_Basic
};

int s_aiEvtParm[27] =
{
	0x00, 0x01, 0x03, 0x04,
	0x05, 0x06, 0x07, 0x09,
	0x0B, 0x0C, 0x0E, 0x11,
	0x13, 0x14, 0x16, 0x1A,
	0x1D, 0x1F, 0x21, 0x22,
	0x24, 0x26, 0x27, 0x28,
	0x29, 0x2B, 0x2C,
};

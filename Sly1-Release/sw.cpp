#include "sw.h"
#include "sound.h"
#include "debug.h"
#include "render.h"
#include "coin.h"
#include "bbmark.h"
#include "proxy.h"
#include "sm.h"
#include "fly.h"
#include "rat.h"
#include "spire.h"
#include "rail.h"
#include "dart.h"
#include "sensor.h"
#include "landing.h"
#include "blip.h"
#include "gl.h"
#include "crusher.h"
#include "path.h"
#include "dialog.h"
#include "binoc.h"
#include "thndflash.h"
#include <algorithm>
#include <cstring>
#include "jump.h"
#include "pipe.h"
#include "keyhole.h"
#include "frame.h"
#include "frm.h"
#include "eval.h"
#include "totals.h"
#include "timer.h"
#include "credit.h"
#include "find.h"
#include "aseg.h"
#include "game.h"

SW* NewSw()
{
	return new SW{};
}

int GetSwLevelDataValue(SW* psw, uint32_t worldLevelKey, int key)
{
	(void)psw;

	LS* pls = PlsFromWorldLevelKey(worldLevelKey);
	if (pls == nullptr)
		return 0;

	for (const LEVELDATA& levelData : pls->alevelData)
	{
		if (levelData.key == key)
			return levelData.value;
	}

	return 0;
}

void SetSwLevelDataValue(SW* psw, uint32_t worldLevelKey, int key, int value)
{
	(void)psw;

	LS* pls = PlsFromWorldLevelKey(worldLevelKey);
	if (pls == nullptr)
		return;

	LEVELDATA* plevelDataFree = nullptr;

	for (LEVELDATA& levelData : pls->alevelData)
	{
		if (levelData.key == key)
		{
			plevelDataFree = &levelData;
			break;
		}

		if (plevelDataFree == nullptr && levelData.key == 0)
			plevelDataFree = &levelData;
	}

	if (plevelDataFree != nullptr)
	{
		plevelDataFree->key = key;
		plevelDataFree->value = value;
	}
}

int GetSwSize()
{
	return sizeof(SW);
}

void InitSwDlHash(SW* psw)
{
	for (int i = 0; i < 512; i++)
		InitDl(&psw->adlHash[i], offsetof(LO, dleOid));
}

void InitSw(SW* psw)
{
	InitLo(psw);
	SetSwDefaultReverb(psw, REVERBK_None, 0);

	// EOPID 538 defines the world's Master Thief Sprint/commentary timer
	// duration with a 180-second default.  SW instances are value-initialized,
	// so establish that default here as well; a value authored in the BRX is
	// loaded later and replaces it.  Leaving this at zero makes the sprint start
	// at 0.99 seconds in SetNoteMtsState and the hourglass immediately vanish.
	psw->dtCommentaryAudio = 180.0f;

	psw->aaobr.clear();
	psw->aaobr.resize(AXIS_COUNT * BOUNDS_PER_AXIS);

	psw->aaoxf.assign(AAOXF_COUNT, 0);

	psw->aox.clear();
	psw->apoxFree.clear();
	psw->axp.clear();

	InitSwAoxa(psw);

	ResetSwRipPool(psw);
	psw->ripMqOwners.clear();
	psw->blipOwners.reserve(128);
	psw->blipspOwners.reserve(32);
	psw->asegaOwners.reserve(256);
	psw->smaOwners.reserve(64);
	psw->explsteOwners.reserve(112);
	psw->ambOwners.clear();
	psw->ambOwners.resize(128);
	// Shared-pointer equivalent of CreateSlotheapSw(&psw->slotheapExc, 12, 32).
	psw->excOwners.clear();
	psw->excOwners.resize(32);

	psw->apaloRemerge.resize(1024);

	InitDl(&psw->dlChild, offsetof(LO, dleChild));
	InitDl(&psw->dlMRD, offsetof(ALO, dleMRD));
	InitDl(&psw->dlMRDRealClock, offsetof(ALO, dleMRD));
	InitDl(&psw->dlBusy, offsetof(ALO, dleBusy));
	InitDl(&psw->dlBusySo, offsetof(SO, dleBusySo));
	InitDl(&psw->dlRoot, offsetof(SO, dleRoot));
	InitDl(&psw->dlAsega, offsetof(ASEGA, dleSw));
	InitDl(&psw->dlAsegaRealClock, offsetof(ASEGA, dleSw));
	InitDl(&psw->dlAsegaPending, offsetof(ASEGA, dleSw));
	InitDl(&psw->dlSma, offsetof(SMA, dleSw));
	InitDl(&psw->dlAmb, offsetof(AMB, dle));
	InitDl(&psw->dlExc, offsetof(EXC, dle));
	InitDl(&psw->dlLight, offsetof(LIGHT, dleLight));
	InitDl(&psw->dlShadow, offsetof(SHADOW, dle));
	InitDl(&psw->dlProxy, offsetof(PROXY, dleProxy));
	InitDl(&psw->dlFly, offsetof(FLY, dleFly));
	InitDl(&psw->dlDprize, offsetof(DPRIZE, dle));
	InitDl(&psw->dlRat, offsetof(RAT, dleRat));
	InitDl(&psw->dlRathole, offsetof(RATHOLE, dleRathole));
	InitDl(&psw->dlDartFree, offsetof(DART, dleDartFree));
	InitDl(&psw->dlSpire, offsetof(SPIRE, dleSpire));
	InitDl(&psw->dlRail, offsetof(RAIL, dleRail));
	InitDl(&psw->dlLanding, offsetof(LANDING, dleLanding));
	InitDl(&psw->dlBusyLasen, offsetof(LASEN, dleBusyLasen));
	InitDl(&psw->dlBlipg, offsetof(BLIPG, dleBlipg));
	InitDl(&psw->dlBlipgFree, offsetof(BLIPG, dleBlipg));
	InitDl(&psw->dlFader, offsetof(FADER, dleFader));
	InitDl(&psw->dlRealClockFader, offsetof(FADER, dleFader));
	InitDl(&psw->dlCrfod, offsetof(CRFOD, dleCrfod));
	InitDl(&psw->dlThndFlash, offsetof(THNDFLASH, dleThndFlash));
	InitDl(&psw->dlShape, offsetof(SHAPE, dleShape));
	InitDl(&psw->dlPathzone, offsetof(PATHZONE, dlePathzone));
	InitDl(&psw->dlExplste, offsetof(EXPLSTE, dle));

	float z = -1500.0;
	if ((g_grfcht & 4U) != 0)
		z = -900.0;

	SetSwGravity(psw, z);
	psw->grfvault = -264241153;
	psw->rDarkenSmooth = 1.0;
	psw->fDprizeEnabled = 1;

	psw->rgbaSky.bRed   = 16 / 255;
	psw->rgbaSky.bGreen = 16 / 255;
	psw->rgbaSky.bBlue  = 40 / 255;
	psw->rgbaSky.bAlpha = 80 * 2;

	psw->lsmDefault.uShadow = 50.0;
	psw->musid = MUSID_Nil;
	psw->ibnk = -1;
	psw->cisi = 0;
	psw->cpsl = 0;
	psw->rDarken = 1.0;
}

void SetSwGravity(SW* psw, float z)
{
	psw->dvGravity = glm::vec3(0.0f, 0.0f, z);
}

const char* GetResponseText(SW* psw, RESPK respk)
{
	return AchzFromRespk(respk);
}

bool FOverflowSwLo(SW* psw, LO* plo, bool fHiPri)
{
	bool fSo = (plo->pvtlo->grfcid & 2) != 0;
	return fSo && psw->cpsoRoot > 246 && !fHiPri;
}

std::shared_ptr<XA> PxaAllocSw(SW* psw)
{
	return std::make_shared<XA>();
}

void FreeSwXaList(SW*, std::shared_ptr<XA>& pxaFirst)
{
	while (pxaFirst != nullptr)
	{
		std::shared_ptr <XA> pxaNext = std::move(pxaFirst->pxaNextTarget);
		pxaFirst->pxaNextTarget.reset();
		pxaFirst = std::move(pxaNext);
	}
}

void FreeSwMqList(SW* psw, std::shared_ptr<MQ>& pmqFirst)
{
	while (pmqFirst)
	{
		std::shared_ptr<MQ> pmqNext = pmqFirst->pmqNext;
		pmqFirst->pmqNext.reset();
		pmqFirst.reset();
		pmqFirst = pmqNext;
	}
}

void EnsureSwCallback(SW* psw, PFNMQ pfnmq, void* pvContext, MSGID msgid, std::shared_ptr<CPair> pvCallbackData)
{
	for (auto pmq = psw->pmqCallbackFirst; pmq; pmq = pmq->pmqNext)
	{
		if (pmq->pfnmq == pfnmq && pmq->pvContext == pvContext && pmq->msgid == msgid && pmq->pvCallbackData.get() == pvCallbackData.get())
			return;
	}

	PostSwCallback(psw, pfnmq, pvContext, msgid, pvCallbackData);
}

void PostSwCallback(SW* psw, PFNMQ pfnmq, void* pvContext, MSGID msgid, std::shared_ptr<CPair> pvCallbackData)
{
	auto pmq = std::make_shared<MQ>();

	pmq->pfnmq = pfnmq;
	pmq->pvContext = pvContext;
	pmq->msgid = msgid;
	pmq->pvCallbackData = pvCallbackData;

	if (!psw->pmqCallbackLast)
		psw->pmqCallbackFirst = pmq;
	else
		psw->pmqCallbackLast->pmqNext = pmq;

	psw->pmqCallbackLast = pmq;
}

void ClearSwCallbacks(SW* psw, int grfcmq, PFNMQ pfnmq, void* pvContext, MSGID msgid, std::shared_ptr<CPair> pvCallback)
{
	std::shared_ptr <MQ> pmq = psw->pmqCallbackFirst;
	std::shared_ptr <MQ> pmqPrev = nullptr;
	std::shared_ptr <MQ> pmqFreeFirst = nullptr;

	while (pmq != nullptr)
	{
		std::shared_ptr<MQ> pmqNext = pmq->pmqNext;

		const bool fMatchPfn = (grfcmq & 1) == 0 || pmq->pfnmq == pfnmq;
		const bool fMatchContext = (grfcmq & 2) == 0 || pmq->pvContext == pvContext;
		const bool fMatchMsgid = (grfcmq & 4) == 0 || pmq->msgid == msgid;
		const bool fMatchCallback = (grfcmq & 8) == 0 || pmq->pvCallbackData == pvCallback;

		if (fMatchPfn && fMatchContext && fMatchMsgid && fMatchCallback)
		{
			if (pmqPrev != nullptr)
				pmqPrev->pmqNext = pmqNext;
			else
				psw->pmqCallbackFirst = pmqNext;

			if (pmqNext == nullptr)
				psw->pmqCallbackLast = pmqPrev;

			pmq->pmqNext = pmqFreeFirst;
			pmqFreeFirst = pmq;
		}
		else
			pmqPrev = pmq;

		pmq = pmqNext;
	}

	FreeSwMqList(psw, pmqFreeFirst);
}

void ProcessSwCallbacks(SW* psw)
{
	while (psw->pmqCallbackFirst)
	{
		std::shared_ptr <MQ> pmq = psw->pmqCallbackFirst;

		if (pmq == psw->pmqCallbackLast)
			psw->pmqCallbackLast.reset();

		psw->pmqCallbackFirst = pmq->pmqNext;
		pmq->pmqNext.reset();

		pmq->pfnmq(static_cast<LO*>(pmq->pvContext), pmq->msgid, pmq->pvCallbackData.get());

		FreeSwMqList(psw, pmq);
	}
}

void ProcessSwSpliceScheduledCallbacks(SW* psw, float dt)
{
	if (!psw || !psw->pframe)
		return;

	CRef refCallbackList;
	CRef refRet;

	if (!psw->pframe->FFindBinding(psw->symidScheduledCallbackList, 0, &refCallbackList))
		return;

	if (refCallbackList.m_tagk != TAGK_Pair)
		return;

	bool fCallbackExecuted = false;

	for (std::shared_ptr<CPair> plist = refCallbackList.m_ppair; plist; plist = plist->m_ppairNext)
	{
		if (plist->m_ref.m_tagk != TAGK_Pair || !plist->m_ref.m_ppair)
			continue;

		std::shared_ptr<CPair> pcallback = plist->m_ref.m_ppair;
		const float dtRemaining = pcallback->m_ref.m_g - dt;

		if (dtRemaining > 0.0f)
		{
			pcallback->m_ref.SetF32(dtRemaining);
			continue;
		}

		refRet = RefEval(pcallback->m_ppairNext.get(), psw->pframe.get());

		// Mark the outer scheduled-callback entry for removal.
		plist->m_ref.SetTag(TAGK_None);
		fCallbackExecuted = true;
	}

	if (!fCallbackExecuted)
		return;

	// A callback may have changed this binding during RefEval.
	if (!psw->pframe->FFindBinding(psw->symidScheduledCallbackList, 0, &refCallbackList))
		return;

	if (refCallbackList.m_tagk != TAGK_Pair)
		return;

	std::shared_ptr<CPair>* ppairLink = &refCallbackList.m_ppair;

	while (*ppairLink)
	{
		std::shared_ptr<CPair>& ppair = *ppairLink;

		if (ppair->m_ref.m_tagk != TAGK_Pair)
			ppair = ppair->m_ppairNext;
		else
			ppairLink = &ppair->m_ppairNext;
	}

	if (refCallbackList.m_ppair)
		refCallbackList.SetPair(refCallbackList.m_ppair);
	else
		refCallbackList.SetTag(TAGK_None);

	psw->pframe->RefSetBinding(psw->symidScheduledCallbackList, &refCallbackList);
}

void AddSwProxySource(SW* psw, LO* ploProxySource, int cploClone)
{
	PSL psl{};

	int cploCloneFree = cploClone - 1;
	psl.cploCloneFree = cploCloneFree;

	if (cploCloneFree > 0)
	{
		psl.aploClone.resize(cploCloneFree);

		for (int i = 0; i < cploCloneFree; i++)
			psl.aploClone[i] = PloCloneLo(ploProxySource, psw, nullptr);
	}

	psw->apsl[psw->cpsl++] = psl;
}

LO* PloGetSwProxySource(SW* psw, int ipsl)
{
	// Loads the psl
	PSL* psl = psw->apsl + ipsl;
	// Returns proxy source LO from that psl
	return psl->aploClone[psl->cploCloneFree -= 1];
}

void IntersectSwBoundingBox(SW* psw, SO* psoIntersect, const glm::vec3* ppos1, const glm::vec3* ppos2, PFNFILTER pfnFilter, void* pvContext, std::vector<SO*>& apso)
{
	apso.clear();
	apso.reserve(psw->cpsoAll);

	if (psoIntersect != nullptr)
	{
		SO* psoIntersectRoot = psoIntersect->paloRoot != nullptr ? static_cast<SO*>(psoIntersect->paloRoot) : psoIntersect;

		if (psoIntersect->fNoXpsSelf || psoIntersectRoot->fNoXpsAll)
			return;
	}

	const glm::vec3 posMin = glm::min(*ppos1, *ppos2);
	const glm::vec3 posMax = glm::max(*ppos1, *ppos2);

	for (SO* psoRoot = static_cast<SO*>(psw->dlBusySo.ploFirst); psoRoot != nullptr; psoRoot = psoRoot->dleBusySo.psoNext)
	{
		if (psoRoot == psoIntersect)
			continue;

		if (psoRoot->posMin.x > posMax.x || posMin.x > psoRoot->posMax.x)
			continue;

		if (psoRoot->posMin.y > posMax.y || posMin.y > psoRoot->posMax.y)
			continue;

		if (psoRoot->posMin.z > posMax.z || posMin.z > psoRoot->posMax.z)
			continue;

		SO* psoPhysicalRoot = psoRoot->paloRoot != nullptr ? static_cast<SO*>(psoRoot->paloRoot) : psoRoot;

		if (psoPhysicalRoot->fNoXpsAll)
			continue;

		if (psoIntersect != nullptr)
		{
			if (psoIntersect->pvtso->pfnFIgnoreSoIntersection(psoIntersect, psoRoot))
				continue;

			if (psoRoot->pvtso->pfnFIgnoreSoIntersection(psoRoot, psoIntersect))
				continue;
		}

		for (SO* pso = psoRoot->dlPhys.psoFirst; pso != nullptr; pso = pso->dlePhys.psoNext)
		{
			if (pfnFilter != nullptr)
			{
				if (pfnFilter(pvContext, pso))
					apso.push_back(pso);
			}
			else if (!pso->fNoXpsSelf)
			{
				apso.push_back(pso);
			}
		}
	}
}

void IntersectSwBoundingSphere(SW* psw, SO* psoIntersect, const glm::vec3* pposCenter, float sRadius, PFNFILTER pfnFilter, void* pvContext, std::vector<SO*>& apso)
{
	apso.clear();
	apso.reserve(psw->cpsoAll);

	if (psoIntersect != nullptr)
	{
		SO* psoIntersectRoot = psoIntersect->paloRoot != nullptr ? static_cast<SO*>(psoIntersect->paloRoot) : psoIntersect;

		if (psoIntersect->fNoXpsSelf || psoIntersectRoot->fNoXpsAll)
			return;
	}

	for (SO* psoRoot = static_cast<SO*>(psw->dlBusySo.ploFirst); psoRoot != nullptr; psoRoot = psoRoot->dleBusySo.psoNext)
	{
		if (psoRoot == psoIntersect)
			continue;

		const glm::vec3 dpos = *pposCenter - psoRoot->xf.posWorld;
		const float sCombinedRadius = sRadius + psoRoot->sRadiusAll;

		if (glm::dot(dpos, dpos) > sCombinedRadius * sCombinedRadius)
			continue;

		SO* psoPhysicalRoot = psoRoot->paloRoot != nullptr ? static_cast<SO*>(psoRoot->paloRoot) : psoRoot;

		if (psoPhysicalRoot->fNoXpsAll)
			continue;

		if (psoIntersect != nullptr)
		{
			if (psoIntersect->pvtso->pfnFIgnoreSoIntersection(psoIntersect, psoRoot))
				continue;

			if (psoRoot->pvtso->pfnFIgnoreSoIntersection(psoRoot, psoIntersect))
				continue;
		}

		for (SO* pso = psoRoot->dlPhys.psoFirst; pso != nullptr; pso = pso->dlePhys.psoNext)
		{
			if (pfnFilter != nullptr)
			{
				if (pfnFilter(pvContext, pso))
					apso.push_back(pso);
			}
			else if (!pso->fNoXpsSelf)
			{
				apso.push_back(pso);
			}
		}
	}
}
void RemoveOxa(OXA* poxa, OXA** ppoxaFirst)
{
	OXA* poxaPrev = poxa->poxaPrev;
	OXA* poxaNext = poxa->poxaNext;

	if (poxaNext != nullptr)
		poxaNext->poxaPrev = poxaPrev;

	if (poxaPrev != nullptr)
		poxaPrev->poxaNext = poxaNext;
	else
		*ppoxaFirst = poxaNext;
}

void InitSwAoxa(SW* psw)
{
	psw->aoxa.clear();
	psw->aoxa.resize(MAX_OXA);

	psw->poxaFree = psw->aoxa.data();
	psw->poxaAlloc = nullptr;

	for (int i = 0; i < MAX_OXA; i++)
	{
		OXA& oxa = psw->aoxa[i];

		oxa.poxaPrev = (i > 0) ? &psw->aoxa[i - 1] : nullptr;
		oxa.poxaNext = (i < MAX_OXA - 1) ? &psw->aoxa[i + 1] : nullptr;

		oxa.pso = nullptr;
		oxa.pox = nullptr;
	}
}

void AddOxa(OXA* poxa, OXA** ppoxaFirst)
{
	OXA* poxaFirst = *ppoxaFirst;

	poxa->poxaPrev = nullptr;
	poxa->poxaNext = poxaFirst;

	if (poxaFirst != nullptr)
		poxaFirst->poxaPrev = poxa;

	*ppoxaFirst = poxa;
}

OXA* PoxaAllocSw(SW* psw, SO* pso)
{
	OXA* poxa = psw->poxaFree;
	RemoveOxa(poxa, &psw->poxaFree);
	AddOxa(poxa, &psw->poxaAlloc);
	poxa->pso = pso;
	return poxa;
}

void FreeSwPoxa(SW* psw, OXA* poxa)
{
	for (OX* pox = poxa->pox; pox != nullptr; )
	{
		OX* poxNext = pox->poxNext;
		OX* poxOther = PoxRemoveSw(psw, pox->psoOther->poxa, poxa);

		if (pox->pxp != nullptr)
		{
			FreeSwXpList(psw, pox->pxp);

			if (!pox->psoOther->fLockedAll)
				ResolveAlo(pox->psoOther);

			if (!poxOther->psoOther->fLockedAll)
				ResolveAlo(poxOther->psoOther);
		}

		poxOther->pxp = nullptr;
		pox->pxp = nullptr;

		FreeSwOx(psw, pox);
		FreeSwOx(psw, poxOther);

		pox = poxNext;
	}

	poxa->pox = nullptr;

	const int ioxa = IoxaFromPoxa(psw, poxa);

	for (OXA* poxaOther = psw->poxaAlloc; poxaOther != nullptr; poxaOther = poxaOther->poxaNext)
	{
		const int ioxaOther = IoxaFromPoxa(psw, poxaOther);

		psw->aaoxf[ioxa + ioxaOther * MAX_OXA] = 0;
		psw->aaoxf[ioxaOther + ioxa * MAX_OXA] = 0;
	}

	poxa->pso = nullptr;

	RemoveOxa(poxa, &psw->poxaAlloc);
	AddOxa(poxa, &psw->poxaFree);
}

void CreateSwDefaultLights(SW* psw)
{
	LIGHT* plight;

	if ((g_grfdfl & 1U) != 0)
	{
		plight = (LIGHT*)PloNew(CID_LIGHT, psw, nullptr, (OID)0x201, -1);
		SetLightHighlightColor(plight, g_vecHighlight);
		SetLightHighlightAngle(plight, g_degHighlight);
		SetLightMidtoneStrength(plight, g_gMidtone);
		SetLightMidtoneAngle(plight, g_degMidtone);
		SetLightShadowStrength(plight, g_gShadow);
		SetLightShadowAngle(plight, g_degShadow);
		SetLightDirection(plight, s_vecDirectionDefault);
	}

	if ((g_grfdfl & 2U) != 0)
	{
		plight = (LIGHT*)PloNew(CID_LIGHT, psw, nullptr, (OID)0x201, -1);
		SetLightKind(plight, LIGHTK_Position);
		SetLightHighlightColor(plight, g_vecHighlight);
		SetLightHighlightAngle(plight, g_degHighlight);
		SetLightMidtoneStrength(plight, g_gMidtone);
		SetLightMidtoneAngle(plight, g_degMidtone);
		SetLightShadowStrength(plight, g_gShadow);
		SetLightShadowAngle(plight, g_degShadow);
		SetLightFallOff(plight, &s_lmFallOffDefault);
		plight->pvtlight->pfnTranslateAloToPos(plight, &s_posDefault);
	}

	if ((g_grfdfl & 4U) != 0)
	{
		plight = (LIGHT*)PloNew(CID_LIGHT, psw, nullptr, (OID)0x201, -1);
		SetLightKind(plight, LIGHTK_Frustrum);
		SetLightHighlightColor(plight, g_vecHighlight);
		SetLightHighlightAngle(plight, g_degHighlight);
		SetLightMidtoneStrength(plight, g_gMidtone);
		SetLightMidtoneAngle(plight, g_degMidtone);
		SetLightShadowStrength(plight, g_gShadow);
		SetLightShadowAngle(plight, g_degShadow);
		SetLightDirection(plight, s_vecDirectionDefault);
		SetLightFallOff(plight, &s_lmFallOffDefault);
		plight->pvtlight->pfnTranslateAloToPos(plight, &s_posDefault);
	}

	if ((g_grfdfl & 8U) != 0)
	{
		plight = (LIGHT*)PloNew(CID_LIGHT, psw, nullptr, (OID)0x201, -1);
		SetLightKind(plight, LIGHTK_Spot);
		SetLightHighlightColor(plight, g_vecHighlight);
		SetLightHighlightAngle(plight, g_degHighlight);
		SetLightMidtoneStrength(plight, g_gMidtone);
		SetLightMidtoneAngle(plight, g_degMidtone);
		SetLightShadowStrength(plight, g_gShadow);
		SetLightShadowAngle(plight, g_degShadow);
		SetLightDirection(plight, s_vecDirectionDefault);
		SetLightFallOff(plight, &s_lmFallOffDefault);
		plight->pvtlight->pfnTranslateAloToPos(plight, &s_posDefault);
	}
}

void CreateSwPrizes(SW* psw)
{
	struct
	{
		CID cid;
		int cdprize;
	}
	prizeInfo[] =
	{
		{ CID_COIN,  0x23 }, // 35 coins
		{ CID_CHARM, 5    }  // 5 charms
	};

	for (int i = 0; i < 2; i++)
	{
		CID cidWanted = prizeInfo[i].cid;
		int countWanted = prizeInfo[i].cdprize;

		DPRIZE* templatePrize = psw->dlDprize.pdprizeFirst;

		while (templatePrize != nullptr)
		{
			if (!templatePrize->fNeverReuse && templatePrize->pvtbasic->cid == cidWanted)
				break;

			templatePrize = templatePrize->dle.pdprizeNext;
		}

		if (templatePrize == nullptr)
			continue;

		for (int j = 0; j < countWanted; j++)
		{
			COIN* newPrize = (COIN*)PloCloneLo(templatePrize, psw, nullptr);

			newPrize->dprizesInit = DPRIZES_Removed;

			if (newPrize->pvtalo->pfnBindAlo != nullptr)
				newPrize->pvtalo->pfnBindAlo(newPrize);

			newPrize->pvtalo->pfnPostAloLoad(newPrize);
		}
	}
}

void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis)
{
	std::cout << "\nLoading World...\n";
	// Setting difficulty for world
	OnDifficultyWorldPreLoad(&g_difficulty);
	g_gc.Startup();
	// Loading unknown debug flag from file
	pbis->U8Read();
	// Loading index sound bank from file
	psw->ibnk = pbis->S16Read();
	LoadNameTableFromBrx(pbis);
	// Making new camera object for world
	g_pcm = (CM*)PloNew(CID_CM, psw, nullptr, (OID)4, -1);

	// The original world loader keeps a small pool of reusable particle-group
	// objects. Removing each newly-created group transfers it to its class free
	// list, where PripgNew/PblipgNew can activate it later.
	for (int i = 0; i < 10; ++i)
	{
		LO* ploRipg = PloNew(CID_RIPG, psw, nullptr, OID_rip_group, -1);
		// PloNew does not add the object to the world.  OnRipgRemove is what
		// transfers a RIPG into psw->pripgFree, so activate it before removing it.
		ploRipg->pvtlo->pfnAddLo(ploRipg);
		ploRipg->pvtlo->pfnRemoveLo(ploRipg);
	}

	for (int i = 0; i < 40; ++i)
	{
		LO* ploBlipg = PloNew(CID_BLIPG, psw, nullptr, OID_blip_group, -1);
		ploBlipg->pvtlo->pfnRemoveLo(ploBlipg);
	}

	// Loads all splice events from binary file
	LoadSwSpliceFromBrx(psw, pbis);
	LoadOptionsFromBrx(psw, pbis);

	// Retail stores the optional commentary VAG as an encrypted 32-byte file
	// descriptor. Preserve the descriptor verbatim: ResolveVagWav decodes its
	// sector and identity when commentary playback is requested.
	psw->commentaryAudioDescriptor.fill(0);
	pbis->ReadBytes(psw->commentaryAudioDescriptor.data(),
		psw->commentaryAudioDescriptor.size());
	uint32_t commentaryWords[8]{};
	std::memcpy(commentaryWords, psw->commentaryAudioDescriptor.data(),
		sizeof(commentaryWords));
	const uint32_t commentarySize = commentaryWords[1] ^ commentaryWords[7];
	psw->audioLocation = commentarySize != 0
		? reinterpret_cast<const char*>(psw->commentaryAudioDescriptor.data())
		: nullptr;

	// The following BRX string names the developers heard in the commentary.
	// Keep owned string storage on SW so pchzSpeakers remains valid for the
	// complete lifetime of the loaded world.
	psw->commentarySpeakers = pbis->ReadStringSw();
	psw->pchzSpeakers = psw->commentarySpeakers.empty()
		? nullptr
		: psw->commentarySpeakers.c_str();

	// Retail worlds store their music as a 32-byte encrypted file descriptor.
	// Its identity selects the matching extracted 989SND bank directory.
	uint32_t musicWords[8]{};
	pbis->ReadBytes(musicWords, sizeof(musicWords));
	const uint32_t musicSize = musicWords[1] ^ musicWords[7];
	const uint32_t musicIdentity = musicWords[2] ^ musicWords[6];
	psw->musid = musicSize != 0 ? static_cast<MUSID>(musicIdentity) : MUSID_Nil;
	// Loads all textures and shader data from file
	LoadShadersFromBrx(pbis);
	// Loads all the scene world objects from the binary file
	LoadSwObjectsFromBrx(psw, nullptr, pbis);

	if (FIsDlEmpty(&psw->dlLight) == true)
		CreateSwDefaultLights(psw);

	rgbaSky.r = psw->rgbaSky.bRed / 255.0;
	rgbaSky.g = psw->rgbaSky.bGreen / 255.0;
	rgbaSky.b = psw->rgbaSky.bBlue / 255.0;
	rgbaSky.a = (psw->rgbaSky.bAlpha / 255.0) * 2;

	// Aligns binary stream to texture data
	pbis->Align(0x10);
	std::cout << "Loading Textures...\n";
	// Loads textures from binary file
	LoadTexturesFromBrx(pbis);
	// Bind/post-load callbacks can immediately start world ambience, so the
	// requested effects bank must already be resident at this point.
	if (psw->ibnk >= 0)
		SbpEnsureBank(psw->ibnk);
	psw->lsmDefault.uShadow *= 0.003921569;
	psw->lsmDefault.uMidtone *= 0.003921569;
	CreateSwPrizes(psw);

	ProcessSwCallbacks(psw);

	if (psw->dlChild.ploFirst != nullptr)
	{
		ALO* currentLo = psw->dlChild.paloFirst;

		while (currentLo != nullptr)
		{
			ALO* lo = currentLo;

			if (lo->pvtlo && lo->pvtlo->pfnBindLo)
				lo->pvtalo->pfnBindAlo(currentLo);

			currentLo = currentLo->dleChild.paloNext;
		}
	}

	ProcessSwCallbacks(psw);

	DLI dlBusyWalker{};

	dlBusyWalker.m_ibDle = psw->dlChild.ibDle;
	dlBusyWalker.m_pdliNext = s_pdliFirst;
	dlBusyWalker.m_pdl = &psw->dlChild;

	LO* currentObject = psw->dlChild.ploFirst;

	// Only valid if we have a first element
	dlBusyWalker.m_ppv = currentObject ? (void**)((uintptr_t)currentObject + dlBusyWalker.m_ibDle) : nullptr;

	s_pdliFirst = &dlBusyWalker;

	while (currentObject != nullptr)
	{
		if (currentObject->pvtlo->pfnPostLoLoad)
			currentObject->pvtlo->pfnPostLoLoad(currentObject);

		currentObject = (ALO*)*dlBusyWalker.m_ppv;

		// Guard before computing next pointer-field address
		dlBusyWalker.m_ppv = currentObject ? (void**)((uintptr_t)currentObject + dlBusyWalker.m_ibDle) : nullptr;
	}

	s_pdliFirst = dlBusyWalker.m_pdliNext;

	ProcessSwCallbacks(psw);

	SetupCm(g_pcm);
	RecalcSwXpAll(psw, 0);

	AllocateRpl();

	glGlobShader.Use();

	glUniform1f(glslLsmShadow, psw->lsmDefault.uShadow);
	glUniform1f(glslLsmDiffuse, psw->lsmDefault.uMidtone);

	glUniform1i(glslFogType, g_fogType);
	glUniform1f(glslFogNear, g_pcm->sNearFog);
	glUniform1f(glslFogFar, g_pcm->sFarFog);
	glUniform1f(glslFogMax, g_pcm->uFogMax);
	glUniform4fv(glslFogColor, 1, glm::value_ptr(g_pcm->rgbaFog));

	InitCameraUbo();
	// DrawGlob appends one ROGL record per queued render instance.  numRo only
	// counts unique loaded glob resources, so it can be smaller than the number
	// of instances submitted across the render passes.
	const size_t ropStreamCapacity =
		g_backGroundPrpl.size() +
		g_backGroundBlendPrpl.size() +
		g_blotContextPrpl.size() +
		g_opaquePrpl.size() +
		g_cutOutBlendAddPrpl.size() +
		g_cutOutPrpl.size() +
		g_projVolumePrpl.size() +
		g_projVolumeAlphaAddPrpl.size() +
		g_projVolumeAddPrpl.size() +
		g_opaqueAfterProjVolumePrpl.size() +
		g_cutOutAfterProjVolumeAddPrpl.size() +
		g_cutOutAfterProjVolumePrpl.size() +
		g_murkClearPrpl.size() +
		g_murkOpaquePrpl.size() +
		g_murkFillPrpl.size() +
		g_translucentAddPrpl.size() +
		g_translucentPrpl.size() +
		g_foreGroundPrpl.size() +
		g_worldMapPrpl.size() +
		g_maxPrpl.size();
	InitFrameStream(&ropStream, static_cast<int>(ropStreamCapacity));
	InitFrameStream(&blipStream, static_cast<int>(g_blipPrpl.size()));

	AllocateLightBlkList();
	AllocateShadows(psw);

	glCelBorderShader.Use();
	const int celStreamCapacity = static_cast<int>(g_celBorderPrpl.size() + g_celBorderAfterProjVolumePrpl.size() + g_translucentCelBorderPrpl.size());
	InitFrameStream(&rcbStream, celStreamCapacity);

	glGeomShader.Use();

	// Loading a world completes the UI transition as well.  In particular,
	// SetWipeWipes(WIPES_WipingOut) puts the UI in its wipe state; without this
	// post-load hook UpdateUi never returns to UIS_Playing, so Start cannot open
	// the pause menu in the destination level.
	PostUiLoad(&g_ui);
	StartMusidSong(psw->musid);
	StartSwIntermittentSounds(psw);

	std::cout << "World Loaded Successfully\n";
}

int FClipLineHomogeneous(glm::vec4* apos)
{
	/*for (int iplane = 0; iplane < 6; ++iplane)
	{
		const glm::vec4& plane = s_avecClip[iplane];

		float g0 = glm::dot(apos[0], plane);
		float g1 = glm::dot(apos[1], plane);

		if (g0 < 0.0f && g1 < 0.0f)
			return 0;

		if (g0 < 0.0f || g1 < 0.0f)
		{
			float u = g1 / (g1 - g0);
			glm::vec4 posClip = apos[0] * u + apos[1] * (1.0f - u);

			if (g0 < 0.0f)
				apos[0] = posClip;
			else
				apos[1] = posClip;
		}
	}*/

	return 1;
}

void DrawLineWorld(glm::vec3* ppos1, glm::vec3* ppos2, RGBA* prgba, CM* pcm, int fDepthTest)
{
	glm::vec4 aposClip[2] =
	{
		pcm->matWorldToClip * glm::vec4(*ppos1, 1.0f),
		pcm->matWorldToClip * glm::vec4(*ppos2, 1.0f)
	};

	if (!FClipLineHomogeneous(aposClip))
		return;

	for (glm::vec4& posClip : aposClip)
	{
		float rw = 1.0f / posClip.w;

		/*posClip.x = posClip.x * rw * g_vecScreenMultiply.x + g_vecScreenAdd.x;
		posClip.y = posClip.y * rw * g_vecScreenMultiply.y + g_vecScreenAdd.y;
		posClip.z = posClip.z * rw * g_vecScreenMultiply.z + g_vecScreenAdd.z;*/
	}

	//DrawLineScreen(aposClip[0].x, aposClip[0].y, aposClip[0].z, aposClip[1].x, aposClip[1].y, aposClip[1].z, prgba, fDepthTest);
}

void DrawBezierWorld(float dt, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, RGBA* prgba, CM* pcm)
{
	constexpr int cpos = 10;
	glm::vec3 apos[cpos];

	TesselateBezier(dt, 0.0f, dt, ppos0, pv0, ppos1, pv1, cpos, apos);

	for (int ipos = 0; ipos < cpos - 1; ++ipos)
		DrawLineWorld(&apos[ipos], &apos[ipos + 1], prgba, pcm, true);
}

void LoadNameTableFromBrx(CBinaryInputStream* pbis)
{
	pbis->U32Read();
}

void LoadWorldTableFromBrx(CBinaryInputStream* pbis)
{
	// Storing number of world tables from binary file
	int worldTableCount = pbis->U32Read();

	// Loading world table from binary file
	for (int i = 0; i < worldTableCount; i++)
		pbis->ReadStringSw();
}

void GetSwParams(SW* psw, SOP** ppsop)
{

}

void* GetSwMusicEnabled(SW* psw)
{
	return &psw->fMusicEnabled;
}

void SetSwMusicEnabled(SW* psw, int fMusicEnabled)
{
	psw->fMusicEnabled = fMusicEnabled;
}

void* GetSwMusicVolume(SW* psw)
{
	return &psw->uMusicVolume;
}

void SetSwMusicVolume(SW* psw, float uMusicVolume)
{
	psw->uMusicVolume = uMusicVolume;
}

void* GetSwGoldEnable(SW* psw)
{
	return &psw->tGoldEnable;
}

void SetSwGoldEnable(SW* psw, float tGoldEnable)
{
	psw->tGoldEnable = tGoldEnable;
}

void* GetSwCommentaryAudioDelay(SW* psw)
{
	return &psw->dtCommentaryAudio;
}

void SetSwCommentaryAudioDelay(SW* psw, float dtCommentaryAudio)
{
	psw->dtCommentaryAudio = dtCommentaryAudio;
}

void* GetSwExcitement(SW* psw)
{
	return &psw->gexcMenu;
}

int* PdifficultyEnsureSwCcoinRichMin(SW* psw, ENSK ensk)
{
	return &PdifficultyEnsureSw()->ccoinRichMin;
}

int* PdifficultyEnsureSwCcoinRichMax(SW* psw, ENSK ensk)
{
	return &PdifficultyEnsureSw()->ccoinRichMax;
}

int* PdifficultyEnsureSwCcoinPoorMin(SW* psw, ENSK ensk)
{
	return &PdifficultyEnsureSw()->ccoinPoorMin;
}

int* PdifficultyEnsureSwCcoinPoorMax(SW* psw, ENSK ensk)
{
	return &PdifficultyEnsureSw()->ccoinPoorMax;
}

void* GetSwDprizeEnabled(SW* psw)
{
	return &psw->fDprizeEnabled;
}

void SetSwDprizeEnabled(SW* psw, int fDprizeEnabled)
{
	psw->fDprizeEnabled = fDprizeEnabled;
}

void* GetSwIllum(SW *psw)
{
	return &psw->lsmDefault.uMidtone;
}

void* GetSwIllumShadow(SW *psw)
{
	return &psw->lsmDefault.uShadow;
}

void SetSwIllum(SW *psw, float uMidtone)
{
	psw->lsmDefault.uMidtone = uMidtone;
	g_cframeStaticLightsInvalid = g_cframe;
}

void SetSwIllumShadow(SW *psw, float uShadow)
{
	psw->lsmDefault.uShadow = uShadow;
	g_cframeStaticLightsInvalid = g_cframe;
}

void* GetSwSkyRgba(SW *psw)
{
	return &psw->rgbaSky;
}

void* GetSwDarken(SW* psw)
{
	return &psw->rDarken;
}

void SetSwDarken(SW *psw, float rDarken)
{
	psw->rDarken = rDarken;
	psw->rDarkenSmooth = rDarken;
}

void* GetSwDarkenSmooth(SW *psw)
{
	return &psw->rDarkenSmooth;
}

void SetSwDarkenSmooth(SW* psw, float rDarkenSmooth)
{
	psw->rDarkenSmooth = rDarkenSmooth;
}

void SetSwExcitement(SW* psw, float gexcMenu)
{
	psw->gexcMenu = gexcMenu;
	g_iexcHyst = -100;
}

void SetSwMusicRegister(SW* psw, int ireg, byte bVal)
{
	SetAMRegister(ireg, bVal);
}

void RefreshSwMidiRegister(SW* psw, int ireg)
{
	RefreshMidiRegister(ireg);
}

int FLevelSwVisited(SW* psw, WID wid)
{
	return GrflsFromWid(wid) & 1;
}

int FLevelSwPrimary(SW* psw, WID wid)
{
	return GrflsFromWid(wid) & 2;
}

int FLevelSwSecondary(SW* psw, WID wid)
{
	return GrflsFromWid(wid) & 4;
}

int FLevelSwTertiary(SW* psw, WID wid)
{
	return GrflsFromWid(wid) & 8;
}

bool FLevelKeyCollected(SW* psw, int levelID)
{
	return (TasksFromLevelID(levelID) & FLS_KeyCollected) != 0 && (g_plsCur->grfls & FLS_Visited) != 0;
}

bool FSecondaryTaskAvailable(SW* psw, int levelID)
{
	const uint32_t tasks = TasksFromLevelID(levelID);
	constexpr uint32_t prerequisiteMask = FLS_Visited | FLS_KeyCollected;

	if ((tasks & FLS_Secondary) == 0)
		return false;

	const uint32_t required = tasks & prerequisiteMask;
	return (g_plsCur->grfls & required) == required;
}

bool FCommentaryUnlockedForLevel(SW* psw, int levelId)
{
	const uint32_t tasks = TasksFromLevelID(levelId);
	constexpr uint32_t prerequisiteMask = FLS_Visited | FLS_KeyCollected | FLS_Secondary;

	if ((tasks & FLS_Tertiary) == 0)
		return false;

	const uint32_t required = tasks & prerequisiteMask;
	return (g_plsCur->grfls & required) == required;
}

bool FWorldVisited(void* context, int iWorld)
{
	return (g_pgsCur->aws[iWorld].fws & FWS_Visited) != 0;
}

bool FWorldComplete(void* context, int iWorld)
{
	return (g_pgsCur->aws[iWorld].fws & FWS_Complete) != 0;
}

int  RecalculatePercentCompletion()
{
	return CalculatePercentCompletion(g_pgsCur);
}

void SpawnRandomCoins(SW* psw, int ccoinMin, int ccoinMax, glm::vec3* ppos)
{
	int ccoin = NRandInRange(ccoinMin, ccoinMax);

	if (ccoin > 0)
		CpdprizeAttractSwDprizes(psw, CID_COIN, ppos, ccoin, nullptr);
}

void SetDifficultySuck(float uSuck)
{
	ChangeSuck(&g_difficulty, uSuck);
	return;
}

void GetSwPlayerSuck(SW* psw, float* puSuck)
{
	*puSuck = g_plsCur->uSuck;
}

void SetSuckRaw(float uSuckRaw)
{
	g_plsCur->uSuckRaw = GLimitLm(&g_lmZeroOne, uSuckRaw);
}

void GetuSuckRaw(SW* psw, float* puSuckRaw)
{
	*puSuckRaw = g_plsCur->uSuckRaw;
}

void IncrementSwHandsOff(SW* psw)
{
	if (++psw->cHandsOff == 1)
		AddGrfusr(2);
}

void DecrementSwHandsOff(SW* psw)
{
	if (--psw->cHandsOff == 0)
		RemoveGrfusr(2);
}

bool FSwHandsOff(SW* psw)
{
	return psw->cHandsOff > 0;
}

void IsSwVagPlaying(SW* psw, int* pfPlaying)
{
	*pfPlaying = FVagPlaying();
}

void CancelSwDialogPlaying(SW* psw)
{
	DIALOG* pdialog = g_binoc.pdialogPlaying;

	if (pdialog == nullptr)
		return;

	if (pdialog->dialogs == DIALOGS_Calling)
		return;

	uint32_t dialogkIndex = static_cast<uint32_t>(pdialog->dialogk + DIALOGK_Nil);

	if (dialogkIndex < 2)
		PopUiActiveBlot(&g_ui);
}

void SetSmaPrompt(PRK prk, SMA* psma)
{
	g_prompt.psma = psma;
	SetPrompt(&g_prompt, PRP_Basic, prk);
}

void AcquireSwLetterbox(SW* psw)
{
	++psw->cLetterbox;

	if (psw->cLetterbox == 1)
	{
		SetBlotDtVisible(&g_letterbox, 0.0f);
		g_letterbox.pvtblot->pfnShowBlot(&g_letterbox);
	}
}

void ReleaseSwLetterbox(SW* psw)
{
	--psw->cLetterbox;

	if (psw->cLetterbox == 0)
		g_letterbox.pvtblot->pfnHideBlot(&g_letterbox);
}

EXC* SetExcitement(SW* psw, IEXC iexc) 
{
	return PexcSetExcitement(iexc);
}

void UnsetExcitementSw(EXC* pexc)
{
	UnsetExcitementHyst(pexc);
}

void SetSwGameplayHud(SW* psw, ALO *palo)
{
	psw->paloHud = palo;
}

void UpdateSwHud(SW* psw)
{
	if (psw->paloHud != nullptr)
		psw->paloHud->pvtalo->pfnUpdateAloBlot(psw->paloHud);
}

void UnlockSwVault(SW* psw, uint32_t iVault)
{
	psw->grfvault |= 1U << (iVault & 31U);
}

void SetSwVaultLocked(SW* psw, uint32_t iVault)
{
	psw->grfvault &= ~(1U << (iVault & 31U));
}

void GetCharmCount(void* context, int* pcCharm)
{
	*pcCharm = g_pgsCur->ccharm;
}

void GetLifeCount(void* context, int* pcLife)
{
	*pcLife = g_pgsCur->clife;
}

void FreeSwStsoList(SW* psw, STSO* pstsoFirst)
{

}

void DeleteWorld(SW *psw)
{
	// These frame-pending lists are global, but their entries are owned by the
	// current SW.  Do not leave their head/tail pointers referring to BLIP and
	// BLIPG objects that will be destroyed with this world.
	ClearAllPendingFrames();

	SwitchToIppo(-1);
	g_pjt = nullptr;
	g_grfusr = 0;
	g_cppo = 0;
	UpdateGrfjoytFromGrfusr();
	if (g_pcm != nullptr)
		ClearCmFadeObjects(g_pcm);

	ResetTargetList();
	ResetJmtList();
	ResetPipeList();
	ResetHideList();
	// Credit animations use an ASEGA's local time while they are active. Return
	// them to the persistent real-time clock before destroying world-owned ASEGAs.
	SetCreditClock(&g_credit, nullptr);
	// Blots retain pointers into g_afontBrx. Reset the UI while those fonts
	// are still alive; UnloadShaders clears the font vector and invalidates
	// every cached BLOT::pfont pointer.
	ResetUi(&g_ui);
	UnloadShaders();

	if (psw != nullptr)
		FreeSwCallbackQueue(psw);

	g_timer.pfntnThreshold = 0;
	g_coinctr.pfnsmack = nullptr;
	g_rtClockPowerUp = 1.0;
	g_pcm = nullptr;
	g_pkeyhole = nullptr;
	g_cdialogTriggered = 0;
	g_coinctr.pv = nullptr;
	g_timer.pfntnExpire = 0;

	numRo = 0;
	numRoCel = 0;

	numFrameObjs = 0;

	g_cframe = 0;

	g_boundVAO = 0;

	g_shdIDBound = -1;

	g_lastAnimateUv = -1;

	g_cFrameGlobs = 0;
	g_cFrameCelGlobs = 0;

	g_dynamicTextureCount = 0;
	g_dynamicTexturePrpl.clear();
	g_dynamicTexturePrpl.shrink_to_fit();

	g_backGroundCount = 0;
	g_backGroundPrpl.clear();
	g_backGroundPrpl.shrink_to_fit();

	g_backGroundBlendCount = 0;
	g_backGroundBlendPrpl.clear();
	g_backGroundBlendPrpl.shrink_to_fit();

	g_blotContextCount = 0;
	g_blotContextPrpl.clear();
	g_blotContextPrpl.shrink_to_fit();

	g_opaqueCount = 0;
	g_opaquePrpl.clear();
	g_opaquePrpl.shrink_to_fit();

	g_cutOutCount = 0;
	g_cutOutPrpl.clear();
	g_cutOutPrpl.shrink_to_fit();

	g_cutOutBlendAddCount = 0;
	g_cutOutBlendAddPrpl.clear();
	g_cutOutBlendAddPrpl.shrink_to_fit();

	g_celBorderCount = 0;
	g_celBorderPrpl.clear();
	g_celBorderPrpl.shrink_to_fit();

	g_projVolumeCount = 0;
	g_projVolumePrpl.clear();
	g_projVolumePrpl.shrink_to_fit();

	g_projVolumeAlphaAddCount = 0;
	g_projVolumeAlphaAddPrpl.clear();
	g_projVolumeAlphaAddPrpl.shrink_to_fit();

	g_projVolumeAddCount = 0;
	g_projVolumeAddPrpl.clear();
	g_projVolumeAddPrpl.shrink_to_fit();

	g_opaqueAfterProjVolumeCount = 0;
	g_opaqueAfterProjVolumePrpl.clear();
	g_opaqueAfterProjVolumePrpl.shrink_to_fit();

	g_cutOutAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumePrpl.clear();
	g_cutOutAfterProjVolumePrpl.shrink_to_fit();

	g_cutOutAfterProjVolumeAddCount = 0;
	g_cutOutAfterProjVolumeAddPrpl.clear();
	g_cutOutAfterProjVolumeAddPrpl.shrink_to_fit();

	g_celBorderAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumePrpl.clear();
	g_celBorderAfterProjVolumePrpl.shrink_to_fit();

	g_murkClearCount = 0;
	g_murkClearPrpl.clear();
	g_murkClearPrpl.shrink_to_fit();

	g_murkOpaqueCount = 0;
	g_murkOpaquePrpl.clear();
	g_murkOpaquePrpl.shrink_to_fit();

	g_murkFillCount = 0;
	g_murkFillPrpl.clear();
	g_murkFillPrpl.shrink_to_fit();
	
	g_translucentCount = 0;
	g_translucentPrpl.clear();
	g_translucentPrpl.shrink_to_fit();

	g_translucentAddCount = 0;
	g_translucentAddPrpl.clear();
	g_translucentAddPrpl.shrink_to_fit();

	g_translucentCelBorderCount = 0;
	g_translucentCelBorderPrpl.clear();
	g_translucentCelBorderPrpl.shrink_to_fit();

	g_blipCount = 0;
	g_blipPrpl.clear();
	g_blipPrpl.shrink_to_fit();

	g_foreGroundCount = 0;
	g_foreGroundPrpl.clear();
	g_foreGroundPrpl.shrink_to_fit();

	g_worldMapCount = 0;
	g_worldMapPrpl.clear();
	g_worldMapPrpl.shrink_to_fit();

	g_maxCount = 0;
	g_maxPrpl.clear();
	g_maxPrpl.shrink_to_fit();

	DeallocateLightBlkList();

	for (int i = 0; i < allSWAloObjs.size(); i++)
		DeleteModel(allSWAloObjs[i]);

	DeleteSwCollision();

	for (int i = 0; i < allWorldObjs.size(); i++)
		allWorldObjs[i]->pvtlo->pfnDeleteLo(allWorldObjs[i]);

	allSWAloObjs.clear();
	allSWAloObjs.shrink_to_fit();
	allWorldObjs.clear();
	allWorldObjs.shrink_to_fit();

	DeallocateSoVector();

	g_psw = nullptr;
	g_pcm = nullptr;

	baseRenderDistance = 0.0;

	glDeleteBuffers(1, &cmUBO);

	DeleteFrameStream(&ropStream);
	DeleteFrameStream(&rcbStream);
	DeleteFrameStream(&blipStream);

	glDeleteBuffers(1, &geomUBO);

	DeallocateSwShadows();

	FreeBinocGL(&g_binoc);

	rgbaSky = glm::vec4(0.0);
	std::cout << "World Deleted\n";
}

void FreeSwCallbackQueue(SW* psw)
{
	FreeSwMqList(psw, psw->pmqCallbackFirst);
	psw->pmqCallbackLast.reset();
}

void DeleteSw(SW* psw)
{
	if (psw == nullptr)
		return;

	KillSoundSystem();
	DeleteSwRipPool(psw);
	delete psw;
}

SW* g_psw = nullptr;
glm::vec4 rgbaSky{0.0};

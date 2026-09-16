#include "xform.h"
#include "aseg.h"
#include "po.h"
#include "button.h"
#include "totals.h"
#include "credit.h"
#include "binoc.h"
#include "wm.h"
#include "sw.h"
#include <cstdio>

XFM* NewXfm()
{
	return new XFM{};
}

void InitXfm(XFM* pxfm)
{
	InitLo(pxfm);
	pxfm->matLocal = glm::mat3(1.0);
}

int GetXfmSize()
{
	return sizeof(XFM);
}

void LoadXfmFromBrx(XFM *pxfm, CBinaryInputStream* pbis)
{
	pxfm->matLocal = pbis->ReadMatrix();
	pxfm->posLocal = pbis->ReadVector();
	LoadOptionsFromBrx(pxfm, pbis);
}

void GetXfmPos(XFM* pxfm, glm::vec3* ppos)
{
	ConvertAloPos(pxfm->paloParent, nullptr, &pxfm->posLocal, ppos);
}

void GetXfmMat(XFM* pxfm, glm::mat3* pmat)
{
	ConvertAloMat(pxfm->paloParent, nullptr, &pxfm->matLocal, pmat);
}

void* GetXfmPosLocal(XFM* pxfm)
{
	return &pxfm->posLocal;
}

void SetXfmPosLocal(XFM* pxfm, glm::vec3* pposlocal)
{
	pxfm->posLocal = *pposlocal;
}

void* GetXfmMatLocal(XFM* pxfm)
{
	return &pxfm->matLocal;
}

void SetXfmMatLocal(XFM* pxfm, glm::mat3* pmatLocal)
{
	pxfm->matLocal = *pmatLocal;
}

void CloneXfm(XFM* pxfm, XFM* pxfmBase)
{
	CloneLo(pxfm, pxfmBase);

	pxfm->posLocal = pxfmBase->posLocal;
	pxfm->matLocal = pxfmBase->matLocal;
}

void SetXfmParent(XFM* pxfm, ALO* paloParent)
{
	ConvertAloPos(pxfm->paloParent, paloParent, &pxfm->posLocal, &pxfm->posLocal);
	ConvertAloMat(pxfm->paloParent, paloParent, &pxfm->matLocal, &pxfm->matLocal);

	SetLoParent(pxfm, paloParent);
}

void ApplyXfmProxy(XFM* pxfm, PROXY* pproxyApply)
{
	ConvertAloPos((ALO*)pproxyApply, nullptr, &pxfm->posLocal, &pxfm->posLocal);
	ConvertAloMat((ALO*)pproxyApply, nullptr, &pxfm->matLocal, &pxfm->matLocal);
}

void ConvertXfmWorldToLocal(XFM* pxfm, glm::vec3* pposWorld, glm::vec3* pposLocal)
{
	glm::vec3 pos;
	ConvertAloPos(nullptr, pxfm->paloParent, pposWorld, &pos);

	glm::vec3 dpos = pos - pxfm->posLocal;

	*pposLocal = glm::vec3(
		glm::dot(pxfm->matLocal[0], dpos),
		glm::dot(pxfm->matLocal[1], dpos),
		glm::dot(pxfm->matLocal[2], dpos)
	);
}

void DeleteXfm(XFM* pxfm)
{
	delete pxfm;
}

WARP* NewWarp()
{
	return new WARP{};
}

int GetWarpSize()
{
	return sizeof(WARP);
}

void* GetWarpV(WARP* pwarp)
{
	return &pwarp->v;
}

void SetWarpV(WARP* pwarp, glm::vec3 v)
{
	pwarp->v = v;
}

void* GetWarpVx(WARP* pwarp)
{
	return &pwarp->v.x;
}

void SetWarpVx(WARP* pwarp, float x)
{
	pwarp->v.x = x;
}

void* GetWarpVy(WARP* pwarp)
{
	return &pwarp->v.y;
}

void SetWarpVy(WARP* pwarp, float y)
{
	pwarp->v.y = y;
}

void* GetWarpVz(WARP* pwarp)
{
	return &pwarp->v.z;
}

void SetWarpVz(WARP* pwarp, float z)
{
	pwarp->v.z = z;
}

void* GetWarpFDefault(WARP* pwarp)
{
	return &pwarp->fDefault;
}

void SetWarpFDefault(WARP* pwarp, int fDefault)
{
	pwarp->fDefault = fDefault;
}

void* GetWarpFDefaultDifficult(WARP* pwarp)
{
	return &pwarp->fDefaultDifficult;
}

void SetWarpFDefaultDifficult(WARP* pwarp, int fDefaultDifficult)
{
	pwarp->fDefaultDifficult = fDefaultDifficult;
}

void* GetWarpOidAlias(WARP* pwarp)
{
	return &pwarp->oidAlias;
}

void SetWarpOidAlias(WARP* pwarp, int oidAlias)
{
	pwarp->oidAlias = (OID)oidAlias;
}

void* GetWarpRadCmInit(WARP* pwarp)
{
	return &pwarp->radCmInit;
}

WARP* PwarpFromOid(OID oid, OID oidContext)
{
	if (oid != OID_Nil)
	{
		LO* ploContext = PloFindSwObject(g_psw, 257, oidContext, nullptr);
		WARP* pwarp = static_cast<WARP*>(PloFindSwObject(g_psw, 257, oid, ploContext));

		if (pwarp != nullptr)
			return pwarp;
	}

	WARP* apwarp[64]{};
	int cpwarp = CploFindSwObjectsByClass(
		g_psw, 517, CID_WARP, nullptr, 256, reinterpret_cast<LO**>(apwarp));
	cpwarp = std::min(cpwarp, 64);

	if (oid != OID_Nil)
	{
		for (int ipwarp = 0; ipwarp < cpwarp; ++ipwarp)
		{
			WARP* pwarp = apwarp[ipwarp];

			if (pwarp->oidAlias == oid)
				return pwarp;
		}

	}

	WARP* pwarpDefault = nullptr;
	WARP* pwarpSpecialDefault = nullptr;

	for (int ipwarp = 0; ipwarp < cpwarp; ++ipwarp)
	{
		WARP* pwarp = apwarp[ipwarp];

		if (pwarp->fDefault)
			pwarpDefault = pwarp;

		if (g_wipe.wipes != WIPES_Idle && g_wipe.wipek == WIPEK_WorldMap && pwarp->fDefaultSpecial)
			pwarpSpecialDefault = pwarp;
	}

	if (pwarpSpecialDefault != nullptr)
		return pwarpSpecialDefault;

	return pwarpDefault;
}

void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis)
{
	pwarp->matLocal = pbis->ReadMatrix();
	pwarp->posLocal = pbis->ReadVector();

	LoadOptionsFromBrx(pwarp, pbis);

	pwarp->cpaseg = pbis->S16Read();
	pwarp->apaseg.resize(pwarp->cpaseg);

	for (int i = 0; i < pwarp->cpaseg; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		ASEG *paseg = (ASEG*)PloNew(cid, pwarp->psw, nullptr, oid, isplice);
		pwarp->apaseg[i] = paseg;
		paseg->pvtaseg->pfnLoadAsegFromBrx(paseg, pbis);
	}

	pbis->ReadStringSw();

	pwarp->coidHide = pbis->S16Read();
	pwarp->aoidHide.resize(pwarp->coidHide);

	for (int i = 0; i < pwarp->coidHide; i++)
		pwarp->aoidHide[i] = (OID)pbis->S16Read();
}

void CloneWarp(WARP* pwarp, WARP* pwarpBase)
{
	CloneXfm(pwarp, pwarpBase);

	//pwarp->widmenu = pwarpBase->widmenu;
	pwarp->v = pwarpBase->v;
	pwarp->fDefault = pwarpBase->fDefault;
	pwarp->fDefaultSpecial = pwarpBase->fDefaultSpecial;
	pwarp->radCmInit = pwarpBase->radCmInit;
	pwarp->cpaseg = pwarpBase->cpaseg;
	pwarp->fDefaultDifficult = pwarpBase->fDefaultDifficult;
	pwarp->fUseRadCmInit = pwarpBase->fUseRadCmInit;

	pwarp->coidHide = pwarpBase->coidHide;

	// Clone aoidHide (vector of OIDs)
	pwarp->aoidHide = pwarpBase->aoidHide;

	pwarp->coidShowWhenDifficult = pwarpBase->coidShowWhenDifficult;

	// Clone aoidShowWhenDifficult (array of OIDs)
	for (int i = 0; i < 4; i++) {
		pwarp->aoidShowWhenDifficult[i] = pwarpBase->aoidShowWhenDifficult[i];
	}

	pwarp->oidAlias = pwarpBase->oidAlias;
	pwarp->crsmg = pwarpBase->crsmg;

	// Clone RSMG array
	for (int i = 0; i < 4; i++) {
		pwarp->arsmg[i] = pwarpBase->arsmg[i];
	}

	pwarp->apaseg.resize(pwarp->cpaseg);

	for (int i = 0; i < pwarp->cpaseg; i++)
		pwarp->apaseg[i] = (ASEG*)PloCloneLo((LO*)pwarpBase->apaseg[i], pwarp->psw, nullptr);
}

void PostWarpLoad(WARP* pwarp)
{
	PostLoLoad(pwarp);
	for (int i = 0; i < pwarp->cpaseg; i++)
	{
		ASEG* paseg = pwarp->apaseg[i];
		paseg->pvtlo->pfnSetLoParent(paseg, pwarp->paloParent);
		SnipLo(paseg);
	}
}

void TriggerWarp(WARP* pwarp)
{
	PO* ppo = PpoStart();

	glm::vec3 posWarp{};
	glm::mat3 matWarp{ 1.0f };

	GetXfmPos(pwarp, &posWarp);
	GetXfmMat(pwarp, &matWarp);

	if (ppo == nullptr)
	{
		constexpr float kCameraDistance = 304.0f;

		const glm::vec3 posEye = posWarp - matWarp[0] * kCameraDistance;

		SetCmLookAt(g_pcm, &posEye, &posWarp);
		SetCmCut(g_pcm, nullptr);
		return;
	}

	ppo->pvtalo->pfnRotateAloToMat(ppo, &matWarp);
	TranslateSoToPosSafe(ppo, &posWarp, &g_normalZ);
	ppo->pvtalo->pfnSetAloVelocityVec(ppo, &pwarp->v);
	ppo->pvtalo->pfnSetAloAngularVelocityVec(ppo, &g_vecZero);

	SwitchToPo(ppo);

	for (int ipaseg = 0; ipaseg < pwarp->cpaseg; ++ipaseg)
		ApplyAseg(pwarp->apaseg[ipaseg], nullptr, 0.0f, 1.0f, 0, nullptr);

	TriggerRsmg(pwarp->psw, pwarp->crsmg, pwarp->arsmg, static_cast<LO*>(pwarp), 1);

	for (int ioid = 0; ioid < pwarp->coidHide; ++ioid)
	{
		LO* plo = PloFindSwObject(pwarp->psw, 260, pwarp->aoidHide[ioid], static_cast<LO*>(pwarp));

		if (plo != nullptr)
			plo->pvtlo->pfnRemoveLo(plo);
	}

	std::array <EXIT*, 64> aexit{};

	int cexit = CploFindSwObjectsByClass(pwarp->psw, 0x205, CID_EXIT, nullptr, 64, reinterpret_cast<LO**>(aexit.data()));
	cexit = std::min(cexit, static_cast<int>(aexit.size()));

	for (int iexit = cexit - 1; iexit >= 0; --iexit)
	{
		EXIT* pexit = aexit[iexit];

		if (pexit->exits != EXITS_Disabled)
			SetExitExits(pexit, EXITS_Blocked);
	}

	if (ppo->fActive)
	{
		if (pwarp->fUseRadCmInit)
		{
			const float radWarp = std::atan2(matWarp[0].y, matWarp[0].x);
			float radCut = RadNormalize(radWarp + pwarp->radCmInit);

			SetCmCut(g_pcm, &radCut);
		}
		else
		{
			SetCmCut(g_pcm, nullptr);
		}
	}

	HandleLoSpliceEvent(pwarp, 2, 0, nullptr);
}

void SetWarpRsmg(WARP* pwarp, int fOnTrigger, int oidRoot, int oidSM, int oidGoal)
{
	FAddRsmg(pwarp->arsmg, 4, &pwarp->crsmg, fOnTrigger, (OID)oidRoot, (OID)oidSM, (OID)oidGoal);
}

void SetWarpCmInitAngle(WARP* pwarp, float radCmInit)
{
	pwarp->radCmInit = radCmInit;
	pwarp->fUseRadCmInit = true;
}

void TeleportSwPlayer(SW* psw, OID oidWarp, OID oidWarpContext)
{
	WARP* pwarp = PwarpFromOid(oidWarp, oidWarpContext);

	if (pwarp != nullptr)
	{
		TriggerWarp(pwarp);
		return;
	}

	if (PpoCur() == nullptr)
	{
		PO* ppo = PpoStart();

		if (ppo != nullptr)
			SwitchToPo(ppo);
	}
}

void DeleteWarp(WARP* pwarp)
{
	delete pwarp;
}

EXIT* NewExit()
{
	return new EXIT{};
}

int GetExitSize()
{
	return sizeof(EXIT);
}

void* GetExitOidWarp(EXIT* pexit)
{
	return &pexit->oidWarp;
}

void SetExitOidWarp(EXIT* pexit, int oidWarp)
{
	pexit->oidWarp = (OID)oidWarp;
}

void* GetExitEdkAlt(EXIT* pexit)
{
	return &pexit->edkAlt;
}

void SetExitEdkAlt(EXIT* pexit, EDK edkAlt)
{
	pexit->edkAlt = edkAlt;
}

void* GetExitOidWarpAlt(EXIT* pexit)
{
	return &pexit->oidWarpAlt;
}

void SetExitOidWarpAlt(EXIT* pexit, int oidWarpAlt)
{
	pexit->oidWarpAlt = oidWarpAlt;
}

void* GetExitWipek(EXIT* pexit)
{
	return &pexit->wipek;
}

void SetExitWipek(EXIT* pexit, WIPEK wipek)
{
	pexit->wipek = wipek;
}

void* GetExitDtUnblock(EXIT* pexit)
{
	return &pexit->dtUnblock;
}

void SetExitDtUnblock(EXIT* pexit, float dtUnblock)
{
	pexit->dtUnblock = dtUnblock;
}

void* GetExitFDefault(EXIT* pexit)
{
	return &pexit->fDefault;
}

void SetExitFDefault(EXIT* pexit, int fDefault)
{
	pexit->fDefault = fDefault;
}

void* GetExitFKeyed(EXIT* pexit)
{
	return &pexit->fKeyed;
}

void SetExitFKeyed(EXIT* pexit, int fKeyed)
{
	pexit->fKeyed = fKeyed;
}

void* GetExitFFollowDefault(EXIT* pexit)
{
	return &pexit->fFollowDefault;
}

void SetExitFFollowDefault(EXIT* pexit, int fFollowDefault)
{
	pexit->fFollowDefault = fFollowDefault;
}

void* GetExitFUseDefaultExit(EXIT* pexit)
{
	return &pexit->fUseDefaultExit;
}

void SetExitFUseDefaultExit(EXIT* pexit, int fUseDefaultExit)
{
	pexit->fUseDefaultExit = fUseDefaultExit;
}

void* GetExitFTotals(EXIT* pexit)
{
	return &pexit->fTotals;
}

void SetExitFTotals(EXIT* pexit, int fTotals)
{
	pexit->fTotals = fTotals;
}

void* GetExitDtTriggerWipe(EXIT* pexit)
{
	return &pexit->dtTriggerWipe;
}

void SetExitDtTriggerWipe(EXIT* pexit, float dtTriggerWipe)
{
	pexit->dtTriggerWipe = dtTriggerWipe;
}

void* GetExitExits(EXIT* pexit)
{
	return &pexit->exits;
}

EXIT* PexitDefault()
{
	std::array<LO*, 64> objects{};

	int count = CploFindSwObjectsByClass(g_psw, 517, CID_EXIT, nullptr, static_cast<int>(objects.size()), objects.data());
	count = std::min(count, static_cast<int>(objects.size()));

	for (int i = 0; i < count; ++i)
	{
		EXIT* pexit = static_cast<EXIT*>(objects[i]);

		if (pexit->fDefault)
			return pexit;
	}

	return nullptr;
}

void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis)
{
	
	LEVELINFOBRX levelBrx{};
	LEVELINFOBRX levelAltBrx{};

	pbis->file.read(reinterpret_cast<char*>(&levelBrx), sizeof(levelBrx));
	pbis->file.read(reinterpret_cast<char*>(&levelAltBrx), sizeof(levelAltBrx));

	const uint32_t searchKey = levelBrx.searchValue ^ levelBrx.searchCipher;
	const uint32_t searchKeyAlt = levelAltBrx.searchValue ^ levelAltBrx.searchCipher;

	pexit->plevel = PlevelinfoFromSearchKey(searchKey);
	pexit->plevelAlt = PlevelinfoFromSearchKey(searchKeyAlt);

	pexit->xf.mat = pbis->ReadMatrix();
	pexit->xf.pos = pbis->ReadVector();

	if (pexit->pvtexit->pfnUpdateAloXfWorld != nullptr)
		pexit->pvtexit->pfnUpdateAloXfWorld(pexit);

	LoadTbspFromBrx(&pexit->ctsurf, pexit->atsurf, &pexit->ctbsp, pexit->atbsp, pbis);
	LoadOptionsFromBrx(pexit, pbis);

	pexit->cpaseg = pbis->S16Read();
	pexit->apaseg.resize(pexit->cpaseg);

	for (int i = 0; i < pexit->cpaseg; ++i)
	{
		const CID cid = static_cast<CID>(pbis->S16Read());
		const OID oid = static_cast<OID>(pbis->S16Read());
		const uint16_t isplice = pbis->U16Read();

		ASEG* paseg = static_cast<ASEG*>(PloNew(cid, pexit->psw, pexit->paloParent, oid, isplice));
		pexit->apaseg[i] = paseg;

		if (paseg == nullptr)
			continue;

		paseg->pvtaseg->pfnLoadAsegFromBrx(paseg, pbis);
		paseg->pvtaseg->pfnRemoveLo(paseg);
	}
}

void CloneExit(EXIT* pexit, EXIT* pexitBase)
{
	CloneAlo(pexit, pexitBase);

	pexit->fDefault = pexitBase->fDefault;
	pexit->fKeyed = pexitBase->fKeyed;
	pexit->fFollowDefault = pexitBase->fFollowDefault;
	pexit->fTotals = pexitBase->fTotals;
	pexit->exits = pexitBase->exits;
	pexit->tExits = pexitBase->tExits;
	pexit->ctsurf = pexitBase->ctsurf;

	// Clone atsurf (pointer to surface data)
	pexit->atsurf = pexitBase->atsurf;

	pexit->ctbsp = pexitBase->ctbsp;

	// Clone atbsp (pointer to BSP data)
	pexit->atbsp = pexitBase->atbsp;

	pexit->widWarp = pexitBase->widWarp;
	pexit->oidWarp = pexitBase->oidWarp;
	pexit->edkAlt = pexitBase->edkAlt;
	pexit->widAlt = pexitBase->widAlt;
	pexit->oidAlt = pexitBase->oidAlt;
	pexit->cpaseg = pexitBase->cpaseg;

	// Clone apaseg (array of void pointers)
	for (int i = 0; i < pexitBase->cpaseg; i++) 
		pexit->apaseg[i] = pexitBase->apaseg[i];

	pexit->wipek = pexitBase->wipek;
	pexit->tWipe = pexitBase->tWipe;
	pexit->dtUnblock = pexitBase->dtUnblock;
	pexit->dtTriggerWipe = pexitBase->dtTriggerWipe;
}

void PostExitLoad(EXIT* pexit)
{
	PostAloLoad(pexit);
	for (int i = 0; i < pexit->cpaseg; ++i)
	{
		ASEG* paseg = pexit->apaseg[i];

		if (paseg->pvtlo->pfnBindLo != nullptr)
			paseg->pvtlo->pfnBindLo(paseg);

		if (paseg->pvtlo->pfnPostLoLoad != nullptr)
			paseg->pvtlo->pfnPostLoLoad(paseg);
	}

	if (!pexit->fKeyed || (g_plsCur->grfls & 2) != 0)
		return;

	ALO* paloRoot = pexit;

	while (paloRoot->paloParent != nullptr)
		paloRoot = paloRoot->paloParent;

	paloRoot->pvtlo->pfnRemoveLo(paloRoot);
}

void SetExitExits(EXIT* pexit, EXITS exits)
{
	if (exits == pexit->exits)
		return;

	if (pexit->exits == EXITS_Totals && exits != EXITS_Exiting)
		g_totals.pvtblot->pfnHideBlot(&g_totals);

	if (exits == EXITS_Exiting)
	{
		pexit->fNoFreeze = 1;
		IncrementSwHandsOff(pexit->psw);
	}

	pexit->exits = exits;
	pexit->tExits = g_clock.t;
}

void TriggerDefaultExit(int fInhibitAsegs, WIPEK wipek)
{
	if (g_pgsCur == nullptr)
		return;

	if (g_pgsCur->gameWorldCur != GAMEWORLD_Clockwerk)
	{
		if (g_pgsCur->worldLevelCur != 0)
		{
			EXIT* pexit = PexitDefault();

			if (pexit != nullptr)
			{
				if (fInhibitAsegs != 0)
					pexit->cpaseg = 0;

				TriggerExit(pexit);
				return;
			}

			LEVELINFO* plevelCurrent = g_transition.m_plevelCurrent;

			if (plevelCurrent == nullptr && !g_transition.m_worldCurrent.empty())
				plevelCurrent = PlevelinfoFromLevelName(g_transition.m_worldCurrent);

			if (plevelCurrent == nullptr)
			{
				std::printf("DEFAULT EXIT ERROR: no current level\n");
				return;
			}

			TRANS trans{};
			trans.fSet = 1;
			trans.plevel = plevelCurrent;
			trans.oidWarp = OID_Nil;
			trans.oidWarpContext = OID_Nil;
			trans.grftrans = static_cast<GRFTRANS>(0);

			ActivateWipe(&g_wipe, &trans, wipek);
			return;
		}

		if (g_pwsCur != nullptr && (g_pwsCur->als[1].grfls & 1U) != 0)
		{
			const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | 1;
			LEVELINFO* plevel = PlevelInfoFromId(levelId);

			if (plevel != nullptr)
				WipeToWorldWarp(plevel, static_cast<OID>(445), wipek);
			else
				std::printf("DEFAULT EXIT ERROR: level ID 0x%X not found\n", levelId);

			return;
		}
	}

	LEVELINFO* plevelHideout = PlevelinfoFromSearchKey(9);

	if (plevelHideout != nullptr)
		WipeToWorldWarp(plevelHideout, OID_Nil, wipek);
	else
		std::printf("DEFAULT EXIT ERROR: Hideout search key 9 not found\n");
}

void TriggerExit(EXIT* pexit)
{
	bool fWipingAseg = false;
	PO* ppo = PpoCur();

	const int destinationLevelId = pexit->plevel != nullptr ? pexit->plevel->levelID : -1;
	const bool fExitsCurrentWorld = g_pgsCur->worldLevelCur == 0 && (destinationLevelId & 0xFF) == 1;
	const bool fFinishSprint = g_note.mtsState == MTSSTATE_Active && (pexit->fFollowDefault || pexit->fKeyed || fExitsCurrentWorld);

	if (fFinishSprint)
	{
		FinishNoteSprint(&g_note);
		SetExitExits(pexit, EXITS_Disabled);
	}
	else
	{
		if (g_note.mtsState >= MTSSTATE_Active)
			SetNoteMtsState(&g_note, MTSSTATE_Declined);

		if (pexit->fUseDefaultExit)
		{
			TriggerDefaultExit(1, pexit->wipek);
			SetExitExits(pexit, EXITS_Blocked);
			return;
		}

		if (ppo == nullptr || !FIsBasicDerivedFrom(ppo, CID_JT))
			RevokeCmPolicy(g_pcm, 2, CPP_Base, &g_pcm->cplook, nullptr, nullptr);

		SetExitExits(pexit, EXITS_Exiting);

		for (int i = 0; i < pexit->cpaseg; ++i)
		{
			ASEG* paseg = pexit->apaseg[i];

			ApplyAseg(paseg, nullptr, 0.0f, 1.0f, static_cast<GRFAPL>(0), nullptr);

			if (FWipingAseg(paseg))
				fWipingAseg = true;
		}

		if (!fWipingAseg)
		{
			if (pexit->dtTriggerWipe > 0.0f)
				pexit->tWipe = g_clock.t + pexit->dtTriggerWipe;
			else if (pexit->cpaseg > 0)
				pexit->tWipe = g_clock.t + pexit->apaseg[0]->tMax;
			else
				pexit->tWipe = g_clock.t;
		}
	}

	HandleLoSpliceEvent(pexit, 2, 0, nullptr);
}

void WipeExit(EXIT* pexit)
{
	if (pexit == nullptr || pexit->plevel == nullptr)
		return;

	bool fUseAlternate = false;

	switch (pexit->edkAlt)
	{
		case EDK_DestinationKey:
		{
			const WID widDestination = static_cast<WID>(pexit->plevel->levelID);
			const uint32_t grflsDestination = GrflsFromWid(widDestination);

			fUseAlternate = (grflsDestination & FLS_KeyCollected) != 0;
			break;
		}

		case EDK_CurrentKey:
		{
			if (g_plsCur != nullptr)
				fUseAlternate = (g_plsCur->grfls & FLS_KeyCollected) != 0;

			break;
		}

		default:
		break;
	}

	if (fUseAlternate && pexit->plevelAlt == nullptr)
		fUseAlternate = false;

	TRANS trans{};
	trans.fSet = 1;
	trans.plevel = fUseAlternate ? pexit->plevelAlt : pexit->plevel;
	trans.oidWarp = fUseAlternate ? (OID)pexit->oidWarpAlt : (OID)pexit->oidWarp;
	trans.oidWarpContext = OID_Nil;
	trans.grftrans = static_cast<GRFTRANS>(0);

	ActivateWipe(&g_wipe, &trans, pexit->wipek);
}

void UpdateExit(EXIT* pexit, float dt)
{
	UpdateAlo(pexit, dt);

	if (pexit->exits == EXITS_Exiting)
	{
		if (pexit->tWipe > 0.0f && g_clock.t > pexit->tWipe)
		{
			WipeExit(pexit);
			pexit->tWipe = 0.0f;
		}

		return;
	}

	if (pexit->exits == EXITS_Totals && !g_wmc.fActive)
	{
		const bool fCanShowTotals =
			!g_prompt.fActive &&
			g_binoc.binocs != BINOCS_Peek &&
			g_binoc.binocs != BINOCS_Dialog &&
			(g_binoc.binocs != BINOCS_Instruct || g_binoc.achzDraw[0] == '\0');

		if (fCanShowTotals)
		{
			SetBlotDtVisible(&g_totals, 0.0f);

			const int levelId = pexit->plevel != nullptr ? pexit->plevel->levelID : -1;
			SetTotalsDestinationText(&g_totals, levelId);

			g_totals.fDrawOverLetterbox = 0;
			g_totals.pvtblot->pfnShowBlot(&g_totals);
		}
		else
			g_totals.pvtblot->pfnHideBlot(&g_totals);
	}

	if (pexit->exits != EXITS_Blocked &&
		pexit->exits != EXITS_Unblocked &&
		pexit->exits != EXITS_Enabled &&
		pexit->exits != EXITS_Totals)
		return;

	PO* ppo = PpoCur();

	if (!ppo)
		return;

	glm::vec3 posLocal{};
	ConvertAloPos(nullptr, pexit, &ppo->xf.posWorld, &posLocal);

	const bool fInsideExit = FCheckTbspPoint(pexit->atbsp.data(), &posLocal);
	const bool fNearExit = !fInsideExit && pexit->fTotals && std::abs(posLocal.z) < 300.0f && glm::length(posLocal) <= 1000.0f;

	switch (pexit->exits)
	{
		case EXITS_Blocked:
		{
			if (!fInsideExit)
				SetExitExits(pexit, EXITS_Unblocked);

			break;
		}

		case EXITS_Unblocked:
		{
			if (fInsideExit)
			{
				SetExitExits(pexit, EXITS_Blocked);
				break;
			}

			if (g_clock.t - pexit->tExits > pexit->dtUnblock)
				SetExitExits(pexit, EXITS_Enabled);

			break;
		}

		case EXITS_Enabled:
		{
			if (fInsideExit)
			{
				TriggerExit(pexit);
				break;
			}

			if (!fNearExit || g_totals.blots != BLOTS_Hidden)
				break;

			if (SphereInFrustum(g_pcm->frustum, pexit->xf.posWorld, 100.0f))
				SetExitExits(pexit, EXITS_Totals);

			break;
		}

		case EXITS_Totals:
		{
			if (fInsideExit)
			{
				TriggerExit(pexit);
				break;
			}

			if (!fNearExit)
				SetExitExits(pexit, EXITS_Enabled);

			break;
		}

		default:
		break;
	}
}

void DeleteExit(EXIT* pexit)
{
	delete pexit;
}

CAMERA* NewCamera()
{
	return new CAMERA{};
}

void InitCamera(CAMERA* pcamera)
{
	InitAlo(pcamera);
	pcamera->oidTarget = OID_Nil;
}

void SetCameraOidTarget(CAMERA* pcamera, OID oidTarget)
{
	pcamera->oidTarget = oidTarget;
}

void* GetCameraOidTarget(CAMERA* pcamera)
{
	return &pcamera->oidTarget;
}

void SetCameraPosEye(CAMERA* pcamera, glm::vec3 posEye)
{
	pcamera->posEye = posEye;
}

void* GetCameraPosEye(CAMERA* pcamera)
{
	return &pcamera->posEye;
}

void SetCameraVecView(CAMERA* pcamera, glm::vec3 vecView)
{
	pcamera->vecView = vecView;
}

void* GetCameraVecView(CAMERA* pcamera)
{
	return &pcamera->vecView;
}

void SetCameraVecUp(CAMERA* pcamera, glm::vec3 vecUp)
{
	pcamera->vecUp = vecUp;
}

void* GetCameraVecUp(CAMERA* pcamera)
{
	return &pcamera->vecUp;
}

int GetCameraSize()
{
	return sizeof(CAMERA);
}

void CloneCamera(CAMERA* pcamera, CAMERA* pcameraBase)
{
	CloneAlo(pcamera, pcameraBase);

	pcamera->oidTarget = pcameraBase->oidTarget;
	pcamera->ppntTarget = pcameraBase->ppntTarget;
	pcamera->paloTarget = pcameraBase->paloTarget;
	pcamera->posEye = pcameraBase->posEye;
	pcamera->vecView = pcameraBase->vecView;
	pcamera->vecUp = pcameraBase->vecUp;
	pcamera->fSetCplcy = pcameraBase->fSetCplcy;
}

void PostCameraLoad(CAMERA* pcamera)
{
	PostAloLoad(pcamera);

	if (pcamera->oidTarget == OID_Nil)
		return;

	ALO *paloTarget = (ALO*)PloFindSwObject(pcamera->psw, 0x104, (OID)pcamera->oidTarget, pcamera);

	if (paloTarget == nullptr)
		return;

	if (paloTarget->pvtlo->grfcid & 1)
		pcamera->paloTarget = paloTarget;
	else if (FIsBasicDerivedFrom((BASIC*)paloTarget, CID_PNT))
		pcamera->ppntTarget = (PNT*)paloTarget;
}

void EnableCamera(CAMERA* pcamera)
{
	if (!pcamera->fSetCplcy) 
	{
		SetCmPolicy(g_pcm, CPP_Animated, &g_pcm->cpaseg, nullptr, pcamera);
		pcamera->fSetCplcy = true;
	}
}

void DisableCamera(CAMERA* pcamera)
{
	if (pcamera->fSetCplcy) 
	{
		RevokeCmPolicy(g_pcm, 11, CPP_Animated, &g_pcm->cpaseg, nullptr, pcamera);
		pcamera->fSetCplcy = false;
	}
}

void DeleteCamera(CAMERA *pcamera)
{
	delete pcamera;
}

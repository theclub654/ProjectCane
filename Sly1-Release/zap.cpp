#include "zap.h"
#include "jt.h"
#include "rat.h"

TZP* NewTzp()
{
	return new TZP{};
}

void InitTzp(TZP* ptzp)
{
	InitSo(ptzp);
	InitZpd(&ptzp->zpd, ptzp);

	ptzp->zok = ZOK_Zap;
	ptzp->egk = EGK_No;
}

void* GetTzpZpk(TZP* ptzp)
{
	return &ptzp->zpd.zpk;
}

void SetTzpZpk(TZP* ptzp, ZPK zpk)
{
	ptzp->zpd.zpk = zpk;
}

void* GetTzpDzThrow(TZP* ptzp)
{
	return &ptzp->zpd.dzThrow;
}

void SetTzpDzThrow(TZP* ptzp, float dzThrow)
{
	ptzp->zpd.dzThrow = dzThrow;
}

void* GetTzpFThrowCut(TZP* ptzp)
{
	return &ptzp->zpd.fThrowCut;
}

void SetTzpFThrowCut(TZP* ptzp, int fThrowCut)
{
	ptzp->zpd.fThrowCut = fThrowCut;
}

int GetTzpSize()
{
	return sizeof(TZP);
}

void CloneTzp(TZP* ptzp, TZP* ptzpBase)
{
	CloneSo(ptzp, ptzpBase);

	ptzp->zpd = ptzpBase->zpd;

	// Clone array of RZPR elements
	ptzp->crzpr = ptzpBase->crzpr;
	for (int i = 0; i < ptzp->crzpr; ++i)
		ptzp->arzpr[i] = ptzpBase->arzpr[i];

	ptzp->zpd.pso = ptzp;
}

void PostTzpLoad(TZP* ptzp)
{
	PostAloLoad(ptzp);
	PostZpdLoad(&ptzp->zpd);
}

void UpdateTzp(TZP* ptzp, float dt)
{
	UpdateSo(ptzp, dt);

	ptzp->crzpr = 0;

	SO* root = (SO*)ptzp->paloRoot;

	for (OX* ox = root->poxa->pox; ox != nullptr; ox = ox->poxNext) 
	{
		for (XP* xp = ox->pxp; xp != nullptr; xp = xp->pxpNext) 
		{
			int tzpSide = root != xp->axpd[0].psoRoot;

			if (!FFindLoParent(xp->axpd[tzpSide].psoLeaf, ptzp))
				continue;

			int ratSide = 1 - tzpSide;
			RAT* rat = (RAT*)xp->axpd[ratSide].psoLeaf;

			if (!FIsBasicDerivedFrom((BASIC*)rat, CID_RAT))
				continue;

			if (ptzp->crzpr >= 10 || rat->rats == RATS_Dead)
				continue;

			RZPR& contact = ptzp->arzpr[ptzp->crzpr++];

			contact.prat = rat;
			contact.pos = xp->axpd[ratSide].pos;
			contact.normal = xp->normal;
		}
	}

	for (int i = 0; i < ptzp->crzpr; ++i) 
	{
		RAT* rat = ptzp->arzpr[i].prat;
		WKR wkr{};

		wkr.pos = rat->xf.posWorld;
		wkr.v = rat->xf.v;
		wkr.ploSource = (LO*)ptzp;
		wkr.ploTarget = (LO*)rat;

		PaloAbsorbWkr(&wkr, 0, nullptr);
	}
}

void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro)
{
	RenderAloAll(ptzp, pcm, pro);
}

int FInflictTzpZap(TZP* ptzp, XP* pxp, ZPR* pzpr)
{
	InflictZpdZap(&ptzp->zpd, pxp, pzpr);
	return 1;
}

void* PzpdEnsureTzp(TZP* ptzp, ENSK ensk)
{
	return &ptzp->zpd;
}

void ResetTzpThrowCount(TZP* ptzp)
{
	ptzp->zpd.cploThrow = 0;
}

void DeleteTzp(TZP *ptzp)
{
	delete ptzp;
}

VOLZP* NewVolzp()
{
	return new VOLZP{};
}

void InitVolzp(VOLZP* pvolzp)
{
	InitTzp(pvolzp);
	pvolzp->fNoXpsSelf = true;
	SetSoConstraints(pvolzp, CT_Locked, nullptr, CT_Locked, nullptr);
}

int GetVolzpSize()
{
	return sizeof(VOLZP);
}

void CloneVolzp(VOLZP* pvolzp, VOLZP* pvolzpBase)
{
	CloneTzp(pvolzp, pvolzpBase);
}

void UpdateVolzp(VOLZP* pvolzp, float dt)
{
	UpdateSo(pvolzp, dt);

	PO* player = PpoCur();

	if (player == nullptr)
		return;

	if (pvolzp->zok != ZOK_Zap && pvolzp->zok != ZOK_ZapAndIgnore)
		return;

	if (player->pvtpo->pfnFInvulnerablePo(player, pvolzp->zpd.zpk))
		return;

	glm::vec3 distance = glm::vec3(pvolzp->xf.posWorld) - glm::vec3(player->xf.posWorld);
	float combinedRadius = pvolzp->sRadiusSelf + player->sRadiusSelf;

	if (glm::dot(distance, distance) > combinedRadius * combinedRadius)
		return;

	XP* pxp = nullptr;

	if (pvolzp->zpd.zpk == ZPK_Pit)
	{
		if (player->xf.v.z > 0.0f)
			return;

		if (PbspPointInBspQuick(&player->xf.posWorld, pvolzp->bspc.absp.data()) == nullptr)
			return;

		if (player->fSphere)
		{
			SBI sbi{};

			if (!CsbiIntersectSphereBsp(&player->xf.posWorld, player->sRadiusSelf, pvolzp->bspc.cbspFull, pvolzp->bspc.absp.data(), nullptr, &pvolzp->geomWorld, 1, &sbi))
				return;
		}
		else
		{
			for (int i = 0; i < player->geomWorld.cedge; ++i)
			{
				EDGE& edge = player->geomWorld.aedge[i];
				glm::vec3* positions = player->geomWorld.apos.data();
				LSG lsg{};

				glm::vec3* start = positions + static_cast<short>(edge.aipos[0]);
				glm::vec3* end = positions + static_cast<short>(edge.aipos[1]);

				if (!ClsgClipEdgeToBsp(pvolzp->bspc.absp.data(), start, end, nullptr, 1, &lsg))
					return;

				if (lsg.data.bsp.apsurf[0] != nullptr || lsg.data.bsp.apsurf[1] != nullptr)
					return;
			}
		}
	}
	else
	{
		if (player->fSphere && pvolzp->fSphere)
			MarkSoContactsSphereSphere(player, pvolzp, &pxp);
		else if (player->fSphere)
			MarkSoContactsSphereBsp(player, &player->xf.posWorld, player->sRadiusSelf, pvolzp, pvolzp->bspc.cbspFull, pvolzp->bspc.absp.data(), nullptr, &pxp);
		else if (pvolzp->fSphere)
			MarkSoContactsSphereBsp(pvolzp, &pvolzp->xf.posWorld, pvolzp->sRadiusSelf, player, player->bspc.cbspFull, player->bspc.absp.data(), nullptr, &pxp);
		else if (pvolzp->bspc.absp.size() != 0)
			MarkSoContactsBspBsp(player, pvolzp, pvolzp->bspc.cbspFull, pvolzp->bspc.absp.data(), pvolzp->bspc.absp.data(), &pxp);

		if (pxp == nullptr)
			return;
	}

	ZPR zpr{};

	InitZpr(&zpr, pvolzp->zpd.zpk, pvolzp);
	InflictZpdZap(&pvolzp->zpd, pxp, &zpr);
	player->pvtpo->pfnFTakePoDamage(player, &zpr);
	FreeSwXpList(pvolzp->psw, pxp);
}

void DeleteVolzp(VOLZP *pvolzp)
{
	delete pvolzp;
}

void InitZpd(ZPD* pzpd, SO* pso)
{
	pzpd->pso = pso;
	pzpd->dzThrow = 150.0;
	pzpd->zpk = ZPK_Blunt;
}

void PostZpdLoad(ZPD* pzpd)
{
	SO* pso = pzpd->pso;

	DLI dli;
	dli.m_pdl = &pso->dlChild;
	dli.m_ibDle = pso->dlChild.ibDle;
	dli.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &dli;

	LO* plo = pso->dlChild.ploFirst;

	while (plo != nullptr) {
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(plo) + dli.m_ibDle);

		CID cid = plo->pvtbasic->cid;

		if ((cid == CID_PNT || cid == CID_SHAPE) && pzpd->cploThrow < 32) {
			pzpd->aploThrow[pzpd->cploThrow] = plo;
			++pzpd->cploThrow;
		}

		plo = *reinterpret_cast<LO**>(dli.m_ppv);
	}

	for (int i = 0; i < pzpd->coidThrow; ++i) {
		LO* ploThrow = PloFindSwObject(pso->psw, 260, pzpd->aoidThrow[i], reinterpret_cast<LO*>(pso));

		if (ploThrow != nullptr)
			AddZpdZapLo(pzpd, ploThrow);
	}
	
	s_pdliFirst = dli.m_pdliNext;
}

void AddZpdZapLo(ZPD* pzpd, LO* plo)
{
	if (plo == nullptr || pzpd->cploThrow >= 32)
		return;

	bool fValid =
		FIsBasicDerivedFrom(plo, CID_PNT) ||
		FIsBasicDerivedFrom(plo, CID_SHAPE) ||
		FIsBasicDerivedFrom(plo, CID_HND) ||
		FIsBasicDerivedFrom(plo, CID_PIPE) ||
		FIsBasicDerivedFrom(plo, CID_RAIL) ||
		FIsBasicDerivedFrom(plo, CID_SPIRE);

	if (!fValid)
		return;

	pzpd->aploThrow[pzpd->cploThrow] = plo;
	++pzpd->cploThrow;
}

void ApplyZpdThrow(ZPD* pzpd, PO* ppo)
{
    if (pzpd == nullptr || ppo == nullptr || !FIsBasicDerivedFrom(ppo, CID_JT))
        return;

    JT* pjt = static_cast<JT*>(ppo);
    std::vector<LO*> jumpObjects;
    jumpObjects.reserve(std::max(pzpd->cploThrow, 0));

    for (int i = 0; i < pzpd->cploThrow; ++i)
    {
        if (pzpd->aploThrow[i] != nullptr)
            jumpObjects.push_back(pzpd->aploThrow[i]);
    }

    JTJUMPTARGET target{};
    float dtJump = 0.0f;
    FindBestJtJumpTarget(pjt, jumpObjects, &target, &dtJump);

    if (pzpd->fThrowCut == 0)
    {
        StartJtZapJumpToTarget(pjt, dtJump, &target);
        return;
    }

    // Retail's cut-throw path evaluates the selected target at time zero
    // before snapping JT there. Using the normal jump duration here projects
    // a moving/parented target forward and causes the visible up/down loop.
    glm::vec3 posGoal{};
    CalculateJtJumpTrajectory(pjt, 0.0f, &target, nullptr, &posGoal, nullptr);
    pjt->pvtalo->pfnTranslateAloToPos(pjt, &posGoal);
    pjt->pvtalo->pfnSetAloVelocityVec(pjt, &g_vecZero);
    pjt->pvtalo->pfnSetAloAngularVelocityVec(pjt, &g_vecZero);

    glm::mat3 matTarget(1.0f);
    LoadRotateMatrixRad(pjt->radTarget, &g_normalZ, &matTarget);
    pjt->pvtalo->pfnRotateAloToMat(pjt, &matTarget);
    SetJtJts(pjt, JTS_Jump, JTBS_Jump_Fall);
}

void InflictZpdZap(ZPD* pzpd, XP* pxp, ZPR* pzpr)
{
	PpoCur();

	pzpr->pv = pzpd;
	pzpr->pfnzap = (PFNZAP)ApplyZpdThrow;
	pzpr->zpk = pzpd->zpk;

	HandleLoSpliceEvent(pzpd->pso, 2, 0, nullptr);
}

void AddZpdZapObject(ZPD* pzpd, OID oid)
{
	if (pzpd->coidThrow >= 32)
		return;

	pzpd->aoidThrow[pzpd->coidThrow++] = oid;
}

void RemoveZpdZapLo(ZPD* pzpd, LO* plo)
{
	for (int i = 0; i < pzpd->cploThrow; ++i)
	{
		if (pzpd->aploThrow[i] != plo)
			continue;

		int moveCount = pzpd->cploThrow - i - 1;
		std::memmove(pzpd->aploThrow + i, pzpd->aploThrow + i + 1, moveCount * sizeof(*pzpd->aploThrow));
		--pzpd->cploThrow;
		return;
	}
}

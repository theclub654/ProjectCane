#include "expl.h"
#include "explg.h"
#include "expls.h"

namespace
{
	bool FExplOwnedBy(const LO* plo, const ALO* paloOwner)
	{
		for (const ALO* palo = plo != nullptr ? plo->paloParent : nullptr;
			palo != nullptr; palo = palo->paloParent)
		{
			if (palo == paloOwner)
				return true;
		}

		return false;
	}

	bool FExplAlreadyFound(LO** aplo, int cplo, const LO* plo)
	{
		for (int i = 0; i < cplo; ++i)
		{
			if (aplo[i] == plo)
				return true;
		}

		return false;
	}

	void FindExplDescendants(ALO* palo, LO** aplo, int cploMax, int* pcplo)
	{
		if (palo == nullptr || aplo == nullptr || pcplo == nullptr || *pcplo >= cploMax)
			return;

		for (LO* plo = palo->dlChild.ploFirst; plo != nullptr && *pcplo < cploMax; plo = plo->dleChild.ploNext)
		{
			if (FIsBasicDerivedFrom(plo, CID_EXPL) != 0)
			{
				aplo[(*pcplo)++] = plo;
				continue;
			}

			// grfcid bit 0x100 identifies objects with an ALO child hierarchy.
			if (plo->pvtlo != nullptr && (plo->pvtlo->grfcid & 0x100U) != 0)
				FindExplDescendants(static_cast<ALO*>(plo), aplo, cploMax, pcplo);
		}
	}

	void FindDetachedExplDescendants(ALO* palo, LO** aplo, int cploMax, int* pcplo)
	{
		if (palo == nullptr || palo->psw == nullptr || aplo == nullptr || pcplo == nullptr)
			return;

		// Removed explosion objects remain in the per-class lists and retain
		// their paloParent relationship even though they no longer appear in
		// dlChild. Search every derived class instead of assuming that all EXPL
		// classes occupy an uninterrupted enum range.
		const int cidCount = static_cast<int>(std::size(palo->psw->aploCidHead));

		for (int cidValue = 0; cidValue < cidCount && *pcplo < cploMax; ++cidValue)
		{
			const CID cid = static_cast<CID>(cidValue);

			if (FIsCidDerivedFrom(cid, CID_EXPL) == 0)
				continue;

			for (LO* plo = palo->psw->aploCidHead[cid];
				plo != nullptr && *pcplo < cploMax; plo = plo->ploCidNext)
			{
				if (FExplOwnedBy(plo, palo) && !FExplAlreadyFound(aplo, *pcplo, plo))
					aplo[(*pcplo)++] = plo;
			}
		}
	}
}

EXPL* NewExpl()
{
	return new EXPL{};
}

int GetExplSize()
{
	return sizeof(EXPL);
}

void CloneExpl(EXPL* pexpl, EXPL* pexplBase)
{
	CloneXfm(pexpl, pexplBase);

	pexpl->pexplgParent = pexplBase->pexplgParent;
}

void PostExplLoad(EXPL* pexpl)
{
	PostLoLoad(pexpl);
	pexpl->pvtlo->pfnRemoveLo(pexpl);
}

void CalculateExplTransform(EXPL* pexpl, glm::vec3* pposLocal, glm::mat3* pmatLocal)
{
	EXPLG* pexplg = static_cast<EXPLG*>(pexpl);

	glm::vec3 pos = pexplg->posLocal;
	glm::mat3 mat = pexpl->matLocal;

	for (EXPLG* pparent = pexplg->pexplgParent; pparent != nullptr; pparent = pparent->pexplgParent)
	{
		pos = pparent->posLocal + pparent->matLocal * pos;
		mat = pparent->matLocal * mat;
	}

	*pposLocal = pos;
	*pmatLocal = mat;
}

void InferExpl(EXPL** ppexpl, ALO* palo)
{
	LO* aplo[32];
	int cplo = CploFindSwObjectsByClass(palo->psw, 514, CID_EXPL, palo, 32, aplo);

	// During BRX loading the authored explosion objects can still live only in
	// the owner's child hierarchy and therefore be absent from the world finder.
	// The original finder included those objects; recover them directly when the
	// converted lookup returns no results.
	if (cplo == 0)
		FindExplDescendants(palo, aplo, static_cast<int>(std::size(aplo)), &cplo);

	if (cplo == 0)
		FindDetachedExplDescendants(palo, aplo, static_cast<int>(std::size(aplo)), &cplo);

	if (cplo == 0) {
		*ppexpl = NULL;
		return;
	}

	if (cplo == 1) {
		*ppexpl = (EXPL*)aplo[0];
		return;
	}

	EXPLG* pexplg = (EXPLG*)PloNew(CID_EXPLG, palo->psw, palo, (OID)514, -1);

	pexplg->cpexpl = cplo;

	if (pexplg->cpexpl > 8)
		pexplg->cpexpl = 8;

	for (int ipexpl = 0; ipexpl < pexplg->cpexpl; ipexpl++)
		pexplg->apexpl[ipexpl] = (EXPL*)aplo[ipexpl];

	for (int iplo = 0; iplo < cplo; iplo++)
	{
		EXPL* pexpl = (EXPL*)aplo[iplo];

		if (iplo < pexplg->cpexpl)
			pexpl->pexplgParent = pexplg;

		pexpl->pvtlo->pfnRemoveLo(pexpl);
	}

	*ppexpl = (EXPL*)pexplg;
}

void FireSwTimedExplodeStyles(SW* psw)
{
	if (!psw)
		return;

	auto& owners = psw->explsteOwners;

	for (auto it = owners.begin(); it != owners.end();)
	{
		EXPLSTE* pexplste = it->get();

		if (pexplste && pexplste->tEmit <= g_clock.t)
		{
			FireExplsExplso(pexplste->pexpls, &pexplste->explso);
			RemoveDlEntry(&psw->dlExplste, pexplste);
			it = owners.erase(it);
		}
		else
			++it;
	}
}

void FireExplsExplso(EXPLS* pexpls, EXPLSO* pexplso)
{
    if (!pexpls || !pexpls->pemitb || !pexplso) {
        return;
    }

    EMITB emitb = *pexpls->pemitb;
    EMITOLXF emitolxf{};

    int cParticle = 0;

    if (pexplso->grfexplso & 128)
        cParticle = pexplso->cParticle;
    else
        cParticle = static_cast<int>(GRandInRange(pexpls->lmcParticle.gMin, pexpls->lmcParticle.gMax));

    if (pexplso->grfexplso & 1)
        emitb.emito.paloReference = pexplso->paloReference;

    if (pexplso->grfexplso & 2)
        emitb.emito.vec = pexplso->vec;

    if (pexplso->grfexplso & 4)
    {
        emitb.emito.emitok = EMITOK_Point;
        emitb.emito.posOrigin = pexplso->posOrigin;
        emitb.emito.pemitolxf = nullptr;
    }
    else if (pexplso->grfexplso & 64)
    {
        emitb.emito.paloReference = pexpls->paloParent;
        emitb.emito.pemitolxf = pexplso->pemitolxf;
    }
    else
    {
        CalculateExplTransform(pexpls, &emitolxf.posLocal, &emitolxf.matLocal);

        emitb.emito.pemitolxf = &emitolxf;

        if (!(pexplso->grfexplso & 1))
            emitb.emito.paloReference = pexpls->paloParent;
    }

    if ((pexplso->grfexplso & 8) &&
        emitb.emitp.emitpk == EMITPK_Blip)
    {
        float scale = pexplso->rScale;

        if (scale == 0.0f)
            scale = pexplso->sRadius * 0.01f;

        const float scaledCount =
            scale < 1.0f
            ? static_cast<float>(cParticle) * scale
            : static_cast<float>(cParticle) / scale;

        const int minimumCount =
            static_cast<int>(pexpls->lmcParticle.gMax) / 2;

        cParticle = std::max(static_cast<int>(scaledCount), minimumCount);

        const float velocityScale = GLimitLm(&s_lmrSv, scale);

        emitb.emitv.lmSv.gMin *= velocityScale;
        emitb.emitv.lmSv.gMax *= velocityScale;

        const float blipScale = std::max(scale, 0.5f);

        emitb.emitp.emitblip.clqScale.g0 *= blipScale;
        emitb.emitp.emitblip.clqScale.g1 *= blipScale;
        emitb.emitp.emitblip.clqScale.g2 *= blipScale;
    }

    if (emitb.emitp.emitpk == EMITPK_Rip)
    {
        if (pexplso->grfexplso & 16)
        {
            emitb.emitp.emitrip.lmGScale.gMin = pexplso->rScale;
            emitb.emitp.emitrip.lmGScale.gMax = pexplso->rScale;
        }

        if (pexplso->grfexplso & 32)
            emitb.emitp.emitrip.psoTouch = pexplso->psoTouch;
    }

    EMITG emitg{};
    EMITG* pemitg = nullptr;

    if (pexpls->fGrouped)
    {
        emitg.ppripg = &pexpls->pripg;
        emitg.ppblipg = &pexpls->pblipg;
        emitg.ploSubscribe = pexpls;
        pemitg = &emitg;
    }

    EmitParticles(cParticle, &emitb, pemitg);

    if (!pexpls->psfx)
        return;

    glm::vec3 pos(0.0f);

    switch (emitb.emito.emitok)
    {
        case EMITOK_Point:
        pos = emitb.emito.posOrigin;
        break;

        case EMITOK_Box:
        pos = (emitb.emito.posOrigin + emitb.emito.boxOrigin.posMax) * 0.5f;
        break;

        case EMITOK_Curve:
        if (emitb.emito.emitcrvOrigin.pcrv && !emitb.emito.emitcrvOrigin.pcrv->mpicvpos.empty())
            pos = emitb.emito.emitcrvOrigin.pcrv->mpicvpos.front();
        break;

        case EMITOK_Skeleton:
        pos = glm::vec3(0.0f);
        break;

        case EMITOK_Mesh:
        pos = emitb.emito.emitmeshOrigin.posCenter;
        break;

        default:
        break;
    }

    if (emitb.emito.pemitolxf)
    {
        pos =
            emitb.emito.pemitolxf->matLocal * pos +
            emitb.emito.pemitolxf->posLocal;
    }

    ConvertAloPos(emitb.emito.paloReference, nullptr, &pos, &pos);

    SFX* psfx = pexpls->psfx.get();

    StartSound(psfx->sfxid, nullptr, nullptr, &pos, psfx->sStart, psfx->sFull, psfx->uVol, psfx->uPitch,
        0.0f,
        nullptr,
        nullptr
    );
}

void ExplodeExpl(EXPL* pexpl)
{
	EXPLSO explso{};
	pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
}

void ExplodeExplParams(EXPL* pexpl, GRFEXPLSO grfexplso, ALO* paloReference, const glm::vec3* pvec, const glm::vec3* pposOrigin, float rScale, float sRadius)
{
	EXPLSO explso{};

	explso.grfexplso = grfexplso;
	explso.paloReference = paloReference;
	explso.vec = *pvec;
	explso.posOrigin = *pposOrigin;
	explso.rScale = rScale;
	explso.sRadius = sRadius;

	pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
}

void DeleteExpl(EXPL* pexpl)
{
	delete pexpl;
}

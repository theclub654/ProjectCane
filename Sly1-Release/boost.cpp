#include "boost.h"
#include "suv.h"

BOOST* NewBoost()
{
	return new BOOST{};
}

void CloneBoost(BOOST* pboost, BOOST* pboostBase)
{
	CloneDprize(pboost, pboostBase);
}

void InitBoost(BOOST* pboost)
{
	InitDprize(pboost);
	pboost->fCollectible = 0;
}

void UpdateBoost(BOOST* pboost, float dt)
{
	UpdateDprize(pboost, dt);

	glm::vec3 posCollect = pboost->xf.posWorld + s_dposBoostCollect;

	SUV* apsuv[6]{};

	int cpo = CploFindSwObjectsByClass(pboost->psw, 5, CID_SUV, 0, 6, (LO**)apsuv);

	for (int ipo = 0; ipo < cpo; ipo++)
	{
		SUV* psuv = apsuv[ipo];

		glm::vec3 dpos = psuv->xf.posWorld - posCollect;

		// Broad-phase check: within 500 units.
		if (glm::dot(dpos, dpos) > 250000.0f)
			continue;

		SBI asbi[1]{};

		if (CsbiIntersectSphereBsp(&posCollect, 100.0f, psuv->bspc.cbspFull, psuv->bspc.absp.data(), nullptr, &psuv->geomWorld, 1, asbi) <= 0)
			continue;

		if (psuv == (SUV*)PpoCur())
			psuv->cBoost = std::min(psuv->cBoost + 1, 99);

		glm::vec3 angularVelocity = psuv->xf.v * s_sBoostCollectAngularVelocity;
		angularVelocity.z = std::max(angularVelocity.z, 0.0f);
		angularVelocity.z = std::max(angularVelocity.z, 0.0f);

		pboost->pvtalo->pfnSetAloAngularVelocityVec(pboost, &angularVelocity);

		if (pboost->pexplCollect != nullptr)
		{
			EXPLSO explso{};
            explso.grfexplso = 1;
            explso.paloReference = pboost;

			pboost->pexplCollect->pvtexpl->pfnExplodeExplExplso((EXPLO*)pboost->pexplCollect, &explso);
		}

		StartSound((SFXID)44, nullptr, nullptr, &pboost->xf.posWorld, 7500.0f, 2500.0f, 2.0f, s_frqBoostCollect, 0.0f, nullptr, nullptr);

		pboost->pvtdprize->pfnSetDprizeDprizes(pboost, DPRIZES_Removed);
		return;
	}
}

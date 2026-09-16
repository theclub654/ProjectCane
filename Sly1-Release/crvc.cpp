#include "crvc.h"

std::shared_ptr <CRVC> NewCrvc()
{
	std::shared_ptr <CRVC> pcrv;
	CRVC crvc{};
	crvc.pvtcrvc = &g_vtcrvc;
	pcrv = std::make_shared <CRVC>(crvc);
	pcrv->crvk = CRVK_Cubic;
	return pcrv;
}

void LoadCrvcFromBrx(CRVC* pcrvc, CBinaryInputStream* pbis)
{
	pcrvc->fClosed = pbis->U8Read();
	pcrvc->ccv = pbis->U8Read();

	pcrvc->mpicvu.resize(pcrvc->ccv);
	pcrvc->mpicvs.resize(pcrvc->ccv);
	pcrvc->mpicvpos.resize(pcrvc->ccv);
	pcrvc->mpicvdposIn.resize(pcrvc->ccv);
	pcrvc->mpicvdposOut.resize(pcrvc->ccv);


	for (int i = 0; i < pcrvc->ccv; i++)
	{
		pcrvc->mpicvu[i] = pbis->F32Read();
		pcrvc->mpicvpos[i] = pbis->ReadVector();
		pcrvc->mpicvdposIn[i] = pbis->ReadVector();
		pcrvc->mpicvdposOut[i] = pbis->ReadVector();
	}

	pcrvc->pvtcrvc->pfnMeasureCrvc(pcrvc);

	InvalidateCrvcCache(pcrvc);
}

void EvaluateCrvcFromU(CRVC* pcrvc, float u, glm::vec3* ppos, glm::vec3* pnormTangent)
{
	float du;
	float duSeg;

	const int icv = IcvFindCrvU(pcrvc, u, &du, &duSeg);

	EvaluateBezierPos(duSeg, du, 1.0f, &pcrvc->mpicvpos[icv], &pcrvc->mpicvdposOut[icv], &pcrvc->mpicvpos[icv + 1], &pcrvc->mpicvdposIn[icv + 1], ppos, pnormTangent, nullptr);

	if (pnormTangent != nullptr)
	{
		const float length = glm::length(*pnormTangent);

		if (length < 0.0001f)
			*pnormTangent = g_normalX;
		else
			*pnormTangent /= length;
	}
}

void EvaluateCrvcFromS(CRVC* pcrvc, float s, glm::vec3* ppos, glm::vec3* pnormTangent)
{
	const float u = pcrvc->pvtcrv->pfnUFromCrvS(pcrvc, s);

	if (pcrvc->pvtcrv->pfnEvaluateCrvFromU != nullptr)
		pcrvc->pvtcrv->pfnEvaluateCrvFromU(pcrvc, u, ppos, pnormTangent);
}

void ConvertCrvc(CRVC* pcrvc, glm::mat4& pmatSrc, glm::mat4& pmatDst)
{
	glm::mat4 dmat4;

	CalculateDmat4(pmatDst, pmatSrc, dmat4);

	const glm::mat3 rot = glm::mat3(dmat4);

	for (int i = 0; i < pcrvc->ccv; ++i)
	{
		// Position gets full 4x4 transform, w = 1
		pcrvc->mpicvpos[i] = glm::vec3(dmat4 * glm::vec4(pcrvc->mpicvpos[i], 1.0f));

		// Handles/deltas get rotation/scale only, no translation
		pcrvc->mpicvdposIn[i] = rot * pcrvc->mpicvdposIn[i];
		pcrvc->mpicvdposOut[i] = rot * pcrvc->mpicvdposOut[i];
	}

	InvalidateCrvcCache(pcrvc);
}

float SFromCrvcU(CRVC* pcrvc, float u)
{
	float du;
	float duSeg;

	const int icv = IcvFindCrvU(pcrvc, u, &du, &duSeg);
	const float sSegment = SBezierPosLength(duSeg, du, &pcrvc->mpicvpos[icv], &pcrvc->mpicvdposOut[icv], &pcrvc->mpicvpos[icv + 1], &pcrvc->mpicvdposIn[icv + 1]);

	return pcrvc->mpicvs[icv] + sSegment;
}

float UFromCrvcS(CRVC* pcrvc, float s)
{
	float ds;
	float dsTess;
	float dsSegTess;

	const int icv = IcvFindCrvS(pcrvc, s, &ds, nullptr);

	FillCrvcCache(pcrvc, icv);

	const int ipos = IposFindAposG(ds, 20, pcrvc->ctce.mpiposs, 0, &dsTess, &dsSegTess);
	const float uTess = (static_cast<float>(ipos) + dsTess / dsSegTess) / 19.0f;

	return (1.0f - uTess) * pcrvc->mpicvu[icv] + uTess * pcrvc->mpicvu[icv + 1];
}

void MeasureCrvc(CRVC* pcrvc)
{
	pcrvc->mpicvs[0] = 0.0f;

	int cseg = pcrvc->ccv - 1;

	for (int i = 0; i < cseg; i++)
	{
		float dtSeg = pcrvc->mpicvu[i + 1] - pcrvc->mpicvu[i];
		float length = SBezierPosLength(dtSeg, dtSeg, &pcrvc->mpicvpos[i], &pcrvc->mpicvdposOut[i], &pcrvc->mpicvpos[i + 1], &pcrvc->mpicvdposIn[i + 1]);

		pcrvc->mpicvs[i + 1] = pcrvc->mpicvs[i] + length;
	}
}

void FindCrvcClosestPointAll(CRVC* pcrvc, glm::vec3* ppos, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps)
{
	if (pcrvc->ccv <= 0)
		return;

	int icvBest = 0;
	float distanceSquaredBest = FLT_MAX;

	for (int icv = pcrvc->ccv - 1; icv >= 0; --icv)
	{
		glm::vec3 dpos = *ppos - pcrvc->mpicvpos[icv];

		if (pconstr != nullptr)
			ApplyConstr(pconstr, &dpos, &dpos, nullptr);

		const float distanceSquared = glm::length2(dpos);

		if (distanceSquared < distanceSquaredBest)
		{
			distanceSquaredBest = distanceSquared;
			icvBest = icv;
		}
	}

	FindCrvcClosestPointFromU(pcrvc, ppos, pcrvc->mpicvu[icvBest], pconstr, pposClosest, pnormTangentClosest, pu, ps);
}

void FindCrvcClosestPointFromU(CRVC* pcrvc, glm::vec3* ppos, float u, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps)
{
	const float uFirst = pcrvc->mpicvu[0];
	const float duInitial = (pcrvc->mpicvu[1] - uFirst) * 0.1f;
	const float uMin = pcrvc->fClosed ? -FLT_MAX : uFirst;
	const float uMax = pcrvc->fClosed ? FLT_MAX : pcrvc->mpicvu[pcrvc->ccv - 1];

	CRVMC crvmc{ pcrvc, ppos, pconstr };

	float uBest;
	MinimizeRange((PFNGG)GMeasureCrvU, &crvmc, u, duInitial, uMin, uMax, &uBest, nullptr);

	if ((pposClosest != nullptr || pnormTangentClosest != nullptr) && pcrvc->pvtcrv->pfnEvaluateCrvFromU != nullptr)
		pcrvc->pvtcrv->pfnEvaluateCrvFromU(pcrvc, uBest, pposClosest, pnormTangentClosest);

	if (pu != nullptr)
		*pu = uBest;

	if (ps != nullptr)
		*ps = pcrvc->pvtcrv->pfnSFromCrvU(pcrvc, uBest);
}

void FindCrvcClosestPointFromS(CRVC* pcrvc, glm::vec3* ppos, float s, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps)
{
	const float u = pcrvc->pvtcrv->pfnUFromCrvS(pcrvc, s);

	if (pcrvc->pvtcrv->pfnFindCrvClosestPointFromU != nullptr)
		pcrvc->pvtcrv->pfnFindCrvClosestPointFromU(pcrvc, ppos, u, pconstr, pposClosest, pnormTangentClosest, pu, ps);
}

void InvalidateCrvcCache(CRVC* pcrvc)
{
	pcrvc->icvCache = -1;
}

void FillCrvcCache(CRVC* pcrvc, int icv)
{
	if (icv == pcrvc->icvCache)
		return;

	const float duSeg = pcrvc->mpicvu[icv + 1] - pcrvc->mpicvu[icv];

	TesselateBezier(duSeg, 0.0f, duSeg, &pcrvc->mpicvpos[icv], &pcrvc->mpicvdposOut[icv], &pcrvc->mpicvpos[icv + 1], &pcrvc->mpicvdposIn[icv + 1], 20, pcrvc->ctce.apos);
	SMeasureApos(20, pcrvc->ctce.apos, pcrvc->ctce.mpiposs);

	pcrvc->icvCache = icv;
}

VTCRVC g_vtcrvc;

#include "crv.h"
#include "crvl.h"
#include "crvc.h"
#include "mark.h"

std::shared_ptr <CRV> PcrvNew(CRVK crvk)
{
	switch (crvk)
	{
		case CRVK_Linear:
		return NewCrvl();
		break;

		case CRVK_Cubic:
		return NewCrvc();
		break;
	}

	return nullptr;
}

void ConvertApos(int cpos, glm::vec3* apos, glm::mat4& pmatSrc, glm::mat4& pmatDst)
{
	glm::mat4 dmat4;

	CalculateDmat4(pmatDst, pmatSrc, dmat4);

	for (int i = 0; i < cpos; ++i)
	{
		const glm::vec4 pos = dmat4 * glm::vec4(apos[i], 1.0f);
		apos[i] = glm::vec3(pos);
	}
}

float SMeasureApos(int cpos, glm::vec3 *apos, float *mpiposs)
{
	float sTotal = 0.0f;

	if (mpiposs != nullptr)
		mpiposs[0] = 0.0f;

	if (cpos <= 1)
		return 0.0f;

	for (int i = 1; i < cpos; ++i)
	{
		glm::vec3 dpos = apos[i] - apos[i - 1];

		sTotal += glm::length(dpos);

		if (mpiposs != nullptr)
			mpiposs[i] = sTotal;
	}

	return sTotal;
}

int IposFindAposG(float g, int cpos, float* mpiposg, int fClosed, float* pdg, float* pdgSeg)
{
	g = GWrapApos(g, cpos, mpiposg, fClosed);

	int ipos = 1;

	while (ipos < cpos && mpiposg[ipos] <= g)
		ipos++;

	if (ipos >= cpos)
		ipos = cpos - 1;

	ipos--;

	if (pdg)
		*pdg = g - mpiposg[ipos];

	if (pdgSeg)
		*pdgSeg = mpiposg[ipos + 1] - mpiposg[ipos];

	return ipos;
}

float GWrapApos(float g, int cpos, float* mpiposg, int fClosed)
{
	if (fClosed && mpiposg != nullptr && cpos > 1)
	{
		float length = mpiposg[cpos - 1] - mpiposg[0];

		// A closed curve with no measured span cannot be wrapped: adding or
		// subtracting zero would leave the loops below running forever.
		if (!std::isfinite(length) || length <= 0.000001f)
			return mpiposg[0];

		while (g < mpiposg[0])
			g += length;

		while (g > mpiposg[cpos - 1])
			g -= length;
	}

	return g;
}

float SMeasureCrvSegmentU(void* pv, float u)
{
	CRVMS* pcrvms = static_cast<CRVMS*>(pv);

	glm::vec3 pos(0.0f);

	if (pcrvms->pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
		pcrvms->pcrv->pvtcrv->pfnEvaluateCrvFromU(pcrvms->pcrv, u, &pos, nullptr);

	float uSegment = 0.0f;
	float distance = 0.0f;

	FindClosestPointOnLineSegment(&pos, pcrvms->ppos0, pcrvms->ppos1, &uSegment, &distance);

	return distance;
}

float DuGetCrvSearchIncrement(CRV* pcrv)
{
	return (pcrv->mpicvu[1] - pcrv->mpicvu[0]) * 0.1f;
}

void FindCrvClosestPointOnLineSegmentFromU(CRV* pcrv, glm::vec3* ppos0, glm::vec3* ppos1, float u, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps)
{
	const float duInitial = pcrv->mpicvu[1] - pcrv->mpicvu[0];
	const float uMin = pcrv->fClosed ? -FLT_MAX : pcrv->mpicvu.front();
	const float uMax = pcrv->fClosed ? FLT_MAX : pcrv->mpicvu[pcrv->ccv - 1];

	CRVMS crvms{};
	crvms.pcrv = pcrv;
	crvms.ppos0 = ppos0;
	crvms.ppos1 = ppos1;

	float uBest = u;
	MinimizeRange(SMeasureCrvSegmentU, &crvms, u, duInitial * 0.1f, uMin, uMax, &uBest, nullptr);

	if ((pposClosest != nullptr || pnormTangentClosest != nullptr) && pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
		pcrv->pvtcrv->pfnEvaluateCrvFromU(pcrv, uBest, pposClosest, pnormTangentClosest);

	if (pu != nullptr)
		*pu = uBest;

	if (ps != nullptr)
		*ps = pcrv->pvtcrv->pfnSFromCrvU(pcrv, uBest);
}

float SFromCrvU(CRV* pcrv, float u)
{
	return 0.0f;
}

float UFromCrvS(CRV* pcrv, float s)
{
	return 0.0f;
}

float UMaxCrv(CRV* pcrv)
{
	return pcrv->mpicvu.empty() ? 0.0f : pcrv->mpicvu.back();
}

float SMaxCrv(CRV* pcrv)
{
	return pcrv->mpicvs.empty() ? 0.0f : pcrv->mpicvs.back();
}

int IcvFindCrvU(CRV* pcrv, float u, float* du, float* duSeg)
{
	return IposFindAposG(u, pcrv->ccv, pcrv->mpicvu.data(), pcrv->fClosed, du, duSeg);
}

int IcvFindCrvS(CRV* pcrv, float s, float* ds, float* dsSeg)
{
	return IposFindAposG(s, pcrv->ccv, pcrv->mpicvs.data(), pcrv->fClosed, ds, dsSeg);
}

float GMeasureCrvU(void* pv, float u)
{
	CRVMC* pcrvmc = static_cast<CRVMC*>(pv);

	glm::vec3 pos(0.0f);

	if (pcrvmc->pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
		pcrvmc->pcrv->pvtcrv->pfnEvaluateCrvFromU(pcrvmc->pcrv, u, &pos, nullptr);

	glm::vec3 dpos = pos - *pcrvmc->ppos;

	if (pcrvmc->pconstr != nullptr)
		ApplyConstr(pcrvmc->pconstr, &dpos, &dpos, nullptr);

	return glm::dot(dpos, dpos);
}

void FindAposClosestPointAll(glm::vec3* ppos, CONSTR* pconstr, int cpos, glm::vec3* apos, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, int* pipos, float* pr)
{
	int iposBest = -1;
	float rBest = -1.0f;
	float distanceSqBest = FLT_MAX;

	for (int ipos = 0; ipos < cpos - 1; ++ipos)
	{
		glm::vec3 dposPoint = *ppos - apos[ipos];
		glm::vec3 dposSegment = apos[ipos + 1] - apos[ipos];

		if (pconstr != nullptr)
		{
			ApplyConstr(pconstr, &dposPoint, &dposPoint, nullptr);
			ApplyConstr(pconstr, &dposSegment, &dposSegment, nullptr);
		}

		const float segmentLength = glm::length(dposSegment);

		if (segmentLength < 0.0001f)
			continue;

		const glm::vec3 segmentDirection = dposSegment / segmentLength;
		const float distanceAlongSegment = glm::clamp(glm::dot(dposPoint, segmentDirection), 0.0f, segmentLength);
		const glm::vec3 dposClosest = dposPoint - segmentDirection * distanceAlongSegment;
		const float distanceSq = glm::dot(dposClosest, dposClosest);

		if (distanceSq < distanceSqBest)
		{
			iposBest = ipos;
			rBest = distanceAlongSegment / segmentLength;
			distanceSqBest = distanceSq;
		}
	}

	if (pipos != nullptr)
		*pipos = iposBest;

	if (pr != nullptr)
		*pr = rBest;

	if (iposBest < 0)
	{
		if (pposClosest != nullptr)
			*pposClosest = cpos > 0 ? apos[0] : glm::vec3(0.0f);

		if (pnormTangentClosest != nullptr)
			*pnormTangentClosest = g_normalX;

		return;
	}

	if (pposClosest != nullptr)
		*pposClosest = glm::mix(apos[iposBest], apos[iposBest + 1], rBest);

	if (pnormTangentClosest != nullptr)
	{
		const glm::vec3 tangent = apos[iposBest + 1] - apos[iposBest];
		const float tangentLength = glm::length(tangent);
		*pnormTangentClosest = tangentLength < 0.0001f ? g_normalX : tangent / tangentLength;
	}
}

void FindAposClosestPointSegment(glm::vec3* ppos, CONSTR* pconstr, int cpos, glm::vec3* apos, int iposStart, int fClosed, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, int* pipos, float* pr)
{
	bool searchedBackward = false;
	bool searchedForward = false;
	float r = 0.0f;

	while (true)
	{
		glm::vec3 dposPoint = *ppos - apos[iposStart];
		glm::vec3 dposSegment = apos[iposStart + 1] - apos[iposStart];

		if (pconstr != nullptr)
		{
			ApplyConstr(pconstr, &dposPoint, &dposPoint, nullptr);
			ApplyConstr(pconstr, &dposSegment, &dposSegment, nullptr);
		}

		const float segmentLength = glm::length(dposSegment);

		if (segmentLength < 0.0001f)
		{
			r = 0.0f;
			break;
		}

		const glm::vec3 segmentDirection = dposSegment / segmentLength;
		const float distanceAlongSegment = glm::dot(dposPoint, segmentDirection);

		if (distanceAlongSegment < 0.0f)
		{
			r = 0.0f;

			if (searchedForward)
				break;

			if (iposStart > 0)
			{
				--iposStart;
			}
			else
			{
				if (!fClosed)
					break;

				iposStart = cpos - 2;
			}

			searchedBackward = true;
			continue;
		}

		if (distanceAlongSegment <= segmentLength)
		{
			r = distanceAlongSegment / segmentLength;
			break;
		}

		if (searchedBackward)
		{
			r = 1.0f;
			break;
		}

		if (iposStart < cpos - 2)
		{
			++iposStart;
		}
		else
		{
			if (!fClosed)
			{
				r = 1.0f;
				break;
			}

			iposStart = 0;
		}

		searchedForward = true;
	}

	if (pipos != nullptr)
		*pipos = iposStart;

	if (pr != nullptr)
		*pr = r;

	if (pposClosest != nullptr)
		*pposClosest = glm::mix(apos[iposStart], apos[iposStart + 1], r);

	if (pnormTangentClosest != nullptr)
	{
		const glm::vec3 tangent = apos[iposStart + 1] - apos[iposStart];
		const float tangentLength = glm::length(tangent);
		*pnormTangentClosest = tangentLength < 0.0001f ? g_normalX : tangent / tangentLength;
	}
}

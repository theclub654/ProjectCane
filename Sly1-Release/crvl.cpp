#include "crvl.h"

std::shared_ptr<CRVL> NewCrvl()
{
	std::shared_ptr <CRVL> pcrv;
	CRVL crvc{};
	crvc.pvtcrvl = &g_vtcrvl;
	pcrv = std::make_shared <CRVL>(crvc);
	pcrv->crvk = CRVK_Linear;
	return pcrv;
}

void LoadCrvlFromBrx(CRVL* pcrvl, CBinaryInputStream* pbis)
{
	pcrvl->fClosed = pbis->U8Read();
	pcrvl->ccv = pbis->U8Read();

	pcrvl->mpicvu.resize(pcrvl->ccv);
	pcrvl->mpicvs.resize(pcrvl->ccv);
	pcrvl->mpicvpos.resize(pcrvl->ccv);

	for (int i = 0; i < pcrvl->ccv; i++)
	{
		pcrvl->mpicvu[i] = pbis->F32Read();
		pcrvl->mpicvpos[i] = pbis->ReadVector();
	}

	pcrvl->pvtcrvl->pfnMeasureCrvl(pcrvl);
}

void EvaluateCrvlFromU(CRVL* pcrvl, float u, glm::vec3* ppos, glm::vec3* pnormTangent)
{
	EvaluateAposG(u, pcrvl->ccv, pcrvl->mpicvpos.data(), pcrvl->mpicvu.data(), pcrvl->fClosed, ppos, pnormTangent);
}

void EvaluateCrvlFromS(CRVL* pcrvl, float s, glm::vec3* ppos, glm::vec3* pnormTangent)
{
	EvaluateAposG(s, pcrvl->ccv, pcrvl->mpicvpos.data(), pcrvl->mpicvs.data(), pcrvl->fClosed, ppos, pnormTangent);
}

void ConvertCrvl(CRVL* pcrvl, glm::mat4* pmatSrc, glm::mat4* pmatDst)
{
	ConvertApos(pcrvl->ccv, pcrvl->mpicvpos.data(), *pmatSrc, *pmatDst);
}

float SFromCrvlU(CRVL* pcrvl, float u)
{
	float du;
	float duSeg;

	const int icv = IcvFindCrvU(static_cast<CRV*>(pcrvl), u, &du, &duSeg);
	const float t = du / duSeg;

	return pcrvl->mpicvs[icv] + t * (pcrvl->mpicvs[icv + 1] - pcrvl->mpicvs[icv]);
}

float UFromCrvlS(CRVL* pcrvl, float s)
{
	float ds;
	float dsSeg;

	const int icv = IcvFindCrvS(static_cast<CRV*>(pcrvl), s, &ds, &dsSeg);
	const float t = ds / dsSeg;

	return pcrvl->mpicvu[icv] + t * (pcrvl->mpicvu[icv + 1] - pcrvl->mpicvu[icv]);
}

void MeasureCrvl(CRVL* pcrvl)
{
	SMeasureApos(pcrvl->ccv, pcrvl->mpicvpos.data(), pcrvl->mpicvs.data());
}

void FindCrvlClosestPointAll(CRVL* pcrvl, glm::vec3* ppos, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps)
{
	int icv = 0;
	float r = 0.0f;

	FindAposClosestPointAll(ppos, pconstr, pcrvl->ccv, pcrvl->mpicvpos.data(), pposClosest, pnormTangentClosest, &icv, &r);

	if (ps != nullptr)
		*ps = glm::mix(pcrvl->mpicvs[icv], pcrvl->mpicvs[icv + 1], r);

	if (pu != nullptr)
		*pu = glm::mix(pcrvl->mpicvu[icv], pcrvl->mpicvu[icv + 1], r);
}

void FindCrvlClosestPointFromU(CRVL* pcrvl, glm::vec3* ppos, float u, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps)
{
	const int iposStart = IposFindAposG(u, pcrvl->ccv, pcrvl->mpicvu.data(), pcrvl->fClosed, nullptr, nullptr);

	int icv = 0;
	float r = 0.0f;

	FindAposClosestPointSegment(ppos, pconstr, pcrvl->ccv, pcrvl->mpicvpos.data(), iposStart, pcrvl->fClosed, pposClosest, pnormTangentClosest, &icv, &r);

	if (ps != nullptr)
		*ps = glm::mix(pcrvl->mpicvs[icv], pcrvl->mpicvs[icv + 1], r);

	if (pu != nullptr)
		*pu = glm::mix(pcrvl->mpicvu[icv], pcrvl->mpicvu[icv + 1], r);
}

void FindCrvlClosestPointFromS(CRVL* pcrvl, glm::vec3* ppos, float s, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps)
{
	const int iposStart = IposFindAposG(s, pcrvl->ccv, pcrvl->mpicvs.data(), pcrvl->fClosed, nullptr, nullptr);

	int icv = 0;
	float r = 0.0f;

	FindAposClosestPointSegment(ppos, pconstr, pcrvl->ccv, pcrvl->mpicvpos.data(), iposStart, pcrvl->fClosed, pposClosest, pnormTangentClosest, &icv, &r);

	if (ps != nullptr)
		*ps = glm::mix(pcrvl->mpicvs[icv], pcrvl->mpicvs[icv + 1], r);

	if (pu != nullptr)
		*pu = glm::mix(pcrvl->mpicvu[icv], pcrvl->mpicvu[icv + 1], r);
}

VTCRVL g_vtcrvl;

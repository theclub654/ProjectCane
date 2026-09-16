#pragma once
#include "crv.h"

struct CRVC : public CRV
{
	public:
	std::vector <glm::vec3> mpicvdposIn;
	std::vector <glm::vec3> mpicvdposOut;
	CTCE ctce;
	int icvCache;
};

std::shared_ptr <CRVC> NewCrvc();
void LoadCrvcFromBrx(CRVC* pcrvc, CBinaryInputStream* pbis);
void EvaluateCrvcFromU(CRVC* pcrvc, float u, glm::vec3* ppos, glm::vec3* pnormTangent);
void EvaluateCrvcFromS(CRVC* pcrvc, float s, glm::vec3* ppos, glm::vec3* pnormTangent);
void ConvertCrvc(CRVC* pcrvc, glm::mat4& pmatSrc, glm::mat4& pmatDst);
float SFromCrvcU(CRVC* pcrvc, float u);
float UFromCrvcS(CRVC* pcrvc, float s);
void MeasureCrvc(CRVC* pcrvc);
void FindCrvcClosestPointAll(CRVC* pcrvc, glm::vec3* ppos, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps);
void FindCrvcClosestPointFromU(CRVC* pcrvc, glm::vec3* ppos, float u, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps);
void FindCrvcClosestPointFromS(CRVC* pcrvc, glm::vec3* ppos, float s, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps);

void InvalidateCrvcCache(CRVC* pcrvc);
void FillCrvcCache(CRVC* pcrvc, int icv);

struct VTCRVC
{
	void  (*pfnLoadCrvcFromBrx)(CRVC*, CBinaryInputStream*) = LoadCrvcFromBrx;
	void  (*pfnEvaluateCrvcFromU)(CRVC*, float, glm::vec3*, glm::vec3*) = EvaluateCrvcFromU;
	void  (*pfnEvaluateCrvcFromS)(CRVC*, float, glm::vec3*, glm::vec3*) = EvaluateCrvcFromS;
	void  (*pfnRenderCrvSegment)(CRV*, float, float) = nullptr;
	void  (*pfnConvertCrvc)(CRVC*, glm::mat4&, glm::mat4&) = ConvertCrvc;
	float (*pfnSFromCrvcU)(CRVC*, float) = SFromCrvcU;
	float (*pfnUFromCrvcS)(CRVC*, float) = UFromCrvcS;
	float (*pfnUMaxCrv)(CRV*) = UMaxCrv;
	float (*pfnSMaxCrv)(CRV*) = SMaxCrv;
	void  (*pfnMeasureCrvc)(CRVC*) = MeasureCrvc;
	void  (*pfnFindCrvcClosestPointAll)(CRVC*, glm::vec3*, CONSTR*, glm::vec3*, glm::vec3*, float*, float*) = FindCrvcClosestPointAll;
	void  (*pfnFindCrvcClosestPointFromU)(CRVC*, glm::vec3*, float, CONSTR*, glm::vec3*, glm::vec3*, float*, float*) = FindCrvcClosestPointFromU;
	void  (*pfnFindCrvcClosestPointFromS)(CRVC*, glm::vec3*, float, CONSTR*, glm::vec3*, glm::vec3*, float*, float*) = FindCrvcClosestPointFromS;
};

extern VTCRVC g_vtcrvc;

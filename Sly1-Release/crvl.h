#pragma once
#include "crv.h"

struct CRVL : public CRV
{

};

std::shared_ptr <CRVL> NewCrvl();
void LoadCrvlFromBrx(CRVL* pcrvl, CBinaryInputStream* pbis);
void EvaluateCrvlFromU(CRVL* pcrvl, float u, glm::vec3* ppos, glm::vec3* pnormTangent);
void EvaluateCrvlFromS(CRVL* pcrvl, float s, glm::vec3* ppos, glm::vec3* pnormTangent);
void ConvertCrvl(CRVL* pcrvl, glm::mat4* pmatSrc, glm::mat4* pmatDst);
float SFromCrvlU(CRVL* pcrvl, float u);
float UFromCrvlS(CRVL* pcrvl, float s);
float UMaxCrv(CRV* pcrv);
float SMaxCrv(CRV* pcrv);
void MeasureCrvl(CRVL* pcrvl);
void FindCrvlClosestPointAll(CRVL* pcrvl, glm::vec3* ppos, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps);
void FindCrvlClosestPointFromU(CRVL* pcrvl, glm::vec3* ppos, float u, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps);
void FindCrvlClosestPointFromS(CRVL* pcrvl, glm::vec3* ppos, float s, CONSTR* pconstr, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps);

struct VTCRVL
{
	void  (*pfnLoadCrvlFromBrx)(CRVL*, CBinaryInputStream*) = LoadCrvlFromBrx;
	void  (*pfnEvaluateCrvlFromU)(CRVL*, float, glm::vec3*, glm::vec3*) = EvaluateCrvlFromU;
	void  (*pfnEvaluateCrvlFromS)(CRVL*, float, glm::vec3*, glm::vec3*) = EvaluateCrvlFromS;
	void  (*pfnRenderCrvSegment)(CRV*, float, float) = nullptr;
	void  (*pfnConvertCrvl)(CRVL*, glm::mat4*, glm::mat4*) = ConvertCrvl;
	float (*pfnSFromCrvlU)(CRVL*, float) = SFromCrvlU;
	float (*pfnUFromCrvlS)(CRVL*, float) = UFromCrvlS;
	float (*pfnUMaxCrv)(CRV*) = UMaxCrv;
	float (*pfnSMaxCrv)(CRV*) = SMaxCrv;
	void  (*pfnMeasureCrvl)(CRVL*) = MeasureCrvl;
	void  (*pfnFindCrvlClosestPointAll)(CRVL*, glm::vec3*, CONSTR*, glm::vec3*, glm::vec3*, float*, float*) = FindCrvlClosestPointAll;
	void  (*pfnFindCrvlClosestPointFromU)(CRVL*, glm::vec3*, float, CONSTR*, glm::vec3*, glm::vec3*, float*, float*) = FindCrvlClosestPointFromU;
	void  (*pfnFindCrvlClosestPointFromS)(CRVL*, glm::vec3*, float, CONSTR*, glm::vec3*, glm::vec3*, float*, float*) = FindCrvlClosestPointFromS;
};

extern VTCRVL g_vtcrvl;

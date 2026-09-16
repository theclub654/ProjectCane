#pragma once
#include "lo.h"
#include "bez.h"

enum CRVK
{
	CRVK_Nil = -1,
	CRVK_Linear = 0,
	CRVK_Cubic = 1,
	CRVK_Max = 2
};

struct CTCE
{
	glm::vec3 apos[20];
	float mpiposs[20];
};

struct CRVMC
{
	struct CRV *pcrv;
	glm::vec3 *ppos;
	CONSTR *pconstr;
};

struct CRVMS
{
	class CRV *pcrv;
	glm::vec3 *ppos0;
	glm::vec3 *ppos1;
};

struct CRV
{
	union
	{
		struct VTCRV* pvtcrv;
		struct VTCRVL* pvtcrvl;
		struct VTCRVC* pvtcrvc;
	};

	CRVK crvk;
	int fClosed;
	int ccv;
	std::vector <float> mpicvu;
	std::vector <float> mpicvs;
	std::vector <glm::vec3> mpicvpos;
};

std::shared_ptr <CRV> PcrvNew(CRVK crvk);
void  ConvertApos(int cpos, glm::vec3* apos, glm::mat4& pmatSrc, glm::mat4& pmatDst);
float SMeasureApos(int cpos, glm::vec3* apos, float* mpiposs);
int   IposFindAposG(float g, int cpos, float* mpiposg, int fClosed, float* pdg, float* pdgSeg);
float GWrapApos(float g, int cpos, float* mpiposg, int fClosed);
float SMeasureCrvSegmentU(void* pv, float u);
float DuGetCrvSearchIncrement(CRV* pcrv);
void FindCrvClosestPointOnLineSegmentFromU(CRV* pcrv, glm::vec3* ppos0, glm::vec3* ppos1, float u, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, float* pu, float* ps);

float SFromCrvU(CRV* pcrv, float u);
float UFromCrvS(CRV* pcrv, float s);
float UMaxCrv(CRV* pcrv);
float SMaxCrv(CRV* pcrv);

int IcvFindCrvU(CRV* pcrv, float u, float* du, float* duSeg);
int IcvFindCrvS(CRV* pcrv, float s, float* ds, float* dsSeg);
float GMeasureCrvU(void* pv, float u);
void FindAposClosestPointAll(glm::vec3* ppos, CONSTR* pconstr, int cpos, glm::vec3* apos, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, int* pipos, float* pr);
void FindAposClosestPointSegment(glm::vec3* ppos, CONSTR* pconstr, int cpos, glm::vec3* apos, int iposStart, int fClosed, glm::vec3* pposClosest, glm::vec3* pnormTangentClosest, int* pipos, float* pr);

struct VTCRV
{
	void  (*pfnLoadCrvFromBrx)(CRV*, CBinaryInputStream*);
	void  (*pfnEvaluateCrvFromU)(CRV*, float, glm::vec3*, glm::vec3*);
	void  (*pfnEvaluateCrvFromS)(CRV*, float, glm::vec3*, glm::vec3*);
	void  (*pfnRenderCrvSegment)(CRV*, float, float);
	void  (*pfnConvertCrv)(CRV*, glm::mat4*, glm::mat4*);
	float (*pfnSFromCrvU)(CRV*, float) = SFromCrvU;
	float (*pfnUFromCrvS)(CRV*, float) = UFromCrvS;
	float (*pfnUMaxCrv)(CRV*) = UMaxCrv;
	float (*pfnSMaxCrv)(CRV*) = SMaxCrv;
	float (*pfnMeasureCrv)(CRV*);
	void  (*pfnFindCrvClosestPointAll)(CRV*, glm::vec3*, CONSTR*, glm::vec3*, glm::vec3*, float*, float*);
	void  (*pfnFindCrvClosestPointFromU)(CRV*, glm::vec3*, float, CONSTR*, glm::vec3*, glm::vec3*, float*, float*);
	void  (*pfnFindCrvClosestPointFromS)(CRV*, glm::vec3*, float, CONSTR*, glm::vec3*, glm::vec3*, float*, float*);
};
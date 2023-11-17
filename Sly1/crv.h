#pragma once
#include "vec.h"
#include "bis.h"

enum CRVK {
	CRVK_Nil = -1,
	CRVK_Linear = 0,
	CRVK_Cubic = 1,
	CRVK_Max = 2
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
	float *mpicvu;
	float *mpicvs;
	glm::vec3 mpicvpos;
};

struct CRVL : public CRV
{
	
};

struct CTCE
{
	glm::vec3 apos[20];
	float mpiposs[20];
};

struct CRVC : public CRV
{
	public:
		glm::vec3 *mpicvdposIn;
		glm::vec3 *mpicvdposOut;
		CTCE ctce;
		int icvCache;
};

void LoadCrvlFromBrx(CBinaryInputStream *pbis);
void LoadCrvcFromBrx(CBinaryInputStream *pbis);
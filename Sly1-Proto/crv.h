#pragma once
#include "lo.h"

enum CRVK
{
	CRVK_Nil = -1,
	CRVK_Linear = 0,
	CRVK_Cubic = 1,
	CRVK_Max = 2
};

struct VTCRVL
{
	void (*pfnLoadCrvlFromBrx)(CRVL*, CBinaryInputStream*) = LoadCrvlFromBrx;
};

struct VTCRVC
{
	void (*pfnLoadCrvcFromBrx)(CRVC*, CBinaryInputStream*) = LoadCrvcFromBrx;
};

struct CRV
{
	union
	{
		struct VTCRV*  pvtcrv;
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
		std::vector <glm::vec3> mpicvdposIn;
		std::vector <glm::vec3> mpicvdposOut;
		CTCE ctce;
		int icvCache;
};

CRV* PcrvNew(CRVK crvk);
void LoadCrvlFromBrx(CRVL *pcrvl, CBinaryInputStream *pbis);
void LoadCrvcFromBrx(CRVC *pcrvc, CBinaryInputStream *pbis);
void DeletePcrv(CRVK crvk, CRV *pcrv);

inline VTCRVL g_vtcrvl;
inline VTCRVC g_vtcrvc;
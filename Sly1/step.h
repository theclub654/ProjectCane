#pragma once
#include "po.h"

struct MRSG 
{
	float t;
	float dt;
};
struct BL
{
	float u;
	struct ASEG* paseg;
	struct CHN* pchn;
};
struct BLSS : public BL
{
	glm::vec3 v;
};
struct BLCL : public BL
{
	float sv;
};
struct BLRUN
{
	glm::vec3 v;
	glm::vec3 posPlant;
	MRSG mpfootsmrsgLeft[2];
	MRSG mpfootsmrsgRight[2];
};
struct SFF
{
	float au[2];
	float auVol[2];
};

class STEP : public PO
{
	public:
		ALO *paloLeftFoot;
		ALO *paloRightFoot;
		ALO *paloLeftHand;
		ALO *paloRightHand;
		ALO *paloLeftHeel;
		ALO *paloRightHeel;
		ALO *paloLeftBall;
		ALO *paloRightBall;
		float radJoy;
		float uJoyDeflect;
		float radTarget;
		float uTarget;
		glm::vec3 vTarget;
		float zBaseTarget;
		glm::mat3 matTarget;
		int fBaseXp;
		float tBaseXp;
		glm::vec3 posBaseXp;
		glm::vec3 normalBaseXp;
		SO *psoBaseXp;
		float uFootfall;
		float tLastFootfall;
		float dzBase;
		int iposBase;
		glm::vec3 *pposBase;
		glm::vec3 *pposBumper;
		SO *psoPhys;
		SMP smpSpin;
		CLQ clqDradToURun;
		float rGravity;
		SMP smpCompress;
		SMP smpExpand;
};

void InitStep(STEP* pstep);
void DeleteStep(LO* plo);
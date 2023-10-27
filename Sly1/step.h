#pragma once
#include "po.h"

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
};

void InitStep(STEP* pstep);
#pragma once
#include "so.h"

class PO : public SO
{
	public:
		int fPlayable;
		int fLockBase;
		glm::vec3 posBasePrev;
		float tWakeRipple;
		ALO *paloUseCharm;
};

void InitPo(PO *ppo);//NOT FINISHED
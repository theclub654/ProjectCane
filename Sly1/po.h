#pragma once
#include "so.h"

struct PZI
{
	int fCollectEnabled;
	struct ALO* paloBag;
	struct ALO* mpccharmpaloCharm[5];
	float sAutoCollect;
	struct ALO* paloCollect;
};

class PO : public SO
{
	public:
		int fPlayable;
		int fLockBase;
		glm::vec3 posBasePrev;
		float tWakeRipple;
		struct AMB* pambWake;
		PZI pzi;
		struct ALO* paloUseCharm;
};

void InitPo(PO *ppo);//NOT FINISHED
void OnPoAdd(PO* ppo);
void DeletePo(LO* plo);
#pragma once
#include "so.h"

enum BDT 
{
	BDT_Nil = -1,
	BDT_Impact = 0,
	BDT_Timed = 1,
	BDT_PropImpact = 2,
	BDT_Manual = 3,
	BDT_Player = 4,
	BDT_Max = 5
};

class BOMB : public SO
{
	public:
		BDT bdt;
		float tDetonate;
		float sDetonateRadius;
		float sDetonateRadiusJt;
		int fTouchDamage;
		float tDelay;
		int iDetonationProp;
		float sdvMax;
		LM lmMass;
		float mMax;
		int cpbrkp;
		struct BRKP* apbrkp[16];
		struct EXPL* pexpl;
		float rExplodeScale;
		CLQ clqRadiusToSv;
		LM lmSv;
		CLQ clqRadiusToSw;
		LM lmSw;
		glm::vec3 posDetonate;
		glm::vec3 normalDetonate;
		int fExplodeEffects;
		int fReclaim;
		ZPK zpk;
		struct SFX* psfxDet;
};

void InitBomb(BOMB* pbomb);
void OnBombAdd(BOMB* pbomb);
void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);
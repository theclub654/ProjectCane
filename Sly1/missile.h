#pragma once
#include "bomb.h"

class MISSILE : public BOMB
{
	public:
		float svMissileLaunch;
		struct ALO* paloTarget;
		glm::vec3 posTargetLocal;
		struct ASEG* pasegFire;
		struct ASEG* pasegaCur;
		int fFollowTrajectory;
};

class ACCMISS : public MISSILE
{
	public:
		float svMissileMax;
		float sdvMissileMax;
		glm::vec3 vTarget;
};

class TARMISS : public ACCMISS
{
	public:
		CLQ clqSwMax;
		LM lmSwMax;
};

class GROUNDMISS : public MISSILE
{
	public:
		float sdvTrackMax;
};

void InitMissile(MISSILE* pmissile);
void InitAccmiss(ACCMISS* paccmiss);
void InitTarmiss(TARMISS* ptarmiss);
void InitGroundmiss(GROUNDMISS* pgroundmiss);
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);
void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro);
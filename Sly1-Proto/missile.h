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

class SPLMISS : public MISSILE
{
	public:

	float dtImpact;
	float svMissileImpact;
	float uBank;
	float tImpact;
	glm::vec3 vImpact;
};

class GROUNDMISS : public MISSILE
{
	public:
		float sdvTrackMax;
};

MISSILE*NewMissile();
void InitMissile(MISSILE* pmissile);
int  GetMissileSize();
void OnMissileRemove(MISSILE* pmissile);
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);
void CloneMissile(MISSILE* pmissile, MISSILE* pmissileBase);
void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro);
void DeleteMissile(MISSILE* pmissile);

ACCMISS*NewAccmiss();
void InitAccmiss(ACCMISS* paccmiss);
int  GetAccmissSize();
void CloneAccmiss(ACCMISS* paccmiss, ACCMISS* paccmissBase);
void DeleteAccmiss(ACCMISS* paccmiss);

TARMISS*NewTarmiss();
void InitTarmiss(TARMISS* ptarmiss);
int  GetTarmissSize();
void CloneTarmiss(TARMISS* ptarmiss, TARMISS* ptarmissBase);
void DeleteTarmiss(TARMISS* ptarmiss);

SPLMISS*NewSplmiss();
int  GetSplmissSize();
void CloneSplmiss(SPLMISS* psplmiss, SPLMISS* psplmissBase);
void DeleteSplmiss(SPLMISS *psplmiss);

GROUNDMISS*NewGroundmiss();
void InitGroundmiss(GROUNDMISS* pgroundmiss);
int  GetGroundmissSize();
void CloneGroundmiss(GROUNDMISS* pgroundmiss, GROUNDMISS* pgroundmissBase);
void DeleteGroundmiss(GROUNDMISS *pgroundmiss);

extern SNIP s_asnipMissile[1];
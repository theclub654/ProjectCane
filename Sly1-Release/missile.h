#pragma once
#include "bomb.h"

class MISSILE : public BOMB
{
	public:
	float svMissileLaunch;
	class ALO* paloTarget;
	glm::vec3 posTargetLocal;
	struct ASEG* pasegFire;
	struct ASEGA* pasegaCur;
	int fFollowTrajectory;
	int coidIgnore;
	short aoidIgnore[4];
	int ccidIgnore;
	short acidIgnore[4];
	SO* psoIgnore;
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
	float dtLifetimeOverride;
	float svMax;
	float rTrajectoryBank;
	float tExpire;
	glm::vec3 vTarget;
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
void SetMissileSvMissileLaunch(MISSILE* pmissile, float svMissileLaunch);
void*GetMissileSvMissileLaunch(MISSILE* pmissile);
void SetMissilePaloTarget(MISSILE* pmissile, ALO* paloTarget);
void*GetMissilePaloTarget(MISSILE* pmissile);
void SetMissilePosTargetLocal(MISSILE* pmissile, glm::vec3 posTargetLocal);
void*GetMissilePosTargetLocal(MISSILE* pmissile);
void SetMissileFFollowTrajectory(MISSILE* pmissile, int fFollowTrajectory);
void*GetMissileFFollowTrajectory(MISSILE* pmissile);
int  GetMissileSize();
void OnMissileRemove(MISSILE* pmissile);
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);
void CloneMissile(MISSILE* pmissile, MISSILE* pmissileBase);
void ProjectMissileTransform(MISSILE* pmissile, float dt, int fParentDirty);
void FireMissile(MISSILE* pmissile, ALO* paloTarget, glm::vec3* pposTargetLocal);
void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro);
int  FIgnoreMissileIntersection(MISSILE* pmissile, SO* psoOther);
void AddMissileIgnoreObject(MISSILE* pmissile, OID oid);
void AddMissileIgnoreClass(MISSILE* pmissile, CID cid);
void DeleteMissile(MISSILE* pmissile);

ACCMISS*NewAccmiss();
void InitAccmiss(ACCMISS* paccmiss);
int  GetAccmissSize();
void CloneAccmiss(ACCMISS* paccmiss, ACCMISS* paccmissBase);
void FireAccmiss(ACCMISS* paccmiss, ALO* paloTarget, glm::vec3* pposTarget);
void PresetAccmissAccel(ACCMISS* paccmiss, float dt);
void DeleteAccmiss(ACCMISS* paccmiss);

TARMISS*NewTarmiss();
void InitTarmiss(TARMISS* ptarmiss);
void*GetTarmissDtLifetimeOverride(TARMISS* ptarmiss);
void SetTarmissDtLifetimeOverride(TARMISS* ptarmiss, float dtLifetimeOverride);
void*GetTarmissSvMax(TARMISS* ptarmiss);
void SetTarmissSvMax(TARMISS* ptarmiss, float svMax);
void*GetTarmissRTrajectoryBank(TARMISS* ptarmiss);
void SetTarmissRTrajectoryBank(TARMISS* ptarmiss, float rTrajectoryBank);
void*GetTarmissVTarget(TARMISS* ptarmiss);
void SetTarmissVTarget(TARMISS* ptarmiss, glm::vec3 vTarget);
int  GetTarmissSize();
void CloneTarmiss(TARMISS* ptarmiss, TARMISS* ptarmissBase);
void FireTarmiss(TARMISS* ptarmiss, ALO* paloTarget, glm::vec3* pposTargetLocal);
void ProjectTarmissTransform(TARMISS* ptarmiss, float dt, int fParentDirty);
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

extern SNIP s_asnipMissile;

static float DT_TarmissileTargetPrediction = 0.5;
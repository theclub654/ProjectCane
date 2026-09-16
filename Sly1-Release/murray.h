#pragma once
#include "stepguard.h"
#include "asega.h"

class MURRAY : public STEPGUARD
{
	public:
	int cpaloEnemy;
	class ALO* apaloEnemy[4];
	class XFM* pxfmPatrol;
	class XFM* pxfmPursue;
	class ASEG* pasegCelebrateKey;
	class TURRET* pturret;
	ASEG* pasegDeath;
	ASEG* pasegStun;
	ASEG* pasegDeathAlt;
	ASEG* pasegStunAlt;
	int murrayMode;
	int cStunHits;
	int fFaceStunProp;
};

MURRAY*NewMurray();
void InitMurray(MURRAY* pmurray);
void PostMurrayLoad(MURRAY* pmurray);
int  GetMurraySize();
void CloneMurray(MURRAY* pmurray, MURRAY* pmurrayBase);
void OnMurrayEnteringSgs(MURRAY* pmurray, SGS sgsPrev, ASEG* pasegOverride);
void OnMurrayExitingSgs(MURRAY* pmurray, SGS sgs);
void UpdateMurrayGoal(MURRAY* pmurray, int fEnter);
void UpdateMurraySgs(MURRAY* pmurray);
int  FInvulnerableMurray(MURRAY* pmurray);
bool FTakeMurrayDamage(MURRAY* pmurray, ZPR* pzpr);
bool FAbsorbMurrayWkr(MURRAY* pmurray, WKR* pwkr);
int FDetectMurray(MURRAY* pmurray);
bool FCanMurrayAttack();
bool FAddMurrayDetectedEnemy(MURRAY* pmurray, STEPGUARD* pstepguard);
void*GetMurrayPursueXfm(MURRAY* pmurray);
void SetMurrayPursueXfm(MURRAY* pmurray, XFM* pxfm);
void*GetMurrayPatrolXfm(MURRAY* pmurray);
void SetMurrayPatrolXfm(MURRAY* pmurray, XFM* pxfm);
void CollectMurrayPrize(MURRAY* pmurray, PCK pck, ALO* paloOther);
int  JthsCurrentMurray(MURRAY* pmurray);
void HandleMurrayMessage(MURRAY* pmurray, MSGID msgid, void* pv);
void DeleteMurray(MURRAY* pmurray);

extern SNIP a_snipPostMurray[2];

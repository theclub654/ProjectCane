#include "missile.h"

MISSILE* NewMissile()
{
	return new MISSILE{};
}

void InitMissile(MISSILE* pmissile)
{
	InitBomb(pmissile);
}

int GetMissileSize()
{
	return sizeof(MISSILE);
}

void OnMissileRemove(MISSILE* pmissile)
{
	OnSoRemove(pmissile);
}

void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis)
{
	LoadBombFromBrx(pmissile, pbis);
}

void CloneMissile(MISSILE* pmissile, MISSILE* pmissileBase)
{
	CloneBomb(pmissile, pmissileBase);

	pmissile->svMissileLaunch = pmissileBase->svMissileLaunch;
	pmissile->paloTarget = pmissileBase->paloTarget;
	pmissile->posTargetLocal = pmissileBase->posTargetLocal;
	pmissile->pasegFire = pmissileBase->pasegFire;
	pmissile->pasegaCur = pmissileBase->pasegaCur;
	pmissile->fFollowTrajectory = pmissileBase->fFollowTrajectory;
}

void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro)
{
	RenderAloAll(pmissile, pcm, pro);
}

void DeleteMissile(MISSILE *pmissile)
{
	delete pmissile;
}

ACCMISS* NewAccmiss()
{
	return new ACCMISS{};
}

void InitAccmiss(ACCMISS* paccmiss)
{
	InitMissile(paccmiss);
}

int GetAccmissSize()
{
	return sizeof(ACCMISS);
}

void CloneAccmiss(ACCMISS* paccmiss, ACCMISS* paccmissBase)
{
	CloneMissile(paccmiss, paccmissBase);

	paccmiss->svMissileMax = paccmissBase->svMissileMax;
	paccmiss->sdvMissileMax = paccmissBase->sdvMissileMax;
	paccmiss->vTarget = paccmissBase->vTarget;
}

void DeleteAccmiss(ACCMISS* paccmiss)
{
	delete paccmiss;
}

TARMISS* NewTarmiss()
{
	return new TARMISS{};
}

void InitTarmiss(TARMISS* ptarmiss)
{
	InitAccmiss(ptarmiss);
}

int GetTarmissSize()
{
	return sizeof(TARMISS);
}

void CloneTarmiss(TARMISS* ptarmiss, TARMISS* ptarmissBase)
{
	CloneAccmiss(ptarmiss, ptarmissBase);

	ptarmiss->clqSwMax = ptarmissBase->clqSwMax;
	ptarmiss->lmSwMax = ptarmissBase->lmSwMax;
}

void DeleteTarmiss(TARMISS *ptarmiss)
{
	delete ptarmiss;
}

SPLMISS* NewSplmiss()
{
	return new SPLMISS{};
}

int GetSplmissSize()
{
	return sizeof(SPLMISS);
}

void CloneSplmiss(SPLMISS* psplmiss, SPLMISS* psplmissBase)
{
	CloneMissile(psplmiss, psplmissBase);
	
	psplmiss->dtImpact = psplmissBase->dtImpact;
	psplmiss->svMissileImpact = psplmissBase->svMissileImpact;
	psplmiss->uBank = psplmissBase->uBank;
	psplmiss->tImpact = psplmissBase->tImpact;
	psplmiss->vImpact = psplmissBase->vImpact;
}

void DeleteSplmiss(SPLMISS *psplmiss)
{
	delete psplmiss;
}

GROUNDMISS* NewGroundmiss()
{
	return new GROUNDMISS{};
}

void InitGroundmiss(GROUNDMISS* pgroundmiss)
{
	InitMissile(pgroundmiss);
	pgroundmiss->fFollowTrajectory = 0;
	pgroundmiss->sdvTrackMax = 2000.0;
}

int GetGroundmissSize()
{
	return sizeof(GROUNDMISS);
}

void CloneGroundmiss(GROUNDMISS* pgroundmiss, GROUNDMISS* pgroundmissBase)
{
	CloneMissile(pgroundmiss, pgroundmissBase);

	pgroundmiss->sdvTrackMax = pgroundmissBase->sdvTrackMax;
}

void DeleteGroundmiss(GROUNDMISS *pgroundmiss)
{
	delete pgroundmiss;
}
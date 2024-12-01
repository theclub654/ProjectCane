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
	LO lo = *pmissile;
	*pmissile = *pmissileBase;
	memcpy(pmissile, &lo, sizeof(LO));

	CloneLo(pmissile, pmissileBase);

	ClearDl(&pmissile->dlChild);

	pmissile->pxa = nullptr;
	pmissile->grfpvaXpValid = 0;
	pmissile->pstso = nullptr;
}

void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro)
{
	RenderAloAll(pmissile, pcm, pro);
}

void DeleteMissile(LO* plo)
{
	delete (MISSILE*)plo;
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
	LO lo = *paccmiss;
	*paccmiss = *paccmissBase;
	memcpy(paccmiss, &lo, sizeof(LO));

	CloneLo(paccmiss, paccmissBase);

	ClearDl(&paccmiss->dlChild);

	paccmiss->pxa = nullptr;
	paccmiss->grfpvaXpValid = 0;
	paccmiss->pstso = nullptr;
}

void DeleteAccmiss(LO* plo)
{
	delete (ACCMISS*)plo;
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
	LO lo = *ptarmiss;
	*ptarmiss = *ptarmissBase;
	memcpy(ptarmiss, &lo, sizeof(LO));

	CloneLo(ptarmiss, ptarmissBase);

	ClearDl(&ptarmiss->dlChild);

	ptarmiss->pxa = nullptr;
	ptarmiss->grfpvaXpValid = 0;
	ptarmiss->pstso = nullptr;
}

void DeleteTarmiss(LO* plo)
{
	delete (TARMISS*)plo;
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
	LO lo = *psplmiss;
	*psplmiss = *psplmissBase;
	memcpy(psplmiss, &lo, sizeof(LO));

	CloneLo(psplmiss, psplmissBase);

	ClearDl(&psplmiss->dlChild);

	psplmiss->pxa = nullptr;
	psplmiss->grfpvaXpValid = 0;
	psplmiss->pstso = nullptr;
}

void DeleteSplmiss(LO* plo)
{
	delete (SPLMISS*)plo;
}

GROUNDMISS* NewGroundmiss()
{
	return new GROUNDMISS{};
}

void InitGroundmiss(GROUNDMISS* pgroundmiss)
{
	InitMissile(pgroundmiss);
}

int GetGroundmissSize()
{
	return sizeof(GROUNDMISS);
}

void CloneGroundmiss(GROUNDMISS* pgroundmiss, GROUNDMISS* pgroundmissBase)
{
	LO lo = *pgroundmiss;
	*pgroundmiss = *pgroundmissBase;
	memcpy(pgroundmiss, &lo, sizeof(LO));

	CloneLo(pgroundmiss, pgroundmissBase);

	ClearDl(&pgroundmiss->dlChild);

	pgroundmiss->pxa = nullptr;
	pgroundmiss->grfpvaXpValid = 0;
	pgroundmiss->pstso = nullptr;
}

void DeleteGroundmiss(LO* plo)
{
	delete (GROUNDMISS*)plo;
}
#include "pzo.h"

void* NewVault()
{
	return new VAULT;
}

void InitVault(VAULT* pvault)
{
	InitPo(pvault);
}

int GetVaultSize()
{
	return sizeof(VAULT);
}

void CloneVault(VAULT* pvault, VAULT* pvaultBase)
{
	LO lo = *pvault;
	*pvault = *pvaultBase;
	memcpy(pvault, &lo, sizeof(LO));

	CloneLo(pvault, pvaultBase);

	ClearDl(&pvault->dlChild);

	pvault->pxa = nullptr;
	pvault->grfpvaXpValid = 0;
	pvault->pstso = nullptr;
}

void DeleteVault(LO* plo)
{
	delete (VAULT*)plo;
}

void* NewSprize()
{
	return new SPRIZE;
}

void InitSprize(SPRIZE* psprize)
{
	InitSo(psprize);
}

int GetSprizeSize()
{
	return sizeof(SPRIZE);
}

void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(psprize, pbis);
}

void CloneSprize(SPRIZE* psprize, SPRIZE* psprizeBase)
{
	LO lo = *psprize;
	*psprize = *psprizeBase;
	memcpy(psprize, &lo, sizeof(LO));

	CloneLo(psprize, psprizeBase);

	ClearDl(&psprize->dlChild);

	psprize->pxa = nullptr;
	psprize->grfpvaXpValid = 0;
	psprize->pstso = nullptr;
}

void DeleteSprize(LO* plo)
{
	delete (SPRIZE*)plo;
}

void* NewScprize()
{
	return new SCPRIZE;
}

void InitScprize(SCPRIZE* pscprize)
{
	InitSprize(pscprize);
}

int GetScprizeSize()
{
	return sizeof(SCPRIZE);
}

void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase)
{
	LO lo = *pscprize;
	*pscprize = *pscprizeBase;
	memcpy(pscprize, &lo, sizeof(LO));

	CloneLo(pscprize, pscprizeBase);

	ClearDl(&pscprize->dlChild);

	pscprize->pxa = nullptr;
	pscprize->grfpvaXpValid = 0;
	pscprize->pstso = nullptr;
}

void DeleteScprize(LO* plo)
{
	delete (SCPRIZE*)plo;
}

void* NewLifetkn()
{
	return new LIFETKN;
}

int GetLifetknSize()
{
	return sizeof(LIFETKN);
}

void CloneLifetkn(LIFETKN* plifetkn, LIFETKN* plifetknBase)
{
	LO lo = *plifetkn;
	*plifetkn = *plifetknBase;
	memcpy(plifetkn, &lo, sizeof(LO));

	CloneLo(plifetkn, plifetknBase);

	ClearDl(&plifetkn->dlChild);

	plifetkn->pxa = nullptr;
	plifetkn->grfpvaXpValid = 0;
	plifetkn->pstso = nullptr;
}

void DeleteLifetkn(LO* plo)
{
	delete (LIFETKN*)plo;
}

void* NewClue()
{
	return new CLUE;
}

void InitClue(CLUE* pclue)
{
	InitSprize(pclue);
	pclue->psw->cclueAll;
}

int GetClueSize()
{
	return sizeof(CLUE);
}

void CloneClue(CLUE* pclue, CLUE* pclueBase)
{
	LO lo = *pclue;
	*pclue = *pclueBase;
	memcpy(pclue, &lo, sizeof(LO));

	CloneLo(pclue, pclueBase);

	ClearDl(&pclue->dlChild);

	pclue->pxa = nullptr;
	pclue->grfpvaXpValid = 0;
	pclue->pstso = nullptr;
}

void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis)
{
	LoadSprizeFromBrx(pclue, pbis);
}

void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro)
{

}

void DeleteClue(LO* plo)
{
	delete (CLUE*)plo;
}

void* NewLock()
{
	return new LOCK;
}

int GetLockSize()
{
	return sizeof(LOCK);
}

void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plock, pbis);
}

void CloneLock(LOCK* plock, LOCK* plockBase)
{
	LO lo = *plock;
	*plock = *plockBase;
	memcpy(plock, &lo, sizeof(LO));

	CloneLo(plock, plockBase);

	ClearDl(&plock->dlChild);
}

void DeleteLock(LO* plo)
{
	delete(LOCK*)plo;
}

void* NewLockg()
{
	return new LOCKG;
}

int GetLockgSize()
{
	return sizeof(LOCKG);
}

void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plockg, pbis);
}

void CloneLockg(LOCKG* plockg, LOCKG* plockgBase)
{
	LO lo = *plockg;
	*plockg = *plockgBase;
	memcpy(plockg, &lo, sizeof(LO));

	CloneLo(plockg, plockgBase);

	ClearDl(&plockg->dlChild);
}

void DeleteLockg(LO* plo)
{
	delete(LOCKG*)plo;
}
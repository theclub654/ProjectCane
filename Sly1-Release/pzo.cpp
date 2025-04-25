#include "pzo.h"

VAULT* NewVault()
{
	return new VAULT{};
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
	ClonePo(pvault, pvaultBase);

	// Shallow copy of the value members
	pvault->nCombination = pvaultBase->nCombination;
	pvault->oidDialogCombo = pvaultBase->oidDialogCombo;
	pvault->oidVolbtnGoad = pvaultBase->oidVolbtnGoad;
	pvault->fGoadStart = pvaultBase->fGoadStart;
	pvault->fvault = pvaultBase->fvault;
	pvault->cpdialogPending = pvaultBase->cpdialogPending;

	// Shallow copy of the pointer members
	pvault->psmVault = pvaultBase->psmVault;
	pvault->psmaVault = pvaultBase->psmaVault;
	pvault->pxfmJt = pvaultBase->pxfmJt;
	std::memcpy(pvault->atmbl, pvaultBase->atmbl, sizeof(pvault->atmbl)); // Copy the array
	pvault->pdialogCombo = pvaultBase->pdialogCombo;
	pvault->pvolbtnGoad = pvaultBase->pvolbtnGoad;

	// Shallow copy the array of GOAD structures
	std::memcpy(pvault->mpgoadkgoad, pvaultBase->mpgoadkgoad, sizeof(pvault->mpgoadkgoad));

	// Shallow copy the arrays of DIALOG pointers
	std::memcpy(pvault->apdialogInstruct, pvaultBase->apdialogInstruct, sizeof(pvault->apdialogInstruct));
	std::memcpy(pvault->apdialogPending, pvaultBase->apdialogPending, sizeof(pvault->apdialogPending));
}

void DeleteVault(VAULT* pvault)
{
	delete pvault;
}

SPRIZE* NewSprize()
{
	return new SPRIZE{};
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
	CloneSo(psprize, psprizeBase);

	psprize->sCollect = psprizeBase->sCollect;
	psprize->fCollect = psprizeBase->fCollect;
	psprize->fNoExit = psprizeBase->fNoExit;
	psprize->coidAseg = psprizeBase->coidAseg;

	// Shallow copy of the arrays
	std::memcpy(psprize->aoidAseg, psprizeBase->aoidAseg, sizeof(psprize->aoidAseg));
	std::memcpy(psprize->mpipasegfDone, psprizeBase->mpipasegfDone, sizeof(psprize->mpipasegfDone));

	// Shallow copy of the pointer members
	psprize->cpaseg = psprizeBase->cpaseg;
	std::memcpy(psprize->apaseg, psprizeBase->apaseg, sizeof(psprize->apaseg));

	// Copy the pointer to the EXPL structure
	psprize->pexpl = psprizeBase->pexpl;

	// Copy the flag for Jt done
	psprize->fJtDone = psprizeBase->fJtDone;
}

void DeleteSprize(SPRIZE *psprize)
{
	delete psprize;
}

SCPRIZE* NewScprize()
{
	return new SCPRIZE{};
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
	int ichkCollected = pscprizeBase->ichkCollected;

	CloneSprize(pscprize, pscprizeBase);

	pscprize->ichkCollected = ichkCollected;
}

void DeleteScprize(SCPRIZE *pscprize)
{
	delete pscprize;
}

CLUE* NewClue()
{
	return new CLUE{};
}

void InitClue(CLUE* pclue)
{
	InitSprize(pclue);
	pclue->ibit = pclue->psw->cclueAll;
	pclue->psw->cclueAll++;
}

int GetClueSize()
{
	return sizeof(CLUE);
}

void CloneClue(CLUE* pclue, CLUE* pclueBase)
{
	CloneSprize(pclue, pclueBase);

	pclue->ibit = pclueBase->ibit;
	pclue->dtFrame = pclueBase->dtFrame;
	pclue->dtFrameMax = pclueBase->dtFrameMax;
	pclue->cpaloRender = pclueBase->cpaloRender;
	pclue->swSpin = pclueBase->swSpin;
	pclue->radSpin = pclueBase->radSpin;
	pclue->apaloRender = pclueBase->apaloRender;
	pclue->paloSmack = pclueBase->paloSmack;
}

void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis)
{
	LoadSprizeFromBrx(pclue, pbis);
}

void RenderClueAll(CLUE *pclue, CM *pcm, RO *pro)
{
	RenderAloAll(pclue, pcm, pro);
	//pclue->pvtclue->pfnRenderClueAll(pclue, pcm, pro);
}

void DeleteClue(CLUE *pclue)
{
	delete pclue;
}

LOCK* NewLock()
{
	return new LOCK{};
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
	CloneAlo(plock, plockBase);

	plock->psm = plockBase->psm;
	plock->psma = plockBase->psma;
	plock->paloKey = plockBase->paloKey;
}

void DeleteLock(LOCK* plock)
{
	delete plock;
}

LOCKG* NewLockg()
{
	return new LOCKG{};
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
	CloneAlo(plockg, plockgBase);

	plockg->grfws = plockgBase->grfws;
	plockg->psm = plockgBase->psm;
	plockg->psma = plockgBase->psma;
	plockg->coidLock = plockgBase->coidLock;

	plockg->cplock = plockgBase->cplock;
	for (int i = 0; i < plockgBase->cplock; ++i)
	{
		plockg->aplock[i] = plockgBase->aplock[i];
	}

	for (int i = 0; i < 8; ++i)
	{
		plockg->aoidLock[i] = plockgBase->aoidLock[i];
	}
}

void DeleteLockg(LOCKG* plockg)
{
	delete plockg;
}
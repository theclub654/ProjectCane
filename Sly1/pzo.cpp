#include "pzo.h"

void* NewVault()
{
	return new VAULT;
}

void InitVault(VAULT* pvault)
{
	InitPo(pvault);
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

void DeleteScprize(LO* plo)
{
	delete (SCPRIZE*)plo;
}

void* NewLifetkn()
{
	return new LIFETKN;
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

void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(psprize, pbis);
}

void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis)
{
	LoadSprizeFromBrx(pclue, pbis);
}

void DeleteClue(LO* plo)
{
	delete (CLUE*)plo;
}

void* NewLock()
{
	return new LOCK;
}

void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plock, pbis);
}

void DeleteLock(LO* plo)
{
	delete(LOCK*)plo;
}

void* NewLockg()
{
	return new LOCKG;
}

void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plockg, pbis);
}

void DeleteLockg(LO* plo)
{
	delete(LOCKG*)plo;
}

void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro)
{

}

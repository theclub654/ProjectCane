#include "pzo.h"

void InitVault(VAULT* pvault)
{
	/*std::cout << "VAULT Size: " << sizeof(VAULT) << "\n";
	std::cout << "SPRIZE Size: " << sizeof(SPRIZE) << "\n";
	std::cout << "SCPRIZE Size: " << sizeof(SCPRIZE) << "\n";
	std::cout << "CLUE Size: " << sizeof(CLUE) << "\n";
	std::cout << "LOCK Size: " << sizeof(LOCK) << "\n";
	std::cout << "LOCKG Size: " << sizeof(LOCKG) << "\n";*/
	InitPo(pvault);
}

void DeleteVault(LO* plo)
{
	delete (VAULT*)plo;
}

void InitSprize(SPRIZE* psprize)
{
	InitSo(psprize);
}

void DeleteSprize(LO* plo)
{
	delete (SPRIZE*)plo;
}

void InitScprize(SCPRIZE* pscprize)
{
	InitSprize(pscprize);
}

void DeleteScprize(LO* plo)
{
	delete (SCPRIZE*)plo;
}

void DeleteLifetkn(LO* plo)
{
	delete (LIFETKN*)plo;
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

void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plock, pbis);
}

void DeleteLock(LO* plo)
{
	delete(LOCK*)plo;
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

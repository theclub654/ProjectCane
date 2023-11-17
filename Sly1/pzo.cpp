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

void InitSprize(SPRIZE* psprize)
{
	InitSo(psprize);
}

void InitScprize(SCPRIZE* pscprize)
{
	InitSprize(pscprize);
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

void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plock, pbis);
}

void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(plockg, pbis);
}

void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro)
{

}

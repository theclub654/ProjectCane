#include "pzo.h"

void InitVault(VAULT* pvault)
{
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

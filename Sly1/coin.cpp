#include "coin.h"

void InitDprize(DPRIZE *pdprize)
{
	pdprize->dprizesInit = DPRIZES_Normal;
	pdprize->dprizes = DPRIZES_Nil;
	pdprize->svcAttract = 30.0;
	pdprize->oidInitialState = OID_Nil;
	//std::cout << "DPRIZE SIZE: " << sizeof(DPRIZE) << "\n";
	InitAlo(pdprize);
	AppendDlEntry(&pdprize->psw->dlDprize, pdprize);
	
}

void DeleteDprize(LO* plo)
{
	delete(DPRIZE*)plo;
}

void InitCharm(CHARM* pcharm)
{
	//std::cout << "CHARM Size: " << sizeof(CHARM) << "\n";
	InitDprize(pcharm);
}

void DeleteCharm(LO* plo)
{
	delete(CHARM*)plo;
}

void InitCoin(COIN* pcoin)
{
	//std::cout << "COIN Size: " << sizeof(COIN) << "\n";
	InitDprize(pcoin);
}

void DeleteCoin(LO* plo)
{
	delete(COIN*)plo;
}

void InitKey(KEY* pkey)
{
	//std::cout << "KEY Size: " << sizeof(KEY) << "\n";
	InitDprize(pkey);
}

void DeleteKey(LO* plo)
{
	delete(KEY*)plo;
}

void InitGold(GOLD* pgold)
{
	//std::cout << "GOLD Size: " << sizeof(GOLD) << "\n";
	InitDprize(pgold);
	pgold->psw->cgoldAll++;
}

void DeleteGold(LO* plo)
{
	delete(GOLD*)plo;
}

void LoadDprizeFromBrx(DPRIZE *pdprize, CBinaryInputStream *pbis)
{
	LoadAloFromBrx(pdprize, pbis);
}

void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro)
{

}

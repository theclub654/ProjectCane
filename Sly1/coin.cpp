#include "coin.h"

void* NewDprize()
{
	return new DPRIZE;
}

void InitDprize(DPRIZE *pdprize)
{
	pdprize->dprizesInit = DPRIZES_Normal;
	pdprize->dprizes = DPRIZES_Nil;
	pdprize->svcAttract = 30.0;
	pdprize->oidInitialState = OID_Nil;
	InitAlo(pdprize);
	AppendDlEntry(&pdprize->psw->dlDprize, pdprize);
	
}

void DeleteDprize(LO* plo)
{
	delete(DPRIZE*)plo;
}

void* NewCharm()
{
	return new CHARM;
}

void InitCharm(CHARM* pcharm)
{
	InitDprize(pcharm);
}

void DeleteCharm(LO* plo)
{
	delete(CHARM*)plo;
}

void* NewCoin()
{
	return new COIN;
}

void InitCoin(COIN* pcoin)
{
	InitDprize(pcoin);
}

void DeleteCoin(LO* plo)
{
	delete(COIN*)plo;
}

void* NewKey()
{
	return new KEY;
}

void InitKey(KEY* pkey)
{
	InitDprize(pkey);
}

void DeleteKey(LO* plo)
{
	delete(KEY*)plo;
}

void* NewGold()
{
	return new GOLD;
}

void InitGold(GOLD* pgold)
{
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

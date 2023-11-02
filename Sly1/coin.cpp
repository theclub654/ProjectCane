#include "coin.h"

void InitDprize(DPRIZE *pdprize)
{
	pdprize->dprizesInit = DPRIZES_Normal;
	pdprize->dprizes = DPRIZES_Nil;
	pdprize->svcAttract = 30.0;
	pdprize->oidInitialState = OID_Nil;
	InitAlo(pdprize);
	AppendDlEntry(&pdprize->psw->dlDprize, pdprize);
	
}

void InitCharm(CHARM* pcharm)
{
	InitDprize(pcharm);
}

void InitCoin(COIN* pcoin)
{
	InitDprize(pcoin);
}

void InitKey(KEY* pkey)
{
	InitDprize(pkey);
}

void InitGold(GOLD* pgold)
{
	InitDprize(pgold);
}

void LoadDprizeFromBrx(DPRIZE *pdprize, CBinaryInputStream *pbis)
{
	LoadAloFromBrx(pdprize, pbis);
}

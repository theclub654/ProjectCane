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

int GetDprizeSize()
{
	return sizeof(DPRIZE);
}

void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pdprize, pbis);
}

void CloneDprize(DPRIZE* pdprize, DPRIZE* pdprizeBase)
{
	LO lo = *pdprize;
	*pdprize = *pdprizeBase;
	memcpy(pdprize, &lo, sizeof(LO));

	CloneLo(pdprize, pdprizeBase);

	ClearDl(&pdprize->dlChild);
}

void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro)
{

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

int GetCharmSize()
{
	return sizeof(CHARM);
}

void CloneCharm(CHARM* pcharm, CHARM* pcharmBase)
{
	LO lo = *pcharm;
	*pcharm = *pcharmBase;
	memcpy(pcharm, &lo, sizeof(LO));

	CloneLo(pcharm, pcharmBase);

	ClearDl(&pcharm->dlChild);
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

int GetCoinSize()
{
	return sizeof(COIN);
}

void CloneCoin(COIN* pcoin, COIN* pcoinBase)
{
	LO lo = *pcoin;
	*pcoin = *pcoinBase;
	memcpy(pcoin, &lo, sizeof(LO));

	CloneLo(pcoin, pcoinBase);

	ClearDl(&pcoin->dlChild);
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

int GetKeySize()
{
	return sizeof(KEY);
}

void CloneKey(KEY* pkey, KEY* pkeyBase)
{
	LO lo = *pkey;
	*pkey = *pkeyBase;
	memcpy(pkey, &lo, sizeof(LO));

	CloneLo(pkey, pkeyBase);

	ClearDl(&pkey->dlChild);
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

int GetGoldSize()
{
	return sizeof(GOLD);
}

void CloneGold(GOLD* pgold, GOLD* pgoldBase)
{
	LO lo = *pgold;
	*pgold = *pgoldBase;
	memcpy(pgold, &lo, sizeof(LO));

	CloneLo(pgold, pgoldBase);

	ClearDl(&pgold->dlChild);
}

void DeleteGold(LO* plo)
{
	delete(GOLD*)plo;
}
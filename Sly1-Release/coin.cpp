#include "coin.h"

DPRIZE* NewDprize()
{
	return new DPRIZE{};
}

void InitSwDprizeDl(SW* psw)
{
	InitDl(&psw->dlDprize, offsetof(DPRIZE, dleDprize));
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
	CloneAlo(pdprize, pdprizeBase);

	pdprize->dprizes = pdprizeBase->dprizes;
	pdprize->tDprizes = pdprizeBase->tDprizes;
	pdprize->dprizesInit = pdprizeBase->dprizesInit;
	pdprize->oidInitialState = pdprizeBase->oidInitialState;
	pdprize->dtInitialSkip = pdprizeBase->dtInitialSkip;

	pdprize->psm = pdprizeBase->psm;
	pdprize->psma = pdprizeBase->psma;
	pdprize->ptarget = pdprizeBase->ptarget;

	pdprize->posCenter = pdprizeBase->posCenter;
	pdprize->vCenter = pdprizeBase->vCenter;
	pdprize->dvCenter = pdprizeBase->dvCenter;

	pdprize->uGlintChance = pdprizeBase->uGlintChance;
	pdprize->ppntFrontGlint = pdprizeBase->ppntFrontGlint;
	pdprize->ppntBackGlint = pdprizeBase->ppntBackGlint;

	pdprize->fLeft = pdprizeBase->fLeft;
	pdprize->tGlint = pdprizeBase->tGlint;

	pdprize->fNeverReuse = pdprizeBase->fNeverReuse;
	pdprize->fReuseCandidate = pdprizeBase->fReuseCandidate;
	pdprize->fLastBounce = pdprizeBase->fLastBounce;

	pdprize->svLastBounceMax = pdprizeBase->svLastBounceMax;
	pdprize->svLastBounce = pdprizeBase->svLastBounce;
	pdprize->sRadiusBounce = pdprizeBase->sRadiusBounce;
	pdprize->sRadiusCollect = pdprizeBase->sRadiusCollect;
	pdprize->rzBounce = pdprizeBase->rzBounce;
	pdprize->rxyBounce = pdprizeBase->rxyBounce;
	pdprize->radSmooth = pdprizeBase->radSmooth;
	pdprize->normalSmooth = pdprizeBase->normalSmooth;

	pdprize->fSwirlDone = pdprizeBase->fSwirlDone;
	pdprize->dleDprize = pdprizeBase->dleDprize;
	pdprize->ichkCollected = pdprizeBase->ichkCollected;

	pdprize->pexplCollect = pdprizeBase->pexplCollect;
	pdprize->pexplAttract = pdprizeBase->pexplAttract;
	pdprize->svcAttract = pdprizeBase->svcAttract;
	pdprize->cAttract = pdprizeBase->cAttract;
}

void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro)
{
	RenderAloAll(pdprize, pcm, pro);
}

void DeleteDprize(DPRIZE* pdprize)
{
	delete pdprize;
}

CHARM* NewCharm()
{
	return new CHARM{};
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
	CloneDprize(pcharm, pcharmBase);
}

void DeleteCharm(CHARM* pcharm)
{
	delete pcharm;
}

void StartupCoinCtr(COINCTR* pcoinctr)
{
	pcoinctr->pvtcoinctr = &g_vtcoinctr;
}

COIN* NewCoin()
{
	return new COIN{};
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
	CloneDprize(pcoin, pcoinBase);
}

void DeleteCoin(COIN* pcoin)
{
	delete pcoin;
}

void StartupLifeCtr(LIFECTR* plifectr)
{
	plifectr->pvtlifectr = &g_vtlifectr;
}

LIFETKN* NewLifetkn()
{
	return new LIFETKN{};
}

int GetLifetknSize()
{
	return sizeof(LIFETKN);
}

void CloneLifetkn(LIFETKN* plifetkn, LIFETKN* plifetknBase)
{
	CloneDprize(plifetkn, plifetknBase);
}

void DeleteLifetkn(LIFETKN* plifetkn)
{
	delete plifetkn;
}

void StartupKeyCtr(KEYCTR* pkeyctr)
{
	pkeyctr->pvtkeyctr = &g_vtkeyctr;
}

KEY* NewKey()
{
	return new KEY{};
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
	CloneDprize(pkey, pkeyBase);
}

void DeleteKey(KEY* pkey)
{
	delete pkey;
}

void StartupGoldCtr(GOLDCTR* pgoldctr)
{
	pgoldctr->pvtgoldctr = &g_vtgoldctr;
}

GOLD* NewGold()
{
	return new GOLD{};
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
	CloneDprize(pgold, pgoldBase);
}

void DeleteGold(GOLD* pgold)
{
	delete pgold;
}

LIFECTR g_lifectr;
COINCTR g_coinctr;
GOLDCTR g_goldctr;
KEYCTR  g_keyctr;
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
	
	pdprize->ficg.grficBomb  = '\x10';
	pdprize->ficg.grficSmash = '\x10';
	pdprize->ficg.grficRush  = '\x10';
	pdprize->ficg.grficSweep = '\x10';
}

int GetDprizeSize()
{
	return sizeof(DPRIZE);
}

void SnipDprizeObject(DPRIZE* pdprize)
{
	SNIP &snip = s_asnipDprize[0];
	uint32_t grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

	if ((snip.grfsnip & 0x20) == 0)
		grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

	LO* plo = PloFindSwObject(pdprize->psw, grffso, snip.oid, pdprize);

	if (plo != nullptr)
	{
		if ((snip.grfsnip & 0x08) == 0)
			pdprize->psm = (SM*)plo;

		if ((snip.grfsnip & 0x04) == 0)
			SnipLo(plo);

		if ((snip.grfsnip & 0x10) != 0)
			SubscribeLoObject(plo, pdprize);
	}

	snip = s_asnipDprize[1];
	grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

	if ((snip.grfsnip & 0x20) == 0)
		grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

	plo = PloFindSwObject(pdprize->psw, grffso, snip.oid, pdprize);

	if (plo != nullptr)
	{
		if ((snip.grfsnip & 0x08) == 0)
			pdprize->ppntFrontGlint = (PNT*)plo;

		if ((snip.grfsnip & 0x04) == 0)
			SnipLo(plo);

		if ((snip.grfsnip & 0x10) != 0)
			SubscribeLoObject(plo, pdprize);
	}

	snip = s_asnipDprize[2];
	grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

	if ((snip.grfsnip & 0x20) == 0)
		grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

	plo = PloFindSwObject(pdprize->psw, grffso, snip.oid, pdprize);

	if (plo != nullptr)
	{
		if ((snip.grfsnip & 0x08) == 0)
			pdprize->ppntBackGlint = (PNT*)plo;

		if ((snip.grfsnip & 0x04) == 0)
			SnipLo(plo);

		if ((snip.grfsnip & 0x10) != 0)
			SubscribeLoObject(plo, pdprize);
	}

	snip = s_asnipDprize[3];
	grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

	if ((snip.grfsnip & 0x20) == 0)
		grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

	plo = PloFindSwObject(pdprize->psw, grffso, snip.oid, pdprize);

	if (plo != nullptr)
	{
		if ((snip.grfsnip & 0x08) == 0)
			pdprize->pexplCollect = (EXPL*)plo;

		if ((snip.grfsnip & 0x04) == 0)
			SnipLo(plo);

		if ((snip.grfsnip & 0x10) != 0)
			SubscribeLoObject(plo, pdprize);
	}

	snip = s_asnipDprize[4];
	grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

	if ((snip.grfsnip & 0x20) == 0)
		grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

	plo = PloFindSwObject(pdprize->psw, grffso, snip.oid, pdprize);

	if (plo != nullptr)
	{
		if ((snip.grfsnip & 0x08) == 0)
			pdprize->pexplAttract = (EXPL*)plo;

		if ((snip.grfsnip & 0x04) == 0)
			SnipLo(plo);

		if ((snip.grfsnip & 0x10) != 0)
			SubscribeLoObject(plo, pdprize);
	}
}

void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis)
{
	SetAloTargetHitTest(pdprize, 1);
	LoadAloFromBrx(pdprize, pbis);
	SnipDprizeObject(pdprize);
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

void* GetDprize(DPRIZE* pdprize)
{
	return &pdprize->dprizes;
}

void* GetTdprizes(DPRIZE* pdprize)
{
	return &pdprize->tDprizes;
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

	pcharm->sRadiusCollect = 60.0;
	pcharm->uGlintChance = 0.35;
	pcharm->svLastBounceMax = 300.0;
	pcharm->rxyBounce = 0.75;
	pcharm->rzBounce = 0.5;
	pcharm->sRadiusBounce = 60.0;
	pcharm->svLastBounce = 0.0;
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

	pcoin->sRadiusBounce = 45.0;
	pcoin->lmDtMaxLifetime.gMax = 10.0;
	pcoin->svLastBounceMax = 250.0;
	pcoin->svLastBounce = 50.0;
	pcoin->rxyBounce = 0.6;
	pcoin->rzBounce = 0.75;
	pcoin->uGlintChance = 0.25;
	pcoin->lmDtMaxLifetime.gMin = 8.0;
	pcoin->sRadiusCollect = 50.0;
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

void StartupGoldCtr(GOLDCTR *pgoldctr)
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
	pgold->sRadiusCollect = 60.0;
	pgold->svLastBounceMax = 500.0;
	pgold->svLastBounce = 250.0;
	pgold->rzBounce = 0.6;
	pgold->uGlintChance = 0.75;
	pgold->dprizesInit = DPRIZES_Removed;
	pgold->sRadiusBounce = 60.0;
	pgold->rxyBounce = 0.6;
	pgold->psw->cgoldAll++;
}

int GetGoldSize()
{
	return sizeof(GOLD);
}

void CloneGold(GOLD* pgold, GOLD* pgoldBase)
{
	CloneDprize(pgold, pgoldBase);

	pgold->fStartTimedChallenge = pgoldBase->fStartTimedChallenge;
}

void DeleteGold(GOLD* pgold)
{
	delete pgold;
}

SNIP s_asnipDprize[5] = 
{
	{ 0,  OID_sm_dprize,          offsetof(DPRIZE, psm)},
	{ 2,  OID_dprize_front_glint, offsetof(DPRIZE, ppntFrontGlint)},
	{ 2,  OID_dprize_back_glint,  offsetof(DPRIZE, ppntBackGlint)},
	{ 2,  OID_xs_dprize_collect,  offsetof(DPRIZE, pexplCollect)},
	{ 2,  OID_xs_dprize_attract,  offsetof(DPRIZE, pexplAttract)}
};

KEYCTR g_keyctr;
COINCTR g_coinctr;
GOLDCTR g_goldctr;
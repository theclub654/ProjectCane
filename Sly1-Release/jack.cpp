#include "jack.h"

JACK* NewJack()
{
	return new JACK{};
}

void InitJack(JACK* pjack)
{
	InitPo(pjack);
}

int GetJackSize()
{
	return sizeof(JACK);
}

void CloneJack(JACK* pjack, JACK* pjackBase)
{
    ClonePo(pjack, pjackBase);

    pjack->jacks = pjackBase->jacks;
    pjack->tJacks = pjackBase->tJacks;
    pjack->pasegaCur = pjackBase->pasegaCur;
    pjack->pasegStop = pjackBase->pasegStop;
    pjack->pasegRim = pjackBase->pasegRim;
    pjack->pasegGap = pjackBase->pasegGap;
    pjack->pasegZap = pjackBase->pasegZap;
    pjack->pasegDead = pjackBase->pasegDead;
    pjack->pasegCelebrate = pjackBase->pasegCelebrate;
    pjack->pjackf = pjackBase->pjackf;
    pjack->pjackb = pjackBase->pjackb;

    for (int i = 0; i < 32; ++i)
    {
        pjack->apjackn[i] = pjackBase->apjackn[i];
    }

    for (int i = 0; i < 4; ++i)
    {
        pjack->apjackfDrop[i] = pjackBase->apjackfDrop[i];
    }

    pjack->cjpr = pjackBase->cjpr;
    for (int i = 0; i < 6; ++i)
    {
        pjack->ajpr[i] = pjackBase->ajpr[i];
    }

    pjack->iCur = pjackBase->iCur;
    pjack->jCur = pjackBase->jCur;
    pjack->iNext = pjackBase->iNext;
    pjack->jNext = pjackBase->jNext;
    pjack->uCur = pjackBase->uCur;
    pjack->radTarget = pjackBase->radTarget;
    pjack->cpjacknActive = pjackBase->cpjacknActive;
    pjack->cpjacknTarget = pjackBase->cpjacknTarget;
    pjack->tNextJackn = pjackBase->tNextJackn;
    pjack->pshdTop = pjackBase->pshdTop;
    pjack->pshdSides = pjackBase->pshdSides;
    pjack->pshdGap = pjackBase->pshdGap;
    pjack->matPosToUv = pjackBase->matPosToUv;
}

void DeleteJack(JACK* pjack)
{
	delete pjack;
}

JACKB* NewJackb()
{
	return new JACKB{};
}

void InitJackb(JACKB* pjackb)
{
	InitAlo(pjackb);
}

int GetJackbSize()
{
	return sizeof(JACKB);
}

void CloneJackb(JACKB* pjackb, JACKB* pjackbBase)
{
    CloneAlo(pjackb, pjackbBase);

    pjackb->pjack = pjackbBase->pjack;
    pjackb->jbs = pjackbBase->jbs;
    pjackb->tJbs = pjackbBase->tJbs;
    pjackb->pasegaCur = pjackbBase->pasegaCur;
    pjackb->jbsNext = pjackbBase->jbsNext;
    pjackb->radTarget = pjackbBase->radTarget;

    // Clone ablWalk array
    for (int i = 0; i < 2; ++i)
    {
        pjackb->ablWalk[i] = pjackbBase->ablWalk[i];
    }

    pjackb->pasegblWalk = pjackbBase->pasegblWalk;
    pjackb->pasegSpike = pjackbBase->pasegSpike;
    pjackb->pasegFly = pjackbBase->pasegFly;
    pjackb->pasegTaunt = pjackbBase->pasegTaunt;
    pjackb->ppntLeftWingTip = pjackbBase->ppntLeftWingTip;
    pjackb->ppntRightWingTip = pjackbBase->ppntRightWingTip;
    pjackb->jbws = pjackbBase->jbws;
    pjackb->jbwsNext = pjackbBase->jbwsNext;
    pjackb->posPlant = pjackbBase->posPlant;
    pjackb->posNextPlant = pjackbBase->posNextPlant;
}

void DeleteJackb(JACKB* pjackb)
{
	delete pjackb;
}

JACKN* NewJackn()
{
	return new JACKN{};
}

void InitJackn(JACKN* pjackn)
{
	InitAlo(pjackn);
}

int GetJacknSize()
{
	return sizeof(JACKN);
}

void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjackn, pbis);
}

void CloneJackn(JACKN* pjackn, JACKN* pjacknBase)
{
    CloneAlo(pjackn, pjacknBase);

    pjackn->pjack = pjacknBase->pjack;
    pjackn->pjackf = pjacknBase->pjackf;
    pjackn->cpaloRender = pjacknBase->cpaloRender;

    // Clone apaloRender array
    for (int i = 0; i < 16; ++i)
    {
        pjackn->apaloRender[i] = pjacknBase->apaloRender[i];
    }

    pjackn->svu = pjacknBase->svu;
    pjackn->u = pjacknBase->u;
    pjackn->fGap = pjacknBase->fGap;
}

void UpdateJackn(JACKN* pjackn, float dt)
{
	pjackn->pvtalo->pfnTranslateAloToPos(pjackn, pjackn->xf.pos);
}

void RenderJacknSelf(JACKN* pjackn, CM* pcm, RO* pro)
{

}

void DeleteJackn(JACKN* pjackn)
{
	delete pjackn;
}

JACKF* NewJackf()
{
	return new JACKF{};
}

void InitJackf(JACKF* pjackf)
{
	InitAlo(pjackf);
}

int GetJackfSize()
{
	return sizeof(JACKF);
}

void CloneJackf(JACKF* pjackf, JACKF* pjackfBase)
{
    CloneAlo(pjackf, pjackfBase);

    pjackf->pjack = pjackfBase->pjack;
    pjackf->cjeRim = pjackfBase->cjeRim;

    // Clone ajeRim array
    if (pjackf->cjeRim > 0)
    {
        pjackf->ajeRim = new JE[pjackf->cjeRim];
        for (int i = 0; i < pjackf->cjeRim; ++i)
        {
            pjackf->ajeRim[i] = pjackfBase->ajeRim[i];
        }
    }

    // Clone ajepRim array
    /*if (pjackf->cjeRim > 0)
    {
        pjackf->ajepRim = new JEP[pjackf->cjeRim];
        for (int i = 0; i < pjackf->cjeRim; ++i)
        {
            pjackf->ajepRim[i] = pjackfBase->ajepRim[i];
        }
    }*/

    pjackf->cjeGap = pjackfBase->cjeGap;

    // Clone ajeGap array
    if (pjackf->cjeGap > 0)
    {
        pjackf->ajeGap = new JE[pjackf->cjeGap];
        for (int i = 0; i < pjackf->cjeGap; ++i)
        {
            pjackf->ajeGap[i] = pjackfBase->ajeGap[i];
        }
    }

    pjackf->aaajk = pjackfBase->aaajk;
    pjackf->tDrop = pjackfBase->tDrop;
    pjackf->dvDrop = pjackfBase->dvDrop;
    pjackf->c = pjackfBase->c;
}

void RenderJackfSelf(JACKF* pjackf, CM* pcm, RO* pro)
{
	RenderAloSelf(pjackf, pcm, pro);
}

void DeleteJackf(JACKF* pjackf)
{
	delete pjackf;
}
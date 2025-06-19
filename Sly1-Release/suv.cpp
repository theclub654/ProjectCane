#include "suv.h"

SUV* NewSuv()
{
	return new SUV{};
}

void InitSuv(SUV* psuv)
{
	InitPo(psuv);
}

int GetSuvSize()
{
	return sizeof(SUV);
}

void UpdateSuvXfWorld(SUV* psuv)
{
	UpdateSoXfWorld(psuv);
}

void RenderSuvSelf(SUV* psuv, CM* pcm, RO* pro)
{
	RenderSoSelf(psuv, pcm, pro);
}

void CloneSuv(SUV* psuv, SUV* psuvBase)
{
    ClonePo(psuv, psuvBase);

    psuv->sRadiusFrontWheel = psuvBase->sRadiusFrontWheel;
    psuv->sRadiusRearWheel = psuvBase->sRadiusRearWheel;
    psuv->svMax = psuvBase->svMax;
    psuv->dyMax = psuvBase->dyMax;
    psuv->clqTune = psuvBase->clqTune;

    for (int i = 0; i < 2; ++i)
        psuv->asvrb[i] = psuvBase->asvrb[i];

    psuv->svrb = psuvBase->svrb;
    psuv->paloShadow = psuvBase->paloShadow;
    psuv->radTarget = psuvBase->radTarget;
    psuv->svTarget = psuvBase->svTarget;
    psuv->radFront = psuvBase->radFront;
    psuv->xsxp = psuvBase->xsxp;

    for (int i = 0; i < 4; ++i)
        psuv->asxp[i] = psuvBase->asxp[i];

    psuv->pshapeTrack = psuvBase->pshapeTrack;
    psuv->sTrackMax = psuvBase->sTrackMax;
    psuv->uTrack = psuvBase->uTrack;
    psuv->sTrack = psuvBase->sTrack;
    psuv->dyTrack = psuvBase->dyTrack;
    psuv->dyTarget = psuvBase->dyTarget;
    psuv->dsTrackFinish = psuvBase->dsTrackFinish;
    psuv->pshapeLine = psuvBase->pshapeLine;
    psuv->uLine = psuvBase->uLine;
    psuv->tUpright = psuvBase->tUpright;
    psuv->tBoost = psuvBase->tBoost;
    psuv->cBoost = psuvBase->cBoost;
    psuv->rsvBalance = psuvBase->rsvBalance;
    psuv->rsvGoal = psuvBase->rsvGoal;
    psuv->tBalance = psuvBase->tBalance;
    psuv->muSxp = psuvBase->muSxp;
    psuv->tPunched = psuvBase->tPunched;
    psuv->csve = psuvBase->csve;

    for (int i = 0; i < 16; ++i)
        psuv->asve[i] = psuvBase->asve[i];

    psuv->psveCheckFirst = psuvBase->psveCheckFirst;
    psuv->psveCheckCur = psuvBase->psveCheckCur;
    psuv->psveFeatureCur = psuvBase->psveFeatureCur;
    psuv->cLap = psuvBase->cLap;
    psuv->cLapMax = psuvBase->cLapMax;
    psuv->nPlace = psuvBase->nPlace;
    psuv->nPlaceMax = psuvBase->nPlaceMax;
    psuv->tPlace = psuvBase->tPlace;
    psuv->pemitterBoost = psuvBase->pemitterBoost;
    psuv->pexplDirt = psuvBase->pexplDirt;
    psuv->pexplDust = psuvBase->pexplDust;
    psuv->cParticleDirt = psuvBase->cParticleDirt;
    psuv->cParticleDust = psuvBase->cParticleDust;
    psuv->paloFrontAxle = psuvBase->paloFrontAxle;
    psuv->paloRearAxle = psuvBase->paloRearAxle;
    psuv->suvgk = psuvBase->suvgk;
    psuv->suvs = psuvBase->suvs;
    psuv->tSuvs = psuvBase->tSuvs;
    psuv->ppathzone = psuvBase->ppathzone;
    psuv->psoPrizeCur = psuvBase->psoPrizeCur;
    psuv->psmPuncher = psuvBase->psmPuncher;
    psuv->psmaPuncher = psuvBase->psmaPuncher;
    psuv->psoPuncher = psuvBase->psoPuncher;
    psuv->cpsoIgnore = psuvBase->cpsoIgnore;

    for (int i = 0; i < 8; ++i)
        psuv->apsoIgnore[i] = psuvBase->apsoIgnore[i];

    psuv->pxpPuncher = psuvBase->pxpPuncher;
    psuv->pmurray = psuvBase->pmurray;
    psuv->fFreeWheeling = psuvBase->fFreeWheeling;
    psuv->pambSkid = psuvBase->pambSkid;
    psuv->pambRunning = psuvBase->pambRunning;
    psuv->pambIdle = psuvBase->pambIdle;
    psuv->pambBoost = psuvBase->pambBoost;
    psuv->volBoostTarget = psuvBase->volBoostTarget;
    psuv->volBoostCur = psuvBase->volBoostCur;
    psuv->volTarget = psuvBase->volTarget;
    psuv->volCur = psuvBase->volCur;
    psuv->dvolInc = psuvBase->dvolInc;
    psuv->frqTarget = psuvBase->frqTarget;
    psuv->frqCur = psuvBase->frqCur;
    psuv->dfrqInc = psuvBase->dfrqInc;
    psuv->dfrqTop = psuvBase->dfrqTop;
}

void DeleteSuv(SUV* psuv)
{
	delete psuv;
}

void StartupLapCtr(LAPCTR* plapctr)
{
    plapctr->pvtlapctr = &g_vtlapctr;
}

void StartupBoostCtr(BOOSTCTR* pboostctr)
{
    pboostctr->pvtboostctr = &g_vtboostctr;
}

void StartupPlaceCtr(PLACECTR* placectr)
{
    placectr->pvtplacectr = &g_vtplacectr;
}

LAPCTR g_lapctr;
BOOSTCTR g_boostctr;
PLACECTR g_placectr;
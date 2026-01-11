#include "rat.h"

RAT* NewRat()
{
	return new RAT{};
}

void InitSwRatDl(SW* psw)
{
	InitDl(&psw->dlRat, offsetof(RAT, dleRat));
}

void InitRat(RAT* prat)
{
	InitSo(prat);
}

int GetRatSize()
{
	return sizeof(RAT);
}

void OnRatAdd(RAT* prat)
{
	OnSoAdd(prat);
	AppendDlEntry(&prat->psw->dlRat, prat);
}

void OnRatRemove(RAT* prat)
{
	OnSoRemove(prat);
	RemoveDlEntry(&prat->psw->dlRat, prat);
}

void CloneRat(RAT* prat, RAT* pratBase)
{
    CloneSo(prat, pratBase);

    // Shallow copy of the value members
    prat->tRats = pratBase->tRats;
    prat->tRatsNext = pratBase->tRatsNext;
    prat->tRatSqueak = pratBase->tRatSqueak;
    prat->ipaloRender = pratBase->ipaloRender;
    prat->cpaloRenderScurry = pratBase->cpaloRenderScurry;
    prat->cpaloRenderFlee = pratBase->cpaloRenderFlee;
    prat->dsFrameScurry = pratBase->dsFrameScurry;
    prat->dsFrameFlee = pratBase->dsFrameFlee;
    prat->sScurry = pratBase->sScurry;
    prat->svScurry = pratBase->svScurry;
    prat->svFlee = pratBase->svFlee;
    prat->svCur = pratBase->svCur;
    prat->svLeap = pratBase->svLeap;
    prat->sGroupRadius = pratBase->sGroupRadius;
    prat->dposTarget = pratBase->dposTarget;
    prat->posWanderNext = pratBase->posWanderNext;
    prat->posFrameLast = pratBase->posFrameLast;
    prat->ccoin = pratBase->ccoin;

    // Shallow copy of the array members (assuming they're pointers)
    for (int i = 0; i < 16; ++i) {
        prat->apaloRenderScurry[i] = pratBase->apaloRenderScurry[i];
        prat->apaloRenderFlee[i] = pratBase->apaloRenderFlee[i];
    }

    // Shallow copy of the pointer members (e.g., pointers to ALO, EXPL, TARGET, etc.)
    prat->paloRenderStopped = pratBase->paloRenderStopped;
    prat->pexpl = pratBase->pexpl;
    prat->ptarget = pratBase->ptarget;
    prat->prathole = pratBase->prathole;

    // Shallow copy of the DLE structure
    prat->dleRat = pratBase->dleRat;

    // Shallow copy of the LM structure
    prat->lmDtStop = pratBase->lmDtStop;
}

void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prat, pbis);
}

void PostRatLoad(RAT* prat)
{
    PostAloLoad(prat);
}

void UpdateRat(RAT* prat, float dt)
{
    UpdateSo(prat, dt);
}

void RenderRatAll(RAT* prat, CM* pcm, RO* pro)
{
	RenderAloAll(prat, pcm, pro);
	//prat->pvtalo->pfnRenderAloAll(prat, pcm, pro);
}

void DeleteRat(RAT *prat)
{
	delete prat;
}

RATHOLE* NewRathole()
{
	return new RATHOLE{};
}

void InitSwRatholeDl(SW* psw)
{
	InitDl(&psw->dlRathole, offsetof(RATHOLE, dleRathole));
}

int GetRatholeSize()
{
	return sizeof(RATHOLE);
}

void OnRatholeRemove(RATHOLE* prathole)
{
	OnLoRemove((LO*)prathole);
	RemoveDlEntry(&prathole->psw->dlRathole, prathole);
}

void CloneRathole(RATHOLE* prathole, RATHOLE* pratholeBase)
{
    CloneVol(prathole, pratholeBase);

    prathole->dleRathole = pratholeBase->dleRathole;
}

void OnRatholeAdd(RATHOLE* prathole)
{
	OnLoAdd(prathole);
	AppendDlEntry(&prathole->psw->dlRathole, prathole);
}

void DeleteRathole(RATHOLE* prathole)
{
	delete prathole;
}
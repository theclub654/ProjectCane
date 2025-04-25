#include "mouthgame.h"

MGC* NewMgc()
{
	return new MGC{};
}

void InitMgc(MGC* pmgc)
{
	InitAlo(pmgc);
	InitDl(&pmgc->dlMgcoFree, offsetof(MGCO, dle));
	InitDl(&pmgc->dlMgco, offsetof(MGCO, dle));
}

int GetMgcSize()
{
	return sizeof(MGC);
}

void CloneMgc(MGC* pmgc, MGC* pmgcBase)
{
    CloneAlo(pmgc, pmgcBase);

    pmgc->psmGenerate = pmgcBase->psmGenerate;
    pmgc->psmaGenerate = pmgcBase->psmaGenerate;
    pmgc->psmCollect = pmgcBase->psmCollect;
    pmgc->psmaCollect = pmgcBase->psmaCollect;
    pmgc->pasegPath = pmgcBase->pasegPath;
    pmgc->pvolSmash = pmgcBase->pvolSmash;
    pmgc->pvolHole = pmgcBase->pvolHole;
    pmgc->dlMgcoFree = pmgcBase->dlMgcoFree;
    pmgc->dlMgco = pmgcBase->dlMgco;
    pmgc->fDamaging = pmgcBase->fDamaging;
    pmgc->pexplGood = pmgcBase->pexplGood;
    pmgc->pexplBad = pmgcBase->pexplBad;
    pmgc->pmgv = pmgcBase->pmgv;
}

void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pmgc, pbis);
}

void DeleteMgc(MGC* pmgc)
{
	delete pmgc;
}

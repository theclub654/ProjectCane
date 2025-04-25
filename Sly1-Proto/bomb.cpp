#include "bomb.h"

BOMB*NewBomb()
{
	return new BOMB{};
}

void InitBomb(BOMB* pbomb)
{
	InitSo(pbomb);
}

int GetBombSize()
{
	return sizeof(BOMB);
}

void OnBombAdd(BOMB* pbomb)
{
	OnSoAdd(pbomb);
}

void CloneBomb(BOMB* pbomb, BOMB* pbombBase)
{
    CloneSo(pbomb, pbombBase);

    pbomb->bdt = pbombBase->bdt;
    pbomb->tDetonate = pbombBase->tDetonate;
    pbomb->sDetonateRadius = pbombBase->sDetonateRadius;
    pbomb->sDetonateRadiusJt = pbombBase->sDetonateRadiusJt;
    pbomb->fTouchDamage = pbombBase->fTouchDamage;
    pbomb->tDelay = pbombBase->tDelay;
    pbomb->iDetonationProp = pbombBase->iDetonationProp;
    pbomb->sdvMax = pbombBase->sdvMax;
    pbomb->lmMass = pbombBase->lmMass;
    pbomb->mMax = pbombBase->mMax;
    pbomb->cpbrkp = pbombBase->cpbrkp;

    for (int i = 0; i < 16; ++i)
        pbomb->apbrkp[i] = pbombBase->apbrkp[i];

    pbomb->pexpl = pbombBase->pexpl;
    pbomb->rExplodeScale = pbombBase->rExplodeScale;
    pbomb->clqRadiusToSv = pbombBase->clqRadiusToSv;
    pbomb->lmSv = pbombBase->lmSv;
    pbomb->clqRadiusToSw = pbombBase->clqRadiusToSw;
    pbomb->lmSw = pbombBase->lmSw;
    pbomb->posDetonate = pbombBase->posDetonate;
    pbomb->normalDetonate = pbombBase->normalDetonate;
    pbomb->fExplodeEffects = pbombBase->fExplodeEffects;
    pbomb->fReclaim = pbombBase->fReclaim;
    pbomb->zpk = pbombBase->zpk;
    pbomb->psfxDet = pbombBase->psfxDet;
}

void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbomb, pbis);
}

void DeleteBomb(BOMB *pbomb)
{
	delete pbomb;
}

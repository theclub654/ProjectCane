#include "squish.h"

SQUISH* NewSquish()
{
	return new SQUISH{};
}

int GetSquishSize()
{
	return sizeof(SQUISH);
}

void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(psquish, pbis);
}

void CloneSquish(SQUISH* psquish, SQUISH* psquishBase)
{
    CloneSo(psquish, psquishBase);

    psquish->elasLast = psquishBase->elasLast;
    psquish->sdvSquish = psquishBase->sdvSquish;
    psquish->tSquish = psquishBase->tSquish;
    psquish->dtSquish = psquishBase->dtSquish;
    psquish->normalSquish = psquishBase->normalSquish;
    psquish->clqDtImpact = psquishBase->clqDtImpact;
    psquish->lmDtImpact = psquishBase->lmDtImpact;
    psquish->clqSquishImpact = psquishBase->clqSquishImpact;
    psquish->lmSquishImpact = psquishBase->lmSquishImpact;
    psquish->clqStretchImpact = psquishBase->clqStretchImpact;
    psquish->lmStretchImpact = psquishBase->lmStretchImpact;
    psquish->clqSquishMoving = psquishBase->clqSquishMoving;
    psquish->lmSquishMoving = psquishBase->lmSquishMoving;
    psquish->clqStretchMoving = psquishBase->clqStretchMoving;
    psquish->lmStretchMoving = psquishBase->lmStretchMoving;
}

void RenderSquishSelf(SQUISH* psquish, CM* pcm, RO* pro)
{
	RenderSoSelf(psquish, pcm, pro);
}

void DeleteSquish(SQUISH *psquish)
{
	delete psquish;
}

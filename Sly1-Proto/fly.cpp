#include "fly.h"

FLY* NewFly()
{
	return new FLY{};
}

void InitSwFlyDl(SW* psw)
{
	InitDl(&psw->dlFly, offsetof(FLY, dleFly));
}

void InitFly(FLY* pfly)
{
	InitSo(pfly);
	AppendDlEntry(&pfly->psw->dlFly, pfly);
}

int GetFlySize()
{
	return sizeof(FLY);
}

void CloneFly(FLY* pfly, FLY* pflyBase)
{
    CloneSo(pfly, pflyBase);

    pfly->flyk = pflyBase->flyk;
    pfly->flys = pflyBase->flys;
    pfly->tFlys = pflyBase->tFlys;
    pfly->tFlysNext = pflyBase->tFlysNext;
    pfly->flysInit = pflyBase->flysInit;
    pfly->cpaloRender = pflyBase->cpaloRender;
    pfly->ipaloRender = pflyBase->ipaloRender;
    std::memcpy(pfly->apaloRender, pflyBase->apaloRender, sizeof(pflyBase->apaloRender));
    pfly->cpaloRenderGround = pflyBase->cpaloRenderGround;
    std::memcpy(pfly->apaloRenderGround, pflyBase->apaloRenderGround, sizeof(pflyBase->apaloRenderGround));
    pfly->paloRenderDead = pflyBase->paloRenderDead;
    pfly->pexpl = pflyBase->pexpl;
    pfly->ptarget = pflyBase->ptarget;
    pfly->dleFly = pflyBase->dleFly;
    pfly->pambBuzz = pflyBase->pambBuzz;
    pfly->posWanderNext = pflyBase->posWanderNext;
    pfly->vWanderNext = pflyBase->vWanderNext;
    pfly->lmSCylinderRadius = pflyBase->lmSCylinderRadius;
    pfly->lmSCylinderHeight = pflyBase->lmSCylinderHeight;
    pfly->lmSvxyFlight = pflyBase->lmSvxyFlight;
    pfly->lmSvzFlight = pflyBase->lmSvzFlight;
    pfly->oidSoar = pflyBase->oidSoar;
    pfly->ipaloRenderSoar = pflyBase->ipaloRenderSoar;
    pfly->uSoar = pflyBase->uSoar;
    pfly->lmDtFlying = pflyBase->lmDtFlying;
    pfly->dtFlyingFrame = pflyBase->dtFlyingFrame;
    pfly->tFlyingFrameNext = pflyBase->tFlyingFrameNext;
    pfly->sGroundMax = pflyBase->sGroundMax;
    pfly->lmDtGround = pflyBase->lmDtGround;
    pfly->lmDtGroundFrame = pflyBase->lmDtGroundFrame;
    pfly->lmDtGroundFramePause = pflyBase->lmDtGroundFramePause;
    pfly->tGroundFrameNext = pflyBase->tGroundFrameNext;
    pfly->tGroundTestLast = pflyBase->tGroundTestLast;
    pfly->lmDtDroppings = pflyBase->lmDtDroppings;
    pfly->pexplsDroppings = pflyBase->pexplsDroppings;
    pfly->ppntDroppings = pflyBase->ppntDroppings;
    pfly->tDroppingsNext = pflyBase->tDroppingsNext;
    pfly->sFlee = pflyBase->sFlee;
    pfly->tLandNext = pflyBase->tLandNext;
    pfly->diRender = pflyBase->diRender;
    pfly->ccoin = pflyBase->ccoin;
}

void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pfly, pbis);
}

void RenderFlySelf(FLY* pfly, CM* pcm, RO* pro)
{
	RenderSoSelf(pfly, pcm, pro);
	//pfly->pvtalo->pfnRenderAloAll(pfly, pcm, pro);
}

void DeleteFly(FLY *pfly)
{
	delete pfly;
}

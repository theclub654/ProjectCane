#include "jt.h"

JT* NewJt()
{
	return new JT{};
}

void InitJt(JT* pjt)
{
	InitStep(pjt);
	pjt->jts = JTS_Nil;
	pjt->jtcs = JTCS_Nil;
	pjt->jtbs = JTBS_Nil;
	pjt->jtjk = JTJK_Nil;
}

void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjt, pbis);
	pjt->tSweepPending = -1.0;
	pjt->tJumpPending = -1.0;
	g_pjt = pjt;
}

void CloneJt(JT* pjt, JT* pjtBase)
{
    CloneStep(pjt, pjtBase);

    // Clone arrays like ablrunRun, ablssSidestep, ablssFlatten, etc.
    for (int i = 0; i < 6; ++i)
    {
        pjt->ablrunRun[i] = pjtBase->ablrunRun[i];
        pjt->ablrunBasket[i] = pjtBase->ablrunBasket[i];
        pjt->ablrunRail[i] = pjtBase->ablrunRail[i];
    }

    for (int i = 0; i < 256; ++i)
    {
        pjt->ablssSidestep[i] = pjtBase->ablssSidestep[i];
        pjt->ablssFlatten[i] = pjtBase->ablssFlatten[i];
    }

    for (int i = 0; i < 8; ++i)
    {
        pjt->ablssDuck[i] = pjtBase->ablssDuck[i];
    }

    for (int i = 0; i < 2; ++i)
    {
        pjt->ablcl[i] = pjtBase->ablcl[i];
    }

    for (int i = 0; i < 57; ++i)
    {
        pjt->mpjtbspaseg[i] = pjtBase->mpjtbspaseg[i];
    }

    for (int i = 0; i < 15; ++i)
    {
        pjt->mpjtspso[i] = pjtBase->mpjtspso[i];
        pjt->mpjthkpso[i] = pjtBase->mpjthkpso[i];
    }

    // Clone simple variables
    pjt->fStandTight = pjtBase->fStandTight;
    pjt->xmgLeftBall = pjtBase->xmgLeftBall;
    pjt->xmgRightBall = pjtBase->xmgRightBall;
    pjt->fRenderCaneTrail = pjtBase->fRenderCaneTrail;
    pjt->tSweepPending = pjtBase->tSweepPending;
    pjt->cposHook = pjtBase->cposHook;

    // Clone arrays of glm::vec3
    for (int i = 0; i < 3; ++i)
    {
        pjt->aposHook[i] = pjtBase->aposHook[i];
    }

    pjt->posSafeLast = pjtBase->posSafeLast;
    pjt->sqtrmCane = pjtBase->sqtrmCane;

    // Clone pointers (assuming these need shallow copying unless deeper copying is necessary)
    pjt->ptargetCur = pjtBase->ptargetCur;
    pjt->phndCur = pjtBase->phndCur;
    pjt->ppipeCur = pjtBase->ppipeCur;
    pjt->phndUnhook = pjtBase->phndUnhook;
    pjt->ppipeUnhook = pjtBase->ppipeUnhook;
    pjt->pwater = pjtBase->pwater;
    pjt->paloShadow = pjtBase->paloShadow;

    // Clone other variables like float and int
    pjt->sPipeCur = pjtBase->sPipeCur;
    pjt->sPipeUnhook = pjtBase->sPipeUnhook;
    pjt->fPipeReverse = pjtBase->fPipeReverse;
    pjt->tJts = pjtBase->tJts;
    pjt->tJtbs = pjtBase->tJtbs;
    pjt->tJtcs = pjtBase->tJtcs;

    // Clone nested structs like ASEGA, SMA, etc.
    pjt->pasegaCur = pjtBase->pasegaCur;
    pjt->psmaCur = pjtBase->psmaCur;
    pjt->jtjk = pjtBase->jtjk;
    pjt->jsglk = pjtBase->jsglk;

    // Clone additional variables
    pjt->grfjtsc = pjtBase->grfjtsc;
    pjt->jtsNext = pjtBase->jtsNext;
    pjt->jtbsNext = pjtBase->jtbsNext;
    pjt->jtcsNext = pjtBase->jtcsNext;
    pjt->jtjkNext = pjtBase->jtjkNext;
    pjt->fShortenJump = pjtBase->fShortenJump;
    pjt->svxyAirSteer = pjtBase->svxyAirSteer;
    pjt->tJumpPending = pjtBase->tJumpPending;
    pjt->vTargetCoast = pjtBase->vTargetCoast;
    pjt->tHangDeflectLast = pjtBase->tHangDeflectLast;
    pjt->jtbsHangLatch = pjtBase->jtbsHangLatch;
    pjt->fHangReverse = pjtBase->fHangReverse;
    pjt->pxaHang = pjtBase->pxaHang;
    pjt->afxHang[0] = pjtBase->afxHang[0];
    pjt->afxHang[1] = pjtBase->afxHang[1];
    pjt->uSwingForward = pjtBase->uSwingForward;
    pjt->uSwingActive = pjtBase->uSwingActive;
    pjt->paloSkinStandard = pjtBase->paloSkinStandard;
    pjt->paloSkinElectric = pjtBase->paloSkinElectric;
    pjt->paloSkinFire = pjtBase->paloSkinFire;
    pjt->jthk = pjtBase->jthk;

    // Clone additional pointers and objects
    pjt->phshape = pjtBase->phshape;
    pjt->sParamHshape = pjtBase->sParamHshape;
    pjt->phpnt = pjtBase->phpnt;
    pjt->phbsk = pjtBase->phbsk;
    pjt->pjmtCur = pjtBase->pjmtCur;
    pjt->pspireCur = pjtBase->pspireCur;
    pjt->prailCur = pjtBase->prailCur;
    pjt->uRailCur = pjtBase->uRailCur;
    pjt->sRailCur = pjtBase->sRailCur;
    pjt->fRailReverse = pjtBase->fRailReverse;
    pjt->pvaultCur = pjtBase->pvaultCur;
    pjt->paloTool = pjtBase->paloTool;
    pjt->paloHook = pjtBase->paloHook;
    pjt->paloKey = pjtBase->paloKey;
    pjt->paloKeyCane = pjtBase->paloKeyCane;
    pjt->paloKeyFree = pjtBase->paloKeyFree;
    pjt->pexplsKeyTrail = pjtBase->pexplsKeyTrail;
    pjt->apmatchKey[0] = pjtBase->apmatchKey[0];
    pjt->apmatchKey[1] = pjtBase->apmatchKey[1];
    pjt->apmatchKey[2] = pjtBase->apmatchKey[2];
    pjt->apmatchKey[3] = pjtBase->apmatchKey[3];
    pjt->apmatchKey[4] = pjtBase->apmatchKey[4];
    pjt->apmatchKey[5] = pjtBase->apmatchKey[5];
    pjt->apmatchKey[6] = pjtBase->apmatchKey[6];
    pjt->apmatchKey[7] = pjtBase->apmatchKey[7];
    pjt->apmatchKey[8] = pjtBase->apmatchKey[8];
    pjt->cpmatchKey = pjtBase->cpmatchKey;
    pjt->posKeyLast = pjtBase->posKeyLast;
    pjt->zpk = pjtBase->zpk;
    pjt->posPipeHandLocal = pjtBase->posPipeHandLocal;
    pjt->posPipeFootLocal = pjtBase->posPipeFootLocal;
    pjt->dposPipeXp = pjtBase->dposPipeXp;
    pjt->sPipeHand = pjtBase->sPipeHand;
    pjt->dsPipeHand = pjtBase->dsPipeHand;
    pjt->lmPipeHandWindow = pjtBase->lmPipeHandWindow;
    pjt->sRadiusHook = pjtBase->sRadiusHook;
    pjt->tContact = pjtBase->tContact;
    pjt->pexplHook = pjtBase->pexplHook;
    pjt->swToolSpinTarget = pjtBase->swToolSpinTarget;
    pjt->swToolSpin = pjtBase->swToolSpin;
    pjt->radToolSpin = pjtBase->radToolSpin;
    pjt->pwater = pjtBase->pwater;
    pjt->paloShadow = pjtBase->paloShadow;
    pjt->sffRun = pjtBase->sffRun;
    pjt->sffSidestep = pjtBase->sffSidestep;
    pjt->sffStand = pjtBase->sffStand;
    pjt->sffFlatten = pjtBase->sffFlatten;
    pjt->sffDuck = pjtBase->sffDuck;

    // Clone complex structs like TAIL, EXPL, etc.
    pjt->ptailTail = pjtBase->ptailTail;
    pjt->ptailMask = pjtBase->ptailMask;
    pjt->fCanBoost = pjtBase->fCanBoost;
}

void RenderJtAll(JT* pjt, CM* pcm, RO* pro)
{
	RenderAloAll(pjt, pcm, pro);
}

void RenderJtSelf(JT* pjt, CM* pcm, RO* pro)
{

}

int GetJtSize()
{
	return sizeof(JT);
}

void DeleteJt(JT* pjt)
{
	delete pjt;
}

#include "lgn.h"

void StartupLgnr(LGNR* plgnr)
{
	plgnr->pvtlgnr = &g_vtlgnr;
}

void DrawLgnr(LGNR* plgnr)
{

}

LGN* NewLgn()
{
	return new LGN{};
}

void InitLgn(LGN* plgn)
{
	InitPo(plgn);
}

int GetLgnSize()
{
	return sizeof(LGN);
}

void CloneLgn(LGN* plgn, LGN* plgnBase)
{
	ClonePo(plgn, plgnBase);

	// Clone the LGNS structure
	plgn->lgns = plgnBase->lgns;

	// Clone the float tLgns
	plgn->tLgns = plgnBase->tLgns;

	// Clone the SM pointer
	plgn->psm = plgnBase->psm;

	// Clone the SMA pointer
	plgn->psma = plgnBase->psma;

	// Clone the ALO pointers (paloTurret, paloPlatform, paloJt, paloGut)
	plgn->paloTurret = plgnBase->paloTurret;
	plgn->paloPlatform = plgnBase->paloPlatform;
	plgn->paloJt = plgnBase->paloJt;
	plgn->paloGut = plgnBase->paloGut;

	// Clone the ACTADJ pointer
	plgn->pactadjGut = plgnBase->pactadjGut;

	// Clone the glm::vec3 posGutTurret
	plgn->posGutTurret = plgnBase->posGutTurret;

	// Clone the int fFlash
	plgn->fFlash = plgnBase->fFlash;

	// Clone the float tFlash, tCharm, tCharmPending
	plgn->tFlash = plgnBase->tFlash;
	plgn->tCharm = plgnBase->tCharm;
	plgn->tCharmPending = plgnBase->tCharmPending;

	// Clone the int fCharmEnabled
	plgn->fCharmEnabled = plgnBase->fCharmEnabled;

	// Clone the XP pointer pxpZap
	plgn->pxpZap = plgnBase->pxpZap;
}

void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro)
{
	RenderAloAll(plgn, pcm, pro);
}

void DeleteLgn(LGN *plgn)
{
	delete plgn;
}

LGNB* NewLgnb()
{
	return new LGNB{};
}

void InitLgnb(LGNB* plgnb)
{
	InitSo(plgnb);
}

int GetLgnbSize()
{
	return sizeof(LGNB);
}

void CloneLgnb(LGNB* plgnb, LGNB* plgnbBase)
{
	CloneSo(plgnb, plgnbBase);

	// Clone the SM pointer
	plgnb->psm = plgnbBase->psm;

	// Clone the SMA pointer
	plgnb->psma = plgnbBase->psma;

	// Clone the int fReset
	plgnb->fReset = plgnbBase->fReset;

	// Clone the float values
	plgnb->radPan = plgnbBase->radPan;
	plgnb->radTilt = plgnbBase->radTilt;
	plgnb->tWkr = plgnbBase->tWkr;
	plgnb->tAttack = plgnbBase->tAttack;
	plgnb->dtAttackMax = plgnbBase->dtAttackMax;

	// Clone the glm::vec3 posPursueLocal
	plgnb->posPursueLocal = plgnbBase->posPursueLocal;

	// Clone the PATHZONE pointer
	plgnb->ppathzone = plgnbBase->ppathzone;
}

void DeleteLgnb(LGNB *plgnb)
{
	delete plgnb;
}

SWP* NewSwp()
{
	return new SWP{};
}

void InitSwp(SWP* pswp)
{
	InitBreak(pswp);
}

int GetSwpSize()
{
	return sizeof(SWP);
}

void CloneSwp(SWP* pswp, SWP* pswpBase)
{
	CloneBreak(pswp, pswpBase);

	// Clone the SHAPE pointer
	pswp->pshape = pswpBase->pshape;

	// Clone the SM pointer
	pswp->psm = pswpBase->psm;

	// Clone the SMA pointer
	pswp->psma = pswpBase->psma;

	// Clone the SMP structure (assumes SMP is a struct with its own members)
	pswp->smpImpact = pswpBase->smpImpact;

	// Clone the float values
	pswp->sdvMax = pswpBase->sdvMax;
	pswp->tMax = pswpBase->tMax;
	pswp->svtLocal = pswpBase->svtLocal;
	pswp->tLocal = pswpBase->tLocal;

	// Clone the ACGB pointer
	pswp->pacgb = pswpBase->pacgb;

	// Clone the EXPL pointer
	pswp->pexplCrash = pswpBase->pexplCrash;
}

void DeleteSwp(SWP *pswp)
{
	delete pswp;
}

LGNR g_lgnr;
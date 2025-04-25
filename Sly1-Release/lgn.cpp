#include "lgn.h"

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
	LO lo = *plgnb;
	*plgnb = *plgnbBase;
	memcpy(plgnb, &lo, sizeof(LO));

	CloneLo(plgnb, plgnbBase);

	ClearDl(&plgnb->dlChild);

	plgnb->pxa = nullptr;
	plgnb->grfpvaXpValid = 0;
	plgnb->pstso = nullptr;
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
	LO lo = *pswp;
	*pswp = *pswpBase;
	memcpy(pswp, &lo, sizeof(LO));

	CloneLo(pswp, pswpBase);

	ClearDl(&pswp->dlChild);

	pswp->pxa = nullptr;
	pswp->grfpvaXpValid = 0;
	pswp->pstso = nullptr;
}

void DeleteSwp(SWP *pswp)
{
	delete pswp;
}

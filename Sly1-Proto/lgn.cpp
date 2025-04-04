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
	LO lo = *plgn;
	*plgn = *plgnBase;
	memcpy(plgn, &lo, sizeof(LO));

	CloneLo(plgn, plgnBase);

	ClearDl(&plgn->dlChild);

	plgn->pxa = nullptr;
	plgn->grfpvaXpValid = 0;
	plgn->pstso = nullptr;
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

#include "lgn.h"

void* NewLgn()
{
	return new LGN;
}

void InitLgn(LGN* plgn)
{
	InitPo(plgn);
}

void DeleteLgn(LO* plo)
{
	delete (LGN*)plo;
}

void* NewLgnb()
{
	return new LGNB;
}

void InitLgnb(LGNB* plgnb)
{
	InitSo(plgnb);
}

void DeleteLgnb(LO* plo)
{
	delete(LGNB*)plo;
}

void* NewSwp()
{
	return new SWP;
}

void InitSwp(SWP* pswp)
{
	InitBreak(pswp);
}

void DeleteSwp(LO* plo)
{
	delete (SWP*)plo;
}

void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro)
{

}

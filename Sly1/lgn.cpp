#include "lgn.h"

void InitLgn(LGN* plgn)
{
	std::cout << "LGN Size: " << sizeof(LGN) << "\n";
	InitPo(plgn);
}

void DeleteLgn(LO* plo)
{
	delete (LGN*)plo;
}

void InitLgnb(LGNB* plgnb)
{
	std::cout << "LGNB Size: " << sizeof(LGNB) << "\n";
	InitSo(plgnb);
}

void DeleteLgnb(LO* plo)
{
	delete(LGNB*)plo;
}

void InitSwp(SWP* pswp)
{
	std::cout << "SWP Size: " << sizeof(SWP) << "\n";
	InitBreak(pswp);
}

void DeleteSwp(LO* plo)
{
	delete (SWP*)plo;
}

void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro)
{

}

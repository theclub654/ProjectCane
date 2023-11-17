#include "lgn.h"

void InitLgn(LGN* plgn)
{
	std::cout << "LGN Size: " << sizeof(LGN) << "\n";
	InitPo(plgn);
}

void InitLgnb(LGNB* plgnb)
{
	std::cout << "LGNB Size: " << sizeof(LGNB) << "\n";
	InitSo(plgnb);
}

void InitSwp(SWP* pswp)
{
	std::cout << "SWP Size: " << sizeof(SWP) << "\n";
	InitBreak(pswp);
}

void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro)
{

}

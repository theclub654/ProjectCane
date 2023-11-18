#include "blip.h"

void InitBlipg(BLIPG* pblipg)
{
	//std::cout << "BLIPG Size: " << sizeof(BLIPG) << "\n";
	InitAlo(pblipg);
}

void OnBlipgAdd(BLIPG* pblipg)
{
	AppendDlEntry(&pblipg->psw->dlBlipg, pblipg);
	OnAloAdd(pblipg);
}

void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro)
{

}

void DeleteBlipg(LO* plo)
{
	delete(BLIPG*)plo;
}

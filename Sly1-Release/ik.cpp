#include "ik.h"

IKH* NewIkh()
{
	return new IKH{};
}

int GetIkhSize()
{
	return sizeof(IKH);
}

void CloneIkh(IKH* pikh, IKH* pikhBase)
{
	CloneSo(pikh, pikhBase);
}

void RenderIkhSelf(IKH* pikh, CM* pcm, RO* pro)
{
	RenderSoSelf(pikh, pcm, pro);
}

void DeleteIkh(IKH *pikh)
{
	delete pikh;
}

LIKH* NewLikh()
{
	return new LIKH{};
}

int GetLikhSize()
{
	return sizeof(LIKH);
}

void CloneLikh(LIKH* plikh, LIKH* plikhBase)
{
	CloneAlo(plikh, plikhBase);
}

void RenderLikhSelf(LIKH* plikh, CM* pcm, RO* pro)
{
	RenderAloSelf(plikh, pcm, pro);
}

void DeleteLikh(LIKH *plikh)
{
	delete plikh;
}

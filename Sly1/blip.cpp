#include "blip.h"

void* NewBlipg()
{
	return new BLIPG{};
}

void InitBlipg(BLIPG* pblipg)
{
	InitAlo(pblipg);
}

int GetBlipgSize()
{
	return sizeof(BLIPG);
}

void CloneBlipg(BLIPG* pblipg, BLIPG* pblipgBase)
{
	LO lo = *pblipg;
	*pblipg = *pblipgBase;
	memcpy(pblipg, &lo, sizeof(LO));

	CloneLo(pblipg, pblipgBase);

	ClearDl(&pblipg->dlChild);
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

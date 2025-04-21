#include "blip.h"

BLIPG* NewBlipg()
{
	return new BLIPG{};
}

void InitSwBlipgDl(SW* psw)
{
	InitDl(&psw->dlBlipg, offsetof(BLIPG, dleBlipg));
}

void InitSwBlipgFreeDl(SW* psw)
{
	InitDl(&psw->dlBlipgFree, offsetof(BLIPG, dleBlipg));
}

void InitBlipg(BLIPG* pblipg)
{
	AppendDlEntry(&pblipg->psw->dlBlipgFree, pblipg);
	InitAlo(pblipg);

	pblipg->sMRD = 1e+10;
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

void DeleteBlipg(BLIPG* pblipg)
{
	delete pblipg;
}

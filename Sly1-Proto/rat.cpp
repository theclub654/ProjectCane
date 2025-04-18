#include "rat.h"

RAT* NewRat()
{
	return new RAT{};
}

void InitSwRatDl(SW* psw)
{
	InitDl(&psw->dlRat, offsetof(RAT, dleRat));
}

void InitRat(RAT* prat)
{
	InitSo(prat);
}

int GetRatSize()
{
	return sizeof(RAT);
}

void OnRatAdd(RAT* prat)
{
	OnSoAdd(prat);
	AppendDlEntry(&prat->psw->dlRat, prat);
}

void OnRatRemove(RAT* prat)
{
	OnSoRemove(prat);
	RemoveDlEntry(&prat->psw->dlRat, prat);
}

void CloneRat(RAT* prat, RAT* pratBase)
{
	LO lo = *prat;
	*prat = *pratBase;
	memcpy(prat, &lo, sizeof(LO));

	CloneLo(prat, pratBase);

	ClearDl(&prat->dlChild);

	prat->pxa = nullptr;
	prat->grfpvaXpValid = 0;
	prat->pstso = nullptr;
}

void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prat, pbis);
}

void RenderRatAll(RAT* prat, CM* pcm, RO* pro)
{
	RenderAloAll(prat, pcm, pro);
	//prat->pvtalo->pfnRenderAloAll(prat, pcm, pro);
}

void DeleteRat(RAT *prat)
{
	delete prat;
}

RATHOLE* NewRathole()
{
	return new RATHOLE{};
}

void InitSwRatholeDl(SW* psw)
{
	InitDl(&psw->dlRathole, offsetof(RATHOLE, dleRathole));
}

int GetRatholeSize()
{
	return sizeof(RATHOLE);
}

void OnRatholeRemove(RATHOLE* prathole)
{
	OnLoRemove((LO*)prathole);
	RemoveDlEntry(&prathole->psw->dlRathole, prathole);
}

void CloneRathole(RATHOLE* prathole, RATHOLE* pratholeBase)
{
	LO lo = *prathole;
	*prathole = *pratholeBase;
	memcpy(prathole, &lo, sizeof(LO));

	CloneLo(prathole, pratholeBase);
}

void OnRatholeAdd(RATHOLE* prathole)
{
	OnLoAdd(prathole);
	AppendDlEntry(&prathole->psw->dlRathole, prathole);
}

void DeleteRathole(RATHOLE* prathole)
{
	delete prathole;
}
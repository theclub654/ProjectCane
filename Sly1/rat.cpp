#include "rat.h"

void* NewRat()
{
	return new RAT{};
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

}

void DeleteRat(LO* plo)
{
	delete (RAT*)plo;
}

void* NewRathole()
{
	return new RATHOLE{};
}

int GetRatholeSize()
{
	return sizeof(RATHOLE);
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

void DeleteRathole(LO* plo)
{
	delete(RATHOLE*)plo;
}
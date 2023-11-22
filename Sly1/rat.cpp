#include "rat.h"

void* NewRat()
{
	return new RAT;
}

void InitRat(RAT* prat)
{
	InitSo(prat);
}

void OnRatAdd(RAT* prat)
{
	OnSoAdd(prat);
	AppendDlEntry(&prat->psw->dlRat, prat);
}

void* NewRathole()
{
	return new RATHOLE;
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

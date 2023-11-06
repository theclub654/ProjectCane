#include "rat.h"

void InitRat(RAT* prat)
{
	InitSo(prat);
}

void OnRatAdd(RAT* prat)
{
	OnSoAdd(prat);
}

void OnRatholeAdd(RATHOLE* prathole)
{
	OnLoAdd(prathole);
}

void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prat, pbis);
}

void RenderRatAll(RAT* prat, CM* pcm, RO* pro)
{

}

#include "rat.h"

void InitRat(RAT* prat)
{
	/*std::cout << "RAT Size: " << sizeof(RAT) << "\n";
	std::cout << "RATHOLE Size: " << sizeof(RATHOLE) << "\n";*/
	InitSo(prat);
}

void OnRatAdd(RAT* prat)
{
	OnSoAdd(prat);
	AppendDlEntry(&prat->psw->dlRat, prat);
}

void OnRatholeAdd(RATHOLE* prathole)
{
	OnLoAdd(prathole);
	AppendDlEntry(&prathole->psw->dlRathole, prathole);
}

void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prat, pbis);
}

void RenderRatAll(RAT* prat, CM* pcm, RO* pro)
{

}

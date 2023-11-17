#include "jack.h"

void InitJack(JACK* pjack)
{
	std::cout << "JACK Size: " << sizeof(JACK) << "\n";
	std::cout << "JACKB Size: " << sizeof(JACKB) << "\n";
	std::cout << "JACKN Size: " << sizeof(JACKN) << "\n";
	std::cout << "JACKF Size: " << sizeof(JACKF) << "\n";
	InitPo(pjack);
}

void InitJackb(JACKB* pjackb)
{
	InitAlo(pjackb);
}

void InitJackn(JACKN* pjackn)
{
	InitAlo(pjackn);
}

void InitJackf(JACKF* pjackf)
{
	InitAlo(pjackf);
}

void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjackn, pbis);
}

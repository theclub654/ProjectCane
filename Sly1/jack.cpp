#include "jack.h"

void InitJack(JACK* pjack)
{
	std::cout << "JACK Size: " << sizeof(JACK) << "\n";
	std::cout << "JACKB Size: " << sizeof(JACKB) << "\n";
	std::cout << "JACKN Size: " << sizeof(JACKN) << "\n";
	std::cout << "JACKF Size: " << sizeof(JACKF) << "\n";
	InitPo(pjack);
}

void DeleteJack(LO* plo)
{
	delete (JACK*)plo;
}

void InitJackb(JACKB* pjackb)
{
	InitAlo(pjackb);
}

void DeleteJackb(LO* plo)
{
	delete(JACKB*)plo;
}

void InitJackn(JACKN* pjackn)
{
	InitAlo(pjackn);
}

void DeleteJackn(LO* plo)
{
	delete(JACKN*)plo;
}

void InitJackf(JACKF* pjackf)
{
	InitAlo(pjackf);
}

void DeleteJackf(LO* plo)
{
	delete(JACKF*)plo;
}

void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjackn, pbis);
}

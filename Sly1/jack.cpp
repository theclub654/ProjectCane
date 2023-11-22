#include "jack.h"

void* NewJack()
{
	return new JACK;
}

void InitJack(JACK* pjack)
{
	InitPo(pjack);
}

void DeleteJack(LO* plo)
{
	delete (JACK*)plo;
}

void* NewJackb()
{
	return new JACKB;
}

void InitJackb(JACKB* pjackb)
{
	InitAlo(pjackb);
}

void DeleteJackb(LO* plo)
{
	delete(JACKB*)plo;
}

void* NewJackn()
{
	return new JACKN;
}

void InitJackn(JACKN* pjackn)
{
	InitAlo(pjackn);
}

void DeleteJackn(LO* plo)
{
	delete(JACKN*)plo;
}

void* NewJackf()
{
	return new JACKF;
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

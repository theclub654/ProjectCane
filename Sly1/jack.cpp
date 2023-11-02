#include "jack.h"

void InitJack(JACK* pjack)
{
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

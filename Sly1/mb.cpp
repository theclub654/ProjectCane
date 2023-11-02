#include "mb.h"

void InitMbg(MBG *pmbg)
{
	InitStepGuard(pmbg);
}

void InitBhg(BHG *pbhg)
{
	InitStepGuard((STEPGUARD*)pbhg);
}

void InitScentmap(SCENTMAP* pscentmap)
{
	InitAlo(pscentmap);
}

void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis)
{
	LoadStepGuardFromBrx(pmbg, pbis);
}

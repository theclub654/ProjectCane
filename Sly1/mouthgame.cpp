#include "mouthgame.h"

void InitMgc(MGC* pmgc)
{
	InitAlo(pmgc);
}

void InitMgv(MGV* pmgv)
{
	InitPo(pmgv);
}

void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pmgc, pbis);
}

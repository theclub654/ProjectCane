#include "mouthgame.h"

void* NewMgc()
{
	return new MGC;
}

void InitMgc(MGC* pmgc)
{
	InitAlo(pmgc);
}

void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pmgc, pbis);
}

void DeleteMgc(LO* plo)
{
	delete(MGC*)plo;
}

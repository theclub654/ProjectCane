#include "mouthgame.h"

MGC* NewMgc()
{
	return new MGC{};
}

void InitMgc(MGC* pmgc)
{
	InitAlo(pmgc);
	InitDl(&pmgc->dlMgcoFree, offsetof(MGCO, dle));
	InitDl(&pmgc->dlMgco, offsetof(MGCO, dle));
}

int GetMgcSize()
{
	return sizeof(MGC);
}

void CloneMgc(MGC* pmgc, MGC* pmgcBase)
{
	LO lo = *pmgc;
	*pmgc = *pmgcBase;
	memcpy(pmgc, &lo, sizeof(LO));

	CloneLo(pmgc, pmgcBase);

	ClearDl(&pmgc->dlChild);
}

void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pmgc, pbis);
}

void DeleteMgc(MGC* pmgc)
{
	delete pmgc;
}

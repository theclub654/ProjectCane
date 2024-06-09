#include "frzg.h"

FRZG* NewFrzg()
{
	return new FRZG{};
}

int GetFrzgSize()
{
	return sizeof(FRZG);
}

void CloneFrzg(FRZG* pfrzg, FRZG* pfrzgBase)
{
	LO lo = *pfrzg;
	*pfrzg = *pfrzgBase;
	memcpy(pfrzg, &lo, sizeof(LO));

	CloneLo(pfrzg, pfrzgBase);
}

void DeleteFrzg(LO* plo)
{
	delete(FRZG*)plo;
}

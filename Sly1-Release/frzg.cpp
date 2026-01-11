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
	CloneLo(pfrzg, pfrzgBase);

	pfrzg->coid = pfrzgBase->coid;
	std::memcpy(pfrzg->aoid, pfrzgBase->aoid, sizeof(pfrzgBase->aoid));
	pfrzg->mrg = pfrzgBase->mrg;
}

void PostFrzgLoad(FRZG* pfrzg)
{
	PostLoLoad(pfrzg);
}

void DeleteFrzg(FRZG* pfrzg)
{
	delete pfrzg;
}

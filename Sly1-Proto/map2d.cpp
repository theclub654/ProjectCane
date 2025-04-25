#include "map2d.h"

MAP* NewMap()
{
	return new MAP{};
}

void CloneMap(MAP* pmap, MAP* pmapBase)
{
    CloneLo(pmap, pmapBase);

    pmap->cpos = pmapBase->cpos;
    pmap->ctri = pmapBase->ctri;
    pmap->cmbsp = pmapBase->cmbsp;
    pmap->cbEach = pmapBase->cbEach;
    pmap->apos = pmapBase->apos;
    pmap->atri = pmapBase->atri;
    pmap->ambsp = pmapBase->ambsp;
    pmap->mpiposab = pmapBase->mpiposab;
}

int GetMapSize()
{
	return sizeof(MAP);
}

void DeleteMap(MAP* pmap)
{
	delete pmap;
}

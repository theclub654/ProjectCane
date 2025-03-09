#include "map2d.h"

MAP* NewMap()
{
	return new MAP{};
}

void CloneMap(MAP* pmap, MAP* pmapBase)
{
	LO lo = *pmap;
	*pmap = *pmapBase;
	memcpy(pmap, &lo, sizeof(LO));

	CloneLo(pmap, pmapBase);
}

int GetMapSize()
{
	return sizeof(MAP);
}

void DeleteMap(MAP* pmap)
{
	delete pmap;
}

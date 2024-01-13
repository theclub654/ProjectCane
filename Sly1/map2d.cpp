#include "map2d.h"

void* NewMap()
{
	return new MAP{};
}

int GetMapSize()
{
	return sizeof(MAP);
}

void CloneMap(MAP* pmap, MAP* pmapBase)
{
	LO lo = *pmap;
	*pmap = *pmapBase;
	memcpy(pmap, &lo, sizeof(LO));

	CloneLo(pmap, pmapBase);
}

void DeleteMap(LO* plo)
{
	delete(MAP*)plo;
}

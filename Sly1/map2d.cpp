#include "map2d.h"

void* NewMap()
{
	return new MAP;
}

void DeleteMap(LO* plo)
{
	delete(MAP*)plo;
}

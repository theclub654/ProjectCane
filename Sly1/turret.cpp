#include "turret.h"

void* NewTurret()
{
	return new TURRET;
}

void DeleteTurret(LO* plo)
{
	delete(TURRET*)plo;
}

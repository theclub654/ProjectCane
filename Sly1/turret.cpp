#include "turret.h"

void* NewTurret()
{
	return new TURRET;
}

int GetTurretSize()
{
	return sizeof(TURRET);
}

void CloneTurret(TURRET* pturret, TURRET* pturretBase)
{
	LO lo = *pturret;
	*pturret = *pturretBase;
	memcpy(pturret, &lo, sizeof(LO));

	CloneLo(pturret, pturretBase);

	ClearDl(&pturret->dlChild);

	pturret->pxa = nullptr;
	pturret->grfpvaXpValid = 0;
	pturret->pstso = nullptr;
}

void DeleteTurret(LO* plo)
{
	delete(TURRET*)plo;
}

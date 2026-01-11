#include "turret.h"

TURRET* NewTurret()
{
	return new TURRET{};
}

int GetTurretSize()
{
	return sizeof(TURRET);
}

void CloneTurret(TURRET* pturret, TURRET* pturretBase)
{
    ClonePo(pturret, pturretBase);

    pturret->psm = pturretBase->psm;
    pturret->psma = pturretBase->psma;
    pturret->prwm = pturretBase->prwm;
    pturret->ppntAnchor = pturretBase->ppntAnchor;
    pturret->pmurray = pturretBase->pmurray;
    pturret->fCharmEnabled = pturretBase->fCharmEnabled;
    pturret->tCharmPending = pturretBase->tCharmPending;
}

void PostTurretLoad(TURRET* pturret)
{
    PostAloLoad(pturret);
}

void UpdateTurret(TURRET* pturret, float dt)
{
    UpdatePo(pturret, dt);
}

void DeleteTurret(TURRET *pturret)
{
	delete pturret;
}

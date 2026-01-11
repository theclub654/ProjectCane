#include "dartgun.h"

DARTGUN* NewDartgun()
{
	return new DARTGUN{};
}

void InitDartgun(DARTGUN* pdartgun)
{
	InitBreak(pdartgun);
}

int GetDartgunSize()
{
	return sizeof(DARTGUN);
}

void CloneDartgun(DARTGUN* pdartgun, DARTGUN* pdartgunBase)
{
    CloneBreak(pdartgun, pdartgunBase);

    pdartgun->oidDart = pdartgunBase->oidDart;
    pdartgun->cDarts = pdartgunBase->cDarts;
    pdartgun->pdartReadyToFire = pdartgunBase->pdartReadyToFire;
    pdartgun->pdartFading = pdartgunBase->pdartFading;
    pdartgun->dtFireInterval = pdartgunBase->dtFireInterval;
    pdartgun->dzTarget = pdartgunBase->dzTarget;
    pdartgun->tLastFire = pdartgunBase->tLastFire;
    pdartgun->paloTarget = pdartgunBase->paloTarget;
    pdartgun->pratTarget = pdartgunBase->pratTarget;
    pdartgun->posOrigDart = pdartgunBase->posOrigDart;
    pdartgun->matOrigDart = pdartgunBase->matOrigDart;
    pdartgun->paloOrigDartParent = pdartgunBase->paloOrigDartParent;
    pdartgun->paloLookAt = pdartgunBase->paloLookAt;
    pdartgun->paloLid = pdartgunBase->paloLid;
    pdartgun->psm = pdartgunBase->psm;
    pdartgun->psma = pdartgunBase->psma;

    pdartgun->coidRatTarget = pdartgunBase->coidRatTarget;
    std::memcpy(pdartgun->aoidRatTarget, pdartgunBase->aoidRatTarget, sizeof(pdartgun->aoidRatTarget));

    pdartgun->cpratTarget = pdartgunBase->cpratTarget;
    std::memcpy(pdartgun->apratTarget, pdartgunBase->apratTarget, sizeof(pdartgun->apratTarget));

    pdartgun->cpratInTargetArea = pdartgunBase->cpratInTargetArea;
    std::memcpy(pdartgun->apratInTargetArea, pdartgunBase->apratInTargetArea, sizeof(pdartgun->apratInTargetArea));

    pdartgun->fJtInTargetArea = pdartgunBase->fJtInTargetArea;
}

void BindDartgun(DARTGUN* pdartgun)
{
    BindAlo(pdartgun);
}

void PostDartgunLoad(DARTGUN* pdartgun)
{
    PostBrkLoad(pdartgun);
}

void UpdateDartgun(DARTGUN* pdartgun, float dt)
{
    UpdateBrk(pdartgun, dt);
}

void DeleteDartgun(DARTGUN* pdartgun)
{
	delete pdartgun;
}

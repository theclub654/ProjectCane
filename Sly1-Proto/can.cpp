#include "can.h"

CAN* NewCan()
{
	return new CAN{};
}

void InitCan(CAN* pcan)
{
	InitBreak(pcan);
}

int GetCanSize()
{
	return sizeof(CAN);
}

void CloneCan(CAN* pcan, CAN* pcanBase)
{
	CloneBreak(pcan, pcanBase);

    pcan->cans = pcanBase->cans;
    pcan->prwm = pcanBase->prwm;
    pcan->psoTurret = pcanBase->psoTurret;
    pcan->fLeadTarget = pcanBase->fLeadTarget;
    pcan->lmRange = pcanBase->lmRange;
    pcan->lmActivate = pcanBase->lmActivate;
    pcan->lmDeactivate = pcanBase->lmDeactivate;
    pcan->radPanMax = pcanBase->radPanMax;
    pcan->dradFireThreshold = pcanBase->dradFireThreshold;
    pcan->dtFireInterval = pcanBase->dtFireInterval;
    pcan->tLastFire = pcanBase->tLastFire;
    pcan->dzAim = pcanBase->dzAim;
    pcan->swTumble = pcanBase->swTumble;
    pcan->smpTracking = pcanBase->smpTracking;
    pcan->fFireEnabled = pcanBase->fFireEnabled;
    pcan->cuFireAccuracy = pcanBase->cuFireAccuracy;

    for (int i = 0; i < 5; ++i)
        pcan->auFireAccuracy[i] = pcanBase->auFireAccuracy[i];

    pcan->posTarget = pcanBase->posTarget;
    pcan->ppntMuzzle = pcanBase->ppntMuzzle;
    pcan->psm = pcanBase->psm;
    pcan->psma = pcanBase->psma;
    pcan->cFired = pcanBase->cFired;
}

void PostCanLoad(CAN* pcan)
{
    PostBrkLoad(pcan);
}

void DeleteCan(CAN *pcan)
{
	delete pcan;
}

#include "mgv.h"

MGV* NewMgv()
{
	return new MGV{};
}

void InitMgv(MGV* pmgv)
{
	InitPo(pmgv);
	pmgv->cpoints = 200;
	pmgv->clives = 5;
}

int GetMgvSize()
{
	return sizeof(MGV);
}

void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pmgv, pbis);
}

void CloneMgv(MGV* pmgv, MGV* pmgvBase)
{
    ClonePo(pmgv, pmgvBase);

    pmgv->psm = pmgvBase->psm;
    pmgv->psma = pmgvBase->psma;
    for (int i = 0; i < 3; ++i)
        pmgv->apmgc[i] = pmgvBase->apmgc[i];
    pmgv->ipmgcCur = pmgvBase->ipmgcCur;
    for (int i = 0; i < 3; ++i)
        pmgv->atmblPoints[i] = pmgvBase->atmblPoints[i];
    pmgv->tmblLives = pmgvBase->tmblLives;
    pmgv->clives = pmgvBase->clives;
    pmgv->cpoints = pmgvBase->cpoints;
}

void DeleteMgv(MGV *pmgv)
{
	delete pmgv;
}

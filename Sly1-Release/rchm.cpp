#include "rchm.h"

RCHM* NewRchm()
{
    return new RCHM{};
}

void InitRchm(RCHM* prchm)
{
    InitLo(prchm);
    prchm->oidHost = OID_Nil;
    prchm->oidTouch = OID_Nil;
}

int GetRchmSize()
{
    return sizeof(RCHM);
}

void LoadRchmFromBrx(RCHM *prchm, CBinaryInputStream *pbis)
{
    prchm->ablrch.resize(24);

    for (int i = 0; i <= 0x17; i++)
        prchm->ablrch[i].oidAseg = (OID)pbis->S16Read();

    prchm->cposGrid = pbis->S16Read();

    ReadGeom(&prchm->geomLocal, pbis);

    for (int i = 0; i < prchm->geomLocal.cpos; i++)
        pbis->S8Read();

    int16_t unk_1 = pbis->S16Read();

    for (int i = 0; i < unk_1; i++)
    {
        for (int i = 0; i <= 3; i++)
            pbis->S16Read();
    }

    int16_t unk_2 = pbis->S16Read();

    for (int i = 0; i < unk_2; i++)
    {
        pbis->S16Read();
        pbis->S16Read();
        pbis->S16Read();
    }

    pbis->F32Read();
    LoadOptionsFromBrx(prchm, pbis);
}

void PostRchmLoad(RCHM* prchm)
{
    PostLoLoad(prchm);
}

void CloneRchm(RCHM* prchm, RCHM* prchmBase)
{
    CloneLo(prchm, prchmBase);

    prchm->oidHost = prchmBase->oidHost;
    prchm->oidTouch = prchmBase->oidTouch;
    prchm->rchmk = prchmBase->rchmk;
    prchm->cposGrid = prchmBase->cposGrid;
    prchm->grftak = prchmBase->grftak;
    prchm->fEnabled = prchmBase->fEnabled;
    prchm->rclNatural = prchmBase->rclNatural;
    prchm->ioNatural = prchmBase->ioNatural;
    prchm->lhubNatural = prchmBase->lhubNatural;
    prchm->dtContactNatural = prchmBase->dtContactNatural;
    prchm->posContactNatural = prchmBase->posContactNatural;
    prchm->fCenterSet = prchmBase->fCenterSet;
    prchm->rclCenter = prchmBase->rclCenter;
    prchm->ioCenter = prchmBase->ioCenter;
    prchm->lhubCenter = prchmBase->lhubCenter;
    prchm->posCenter = prchmBase->posCenter;
    prchm->geomLocal = prchmBase->geomLocal;
    prchm->gRadiusSquared = prchmBase->gRadiusSquared;
    prchm->dtPause = prchmBase->dtPause;

    // Shallow copy of the pointer members
    prchm->paloHost = prchmBase->paloHost;
    prchm->paloTouch = prchmBase->paloTouch;
    prchm->pasegbl = prchmBase->pasegbl;

    // Shallow copy of the std::vector members
    prchm->mpiposrch = prchmBase->mpiposrch; // Shallow copy the vector of RCH objects
    prchm->atwr = prchmBase->atwr; // Shallow copy the vector of TWR objects
    prchm->mpibsptwdCat = prchmBase->mpibsptwdCat; // Shallow copy the vector of TWD objects
    prchm->ablrch = prchmBase->ablrch; // Shallow copy the vector of BLRCH objects
    prchm->bspcCat = prchmBase->bspcCat;
}

void DeleteRchm(RCHM* prchm)
{
    delete prchm;
}

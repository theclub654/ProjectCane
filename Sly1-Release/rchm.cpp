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

void DeleteRchm(RCHM* prchm)
{
    delete prchm;
}

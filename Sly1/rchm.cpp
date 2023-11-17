#include "rchm.h"

void InitRchm(RCHM* prchm)
{
    //std::cout << "RCHM Size: " << sizeof(RCHM) << "\n";
    InitLo(prchm);
}

void LoadRchmFromBrx(RCHM* prchm, CBinaryInputStream* pbis)
{
    for (int i = 0; i <= 0x17; i++)
        pbis->S16Read();

    prchm->cposGrid = pbis->S16Read();

    pbis->ReadGeom(&prchm->geomLocal);

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
    LoadOptionFromBrx(0, pbis);
}

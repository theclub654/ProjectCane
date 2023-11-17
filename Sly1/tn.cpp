#include "tn.h"

void InitTn(TN* ptn)
{
    //std::cout << "TN Size: " << sizeof(TN) << "\n";
    InitAlo(ptn);
}

void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis)
{
    ptn->xf.mat = pbis->ReadMatrix();
    ptn->xf.pos = pbis->ReadVector();

    LoadTbspFromBrx(pbis);

    int8_t crvk = pbis->S8Read();

    if (crvk == -1)
    {
        ptn->matXfm = pbis->ReadMatrix();
        ptn->posXfm = pbis->ReadVector();
        ptn->sTotal = 0.0;
    }

    else
    {
        switch (crvk)
        {
        case 0x0:
            LoadCrvlFromBrx(pbis);
            break;
        case 0x1:
            LoadCrvcFromBrx(pbis);
            break;
        }
    }

    LoadOptionFromBrx(0, pbis);
}

void LoadTbspFromBrx(CBinaryInputStream* pbis)
{
    uint16_t unk_0 = pbis->U16Read();

    for (int i = 0; i < unk_0; i++)
    {
        pbis->F32Read();
        pbis->ReadVector();
    }

    uint16_t unk_1 = pbis->U16Read();

    for (int i = 0; i < unk_1; i++)
    {
        pbis->U16Read();
        pbis->S16Read();
        pbis->S16Read();
    }
}

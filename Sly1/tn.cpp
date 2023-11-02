#include "tn.h"

void InitTn(TN* ptn)
{
    InitAlo(ptn);
}

void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis)
{
    pbis->ReadMatrix();
    pbis->ReadVector();

    LoadTbspFromBrx(pbis);

    int8_t unk_0 = pbis->S8Read();

    if (unk_0 == -1)
    {
        pbis->ReadMatrix();
        pbis->ReadVector();
    }

    else
    {
        switch (unk_0)
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

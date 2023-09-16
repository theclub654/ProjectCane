#include "keyhole.h"

void InitKeyhole(KEYHOLE *pkeyhole)
{
	InitLo(pkeyhole);
	g_pkeyhole = pkeyhole;
}

void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis)
{
	LoadOptionFromBrx(pkeyhole, pbis);

    uint16_t unk_0 = pbis->U16Read();

    for (int i = 0; i < unk_0; i++)
    {
        pbis->F32Read();
        pbis->F32Read();
    }

    pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();

    for (int i = 0; i < 5; i++)
    {
        uint16_t unk_5 = pbis->U16Read();

        for (int i = 0; i < unk_5; i++)
        {
            pbis->U16Read();
            pbis->U16Read();
            pbis->U16Read();
        }
    }
}

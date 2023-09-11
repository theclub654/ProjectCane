#include "path.h"

void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis)
{
    LoadLoFromBrx(0, pbis);
    uint16_t unk_0 = pbis->U16Read();
    uint16_t unk_1 = pbis->U16Read();
    uint16_t unk_2 = pbis->U16Read();
    uint16_t unk_3 = pbis->U16Read();

    uint16_t unk = unk_1 + unk_2;

    for (int i = 0; i < unk_0; i++)
    {
        pbis->ReadVector();
        byte unk_4 = pbis->U8Read();

        for (int i = 0; i < unk_4; i++)
            pbis->U16Read();
    }


    for (int i = 0; i < unk; i++)
    {
        for (int i = 0; i < 2; i++)
            pbis->U16Read();
    }

    for (int i = 0; i < unk_3; i++)
    {
        for (int i = 0; i <= 2; i++)
            pbis->U16Read();
    }

    uint16_t unk_5 = pbis->U16Read();
    for (int i = 0; i < unk_5; i++)
    {
        pbis->ReadVector();
        pbis->F32Read();
        pbis->S16Read();
        pbis->S16Read();
    }
}
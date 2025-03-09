#include "serialize.h"

void PpairSerializeIn(CBinaryInputStream* pbis)
{
    byte unk_0 = pbis->U8Read();

    if (unk_0 != 0xff)
    {
        uint32_t temp0 = (uint32_t)unk_0 & 0x7F;

        switch (temp0)
        {
        case 0x14:
            pbis->U32Read();
            break;
        case 0x15:
            pbis->U32Read();
            break;
        case 0x16:
            for (int i = 0; i < 15; i++)
                pbis->U32Read();
            break;
        case 0x17:
            for (int i = 0; i < 15; i++)
                pbis->U32Read();
            break;
        case 0x18:
            for (int i = 0; i < 2; i++)
                pbis->U32Read();
            break;
        case 0x19:
            pbis->U32Read();
            pbis->U32Read();
            break;
        case 0x1a:
            pbis->U32Read();
            pbis->U32Read();
            pbis->U32Read();
            break;
        case 0x1b:
            pbis->U32Read();
            break;
        case 0x1c:
            pbis->U32Read();
            break;
        case 0x1d:
            pbis->U32Read();
            break;
        case 0x1e:
            PpairSerializeIn(pbis);
            break;
        case 0x20:
            pbis->U32Read();
        }

        if ((unk_0 & 0x80) == 0)
        {

        }
        else
        {
            PpairSerializeIn(pbis);
        }
    }
}

#include "aseg.h"

void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis)
{
    /*LoadAsegaCount++;
    std::cout << "LoadAsegFromBrx: " << std::dec << LoadAsegaCount << "\n";
    std::cout << std::hex << pbis->file.tellg() << "\n";*/
    int16_t unk_0 = pbis->U16Read();
    int16_t unk_1 = pbis->U16Read();
    uint16_t unk_2 = pbis->U16Read();

    for (int i = 0; i < unk_2; i++)
        int16_t unk_3 = pbis->S16Read();

    byte unk_4 = pbis->U8Read();

    for (int i = 0; i < unk_4; i++)
    {
        int16_t unk_5 = pbis->S16Read();
        byte unk_6 = pbis->U8Read();
        byte unk_7;

        if ((unk_6 & 1) != 0)
        {
            unk_7 = pbis->S8Read();
            switch (unk_7)
            {
            case 0:
                LoadAcpcFromBrx(pbis);
                break;
            case 1:
                LoadAcpbFromBrx(pbis);
                break;
            }
        }

        if ((unk_6 & 2) != 0)
        {
            unk_7 = pbis->S8Read();

            switch (unk_7)
            {
            case 0:
                LoadAcrcFromBrx(pbis);
                break;
            case 1:
                LoadAcrbFromBrx(pbis);
                break;
            }
        }

        if ((unk_6 & 4) != 0)
        {
            unk_7 = pbis->S8Read();

            switch (unk_7)
            {
            case 0:
                LoadAcscFromBrx(pbis);
                break;
            }
        }

        if ((unk_6 & 8) != 0)
        {
            int8_t acgk = pbis->S8Read();

            if (acgk == 0)
                LoadAcgbFromBrx(pbis);
            else if (acgk == 1)
                LoadAcgbwFromBrx(pbis);
            else if (acgk == 2)
                LoadAcglFromBrx(pbis);
        }

        if ((unk_6 & 0x10) != 0)
        {
            byte unk_7 = pbis->U8Read();

            for (int i = 0; i < unk_7; i++)
            {
                int acgk = pbis->S8Read();

                switch (acgk)
                {
                case 0:
                    LoadAcgbFromBrx(pbis);
                    break;
                case 1:
                    LoadAcgbwFromBrx(pbis);
                    break;
                case 2:
                    LoadAcglFromBrx(pbis);
                    break;
                }
            }
        }
    }

    LoadAsegEventsFromBrx(pbis, 0);
    LoadAsegEventsFromBrx(pbis, 1);
    LoadAsegEventsFromBrx(pbis, 0);
    LoadOptionFromBrx(0, pbis);
}

void LoadAsegEventsFromBrx(CBinaryInputStream* pbis, int fFrame)
{
    int8_t unk_0 = pbis->S8Read();

    for (int i = 0; i < unk_0; i++)
    {
        int8_t unk_1 = pbis->S8Read();

        if (fFrame != 0)
            pbis->S16Read();

        switch (unk_1)
        {
        case 0x0:
            pbis->S16Read();
            pbis->S16Read();
            pbis->S32Read();
            pbis->S32Read();
            pbis->F32Read();
            pbis->F32Read();
            pbis->F32Read();
            break;
        case 0x4:
            pbis->S16Read();
            pbis->F32Read();
            break;
        case 0x7:
            pbis->S16Read();
            pbis->F32Read();
            break;
        case 0x5:
            pbis->F32Read();
            break;
        case 0x6:
            pbis->S16Read();
            break;
        case 0xC:
            pbis->S16Read();
            break;
        case 0xD:
            pbis->S16Read();
            break;
        case 0x11:
            pbis->S16Read();
            break;
        case 0x12:
            pbis->S16Read();
            break;
        case 0x16:
            pbis->S16Read();
            break;
        case 0x18:
            pbis->S16Read();
            break;
        case 0x1D:
            pbis->S16Read();
            break;
        case 0x1E:
            pbis->S16Read();
            break;
        case 0x1F:
            pbis->S16Read();
            break;
        case 0x20:
            pbis->S16Read();
            break;
        case 0x23:
            pbis->S16Read();
            break;
        case 0x24:
            pbis->S16Read();
            break;
        case 0x27:
            pbis->S16Read();
            break;
        case 0x29:
            pbis->S16Read();
            break;
        case 0x3B:
            pbis->S16Read();
            break;
        case 0x21:
            pbis->F32Read();
            break;
        case 0x39:
            pbis->F32Read();
            break;
        case 0x3A:
            pbis->F32Read();
            break;
        case 0x9:
            pbis->S16Read();
            pbis->F32Read();
            break;
        case 0xA:
            pbis->S16Read();
            pbis->F32Read();
            break;
        case 0x10:
            pbis->S16Read();
            pbis->S8Read();
            pbis->S8Read();
            break;
        case 0x13:
            pbis->S16Read();
            pbis->U8Read();
            break;
        case 0x14:
            pbis->S16Read();
            pbis->U8Read();
            break;
        case 0x15:
            pbis->S16Read();
            pbis->F32Read();
            pbis->F32Read();
            break;
        case 0x17:
            pbis->S16Read();
            pbis->F32Read();
            pbis->F32Read();
            break;
        case 0x19:
            pbis->S16Read();
            pbis->S16Read();
            pbis->S16Read();
            break;
        case 0x1A:
            pbis->S16Read();
            pbis->S16Read();
            pbis->S16Read();
            break;
        case 0x1B:
            pbis->U8Read();
            pbis->S16Read();
            break;
        case 0x1C:
            pbis->S16Read();
            pbis->S16Read();
            pbis->F32Read();
            pbis->F32Read();
            break;
        case 0x25:
            pbis->S16Read();
            pbis->S16Read();
            break;
        case 0x26:
            pbis->S16Read();
            pbis->S16Read();
            break;
        case 0x2D:
            pbis->S16Read();
            pbis->S16Read();
            break;
        case 0x2F:
            pbis->S16Read();
            pbis->S16Read();
            break;
        case 0x28:
            pbis->U16Read();
            break;
        case 0x2C:
            pbis->ReadStringSw();
            break;
        case 0x33:
            pbis->ReadStringSw();
            break;
        case 0x2E:
            pbis->S16Read();
            pbis->S16Read();
            pbis->S16Read();
            break;
        case 0x34:
            pbis->S16Read();
            pbis->S16Read();
            pbis->S16Read();
            break;
        case 0x30:
            pbis->S16Read();
            pbis->S16Read();
            pbis->S16Read();
            pbis->F32Read();
            break;
        /*case 0x31:
            pbis->ReadStringSw();
            break;*/
        case 0x36:
            pbis->S16Read();
            pbis->S16Read();
            pbis->ReadStringSw();
            break;
        }
    }
}

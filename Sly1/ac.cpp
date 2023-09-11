#include "ac.h"

void LoadAcpcFromBrx(CBinaryInputStream* pbis)
{
	LoadApacgFromBrx(pbis);
}

void LoadAcpvFromBrx(CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(pbis);
}

void LoadAkvbFromBrx(CBinaryInputStream* pbis)
{
    uint16_t unk_0 = pbis->U16Read();

    for (int i = 0; i < unk_0; i++)
    {
        pbis->S16Read();
        pbis->ReadVector();
        pbis->ReadVector();
        pbis->ReadVector();
    }
}

void LoadAcrcFromBrx(CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pbis);
}

void LoadAcscFromBrx(CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pbis);
}

void LoadAcrbFromBrx(CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(pbis);
}

void LoadApacgFromBrx(CBinaryInputStream* pbis)
{
    uint32_t temp0;
    byte unk_0 = pbis->U8Read();

    pbis->ReadVector();

    for (int i = 0; i <= 2; i++)
    {
        temp0 = unk_0 & 1;
        unk_0 = (int)unk_0 >> 1;

        if (temp0 != 0)
        {
            byte acgk = pbis->U8Read();

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

void LoadAcgbFromBrx(CBinaryInputStream* pbis)
{
    uint16_t unk_0 = pbis->U16Read();

    for (int i = 0; i < unk_0; i++)
    {
        pbis->S16Read();
        pbis->F32Read();
        pbis->S8Read();
        pbis->S8Read();
        pbis->F32Read();
        pbis->F32Read();
    }
}

void LoadAcgbwFromBrx(CBinaryInputStream* pbis)
{
    uint16_t unk_0 = pbis->U16Read();

    for (int i = 0; i < unk_0; i++)
    {
        pbis->S16Read();
        pbis->F32Read();
        pbis->S8Read();
        pbis->S8Read();
        pbis->F32Read();
        pbis->F32Read();
        pbis->F32Read();
        pbis->F32Read();
    }
}

void LoadAcglFromBrx(CBinaryInputStream* pbis)
{
    uint16_t unk_0 = pbis->U16Read();

    for (int i = 0; i < unk_0; i++)
    {
        pbis->S16Read();
        pbis->F32Read();
    }
}

void LoadAcpbFromBrx(CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(pbis);
}

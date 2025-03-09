#include "shdanim.h"

int CbFromSaak(SAAK saak)
{
    switch (saak)
    {
        case SAAK_Loop:
        case SAAK_PingPong:
        return 0x4c;
        case SAAK_Shuffle:
        case SAAK_Hologram:
        return 0x38;
        case SAAK_Eyes:
        return 0x78;
        case SAAK_Scroller:
        return 0x44;
        case SAAK_Circler:
        return 0x3c;
        case SAAK_Looker:
        return 0x50;
        default:
        return 0;
    }
}

void PsaaLoadFromBrx(CBinaryInputStream *pbis)
{
	uint16_t saakType = pbis->U16Read();

    if (saakType != 0)
        pbis->file.seekg(0x1C, SEEK_CUR);
}

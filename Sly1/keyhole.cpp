#include "keyhole.h"

KEYHOLE* NewKeyhole()
{
    return new KEYHOLE{};
}

void InitKeyhole(KEYHOLE *pkeyhole)
{
	InitLo(pkeyhole);
	g_pkeyhole = pkeyhole;
}

int GetKeyholeSize()
{
    return sizeof(KEYHOLE);
}

void LoadKeyholeFromBrx(KEYHOLE* pkeyhole, CBinaryInputStream* pbis)
{
	LoadOptionsFromBrx(pkeyhole, pbis);

    pkeyhole->cpos = pbis->U16Read();
    pkeyhole->apos.resize(pkeyhole->cpos);

    for (int i = 0; i < pkeyhole->cpos; i++)
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

void CloneKeyhole(KEYHOLE* pkeyhole, KEYHOLE* pkeyholeBase)
{
    LO lo = *pkeyhole;
    *pkeyhole = *pkeyholeBase;
    memcpy(pkeyhole, &lo, sizeof(LO));

    CloneLo(pkeyhole, pkeyholeBase);
}

void DeleteKeyhole(LO* plo)
{
    delete(KEYHOLE*)plo;
}

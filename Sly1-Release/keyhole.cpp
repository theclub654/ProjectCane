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

void LoadKeyholeFromBrx(KEYHOLE *pkeyhole, CBinaryInputStream *pbis)
{
	LoadOptionsFromBrx(pkeyhole, pbis);

    pkeyhole->cpos = pbis->U16Read();
    pkeyhole->apos.resize(pkeyhole->cpos);

    for (int i = 0; i < pkeyhole->cpos; i++)
    {
        pkeyhole->apos[i].x =  pbis->F32Read();
        pkeyhole->apos[i].y = -pbis->F32Read();
        pkeyhole->apos[i].w = 1.0;
    }

    pkeyhole->posMin.x = pbis->F32Read();
    pkeyhole->posMin.y = pbis->F32Read();
    pkeyhole->posMin.w = 1.0;

    pkeyhole->posMax.x = pbis->F32Read();
    pkeyhole->posMax.y = pbis->F32Read();
    pkeyhole->posMax.w = 1.0;
    pkeyhole->dx = pkeyhole->posMax.x - pkeyhole->posMin.x;

    for (int i = 0; i < 5; i++)
    {
        int oid = i + 1166;
        SHD *pshd = PshdFindShader((OID)oid);

        if (pshd != nullptr)
            pkeyhole->mpkpks[i].rgba = pshd->rgba;

        pkeyhole->mpkpks[i].ctri = pbis->U16Read();
        pkeyhole->mpkpks[i].atri.resize(pkeyhole->mpkpks[i].ctri);

        for (int a = 0; a < pkeyhole->mpkpks[i].ctri; a++)
        {
            pkeyhole->mpkpks[i].atri[a].aipos[0] = pbis->U16Read();
            pkeyhole->mpkpks[i].atri[a].aipos[1] = pbis->U16Read();
            pkeyhole->mpkpks[i].atri[a].aipos[2] = pbis->U16Read();
        }
    }
}

void CloneKeyhole(KEYHOLE* pkeyhole, KEYHOLE* pkeyholeBase)
{
    CloneLo(pkeyhole, pkeyholeBase);

    pkeyhole->cpos = pkeyholeBase->cpos;

    pkeyhole->apos = pkeyholeBase->apos;

    pkeyhole->posMin = pkeyholeBase->posMin;
    pkeyhole->posMax = pkeyholeBase->posMax;

    pkeyhole->dx = pkeyholeBase->dx;

    std::copy(std::begin(pkeyholeBase->mpkpks), std::end(pkeyholeBase->mpkpks), std::begin(pkeyhole->mpkpks));
}

void DeleteKeyhole(KEYHOLE* pkeyhole)
{
    delete pkeyhole;
}

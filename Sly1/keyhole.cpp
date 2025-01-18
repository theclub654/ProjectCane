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
        int oid = i + OID_shd_keyhole_background;
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
    LO lo = *pkeyhole;
    *pkeyhole = *pkeyholeBase;
    memcpy(pkeyhole, &lo, sizeof(LO));

    CloneLo(pkeyhole, pkeyholeBase);
}

void DeleteKeyhole(LO* plo)
{
    delete(KEYHOLE*)plo;
}

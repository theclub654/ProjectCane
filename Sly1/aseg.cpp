#include "aseg.h"

void* NewAseg()
{
    return new ASEG{};
}

void InitAseg(ASEG* paseg)
{
    InitLo(paseg);
}

int GetAsegSize()
{
    return sizeof(ASEG);
}

void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis)
{
    paseg->oidRoot = (OID)pbis->U16Read();
    paseg->tMax = pbis->U16Read() * 0.01666667;
    paseg->coidSearchRoot = pbis->U16Read();

    paseg->aoidSearchRoot.resize(paseg->coidSearchRoot);

    for (int i = 0; i < paseg->coidSearchRoot; i++)
        paseg->aoidSearchRoot[i] = (OID)pbis->S16Read();

    paseg->cchn = pbis->U8Read();
    paseg->achn.resize(paseg->cchn);

    for (int i = 0; i < paseg->cchn; i++)
    {
        paseg->achn[i].oid = (OID)pbis->S16Read();
        byte unk_6 = pbis->U8Read();

        byte unk_7;

        if ((unk_6 & 1) != 0)
        {
            ACVK acvk = (ACVK)pbis->S8Read();

            ACP *acp = PacpNew(acvk);
            acp->pvtacpc->pfnLoadAcpcFromBrx((ACPC*)acp, pbis);
        }

        if ((unk_6 & 2) != 0)
        {
            ACVK acvk = (ACVK)pbis->S8Read();

            ACR* acr = PacrNew(acvk);
            acr->pvtacrc->pfnLoadAcrcFromBrx((ACRC*)acr, pbis);
        }

        if ((unk_6 & 4) != 0)
        {
            ACVK acvk = (ACVK)pbis->S8Read();

            ACS* acs = PacsNew(acvk);
            acs->pvtacsb->pfnLoadAcsbFromBrx((ACSB*)acs, pbis);
        }

        if ((unk_6 & 8) != 0)
        {
            ACGK acgk = (ACGK)pbis->S8Read();

            ACG* acg = PacgNew(acgk);
            acg->pvtacgb->pfnLoadAcgbFromBrx((ACGB*)acg, pbis);
        }

        if ((unk_6 & 0x10) != 0)
        {
            byte cpacgPose = pbis->U8Read();

            for (int i = 0; i < cpacgPose; i++)
            {
                ACGK acgk = (ACGK)pbis->S8Read();

                if (acgk != -1)
                {
                    ACG* acg = PacgNew(acgk);
                    acg->pvtacgb->pfnLoadAcgbFromBrx((ACGB*)acg, pbis);
                }
            }
        }
    }

    LoadAsegEventsFromBrx(pbis, 0);
    LoadAsegEventsFromBrx(pbis, 1);
    LoadAsegEventsFromBrx(pbis, 0);
    LoadOptionsFromBrx(paseg, pbis);
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

void CloneAseg(ASEG* paseg, ASEG* pasegBase)
{
    LO lo = *paseg;
    *paseg = *pasegBase;
    memcpy(paseg, &lo, sizeof(LO));

    CloneLo(paseg, pasegBase);
}

void ApplyAseg(ASEG* paseg, ALO* paloAsegRoot, float tLocal, float svtLocal, int grfapl, ASEGA** ppasega)
{

}

void DeleteAseg(LO* plo)
{
    delete(ASEG*)plo;
}

void* NewAsegbl()
{
    return new ASEGBL;
}

void DeleteAsegbl(LO* plo)
{
    delete(ASEGBL*)plo;
}

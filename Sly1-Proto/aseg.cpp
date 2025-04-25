#include "aseg.h"

ASEG* NewAseg()
{
    return new ASEG{};
}

void InitAseg(ASEG* paseg)
{
    InitLo(paseg);
    InitDl(&paseg->dlAsega, offsetof(ASEGA, dleAseg));
    
    paseg->clqMasterSuck.g0 = 1.0;
    paseg->clqMasterSuck.g2 = 0.0;
    paseg->clqMasterSuck.g1 = 0.0;
    paseg->svtMasterSuck = 1.0;
    paseg->lmMasterSuck.gMin = 0.0;
    paseg->lmMasterSuck.gMax = 1.0;
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
            std::shared_ptr <ACP> pacp = PacpNew(acvk);
            paseg->achn[i].pacp = pacp;

            pacp->pvtacpc->pfnLoadAcpcFromBrx(std::static_pointer_cast <ACPC>(pacp), pbis);
        }

        if ((unk_6 & 2) != 0)
        {
            ACVK acvk = (ACVK)pbis->S8Read();

            std::shared_ptr <ACR> pacr = PacrNew(acvk);
            paseg->achn[i].pacr = pacr;
            pacr->pvtacrc->pfnLoadAcrcFromBrx(std::static_pointer_cast <ACRC>(pacr), pbis);
        }

        if ((unk_6 & 4) != 0)
        {
            ACVK acvk = (ACVK)pbis->S8Read();

            std::shared_ptr <ACS> pacs = PacsNew(acvk);
            paseg->achn[i].pacs = pacs;
            pacs->pvtacsb->pfnLoadAcsbFromBrx(std::static_pointer_cast <ACSB>(pacs), pbis);
        }

        if ((unk_6 & 8) != 0)
        {
            ACGK acgk = (ACGK)pbis->S8Read();

            std::shared_ptr <ACG> pacg = PacgNew(acgk);
            paseg->achn[i].pacgTwist = pacg;
            pacg->pvtacgb->pfnLoadAcgbFromBrx(std::static_pointer_cast <ACGB>(pacg), pbis);
        }

        if ((unk_6 & 0x10) != 0)
        {
            byte cpacgPose = pbis->U8Read();
            paseg->achn[i].cpacgPose = cpacgPose;

            paseg->achn[i].apacgPose.resize(cpacgPose);

            for (int a = 0; a < cpacgPose; a++)
            {
                ACGK acgk = (ACGK)pbis->S8Read();

                if (acgk != -1)
                {
                    std::shared_ptr <ACG> pacgPose = PacgNew(acgk);
                    paseg->achn[i].apacgPose[a] = pacgPose;
                    pacgPose->pvtacgb->pfnLoadAcgbFromBrx(std::static_pointer_cast <ACGB>(pacgPose), pbis);
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
    CloneLo(paseg, pasegBase);

    paseg->tMax = pasegBase->tMax;
    paseg->cchn = pasegBase->cchn;
    paseg->achn = pasegBase->achn;
    paseg->oidRoot = pasegBase->oidRoot;
    paseg->segrpt = pasegBase->segrpt;
    paseg->fDefault = pasegBase->fDefault;
    paseg->fHandsOff = pasegBase->fHandsOff;
    paseg->fRealClock = pasegBase->fRealClock;
    paseg->ceaApply = pasegBase->ceaApply;
    paseg->aeaApply = pasegBase->aeaApply;
    paseg->ceaFrame = pasegBase->ceaFrame;
    paseg->aeaFrame = pasegBase->aeaFrame;
    paseg->ceaRetract = pasegBase->ceaRetract;
    paseg->aeaRetract = pasegBase->aeaRetract;
    paseg->dlAsega = pasegBase->dlAsega;
    paseg->nPriority = pasegBase->nPriority;
    paseg->svtMaster = pasegBase->svtMaster;
    paseg->clqMasterSuck = pasegBase->clqMasterSuck;
    paseg->lmMasterSuck = pasegBase->lmMasterSuck;
    paseg->svtMasterSuck = pasegBase->svtMasterSuck;
    paseg->pchnStrip = pasegBase->pchnStrip;
    paseg->coidSearchRoot = pasegBase->coidSearchRoot;
    paseg->aoidSearchRoot = pasegBase->aoidSearchRoot;
}

void ApplyAseg(ASEG* paseg, ALO* paloAsegRoot, float tLocal, float svtLocal, int grfapl, ASEGA** ppasega)
{

}

void DeleteAseg(ASEG *paseg)
{
    delete paseg;
}

void* NewAsegbl()
{
    return new ASEGBL;
}

void DeleteAsegbl(LO* plo)
{
    delete(ASEGBL*)plo;
}

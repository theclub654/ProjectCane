#include "ac.h"

VTACPB g_vtacpb;
VTACPC g_vtacpc;

VTACRB g_vtacrb;
VTACRC g_vtacrc;

VTACSC g_vtacsc;
VTACSB g_vtacsb;

VTACGB g_vtacgb;
VTACGBW g_vtacgbw;
VTACGL g_vtacgl;
VTACGBLT g_vtacgblt;
VTACGBLP g_vtacgblp;

std::shared_ptr <ACP> PacpNew(ACVK acvk)
{
    std::shared_ptr <ACP> pacp{};

    if (acvk == ACVK_Bezier)
    {
        ACPB acpb{};
        acpb.pvtacpb = &g_vtacpb;
        pacp = std::make_shared <ACPB>(acpb);
    }

    else if (acvk < ACVK_Blend)
    {
        if (acvk == ACVK_Component)
        {
            ACPC acpc{};
            acpc.pvtacpc = &g_vtacpc;
            pacp = std::make_shared <ACPC>(acpc);
        }
    }

    if (pacp != nullptr)
        pacp->acvk = acvk;

    return pacp;
}

void LoadAcpcFromBrx(std::shared_ptr <ACPC> pacpc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacpc->apacg, pacpc->posDefault, pbis);
}

void LoadAcpbFromBrx(std::shared_ptr <ACPB> pacpb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacpb->ckvb, pacpb->akvb, pbis);
}

std::shared_ptr <ACR> PacrNew(ACVK acvk)
{
    std::shared_ptr <ACR> pacr{};

    if (acvk == ACVK_Bezier)
    {
        ACRB acrb{};
        acrb.pvtacrb = &g_vtacrb;
        pacr = std::make_shared <ACRB>(acrb);
    }

    else if (acvk < ACVK_Blend)
    {
        if (acvk == ACVK_Component)
        {
            ACRC acrc{};
            acrc.pvtacrc = &g_vtacrc;
            pacr = std::make_shared <ACRC>(acrc);
        }
    }

    if (pacr != nullptr)
        pacr->acvk = acvk;

    return pacr;
}

void LoadAcrcFromBrx(std::shared_ptr <ACRC> pacrc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacrc->apacg, pacrc->eulDefault, pbis);
}

void LoadAcrbFromBrx(std::shared_ptr <ACRB> pacrb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacrb->ckvb, pacrb->akvb, pbis);
}

std::shared_ptr <ACS> PacsNew(ACVK acvk)
{
    std::shared_ptr <ACS> pacs{};

    if (acvk == ACVK_Component)
    {
        ACSC acsc{};
        acsc.pvtacsc = &g_vtacsc;
        acsc.acvk = acvk;
        pacs = std::make_shared <ACSC>(acsc);
    }
    else
    {
        if (acvk == ACVK_Bezier)
        {
            ACSB acsb{};
            acsb.pvtacsb = &g_vtacsb;
            acsb.acvk = acvk;
            pacs = std::make_shared <ACSB>(acsb);
        }
    }

    if (pacs != nullptr)
        pacs->acvk = acvk;

    return pacs;
}

void LoadAcscFromBrx(std::shared_ptr <ACSC> pacsc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacsc->apacg, pacsc->vecDefault, pbis);
}

void LoadAcsbFromBrx(std::shared_ptr <ACSB> pacsb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacsb->ckvb, pacsb->akvb, pbis);
}

std::shared_ptr <ACG> PacgNew(ACGK acgk)
{
    std::shared_ptr <ACG> pacg{};

    switch (acgk)
    {
        case ACGK_Bezier:
        {
            ACGB acgb{};
            acgb.pvtacgb = &g_vtacgb;
            pacg = std::make_shared <ACGB>(acgb);
            break;
        }

        case ACGK_BezierWeighted:
        {
            ACGBW acgbw{};
            acgbw.pvtacgbw = &g_vtacgbw;
            pacg = std::make_shared <ACGBW>(acgbw);
            break;
        }

        case ACGK_Linear:
        {
            ACGL acgl{};
            acgl.pvtacgl = &g_vtacgl;
            pacg = std::make_shared <ACGL>(acgl);
            break;
        }

        case ACGK_BlendTwist:
        //acg = new ACGBLT;
        //acg->pvtacgblt = &g_vtacgblt;
        break;

        case ACGK_BlendPose:
        //
        //
        break;

        default:
        pacg = nullptr;
        break;
    }

    if (pacg != nullptr)
        pacg->acgk = acgk;

    return pacg;
}

void LoadAcgbFromBrx(std::shared_ptr <ACGB> pacgb, CBinaryInputStream* pbis)
{
    pacgb->ckgb = pbis->U16Read();

    for (int i = 0; i < pacgb->ckgb; i++)
    {
        pbis->S16Read();
        pbis->F32Read();
        pbis->S8Read();
        pbis->S8Read();
        pbis->F32Read();
        pbis->F32Read();
    }
}

void LoadAcgbwFromBrx(std::shared_ptr <ACGBW> pacgbw, CBinaryInputStream* pbis)
{
    pacgbw->ckgbw = pbis->U16Read();
    pacgbw->akgbw.resize(pacgbw->ckgbw);

    for (int i = 0; i < pacgbw->ckgbw; i++)
    {
        pbis->S16Read() * 0.01666667;
        pbis->F32Read();
        pbis->S8Read();
        pbis->S8Read();
        pbis->F32Read();
        pbis->F32Read();
        pbis->F32Read();
        pbis->F32Read();
    }
}

void LoadAcglFromBrx(std::shared_ptr <ACGL> pacgl, CBinaryInputStream* pbis)
{
    pacgl->ckgl = pbis->U16Read();
    pacgl->akgl.resize(pacgl->ckgl);

    for (int i = 0; i < pacgl->ckgl; i++)
    {
        pacgl->akgl[i].t = pbis->S16Read() * 0.01666667;
        pacgl->akgl[i].g = pbis->F32Read();
    }
}

void LoadAkvbFromBrx(int* pckvb, std::vector <KVB> &pakvb, CBinaryInputStream* pbis)
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

void LoadApacgFromBrx(std::shared_ptr<ACG>(&apacg)[3], glm::vec3 &pvecDefault, CBinaryInputStream* pbis)
{
    uint32_t temp0;
    byte unk_0 = pbis->U8Read();

    pvecDefault = pbis->ReadVector();

    for (int i = 0; i <= 2; i++)
    {
        temp0 = unk_0 & 1;
        unk_0 = (int)unk_0 >> 1;

        if (temp0 != 0)
        {
            ACGK acgk = (ACGK)pbis->U8Read();

            std::shared_ptr <ACG> acg = PacgNew(acgk);
            acg->pvtacgb->pfnLoadAcgbFromBrx(std::static_pointer_cast<ACGB>(acg), pbis);
            apacg[i] = acg;
        }
    }
}
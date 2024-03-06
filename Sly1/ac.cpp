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

ACP* PacpNew(ACVK acvk)
{
    ACP* acp{};
    if (acvk == ACVK_Bezier)
    {
        acp = new ACPB{};
        acp->pvtacpb = &g_vtacpb;
    }

    else if (acvk < ACVK_Blend)
    {
        if (acvk == ACVK_Component)
        {
            acp = new ACPC{};
            acp->pvtacpc = &g_vtacpc;
        }
    }

    if (acp != nullptr)
        acp->acvk = acvk;

    return acp;
}

void LoadAcpcFromBrx(ACPC* pacpc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacpc->apacg, pacpc->posDefault, pbis);
}

void LoadAcpbFromBrx(ACPB* pacpb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacpb->ckvb, pacpb->akvb, pbis);
}

ACR* PacrNew(ACVK acvk)
{
    ACR* acr{};

    if (acvk == ACVK_Bezier)
    {
        acr = new ACRB{};
        acr->pvtacrb = &g_vtacrb;
    }

    else if (acvk < ACVK_Blend)
    {
        if (acvk == ACVK_Component)
        {
            acr = new ACRC{};
            acr->pvtacrc = &g_vtacrc;
        }
    }

    if (acr != nullptr)
        acr->acvk = acvk;

    return acr;
}

void LoadAcrcFromBrx(ACRC* pacrc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacrc->apacg, pacrc->eulDefault, pbis);
}

void LoadAcrbFromBrx(ACRB* pacrb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacrb->ckvb, pacrb->akvb, pbis);
}

ACS* PacsNew(ACVK acvk)
{
    ACS* acs{};

    if (acvk == ACVK_Component)
    {
        acs = new ACSC{};
        acs->pvtacsc = &g_vtacsc;
    }
    else
    {
        if (acvk == ACVK_Bezier)
        {
            acs = new ACSB{};
            acs->pvtacsb = &g_vtacsb;
        }
    }

    if (acs != nullptr)
        acs->acvk = acvk;

    return acs;
}

void LoadAcscFromBrx(ACSC* pacsc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacsc->apacg, pacsc->vecDefault, pbis);
}

void LoadAcsbFromBrx(ACSB* pacsb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacsb->ckvb, pacsb->akvb, pbis);
}

ACG* PacgNew(ACGK acgk)
{
    ACG* acg{};

    switch (acgk) 
    {
        case ACGK_Bezier:
            acg = new ACGB{};
            acg->pvtacgb = &g_vtacgb;
        break;

        case ACGK_BezierWeighted:
            acg = new ACGBW{};
            acg->pvtacgbw = &g_vtacgbw;
        break;

        case ACGK_Linear:
            acg = new ACGL{};
            acg->pvtacgl = &g_vtacgl;
        break;

        case ACGK_BlendTwist:
            //acg = new ACGBLT;
            //acg->pvtacgblt = &g_vtacgblt;
        break;

        case ACGK_BlendPose:
            //
            //
        break;

        default:
            acg = nullptr;
        break;
    }

    if (acg != nullptr)
        acg->acgk = acgk;

    return acg;
}

void LoadAcgbFromBrx(ACGB* pacgb, CBinaryInputStream* pbis)
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

void LoadAcgbwFromBrx(ACGBW* pacgbw, CBinaryInputStream* pbis)
{
    pacgbw->ckgbw = pbis->U16Read();
    pacgbw->akgbw.resize(pacgbw->ckgbw);

    for (int i = 0; i < pacgbw->ckgbw; i++)
    {
        pacgbw->akgbw[i].t = pbis->S16Read() * 0.01666667;
        pacgbw->akgbw[i].g = pbis->F32Read();
        pacgbw->akgbw[i].kgbwtIn.kgbtk = (KGBTK)pbis->S8Read();
        pacgbw->akgbw[i].kgbwtOut.kgbtk = (KGBTK)pbis->S8Read();
        pacgbw->akgbw[i].kgbwtIn.dt = pbis->F32Read();
        pacgbw->akgbw[i].kgbwtIn.g = pbis->F32Read();
        pacgbw->akgbw[i].kgbwtOut.dt = pbis->F32Read();
        pacgbw->akgbw[i].kgbwtOut.g = pbis->F32Read();
    }
}

void LoadAcglFromBrx(ACGL* pacgl, CBinaryInputStream* pbis)
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

void LoadApacgFromBrx(std::vector <ACG> &apacg, glm::vec3& pvecDefault, CBinaryInputStream* pbis)
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

            ACG* acg = PacgNew(acgk);
            acg->pvtacgb->pfnLoadAcgbFromBrx((ACGB*)acg, pbis);
        }
    }
}
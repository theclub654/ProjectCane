#include "acgb.h"

void LoadAcgbFromBrx(ACGB* pacgb, CBinaryInputStream* pbis)
{
    pacgb->ckgb = pbis->U16Read();
    pacgb->akgb.resize(pacgb->ckgb);

    for (int i = 0; i < pacgb->ckgb; ++i)
    {
        KGB& kgb = pacgb->akgb[i];

        kgb.t = pbis->S16Read() * (1.0f / 60.0f);
        kgb.g = pbis->F32Read();

        kgb.kgbtIn.kgbtk  = static_cast<KGBTK>(pbis->S8Read());
        kgb.kgbtOut.kgbtk = static_cast<KGBTK>(pbis->S8Read());

        kgb.kgbtIn.gSlope  = pbis->F32Read();
        kgb.kgbtOut.gSlope = pbis->F32Read();
    }
}

void EvaluateAcgb(ACGB* pacgb, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg)
{
    float dt;
    float dtSeg;
    KGB* pkgb;

    FindKey(t, grfeval, sizeof(KGB), pacgb->ckgb, (char*)pacgb->akgb.data(), &dt, &dtSeg, reinterpret_cast<void**>(&pkgb));

    if (pkgb->kgbtOut.kgbtk == KGBTK_Step)
    {
        if (pg)
            *pg = pkgb->g;

        if (pdg)
            *pdg = 0.0f;
    }
    else
        EvaluateBezierFloat(dtSeg, dt, svt, pkgb->g, pkgb->kgbtOut.gSlope, pkgb[1].g, pkgb[1].kgbtIn.gSlope, pg, pdg, nullptr);
}

void GetAcgbTimes(ACGB* pacgb, int* pct, std::vector <float> &pat)
{
    pat.clear();

    if (!pacgb)
    {
        *pct = 0;
        return;
    }

    *pct = pacgb->ckgb;

    for (int i = 0; i < pacgb->ckgb; ++i)
        pat.push_back(pacgb->akgb[i].t);
}

VTACGB g_vtacgb;
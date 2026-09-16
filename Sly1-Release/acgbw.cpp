#include "acgbw.h"

void LoadAcgbwFromBrx(ACGBW* pacgbw, CBinaryInputStream* pbis)
{
    pacgbw->ckgbw = pbis->U16Read();
    pacgbw->akgbw.resize(pacgbw->ckgbw);

    for (int i = 0; i < pacgbw->ckgbw; ++i)
    {
        KGBW& kgbw = pacgbw->akgbw[i];

        kgbw.t = pbis->S16Read() * (1.0f / 60.0f);
        kgbw.g = pbis->F32Read();

        kgbw.kgbwtIn.kgbtk  = static_cast<KGBTK>(pbis->S8Read());
        kgbw.kgbwtOut.kgbtk = static_cast<KGBTK>(pbis->S8Read());

        kgbw.kgbwtIn.dt = pbis->F32Read();
        kgbw.kgbwtIn.g  = kgbw.g - pbis->F32Read();

        kgbw.kgbwtOut.dt = pbis->F32Read();
        kgbw.kgbwtOut.g  = kgbw.g + pbis->F32Read();
    }
}

void EvaluateAcgbw(ACGBW* pacgbw, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg)
{
    float dt;
    float dtSeg;
    KGBW* pkgbw;

    FindKey(t, grfeval, sizeof(KGBW), pacgbw->ckgbw, reinterpret_cast<char*>(pacgbw->akgbw.data()), &dt, &dtSeg, reinterpret_cast<void**>(&pkgbw));

    if (pkgbw->kgbwtOut.kgbtk == KGBTK_Step)
    {
        if (pg)
            *pg = pkgbw->g;

        if (pdg)
            *pdg = 0.0f;
    }
    else
        EvaluateBezierWeightedFloat(dtSeg, dt, svt, pkgbw->g, pkgbw->kgbwtOut.dt, pkgbw->kgbwtOut.g, pkgbw[1].g, pkgbw[1].kgbwtIn.dt, pkgbw[1].kgbwtIn.g, pg, pdg, nullptr);
}

void GetAcgbwTimes(ACGBW* pacgbw, int* pct, std::vector<float>& pat)
{
    *pct = pacgbw->ckgbw;

    pat.clear();
    pat.reserve(pacgbw->ckgbw);

    for (int i = 0; i < pacgbw->ckgbw; i++)
        pat.push_back(pacgbw->akgbw[i].t);
}

VTACGBW g_vtacgbw;
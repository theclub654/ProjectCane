#include "acgl.h"

void LoadAcglFromBrx(ACGL* pacgl, CBinaryInputStream* pbis)
{
    pacgl->ckgl = pbis->U16Read();
    pacgl->akgl.resize(pacgl->ckgl);

    for (int i = 0; i < pacgl->ckgl; ++i)
    {
        KGL& kgl = pacgl->akgl[i];

        kgl.t = pbis->S16Read() * (1.0f / 60.0f);
        kgl.g = pbis->F32Read();
    }
}

void EvaluateAcgl(ACGL* pacgl, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg)
{
    float dt;
    float dtSeg;
    KGL* pkgl;

    FindKey(t, grfeval, sizeof(KGL), pacgl->ckgl, reinterpret_cast<char*>(pacgl->akgl.data()), &dt, &dtSeg, reinterpret_cast<void**>(&pkgl));

    if (dtSeg < 0.0001f)
    {
        if (pg)
            *pg = pkgl->g;

        if (pdg)
            *pdg = 0.0f;
    }
    else
    {
        if (pg)
            *pg = (1.0f - dt / dtSeg) * pkgl->g + (dt / dtSeg) * pkgl[1].g;

        if (pdg)
            *pdg = (svt * (pkgl[1].g - pkgl->g)) / dtSeg;
    }
}

void GetAcglTimes(ACGL* pacgl, int* pct, std::vector <float>& pat)
{
    *pct = pacgl->ckgl;

    pat.clear();
    pat.reserve(pacgl->ckgl);

    for (int i = 0; i < pacgl->ckgl; i++)
        pat.push_back(pacgl->akgl[i].t);
}

VTACGL g_vtacgl;
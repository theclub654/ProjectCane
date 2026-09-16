#include "acgblp.h"
#include "blend.h"

void EvaluateAcgblp(ACGBLP* pacgblp, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg)
{
    ASEGBL* pasegbl = pacgblp->pasegbl;
    MRSGC* pmrsgc = pacgblp->pmrsgc;

    float gBlend = 0.0f;
    float dgBlend = 0.0f;
    float phase = 0.0f;
    int imrsg = -1;

    if (pmrsgc == nullptr)
    {
        if (pasegbl->tMax != 0.0f)
            phase = t / pasegbl->tMax;
    }
    else
    {
        float dtMarker = 0.0f;
        imrsg = ImrsgLookUp(t, pasegbl->tMax, pmrsgc->cmrsg, pmrsgc->amrsg.data(), &dtMarker);

        if (imrsg >= 0 && imrsg < pmrsgc->cmrsg && pmrsgc->amrsg[imrsg].dt != 0.0f)
            phase = dtMarker / pmrsgc->amrsg[imrsg].dt;
    }

    for (int ibl = 0; ibl < pasegbl->cbl; ++ibl)
    {
        std::byte* record = pasegbl->abl.data() + static_cast<std::size_t>(ibl) * pasegbl->cbBl;
        BL* pbl = reinterpret_cast<BL*>(record);

        const float weight = pbl->u;

        if (weight == 0.0f)
            continue;

        ACG* pacg = ibl < static_cast<int>(pacgblp->mpiblpacg.size()) ? pacgblp->mpiblpacg[ibl] : nullptr;

        if (pacg == nullptr)
        {
            if (palo != nullptr && pacgblp->ipose >= 0)
                gBlend += weight * palo->globset.agPosesOrig[pacgblp->ipose];

            continue;
        }

        if (pbl->paseg == nullptr)
            continue;

        float tLocal;
        float svtLocal;

        if (pmrsgc == nullptr)
        {
            tLocal = phase * pbl->paseg->tMax;
            svtLocal = pasegbl->tMax != 0.0f ? svt * pbl->paseg->tMax / pasegbl->tMax : 0.0f;
        }
        else
        {
            MRSG* sourceMarkers = reinterpret_cast<MRSG*>(record + pmrsgc->ibMrsg);
            const MRSG& sourceMarker = sourceMarkers[imrsg];
            const MRSG& blendMarker = pmrsgc->amrsg[imrsg];

            tLocal = sourceMarker.t + phase * sourceMarker.dt;

            if (tLocal >= pbl->paseg->tMax)
                tLocal -= pbl->paseg->tMax;

            svtLocal = blendMarker.dt != 0.0f ? svt * sourceMarker.dt / blendMarker.dt : 0.0f;
        }

        float gCurrent = 0.0f;
        float dgCurrent = 0.0f;

        pacg->pvtacg->pfnEvaluateAcg(pacg, palo, tLocal, svtLocal, grfeval, pg != nullptr ? &gCurrent : nullptr, pdg != nullptr ? &dgCurrent : nullptr);

        gBlend += weight * gCurrent;
        dgBlend += weight * dgCurrent;
    }

    if (pg != nullptr)
        *pg = gBlend;

    if (pdg != nullptr)
        *pdg = dgBlend;
}

VTACGBLP g_vtacgblp;
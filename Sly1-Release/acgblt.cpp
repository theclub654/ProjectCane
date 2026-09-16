#include "acgblt.h"
#include "blend.h"

void EvaluateAcgblt(ACGBLT* pacgblt, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg)
{
    ASEGBL* pasegbl = pacgblt->pasegbl;
    MRSGC* pmrsgc = pacgblt->pmrsgc;

    float radBlend = 0.0f;
    float angularVelocityBlend = 0.0f;
    float weightTotal = 0.0f;

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

        ACG* pacg = ibl < static_cast<int>(pacgblt->mpiblpacg.size()) ? pacgblt->mpiblpacg[ibl] : nullptr;

        float radCurrent = radBlend;
        float angularVelocityCurrent = 0.0f;

        if (pacg != nullptr && pbl->paseg != nullptr)
        {
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

            pacg->pvtacg->pfnEvaluateAcg(pacg, palo, tLocal, svtLocal, grfeval, &radCurrent, &angularVelocityCurrent);
        }
        else if (palo != nullptr)
            radCurrent = palo->palox->ikh.radTwist;

        if (weightTotal == 0.0f)
            radBlend = radCurrent;
        else
        {
            const float angularDifference = RadNormalize(RadNormalize(radCurrent) - radBlend);
            radBlend += angularDifference * weight / (weightTotal + weight);
        }

        angularVelocityBlend += weight * angularVelocityCurrent;
        weightTotal += weight;
        radBlend = RadNormalize(radBlend);
    }

    if (pg != nullptr)
        *pg = radBlend;

    if (pdg != nullptr)
        *pdg = angularVelocityBlend;
}

VTACGBLT g_vtacgblt;
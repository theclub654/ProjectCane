#include "acpbl.h"
#include "asegbl.h"

void EvaluateAcpbl(ACPBL* pacpbl, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* ppos, glm::vec3* pv)
{
    if (ppos != nullptr)
        *ppos = glm::vec3(0.0f);

    if (pv != nullptr)
        *pv = glm::vec3(0.0f);

    ASEGBL* pasegbl = pacpbl->pasegbl;
    MRSGC* pmrsgc = pacpbl->pmrsgc;

    float phase;
    int imrsg = 0;

    if (pmrsgc == nullptr)
    {
        phase = t / pasegbl->tMax;
    }
    else
    {
        float dtMarker;
        imrsg = ImrsgLookUp(t, pasegbl->tMax, pmrsgc->cmrsg, pmrsgc->amrsg.data(), &dtMarker);
        phase = dtMarker / pmrsgc->amrsg[imrsg].dt;
    }

    for (int ibl = 0; ibl < pasegbl->cbl; ++ibl)
    {
        std::byte* record = pasegbl->abl.data() + static_cast<std::size_t>(ibl) * pasegbl->cbBl;
        BL* pbl = reinterpret_cast<BL*>(record);

        const float weight = pbl->u;

        if (weight == 0.0f)
            continue;

        ACP* pacp = pacpbl->mpiblpacp[ibl];

        if (pacp == nullptr)
        {
            if (ppos != nullptr)
                *ppos += (palo != nullptr ? palo->posOrig : g_vecZero) * weight;

            continue;
        }

        ASEG* paseg = pbl->paseg;
        float tLocal;
        float svtLocal;

        if (pmrsgc == nullptr)
        {
            tLocal = phase * paseg->tMax;
            svtLocal = svt * paseg->tMax / pasegbl->tMax;
        }
        else
        {
            MRSG* sourceMarkers = reinterpret_cast<MRSG*>(record + pmrsgc->ibMrsg);
            const MRSG& sourceMarker = sourceMarkers[imrsg];

            tLocal = sourceMarker.t + phase * sourceMarker.dt;

            if (tLocal > paseg->tMax)
                tLocal -= paseg->tMax;

            svtLocal = svt * sourceMarker.dt / pmrsgc->amrsg[imrsg].dt;
        }

        glm::vec3 pos{};
        glm::vec3 velocity{};

        pacp->pvtacp->pfnEvaluateAcp(pacp, palo, tLocal, svtLocal, grfeval, ppos != nullptr ? &pos : nullptr, pv != nullptr ? &velocity : nullptr);

        if (ppos != nullptr)
            *ppos += pos * weight;

        if (pv != nullptr)
            *pv += velocity * weight;
    }

    if (palo != nullptr && palo->pvtalo->pfnAdjustAloPosition != nullptr)
        palo->pvtalo->pfnAdjustAloPosition(palo, ppos, pv);

}

VTACPBL g_vtacpbl;

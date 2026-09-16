#include "steprun.h"
#include "acpbl.h"
#include "acrbl.h"

void PostStepLoadRun(STEP* pstep, BLRUN* ablrun, ASEGBL** ppasegbl)
{
    constexpr int cbl = 6;
    constexpr int cbBl = sizeof(BLRUN);

    if (ablrun[0].paseg == nullptr)
    {
        if (ppasegbl != nullptr)
            *ppasegbl = nullptr;

        return;
    }

    ablrun[0].u = 1.0f;

    FillBlAmrsg(cbBl, cbl, ablrun, offsetof(BLRUN, mpfootsmrsgLeft), 2, (OID*)s_mpfootsoidLeft);
    FillBlAmrsg(cbBl, cbl, ablrun, offsetof(BLRUN, mpfootsmrsgRight), 2, (OID*)s_mpfootsoidRight);

    for (int ibl = 0; ibl < cbl; ++ibl)
    {
        BLRUN& blrun = ablrun[ibl];
        ASEG* paseg = blrun.paseg;

        if (paseg == nullptr || paseg->pchnStrip == nullptr || paseg->pchnStrip->pacp == nullptr)
            continue;

        ACP* pacpStrip = paseg->pchnStrip->pacp.get();

        glm::vec3 posStart{};
        glm::vec3 posEnd{};
        glm::vec3 posPlantLeft{};
        glm::vec3 posPlantRight{};

        pacpStrip->pvtacp->pfnEvaluateAcp(pacpStrip, pstep, 0.0f, 1.0f, static_cast<GRFEVAL>(0), &posStart, nullptr);
        pacpStrip->pvtacp->pfnEvaluateAcp(pacpStrip, pstep, paseg->tMax, 1.0f, static_cast<GRFEVAL>(0), &posEnd, nullptr);

        blrun.v = paseg->tMax != 0.0f ? (posStart - posEnd) / paseg->tMax : glm::vec3(0.0f);

        CHN* pchnLeft = PchnFindAseg(paseg, pstep->paloLeftFoot);

        if (pchnLeft != nullptr && pchnLeft->pacp != nullptr)
        {
            ACP* pacpLeft = pchnLeft->pacp.get();
            pacpLeft->pvtacp->pfnEvaluateAcp(pacpLeft, pstep->paloLeftFoot, blrun.mpfootsmrsgLeft[0].t, 1.0f, static_cast<GRFEVAL>(0), &posPlantLeft, nullptr);
        }

        CHN* pchnRight = PchnFindAseg(paseg, pstep->paloRightFoot);

        if (pchnRight != nullptr && pchnRight->pacp != nullptr)
        {
            ACP* pacpRight = pchnRight->pacp.get();
            pacpRight->pvtacp->pfnEvaluateAcp(pacpRight, pstep->paloRightFoot, blrun.mpfootsmrsgRight[0].t, 1.0f, static_cast<GRFEVAL>(0), &posPlantRight, nullptr);
        }

        blrun.posPlant = (posPlantLeft + posPlantRight) * 0.5f;
    }

    ASEGBL* pasegbl = nullptr;

    EnsureAsegBlendDynamic(pstep, cbBl, cbl, ablrun, 2, s_mpimrsgcibRun, s_mpimrsgccmrsgRun, &pasegbl);

    if (pasegbl == nullptr)
    {
        if (ppasegbl != nullptr)
            *ppasegbl = nullptr;

        return;
    }

    MRSGC* pmrsgcRight = pasegbl->amrsgc.size() > 1 ? &pasegbl->amrsgc[1] : nullptr;

    for (CHN& chn : pasegbl->achn)
    {
        // Retail run blends reserve channels 53..58 for the two foot
        // marker groups.  The reconstructed 55..60 range shifted every
        // association by two, so the body translated while the planted
        // feet/run pose were blended from the wrong channels.
        if (chn.oid < 53 || chn.oid > 58)
        {
            if (chn.pacp != nullptr)
                static_cast<ACPBL*>(chn.pacp.get())->pmrsgc = nullptr;

            if (chn.pacr != nullptr)
                static_cast<ACRBL*>(chn.pacr.get())->pmrsgc = nullptr;
        }
        else if (chn.oid >= 56)
        {
            if (chn.pacp != nullptr)
                static_cast<ACPBL*>(chn.pacp.get())->pmrsgc = pmrsgcRight;

            if (chn.pacr != nullptr)
                static_cast<ACRBL*>(chn.pacr.get())->pmrsgc = pmrsgcRight;
        }
    }

    ReblendAsegbl(pasegbl, cbBl, cbl, ablrun);

    if (ppasegbl != nullptr)
        *ppasegbl = pasegbl;
}

void CalculateBlrunV(int cblrun, const BLRUN* ablrun, const MRSG* pmrsgBlend, glm::vec3* pv)
{
    if (pv == nullptr)
        return;

    *pv = glm::vec3(0.0f);

    if (ablrun == nullptr ||
        pmrsgBlend == nullptr ||
        pmrsgBlend->dt == 0.0f)
    {
        return;
    }

    for (int iblrun = 0; iblrun < cblrun; ++iblrun)
    {
        const BLRUN& blrun = ablrun[iblrun];

        if (blrun.paseg == nullptr || blrun.u == 0.0f)
            continue;

        const float sourceDt = blrun.mpfootsmrsgLeft[0].dt;

        if (sourceDt == 0.0f)
            continue;

        const float timeScale = sourceDt / pmrsgBlend->dt;
        *pv += blrun.v * (blrun.u * timeScale);
    }
}

void ProjectStepRunPlant(STEP* pstep, const BLRUN* ablrun, float uPlant, float rJt, int iblrunWalk, int iblrunRun, glm::vec3* pposPlant)
{
    const BLRUN& walk = ablrun[iblrunWalk];
    const BLRUN& run = ablrun[iblrunRun];

    const float walkWeight = 1.0f - rJt;
    const float runWeight = rJt;

    const float dtWalk = walkWeight * walk.mpfootsmrsgLeft[0].dt;
    const float dtRun = runWeight * run.mpfootsmrsgLeft[0].dt;
    const float dtTotal = dtWalk + dtRun;

    glm::vec3 vBaseLocal(0.0f);

    if (dtTotal != 0.0f)
        vBaseLocal = walk.v * (dtWalk / dtTotal) + run.v * (dtRun / dtTotal);

    const float dtPlant = uPlant * (walkWeight * walk.mpfootsmrsgLeft[1].dt + runWeight * run.mpfootsmrsgLeft[1].dt);

    const glm::vec3 posBaseLocal = pstep->geomLocal.apos[pstep->iposBase] + vBaseLocal * dtPlant;
    const glm::vec3 posPlantLocal = glm::mix(walk.posPlant, run.posPlant, rJt);
    const glm::vec3 dposLocal = posPlantLocal - posBaseLocal;
    const glm::vec3 dposWorld = pstep->matTarget * dposLocal;

    *pposPlant = *pstep->pposBase + dposWorld;
}

int FIntersectStepRunPlant(STEP* pstep, const glm::vec3* pposUp, const glm::vec3* pposDown, float* pu)
{
    glm::vec3 posUp = *pposUp;
    glm::vec3 posDown = *pposDown;

    LSG lsg{};
    int cpso = 0;
    std::vector <SO*> apso;

    IntersectSwBoundingBox(pstep->psw, static_cast<SO*>(pstep), &posUp, &posDown, nullptr, nullptr, apso);

    SO* psoHit = PsoHitTestLineObjects(0, &posUp, &posDown, apso, &lsg);

    if (pu != nullptr)
    {
        if (psoHit == nullptr)
            *pu = 1.0f;
        else
        {
            const float segmentLength = glm::distance(posUp, posDown);
            *pu = segmentLength > 0.0001f ? glm::distance(posUp, lsg.apos[0]) / segmentLength : 0.0f;
        }
    }

    return psoHit != nullptr;
}

int FReblendAbl2D(int cbBl, int c1, int c2, BL* abl, float rUp, float rDown, float rJt)
{
    if (abl == nullptr || c1 != 3 || c2 != 2)
        return 0;

    float au1[3] =
    {
        1.0f - rUp - rDown,
        rUp,
        rDown
    };

    float au2[2] =
    {
        1.0f - rJt,
        rJt
    };

    bool changed = false;
    unsigned char* blendData = reinterpret_cast<unsigned char*>(abl);

    for (int i1 = 0; i1 < c1; ++i1)
    {
        for (int i2 = 0; i2 < c2; ++i2)
        {
            const int ibl = i1 * c2 + i2;
            BL* pbl = reinterpret_cast<BL*>(blendData + static_cast<std::size_t>(ibl) * cbBl);
            const float weight = au1[i1] * au2[i2];

            if (pbl->u != weight)
            {
                pbl->u = weight;
                changed = true;
            }
        }
    }

    return changed;
}

void AdjustStepRun(STEP* pstep, BLRUN* ablrun, ASEGBL* pasegbl, ASEGA* pasega)
{
    float rJt = 0.0f;

    if (ablrun[1].paseg != nullptr)
    {
        const float targetVelocity = pstep->vTarget.x;
        const float numerator = ablrun[0].mpfootsmrsgLeft[0].dt * (targetVelocity - ablrun[0].v.x);
        const float denominator = numerator - ablrun[1].mpfootsmrsgLeft[0].dt * (targetVelocity - ablrun[1].v.x);

        if (denominator != 0.0f)
            rJt = glm::clamp(numerator / denominator, 0.0f, 1.0f);
    }

    float rUp = 0.0f;
    float rDown = 0.0f;

    if (ablrun[2].paseg != nullptr && pasegbl->amrsgc.size() >= 2 && pasegbl->amrsgc[0].amrsg.size() >= 2 && !pasegbl->amrsgc[1].amrsg.empty())
    {
        const std::vector<MRSG>& amrsgLeft = pasegbl->amrsgc[0].amrsg;
        const std::vector<MRSG>& amrsgRight = pasegbl->amrsgc[1].amrsg;

        float dtLeft = amrsgLeft[0].t - pasega->tLocal;

        if (dtLeft <= 0.0f)
            dtLeft += pasegbl->tMax;

        float dtRight = amrsgRight[0].t - pasega->tLocal;

        if (dtRight <= 0.0f)
            dtRight += pasegbl->tMax;

        const float markerDuration = amrsgLeft[1].dt;
        float uPlant = 0.0f;

        if (markerDuration != 0.0f)
            uPlant = std::min(dtLeft / markerDuration, dtRight / markerDuration);

        glm::vec3 posPlantUp{};
        glm::vec3 posPlantFlat{};
        glm::vec3 posPlantDown{};

        ProjectStepRunPlant(pstep, ablrun, uPlant, rJt, 2, 3, &posPlantUp);
        ProjectStepRunPlant(pstep, ablrun, uPlant, rJt, 0, 1, &posPlantFlat);

        float uIntersect = 1.0f;

        if (FIntersectStepRunPlant(pstep, &posPlantUp, &posPlantFlat, &uIntersect))
            rUp = 1.0f - uIntersect;
        else if (ablrun[4].paseg != nullptr)
        {
            ProjectStepRunPlant(pstep, ablrun, uPlant, rJt, 4, 5, &posPlantDown);

            if (FIntersectStepRunPlant(pstep, &posPlantFlat, &posPlantDown, &uIntersect))
                rDown = uIntersect;
            else
                rDown = 1.0f;
        }
    }

    if (FReblendAbl2D(sizeof(BLRUN), 3, 2, reinterpret_cast<BL*>(ablrun), rUp, rDown, rJt))
    {
        ReblendAsegbl(pasegbl, sizeof(BLRUN), 6, ablrun);
        CalculateBlrunV(6, ablrun, pasegbl->amrsgc[0].amrsg.data(), &pstep->vTarget);
    }
}

void SetStepRunTarget(STEP* pstep, float rad, float u, BLRUN* ablrun, ASEGBL* pasegbl)
{
    pstep->radTarget = rad;

    if (ablrun[1].paseg == nullptr)
    {
        ablrun[0].u = 1.0f;
        ablrun[1].u = 0.0f;
    }
    else
    {
        ablrun[0].u = 1.0f - u;
        ablrun[1].u = u;
    }

    ablrun[2].u = 0.0f;
    ablrun[3].u = 0.0f;
    ablrun[4].u = 0.0f;
    ablrun[5].u = 0.0f;

    ReblendAsegbl(pasegbl, sizeof(BLRUN), 6, ablrun);
    CalculateBlrunV(6, ablrun, pasegbl->amrsgc[0].amrsg.data(), &pstep->vTarget);
}

int s_mpfootsoidLeft[2] = { 0x11E, 0x11F };
int s_mpfootsoidRight[2] = { 0x120, 0x121 };
int s_mpimrsgccmrsgRun[2] = { 2, 2 };
int s_mpimrsgcibRun[2] =
{
    static_cast<int>(offsetof(BLRUN, mpfootsmrsgLeft)),
    static_cast<int>(offsetof(BLRUN, mpfootsmrsgRight))
};

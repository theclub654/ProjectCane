#include "acrbl.h"
#include "alo.h"
#include "asegbl.h"
#include "blend.h"

void EvaluateAcrbl(ACRBL* pacrbl, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::mat3* pmat, glm::vec3* pw)
{
    if (pmat != nullptr)
        *pmat = glm::mat3(1.0f);
    if (pw != nullptr)
        *pw = glm::vec3(0.0f);

    if (pacrbl == nullptr || pacrbl->pasegbl == nullptr)
        return;

    ASEGBL* pasegbl = pacrbl->pasegbl;
    MRSGC* pmrsgc = pacrbl->pmrsgc;

    glm::vec3 eulSum(0.0f);
    glm::vec3 deulSum(0.0f);

    float blendT = 0.0f;
    int imrsg = 0;

    if (pmrsgc != nullptr && pmrsgc->cmrsg > 0 && !pmrsgc->amrsg.empty())
    {
        float dtBest = 0.0f;
        imrsg = ImrsgLookUp(t, pasegbl->tMax, pmrsgc->cmrsg,
            pmrsgc->amrsg.data(), &dtBest);

        if (imrsg < 0 || imrsg >= static_cast<int>(pmrsgc->amrsg.size()))
            imrsg = 0;

        const float markerDuration = pmrsgc->amrsg[imrsg].dt;
        if (std::abs(markerDuration) > 0.000001f)
            blendT = dtBest / markerDuration;
    }
    else if (std::abs(pasegbl->tMax) > 0.000001f)
    {
        blendT = t / pasegbl->tMax;
    }

    float weightSum = 0.0f;

    for (int ibl = 0; ibl < pasegbl->cbl; ibl++)
    {
        const std::size_t recordOffset = static_cast<std::size_t>(ibl) * pasegbl->cbBl;
        if (pasegbl->cbBl < static_cast<int>(sizeof(BL)) ||
            recordOffset + sizeof(BL) > pasegbl->abl.size())
            break;

        std::byte* record = pasegbl->abl.data() + recordOffset;
        BL* pbl = reinterpret_cast<BL*>(record);
        const float weight = pbl->u;

        if (weight == 0.0f || !std::isfinite(weight))
            continue;

        ACR* pacr = ibl < static_cast<int>(pacrbl->mpiblpacr.size())
            ? pacrbl->mpiblpacr[ibl]
            : nullptr;

        glm::vec3 eul(0.0f);
        glm::vec3 deul(0.0f);

        if (pacr == nullptr)
        {
            if (pmat == nullptr)
                continue;

            eul = palo ? palo->eulOrig : g_vecZero;
        }
        else
        {
            ASEG* paseg = pbl->paseg;
            if (paseg == nullptr || pacr->pvtacr == nullptr ||
                pacr->pvtacr->pfnEvaluateAcrEul == nullptr)
                continue;

            float localT;
            float localSvt;

            if (pmrsgc != nullptr && pmrsgc->cmrsg > 0 && !pmrsgc->amrsg.empty())
            {
                const std::size_t markerOffset = static_cast<std::size_t>(pmrsgc->ibMrsg) +
                    static_cast<std::size_t>(imrsg) * sizeof(MRSG);
                if (markerOffset + sizeof(MRSG) > static_cast<std::size_t>(pasegbl->cbBl))
                    continue;

                const MRSG* sourceMarker = reinterpret_cast<const MRSG*>(record + markerOffset);
                localT = sourceMarker->t + blendT * sourceMarker->dt;

                if (localT > paseg->tMax)
                    localT -= paseg->tMax;

                const float markerDuration = pmrsgc->amrsg[imrsg].dt;
                localSvt = std::abs(markerDuration) > 0.000001f
                    ? svt * sourceMarker->dt / markerDuration
                    : 0.0f;
            }
            else
            {
                localT = blendT * paseg->tMax;
                localSvt = std::abs(pasegbl->tMax) > 0.000001f
                    ? svt * paseg->tMax / pasegbl->tMax
                    : 0.0f;
            }

            glm::vec3* peul = pmat ? &eul : nullptr;
            glm::vec3* pdeul = pw ? &deul : nullptr;

            pacr->pvtacr->pfnEvaluateAcrEul(
                pacr, palo, localT, localSvt, grfeval, peul, pdeul);

            if (pw)
                deulSum += weight * deul;
        }

        if (pmat)
        {
            if (!pacrbl->fSlerp)
            {
                eulSum += weight * eul;
            }
            else
            {
                glm::mat3 mat;

                if (weightSum == 0.0f)
                {
                    LoadRotateMatrixEuler(&eul, pmat);
                }
                else
                {
                    LoadRotateMatrixEuler(&eul, &mat);

                    glm::mat3 dmat;
                    glm::vec3 normal;
                    float rad;

                    CalculateDmat(&mat, pmat, &dmat);
                    DecomposeRotateMatrixRad(&dmat, &rad, &normal);
                    const float totalWeight = weightSum + weight;
                    if (std::abs(totalWeight) <= 0.000001f)
                        continue;

                    LoadRotateMatrixRad(rad * weightSum / totalWeight, &normal, &dmat);

                    *pmat = dmat * mat;
                }

                weightSum += weight;
            }
        }
    }

    if (pmat && !pacrbl->fSlerp)
        LoadRotateMatrixEuler(&eulSum, pmat);

    if (pw)
        ConvertDeulToW(&eulSum, &deulSum, pw);

    if (palo && palo->pvtalo && palo->pvtalo->pfnAdjustAloRotation)
        palo->pvtalo->pfnAdjustAloRotation(palo, pmat, pw);
}

VTACRBL g_vtacrbl;

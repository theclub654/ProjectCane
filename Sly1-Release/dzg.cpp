#include "dzg.h"
#include <cmath>
#include <limits>

namespace
{
    float EeSaturateFloat(double value)
    {
        const double maxFloat = static_cast<double>((std::numeric_limits<float>::max)());
        if (value > maxFloat)
            return (std::numeric_limits<float>::max)();
        if (value < -maxFloat)
            return -(std::numeric_limits<float>::max)();
        return static_cast<float>(value);
    }

    void AddScaledEe(glm::vec3* pdst, const glm::vec3& src, float scale)
    {
        pdst->x = EeSaturateFloat(static_cast<double>(pdst->x) + static_cast<double>(src.x) * scale);
        pdst->y = EeSaturateFloat(static_cast<double>(pdst->y) + static_cast<double>(src.y) * scale);
        pdst->z = EeSaturateFloat(static_cast<double>(pdst->z) + static_cast<double>(src.z) * scale);
    }

    glm::vec3 SubtractEe(const glm::vec3& lhs, const glm::vec3& rhs)
    {
        return glm::vec3(
            EeSaturateFloat(static_cast<double>(lhs.x) - rhs.x),
            EeSaturateFloat(static_cast<double>(lhs.y) - rhs.y),
            EeSaturateFloat(static_cast<double>(lhs.z) - rhs.z));
    }

    float DotEe(const glm::vec3& lhs, const glm::vec3& rhs)
    {
        return EeSaturateFloat(
            static_cast<double>(lhs.x) * rhs.x +
            static_cast<double>(lhs.y) * rhs.y +
            static_cast<double>(lhs.z) * rhs.z);
    }

    void ClampVectorLength(glm::vec3* pvec, float lengthMax)
    {
        const double x = pvec->x;
        const double y = pvec->y;
        const double z = pvec->z;
        const double lengthSq = x * x + y * y + z * z;
        const double lengthMaxSq = static_cast<double>(lengthMax) * lengthMax;

        if (lengthSq > lengthMaxSq)
        {
            const double scale = static_cast<double>(lengthMax) / std::sqrt(lengthSq);
            pvec->x = static_cast<float>(x * scale);
            pvec->y = static_cast<float>(y * scale);
            pvec->z = static_cast<float>(z * scale);
        }
    }

}

void InitDzg(DZG* pdzg, int cpxp)
{
    *pdzg = {};

    pdzg->cdzMax = cpxp * 2;
    pdzg->adz.resize(pdzg->cdzMax);

    InitDl(&pdzg->dlPos,   offsetof(DZ, dle));
    InitDl(&pdzg->dlZero,  offsetof(DZ, dle));
    InitDl(&pdzg->dlMax,   offsetof(DZ, dle));
    InitDl(&pdzg->dlUncat, offsetof(DZ, dle));

    int cag = pdzg->cdzMax * pdzg->cdzMax;

    pdzg->aagPos.assign(cag, 0.0f);
    pdzg->aagPosCrout.assign(cag, 0.0f);
    pdzg->asdv.assign(pdzg->cdzMax, 0.0f);
    pdzg->adsfPos.assign(pdzg->cdzMax, 0.0f);
}

void AppendDzgDz(DZG* pdzg, DZ* pdzOther)
{
    for (int i = 0; i < pdzg->cdz; ++i) 
    {
        DZ* pdz = &pdzg->adz[i];

        if (pdz->mpipxpdv.empty()) {
            continue;
        }

        pdz->mpidzg[pdzg->cdz] = DotEe(pdzOther->f, pdz->mpipxpdv[pdzOther->pxp->ipxp]);
    }

    AppendDlEntry(&pdzg->dlUncat, pdzOther);
    ++pdzg->cdz;
}

void FillDzgDz(DZG* pdzg, GRFSG grfsg, DZ* pdzForce, int cpxp, XP** apxp, int* acpso, SO*** aapso)
{
    XP* pxpForce = pdzForce->pxp;
    for (int ixpd = 0; ixpd < 2; ixpd++) {
        XPD* pxpdForce = &pxpForce->axpd[ixpd];
        SO* psoRoot = pxpdForce->psoRoot;

        pdzForce->aafxOwner[ixpd].reset();
        pdzForce->aafx[ixpd] = nullptr;

        if (psoRoot == nullptr || psoRoot->ipsoRoot < 0) {
            continue;
        }

        int ipsoRoot = psoRoot->ipsoRoot;
        int cpso = acpso[ipsoRoot];

        pdzForce->aafxOwner[ixpd] = std::make_unique<FX[]>(cpso);
        pdzForce->aafx[ixpd] = pdzForce->aafxOwner[ixpd].get();

        psoRoot->pvtso->pfnPropagateSoForce(psoRoot, grfsg, pxpForce, ixpd, pdzForce, pdzForce->aafx[ixpd]);
        psoRoot->pvtso->pfnDistributeSoEffects(psoRoot, grfsg, cpso, aapso[ipsoRoot], pdzForce->aafx[ixpd]);

    }

    pdzForce->mpipxpdv.assign(cpxp, glm::vec3(0.0f));

    for (int ipxp = 0; ipxp < cpxp; ipxp++) {
        XP* pxp = apxp[ipxp];
        glm::vec3 adv[2] = { glm::vec3(0.0f), glm::vec3(0.0f) };

        for (int ixpd = 0; ixpd < 2; ixpd++) {
            XPD* pxpd = &pxp->axpd[ixpd];
            SO* psoRoot = pxpd->psoRoot;

            if (psoRoot == nullptr || psoRoot->ipsoRoot < 0) {
                continue;
            }

            if (psoRoot == pxpForce->axpd[0].psoRoot) {
                SO* psoLeaf = pxpd->psoLeaf;
                CalculateSoEffect(psoLeaf, &pxpd->pos, pdzForce->aafx[0] + psoLeaf->ipso, &adv[ixpd]);
            }
            else if (psoRoot == pxpForce->axpd[1].psoRoot) {
                SO* psoLeaf = pxpd->psoLeaf;
                CalculateSoEffect(psoLeaf, &pxpd->pos, pdzForce->aafx[1] + psoLeaf->ipso, &adv[ixpd]);
            }
        }

        pdzForce->mpipxpdv[ipxp] = SubtractEe(adv[0], adv[1]);
    }

    pdzForce->mpidzg.assign(pdzg->cdzMax, 0.0f);

    for (int idz = 0; idz < pdzg->cdz; idz++) {
        DZ* pdz = &pdzg->adz[idz];
        int ipxp = pdz->pxp->ipxp;

        pdzForce->mpidzg[idz] = DotEe(pdz->f, pdzForce->mpipxpdv[ipxp]);
    }
}

void EnforceDzgDz(DZG* pdzg, DZ* pdzAdd)
{
    constexpr float kEpsilon = 0.0001f;
    constexpr int kMaxIterations = 10;

    if (pdzAdd != nullptr) {
        RemoveDlEntry(&pdzg->dlUncat, pdzAdd);
        AppendDlEntry(&pdzg->dlPos, pdzAdd);
        pdzg->cdzPos++;
    }

    for (int iteration = 0; iteration < kMaxIterations; iteration++) {
        pdzg->aagPos.assign(pdzg->cdzPos * pdzg->cdzPos, 0.0f);
        pdzg->aagPosCrout.assign(pdzg->cdzPos * pdzg->cdzPos, 0.0f);
        pdzg->asdv.assign(pdzg->cdzPos, 0.0f);
        pdzg->adsfPos.assign(pdzg->cdzPos, 0.0f);

        int row = 0;
        for (DZ* pdzRow = (DZ*)pdzg->dlPos.pvFirst; pdzRow != nullptr; pdzRow = (DZ*)pdzRow->dle.pvNext, row++) {
            int col = 0;
            for (DZ* pdzCol = (DZ*)pdzg->dlPos.pvFirst; pdzCol != nullptr; pdzCol = (DZ*)pdzCol->dle.pvNext, col++) {
                const int idzCol = int(pdzCol - pdzg->adz.data());
                // Original EnforceDzgDz advances through the outer DZ's
                // mpidzg array using the inner DZ index.
                pdzg->aagPos[row * pdzg->cdzPos + col] = pdzRow->mpidzg[idzCol];
            }
        }

        CroutDecomp(pdzg->cdzPos, pdzg->aagPos.data(), pdzg->aagPosCrout.data());

        int ipos = 0;
        for (DZ* pdz = (DZ*)pdzg->dlPos.pvFirst; pdz != nullptr; pdz = (DZ*)pdz->dle.pvNext, ipos++) {
            pdzg->asdv[ipos] = -pdz->sdv;
        }

        CroutSolve(pdzg->cdzPos, pdzg->aagPosCrout.data(), pdzg->asdv.data(), pdzg->adsfPos.data());

        for (int idz = 0; idz < pdzg->cdz; idz++) {
            DZ* pdz = &pdzg->adz[idz];

            pdz->dsdv = 0.0f;

            int ipos = 0;
            for (DZ* pdzPos = (DZ*)pdzg->dlPos.pvFirst; pdzPos != nullptr; pdzPos = (DZ*)pdzPos->dle.pvNext, ipos++) {
                pdz->dsdv += pdzg->adsfPos[ipos] * pdzPos->mpidzg[idz];
            }
        }

        DZ* pdzLimit = nullptr;
        int limitKind = 0;
        float sStep = 1.0f;

        ipos = 0;
        for (DZ* pdz = (DZ*)pdzg->dlPos.pvFirst; pdz != nullptr; pdz = (DZ*)pdz->dle.pvNext, ipos++) {
            float dsf = pdzg->adsfPos[ipos];

            if (pdz->dzk == DZK_Equality) {
                continue;
            }

            if (dsf > kEpsilon && pdz->fMax != 0) {
                float sCandidate = (pdz->sfMax - pdz->sf) / dsf;

                if (sCandidate < sStep) {
                    sStep = sCandidate;
                    pdzLimit = pdz;
                    limitKind = 4;
                }
            }
            else if (dsf < -kEpsilon) {
                float sCandidate = -pdz->sf / dsf;

                if (sCandidate < sStep) {
                    sStep = sCandidate;
                    pdzLimit = pdz;
                    limitKind = 2;
                }
            }
        }

        for (DZ* pdz = (DZ*)pdzg->dlZero.pvFirst; pdz != nullptr; pdz = (DZ*)pdz->dle.pvNext) {
            if (pdz->dsdv <= -kEpsilon) {
                float sCandidate = -pdz->sdv / pdz->dsdv;

                if (sCandidate < sStep) {
                    sStep = sCandidate;
                    pdzLimit = pdz;
                    limitKind = 1;
                }
            }
        }

        for (DZ* pdz = (DZ*)pdzg->dlMax.pvFirst; pdz != nullptr; pdz = (DZ*)pdz->dle.pvNext) {
            if (pdz->dsdv >= kEpsilon) {
                float sCandidate = -pdz->sdv / pdz->dsdv;

                if (sCandidate < sStep) {
                    sStep = sCandidate;
                    pdzLimit = pdz;
                    limitKind = 3;
                }
            }
        }

        for (int idz = 0; idz < pdzg->cdz; idz++) {
            DZ* pdz = &pdzg->adz[idz];
            pdz->sdv += sStep * pdz->dsdv;
        }

        ipos = 0;
        for (DZ* pdz = (DZ*)pdzg->dlPos.pvFirst; pdz != nullptr; pdz = (DZ*)pdz->dle.pvNext, ipos++) {
            pdz->sf += sStep * pdzg->adsfPos[ipos];
        }

        switch (limitKind) {
            case 0:
            return;

            case 1:
            pdzg->cdzPos++;
            RemoveDlEntry(&pdzg->dlZero, pdzLimit);
            AppendDlEntry(&pdzg->dlPos, pdzLimit);
            break;

            case 2:
            pdzg->cdzPos--;
            RemoveDlEntry(&pdzg->dlPos, pdzLimit);
            AppendDlEntry(&pdzg->dlZero, pdzLimit);

            if (pdzLimit == pdzAdd) {
                return;
            }
            break;

            case 3:
            pdzg->cdzPos++;
            RemoveDlEntry(&pdzg->dlMax, pdzLimit);
            AppendDlEntry(&pdzg->dlPos, pdzLimit);
            break;

            case 4:
            pdzg->cdzPos--;
            RemoveDlEntry(&pdzg->dlPos, pdzLimit);
            AppendDlEntry(&pdzg->dlMax, pdzLimit);

            if (pdzLimit == pdzAdd) {
                return;
            }
            break;
        }
    }
}

void CroutDecomp(int c, float* aag, float* aagCrout)
{
    constexpr float kSingularEpsilon = 1.0e-8f;

    if (c == 0) {
        return;
    }

    for (int col = 0; col < c; col++) {
        aagCrout[col] = aag[col];
    }

    if (std::abs(aagCrout[0]) < kSingularEpsilon) {
        for (int row = 0; row < c; row++) {
            aagCrout[row * c] = 0.0f;
        }
    }
    else {
        for (int row = 1; row < c; row++) {
            aagCrout[row * c] = aag[row * c] / aagCrout[0];
        }
    }

    for (int pivot = 1; pivot < c - 1; pivot++) {
        for (int col = pivot; col < c; col++) {
            float sum = 0.0f;

            for (int k = 0; k < pivot; k++) {
                sum += aagCrout[k * c + col] * aagCrout[pivot * c + k];
            }

            aagCrout[pivot * c + col] = aag[pivot * c + col] - sum;
        }

        if (std::abs(aagCrout[pivot * c + pivot]) < kSingularEpsilon) {
            for (int row = pivot; row < c; row++) {
                aagCrout[row * c + pivot] = 0.0f;
            }
        }
        else {
            for (int row = pivot + 1; row < c; row++) {
                float sum = 0.0f;

                for (int k = 0; k < pivot; k++) {
                    sum += aagCrout[k * c + pivot] * aagCrout[row * c + k];
                }

                aagCrout[row * c + pivot] = (aag[row * c + pivot] - sum) / aagCrout[pivot * c + pivot];
            }
        }
    }

    int last = c - 1;
    float sum = 0.0f;

    for (int k = 0; k < last; k++) {
        sum += aagCrout[k * c + last] * aagCrout[last * c + k];
    }

    float diagonal = aag[last * c + last] - sum;
    aagCrout[last * c + last] = diagonal;

    if (std::abs(diagonal) < kSingularEpsilon) {
        aagCrout[last * c + last] = 0.0f;
    }
}

void CroutSolve(int c, float* aagCrout, float* agConst, float* agSoln)
{
    for (int row = 0; row < c; row++) {
        float diagonal = aagCrout[row * c + row];

        if (diagonal == 0.0f) {
            agSoln[row] = 0.0f;
            continue;
        }

        float sum = 0.0f;

        for (int k = 0; k < row; k++) {
            sum += aagCrout[k * c + row] * agSoln[k];
        }

        agSoln[row] = (agConst[row] - sum) / diagonal;
    }

    for (int row = c - 2; row >= 0; row--) {
        if (aagCrout[row * c + row] == 0.0f) {
            continue;
        }

        float sum = 0.0f;

        for (int k = row + 1; k < c; k++) {
            sum += aagCrout[k * c + row] * agSoln[k];
        }

        agSoln[row] -= sum;
    }
}

void ApplyDzg(DZG* pdzg, int cpsoRoot, int* acpso, SO*** aapso, float sdvMax, float sdwMax)
{
    std::vector<std::vector<FX>> aafxSum(cpsoRoot);
    for (int ipsoRoot = 0; ipsoRoot < cpsoRoot; ipsoRoot++) {
        aafxSum[ipsoRoot].assign(acpso[ipsoRoot], FX{});
    }

    for (int idz = 0; idz < pdzg->cdz; idz++) {
        DZ* pdz = &pdzg->adz[idz];
        if (pdz->sf == 0.0f) {
            continue;
        }

        for (int ixpd = 0; ixpd < 2; ixpd++) {
            SO* psoRoot = pdz->pxp->axpd[ixpd].psoRoot;

            if (psoRoot == nullptr || psoRoot->ipsoRoot < 0) {
                continue;
            }

            int ipsoRoot = psoRoot->ipsoRoot;
            FX* afxSrc = pdz->aafx[ixpd];
            FX* afxDst = aafxSum[ipsoRoot].data();

            for (int ipso = 0; ipso < acpso[ipsoRoot]; ipso++) {
                // The EE/VU saturates overflowing float operations. Preserve
                // that behavior before ApplyDzg clamps the resulting motion.
                AddScaledEe(&afxDst[ipso].dv, afxSrc[ipso].dv, pdz->sf);
                AddScaledEe(&afxDst[ipso].dw, afxSrc[ipso].dw, pdz->sf);

            }
        }
    }

    for (int ipsoRoot = 0; ipsoRoot < cpsoRoot; ipsoRoot++) {
        FX* afxRoot = aafxSum[ipsoRoot].data();
        SO** apso = aapso[ipsoRoot];

        for (int ipso = 0; ipso < acpso[ipsoRoot]; ipso++) {
            SO* pso = apso[ipso];
            SO* psoParent = (SO*)pso->paloParent;
            FX* pfx = &afxRoot[pso->ipso];

            if (psoParent == nullptr) {
                glm::vec3 dv = pfx->dv;
                glm::vec3 dw = pfx->dw;
                ClampVectorLength(&dv, sdvMax);

                pso->dpos += dv;

                ClampVectorLength(&dw, sdwMax);

                pso->drot += dw;

            }
            else {
                FX* pfxParent = &afxRoot[psoParent->ipso];

                glm::vec3 dvParent;
                CalculateSoEffect(psoParent, &pso->xf.posWorld, pfxParent, &dvParent);

                glm::vec3 dvWorld = pfx->dv - dvParent;
                glm::vec3 dwWorld = pfx->dw - pfxParent->dw;

                glm::vec3 dvLocal = glm::vec3(glm::dot(psoParent->xf.matWorld[0], dvWorld), glm::dot(psoParent->xf.matWorld[1], dvWorld), glm::dot(psoParent->xf.matWorld[2], dvWorld));
                glm::vec3 dwLocal = glm::vec3(glm::dot(psoParent->xf.matWorld[0], dwWorld), glm::dot(psoParent->xf.matWorld[1], dwWorld), glm::dot(psoParent->xf.matWorld[2], dwWorld));

                ApplySoConstraintLocal(pso, &pso->constrForce, &dvLocal, &dvLocal, (glm::vec3*)nullptr);
                ApplySoConstraintLocal(pso, &pso->constrTorque, &dwLocal, &dwLocal, (glm::vec3*)nullptr);

                ClampVectorLength(&dvLocal, sdvMax);

                pso->dpos += dvLocal;

                ClampVectorLength(&dwLocal, sdwMax);

                pso->drot += dwLocal;
            }
        }
    }

}

void SolveInequalities(int c, const float* aag, const float* ag, float* agSoln)
{
    DZG dzg;

    InitDzg(&dzg, c);

    for (int i = 0; i < c; ++i)
    {
        DZ& dz = dzg.adz[i];

        dz.mpidzg.assign(aag + i * c, aag + (i + 1) * c);
        // Original SolveInequalities writes ag[i] to DZ::sdv (+56).
        // DZ::dsdv is working storage populated by EnforceDzgDz.
        dz.sdv = ag[i];

        AppendDlEntry(&dzg.dlUncat, &dz);
    }

    dzg.cdz = c;

    SolveDzg(&dzg, 0, 0, nullptr, nullptr, nullptr);

    for (int i = 0; i < c; ++i)
        agSoln[i] = dzg.adz[i].sf;
}

void ClearDzgSolution(DZG* pdzg)
{
    pdzg->cdzPos = 0;

    ClearDl(&pdzg->dlPos);
    ClearDl(&pdzg->dlZero);
    ClearDl(&pdzg->dlMax);
    ClearDl(&pdzg->dlUncat);

    for (int idz = 0; idz < pdzg->cdz; idz++) {
        DZ* pdz = &pdzg->adz[idz];

        // The original clears only DZ::sf (+52) and DZ::sdv (+56).
        // The force/effect matrices remain cached across the penetration and
        // impact phases of the same SolveSwGroup call.
        pdz->sf = 0.0f;
        pdz->sdv = 0.0f;

        AppendDlEntry(&pdzg->dlUncat, pdz);
    }
}

void SolveDzgFric(DZG* pdzg, GRFSG grfsg, int cpxp, XP** apxp, int* acpso, SO*** aapso, float dt)
{
    while (true) {
        SolveDzg(pdzg, grfsg, cpxp, apxp, acpso, aapso);

        int ipxpBest = -1;
        XP* pxpBest = nullptr;
        glm::vec3 dvBest(0.0f);
        float sdvBest = 0.01f;

        for (int ipxp = 0; ipxp < cpxp; ipxp++) {
            DZ* pdzNormal = &pdzg->adz[ipxp];
            XP* pxp = apxp[ipxp];

            if (pdzNormal->pdzOther != nullptr) {
                continue;
            }

            if (pdzNormal->sf < 0.0001f) {
                continue;
            }

            if (pdzNormal->dzk == DZK_Friction) {
                continue;
            }

            if (pxp->mu < 0.0001f) {
                continue;
            }

            glm::vec3 dv = pxp->vRelAdj;

            for (int idz = 0; idz < pdzg->cdz; idz++) {
                DZ* pdz = &pdzg->adz[idz];

                if (pdz->sf != 0.0f) {
                    dv += pdz->mpipxpdv[ipxp] * pdz->sf;
                }
            }

            float svNormal = glm::dot(dv, pxp->normal);
            glm::vec3 dvTangent = dv - pxp->normal * svNormal;
            float sdvTangent = glm::dot(dvTangent, dvTangent);

            if (sdvTangent > sdvBest) {
                ipxpBest = ipxp;
                pxpBest = pxp;
                dvBest = dvTangent;
                sdvBest = sdvTangent;
            }
        }

        if (pxpBest == nullptr) {
            break;
        }

        DZ* pdzNormal = &pdzg->adz[ipxpBest];
        DZ* pdzFric = &pdzg->adz[pdzg->cdz];

        pdzFric->pdzOther = pdzNormal;
        pdzNormal->pdzOther = pdzFric;

        pdzFric->pxp = pxpBest;
        pdzFric->sdv = -std::sqrt(sdvBest);
        pdzFric->f = dvBest * (1.0f / -std::sqrt(sdvBest));
        pdzFric->dzk = DZK_Friction;
        pdzFric->fMax = 1;
        pdzFric->sfMax = pdzNormal->sf * pxpBest->mu;

        for (int ixpd = 0; ixpd < 2; ixpd++) {
            SO* psoHook = PsoFindSoPhysHook(pxpBest->axpd[ixpd].psoLeaf, offsetof(VTSO, pfnAdjustSoDz));

            if (psoHook != nullptr && psoHook->pvtso->pfnAdjustSoDz != nullptr) {
                psoHook->pvtso->pfnAdjustSoDz(psoHook, grfsg | 16, pdzFric, ixpd, dt);
            }
        }

        AppendDzgDz(pdzg, pdzFric);
    }
}

void SolveDzg(DZG* pdzg, GRFSG grfsg, int cpxp, XP** apxp, int* acpso, SO*** aapso)
{
    for (DZ* pdz = (DZ*)pdzg->dlUncat.pvFirst; pdz != nullptr;) {
        DZ* pdzNext = (DZ*)pdz->dle.pvNext;

        if (pdz->dzk == DZK_Equality) {
            if (pdz->mpidzg.empty()) {
                FillDzgDz(pdzg, grfsg, pdz, cpxp, apxp, acpso, aapso);
            }

            RemoveDlEntry(&pdzg->dlUncat, pdz);
            AppendDlEntry(&pdzg->dlPos, pdz);
            pdzg->cdzPos++;
        }

        pdz = pdzNext;
    }

    EnforceDzgDz(pdzg, nullptr);

    while (true) {
        DZ* pdzBest = nullptr;
        float bestSdv = -0.01f;

        for (DZ* pdz = (DZ*)pdzg->dlUncat.pvFirst; pdz != nullptr; pdz = (DZ*)pdz->dle.pvNext) {
            if (pdz->sdv < bestSdv) {
                bestSdv = pdz->sdv;
                pdzBest = pdz;
            }
        }

        if (pdzBest == nullptr) {
            break;
        }

        if (pdzBest->mpidzg.empty()) {
            FillDzgDz(pdzg, grfsg, pdzBest, cpxp, apxp, acpso, aapso);
        }

        EnforceDzgDz(pdzg, pdzBest);
    }
}

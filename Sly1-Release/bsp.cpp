#include "bsp.h"
#include "mark.h"

void ReadBspc(GEOM *pgeom, BSPC *pbspc, CBinaryInputStream *pbis)
{
    pbspc->cbsp = pbis->U16Read();
    pbspc->cbspFull = pbis->U16Read();

    pbspc->absp.assign(pbspc->cbsp, BSP{});

    for (int i = 0; i < pbspc->cbsp; ++i)
    {
        BSP* pbsp = &pbspc->absp[i];

        uint16_t isurf = pbis->U16Read();

        if (isurf >= pgeom->asurf.size())
        {
            __debugbreak();
            return;
        }

        pbsp->psurf = &pgeom->asurf[isurf];

        uint16_t ibspNeg = pbis->U16Read();

        if (ibspNeg != 0xFFFF)
        {
            if (ibspNeg >= pbspc->absp.size())
            {
                __debugbreak();
                return;
            }

            pbsp->pbspNeg = &pbspc->absp[ibspNeg];
        }

        uint16_t ibspPos = pbis->U16Read();

        if (ibspPos != 0xFFFF)
        {
            if (ibspPos >= pbspc->absp.size())
            {
                __debugbreak();
                return;
            }

            pbsp->pbspPos = &pbspc->absp[ibspPos];
        }
    }
}

void CloneBspc(GEOM* pgeomSrc, BSPC* pbspcSrc, GEOM* pgeomDst, BSPC* pbspcDst)
{
    pbspcDst->cbsp = pbspcSrc->cbsp;
    pbspcDst->cbspFull = pbspcSrc->cbspFull;

    pbspcDst->absp.resize(pbspcSrc->cbsp);

    for (int i = 0; i < pbspcDst->cbsp; i++)
    {
        BSP *pbspSrc = &pbspcSrc->absp[i];
        BSP *pbspDst = &pbspcDst->absp[i];

        pbspDst->psurf = &pgeomDst->asurf[pbspSrc->psurf - pgeomSrc->asurf.data()];

        pbspDst->pbspPos =
            pbspSrc->pbspPos
            ? &pbspcDst->absp[pbspSrc->pbspPos - pbspcSrc->absp.data()]
            : nullptr;

        pbspDst->pbspNeg =
            pbspSrc->pbspNeg
            ? &pbspcDst->absp[pbspSrc->pbspNeg - pbspcSrc->absp.data()]
            : nullptr;
    }
}

PRUNE PruneBsp(BSP* pbsp, glm::vec3* ppos, float sRadius, BSP** ppbspNext)
{
    if (pbsp == nullptr)
        return PRUNE_Neg;

    while (pbsp != nullptr)
    {
        SURF* psurf = pbsp->psurf;

        float g = ppos->x * psurf->normal.x + ppos->y * psurf->normal.y + ppos->z * psurf->normal.z - psurf->gDot;

        if (sRadius < g)
        {
            if (pbsp->pbspPos == nullptr)
                return PRUNE_Pos;

            pbsp = pbsp->pbspPos;
            continue;
        }

        if (g < -sRadius)
        {
            if (pbsp->pbspNeg == nullptr)
                return PRUNE_Neg;

            pbsp = pbsp->pbspNeg;
            continue;
        }

        BSP* pbspNegLast = nullptr;
        BSP* pbspPosLast = nullptr;

        PRUNE pruneNeg;
        if (pbsp->pbspNeg == nullptr)
            pruneNeg = PRUNE_Neg;
        else
        {
            pruneNeg = PruneBsp(pbsp->pbspNeg, ppos, sRadius, ppbspNext);
            pbspNegLast = *ppbspNext - 1;
        }

        PRUNE prunePos;
        if (pbsp->pbspPos == nullptr)
            prunePos = PRUNE_Pos;
        else
        {
            prunePos = PruneBsp(pbsp->pbspPos, ppos, sRadius, ppbspNext);
            pbspPosLast = *ppbspNext - 1;
        }

        if (pruneNeg == prunePos && pruneNeg != PRUNE_Mixed)
            return pruneNeg;

        BSP* pbspOut = *ppbspNext;

        pbspOut->psurf = psurf;

        if (pruneNeg == PRUNE_Pos)
            pbspOut->pbspNeg = &s_bspAlwaysPos;
        else if (pruneNeg == PRUNE_Neg)
            pbspOut->pbspNeg = nullptr;
        else
            pbspOut->pbspNeg = pbspNegLast;

        if (prunePos == PRUNE_Pos)
            pbspOut->pbspPos = nullptr;
        else if (prunePos == PRUNE_Neg)
            pbspOut->pbspPos = &s_bspAlwaysNeg;
        else
            pbspOut->pbspPos = pbspPosLast;

        *ppbspNext = pbspOut + 1;
        return PRUNE_Mixed;
    }

    return PRUNE_Neg;
}

int ClsgClipEdgeToBsp(BSP* pbspRoot, glm::vec3* ppos1, glm::vec3* ppos2, int* mpibspn, int clsgMax, LSG* alsg)
{
    CEB ceb;
    CEB aceb[32] = {};

    ceb.g = 0.0f;
    ceb.pbsp = pbspRoot;
    ceb.pbspNeg = nullptr;
    ceb.psurf = nullptr;

    CEB* pceb = aceb;
    pceb->pbsp = nullptr;
    pceb->g = 1.0f;
    pceb->pbspNeg = nullptr;
    pceb->psurf = nullptr;

    LSG* plsgLast = nullptr;
    LSG* plsgOut = alsg;

    while (true)
    {
        while (true)
        {
            BSP* pbsp = ceb.pbsp;

            while (pbsp == nullptr)
            {
                LSG* plsgNext = plsgOut;

                if (ceb.pbspNeg != nullptr)
                {
                    bool mergeWithLast = false;

                    if (plsgLast != nullptr && ceb.psurf == plsgLast->data.bsp.apsurf[1])
                    {
                        mergeWithLast = true;

                        if (mpibspn != nullptr)
                        {
                            // A pruned BSP is assembled in a temporary array, so its
                            // nodes are not necessarily part of pbspRoot's array.  The
                            // original game only performs this subtraction when the
                            // caller supplied a BSP-number map.
                            const int ibspNeg = static_cast<int>(ceb.pbspNeg - pbspRoot);
                            if (plsgLast->id.n != mpibspn[ibspNeg])
                                mergeWithLast = false;
                        }
                    }

                    if (!mergeWithLast)
                    {
                        int clsg = static_cast<int>(plsgOut - alsg);

                        if (clsg >= clsgMax)
                            return clsgMax;

                        plsgNext = plsgOut + 1;
                        *plsgOut = LSG{};

                        plsgOut->apos[0] = (*ppos1 * (1.0f - ceb.g)) + (*ppos2 * ceb.g);
                        plsgOut->apos[1] = (*ppos1 * (1.0f - pceb->g)) + (*ppos2 * pceb->g);
                        plsgOut->data.bsp.apsurf[0] = ceb.psurf;
                        plsgOut->data.bsp.apsurf[1] = pceb->psurf;

                        if (ceb.psurf != nullptr)
                            plsgOut->anormal[0] = ceb.psurf->normal;

                        if (pceb->psurf != nullptr)
                            plsgOut->anormal[1] = pceb->psurf->normal;

                        plsgOut->au[0] = ceb.g;
                        plsgOut->au[1] = pceb->g;

                        if (mpibspn != nullptr)
                        {
                            const int ibspNeg = static_cast<int>(ceb.pbspNeg - pbspRoot);
                            plsgOut->id.n = mpibspn[ibspNeg];
                        }

                        plsgLast = plsgOut;
                    }
                    else
                    {
                        plsgLast->apos[1] = (*ppos1 * (1.0f - pceb->g)) + (*ppos2 * pceb->g);
                        plsgLast->data.bsp.apsurf[1] = pceb->psurf;
                        plsgLast->anormal[1] = (pceb->psurf != nullptr) ? pceb->psurf->normal : glm::vec3(0.0f);
                        plsgLast->au[1] = pceb->g;
                    }
                }

                if (pceb <= aceb)
                    return static_cast<int>(plsgNext - alsg);

                ceb = *pceb;
                pceb--;
                plsgOut = plsgNext;
                pbsp = ceb.pbsp;
            }

            SURF* psurf = pbsp->psurf;
            float g1 = glm::dot(*ppos1, psurf->normal) - psurf->gDot;
            float g2 = glm::dot(*ppos2, psurf->normal) - psurf->gDot;

            if (g1 > 0.0f || g2 > 0.0f)
            {
                if (g1 <= 0.0f || g2 <= 0.0f)
                {
                    float t = g1 / (g1 - g2);
                    float gMid = g2 * ceb.g + (1.0f - ceb.g) * g1;

                    if (ceb.g < t && t < pceb->g)
                    {
                        CEB* pcebNew = pceb + 1;
                        pcebNew->g = t;
                        pcebNew->psurf = psurf;

                        if (gMid > 0.0f)
                        {
                            pcebNew->pbspNeg = pbsp;
                            pcebNew->pbsp = pbsp->pbspNeg;
                        }
                        else
                        {
                            pcebNew->pbspNeg = nullptr;
                            pcebNew->pbsp = pbsp->pbspPos;
                        }

                        pceb = pcebNew;
                    }

                    if (gMid < 0.0f)
                    {
                        ceb.pbspNeg = pbsp;
                        ceb.pbsp = pbsp->pbspNeg;
                        break;
                    }

                    if (gMid == 0.0f)
                    {
                        float gAtTop = g2 * pceb->g + (1.0f - pceb->g) * g1;

                        if (gAtTop <= 0.0f)
                        {
                            ceb.pbspNeg = pbsp;
                            ceb.pbsp = pbsp->pbspNeg;
                            break;
                        }
                    }
                }

                // The original's `ceb._8_8_ = ZEXT48(ceb.psurf) << 32`
                // clears pbspNeg while retaining psurf.  pbspNeg describes an
                // active negative interval; carrying it into a positive branch
                // makes a later outside leaf look like an inside interval.
                ceb.pbspNeg = nullptr;
                ceb.pbsp = pbsp->pbspPos;
            }
            else
            {
                ceb.pbspNeg = pbsp;
                ceb.pbsp = pbsp->pbspNeg;
            }
        }
    }
}

int ClsgClipEdgeToSphere(glm::vec3* pposSphere, float sRadius, glm::vec3* ppos0, glm::vec3* ppos1, int clsgMax, LSG* alsg)
{
    if (clsgMax <= 0) {
        return 0;
    }

    glm::vec3 dposSphere = *ppos0 - *pposSphere;
    glm::vec3 dposEdge = *ppos1 - *ppos0;

    float a = glm::dot(dposEdge, dposEdge);
    if (a < 0.0001f) {
        return 0;
    }

    float b = 2.0f * glm::dot(dposEdge, dposSphere);
    float c = glm::dot(dposSphere, dposSphere) - sRadius * sRadius;
    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0001f) {
        return 0;
    }

    float sqrtDiscriminant = sqrtf(discriminant);
    float u0 = (-b - sqrtDiscriminant) / (2.0f * a);
    float u1 = (-b + sqrtDiscriminant) / (2.0f * a);

    if (u0 < 0.0f && u1 < 0.0f) {
        return 0;
    }

    if (u0 > 1.0f && u1 > 1.0f) {
        return 0;
    }

    alsg->au[0] = glm::max(u0, 0.0f);
    alsg->au[1] = glm::min(u1, 1.0f);

    for (int ilsg = 0; ilsg < 2; ilsg++) {
        float u = alsg->au[ilsg];

        alsg->apos[ilsg] = glm::mix(*ppos0, *ppos1, u);

        glm::vec3 normal = alsg->apos[ilsg] - *pposSphere;
        float normalLength = glm::length(normal);

        if (normalLength < 0.0001f) {
            alsg->anormal[ilsg] = g_normalX;
        }
        else {
            alsg->anormal[ilsg] = normal / normalLength;
        }
    }

    alsg->lsgk = LSGK_EdgeToSphere;
    return 1;
}

int ClsgClipCylinderToObject(SO* pso, glm::vec3* ppos0, glm::vec3* ppos1, float sRadius, int clsgMax, LSG* alsg)
{
    if (pso->bspc.absp.size() != 0)
        return ClsgClipCylinderToBsp(&pso->bspc, &pso->geomWorld, ppos0, ppos1, sRadius, clsgMax, alsg);

    if (!pso->fSphere)
        return 0;

    return ClsgClipCylinderToSphere(&pso->xf.posWorld, pso->sRadiusSelf, ppos0, ppos1, sRadius, clsgMax, alsg);
}

int ClsgClipCylinderToSphere(glm::vec3* pposSphere, float sRadiusSphere, glm::vec3* ppos0, glm::vec3* ppos1, float sRadiusCylinder, int clsgMax, LSG* alsg)
{
    float u = 0.0f;
    float s = 0.0f;

    FindClosestPointOnLineSegment(pposSphere, ppos0, ppos1, &u, &s);

    glm::vec3 pos = (1.0f - u) * *ppos0 + u * *ppos1;
    glm::vec3 dpos = *pposSphere - pos;

    LimitVectorLength(&dpos, sRadiusCylinder, &dpos);

    glm::vec3 pos0 = *ppos0 + dpos;
    glm::vec3 pos1 = *ppos1 + dpos;

    return ClsgClipEdgeToSphere(pposSphere, sRadiusSphere, &pos0, &pos1, clsgMax, alsg);
}

int ClsgClipCylinderToBsp(BSPC* pbspc, GEOM* pgeom, glm::vec3* ppos0, glm::vec3* ppos1, float sRadiusCylinder, int clsgMax, LSG* alsg)
{
    if (clsgMax == 0 || pbspc->absp.empty())
        return 0;

    int clsg = 0;

    glm::vec3 posMid = (*ppos0 + *ppos1) * 0.5f;
    float sSegment = glm::length(*ppos0 - *ppos1);

    std::vector<BSP> abspPruned(pbspc->cbspFull);
    BSP* pbspNext = abspPruned.data();

    PRUNE prune = PruneBsp(pbspc->absp.data(), &posMid, sSegment * 0.5f + sRadiusCylinder, &pbspNext);

    if (prune == PRUNE_Pos)
        return 0;

    if (prune == PRUNE_Neg) {
        alsg[0] = {};
        alsg[0].apos[0] = *ppos0;
        alsg[0].apos[1] = *ppos1;
        alsg[0].au[0] = 0.0f;
        alsg[0].au[1] = 1.0f;
        return 1;
    }

    int cbspPruned = (int)(pbspNext - abspPruned.data());

    clsg = ClsgClipEdgeToBsp(pbspNext - 1, ppos0, ppos1, nullptr, clsgMax, alsg);

    for (int i = 0; i < cbspPruned; i++)
        abspPruned[i].psurf->grfsurf |= 0x100;

    for (int i = 0; i < cbspPruned; i++) {
        SURF* psurf = abspPruned[i].psurf;

        if (psurf->grfsurf & 0x200)
            continue;

        psurf->grfsurf |= 0x200;

        for (EDGE* pedge = psurf->pedge; pedge != nullptr; pedge = pedge->pedgeNext) {
            SURF* psurfOther = &pgeom->asurf[(short)pedge->aisurf[1]];

            if (!(psurfOther->grfsurf & 0x100))
                continue;

            LSG* plsgAdd = &alsg[clsg];
            int clsgAdd = ClsgClipEdgeToCylinder(ppos0, ppos1, sRadiusCylinder, &pgeom->apos[(short)pedge->aipos[0]], &pgeom->apos[(short)pedge->aipos[1]], clsgMax - clsg, plsgAdd);

            for (int j = 0; j < clsgAdd; j++) {
                float u0 = plsgAdd[j].data.cyl.au[0];
                float u1 = plsgAdd[j].data.cyl.au[1];

                plsgAdd[j].au[0] = glm::min(u0, u1);
                plsgAdd[j].au[1] = glm::max(u0, u1);
            }

            if (clsgAdd > 0)
                clsg = ClsgMergeAlsg(clsg + clsgAdd, alsg);
        }
    }

    for (int i = 0; i < cbspPruned; i++)
        abspPruned[i].psurf->grfsurf &= ~0x300;

    return clsg;
}

int ClsgClipEdgeToCylinder(glm::vec3* pposCylinder0, glm::vec3* pposCylinder1, float sRadiusCylinder, glm::vec3* pposEdge0, glm::vec3* pposEdge1, int clsgMax, LSG* alsg)
{
    if (clsgMax == 0)
        return 0;

    glm::vec3 dposEdge = *pposEdge1 - *pposEdge0;
    glm::vec3 dposCylinder = *pposCylinder1 - *pposCylinder0;
    glm::vec3 dpos = *pposCylinder0 - *pposEdge0;

    glm::vec3 normal = glm::cross(dposEdge, dposCylinder);
    float sNormalSq = glm::dot(normal, normal);

    if (sNormalSq < 0.0001f) {
        const float sCylinderSq = glm::dot(dposCylinder, dposCylinder);

        // A zero-length cylinder reduces to a sphere.
        if (sCylinderSq < 0.0001f)
            return ClsgClipEdgeToSphere(pposCylinder0, sRadiusCylinder, pposEdge0, pposEdge1, clsgMax, alsg);

        const float sEdgeSq = glm::dot(dposEdge, dposEdge);
        if (sEdgeSq < 0.0001f)
            return 0;

        const glm::vec3 axis = dposCylinder / sqrtf(sCylinderSq);
        const glm::vec3 fromCylinder = *pposEdge0 - *pposCylinder0;
        const glm::vec3 radial = fromCylinder - axis * glm::dot(fromCylinder, axis);

        if (glm::dot(radial, radial) > sRadiusCylinder * sRadiusCylinder)
            return 0;

        const float edgeAlongAxis = glm::dot(dposEdge, axis);
        if (fabsf(edgeAlongAxis) < 0.0001f)
            return 0;

        float u0 = -glm::dot(fromCylinder, axis) / edgeAlongAxis;
        float u1 = (sqrtf(sCylinderSq) - glm::dot(fromCylinder, axis)) / edgeAlongAxis;
        if (u1 < u0)
            std::swap(u0, u1);

        u0 = glm::max(u0, 0.0f);
        u1 = glm::min(u1, 1.0f);
        if (u1 < u0)
            return 0;

        alsg->au[0] = u0;
        alsg->au[1] = u1;
        alsg->data.cyl.au[0] = glm::dot(glm::mix(*pposEdge0, *pposEdge1, u0) - *pposCylinder0, dposCylinder) / sCylinderSq;
        alsg->data.cyl.au[1] = glm::dot(glm::mix(*pposEdge0, *pposEdge1, u1) - *pposCylinder0, dposCylinder) / sCylinderSq;

        for (int i = 0; i < 2; ++i) {
            alsg->apos[i] = glm::mix(*pposEdge0, *pposEdge1, alsg->au[i]);

            const float uCylinder = alsg->data.cyl.au[i];
            if (uCylinder <= 0.0f) {
                alsg->anormal[i] = -axis;
            }
            else if (uCylinder >= 1.0f) {
                alsg->anormal[i] = axis;
            }
            else if (glm::dot(radial, radial) < 0.0001f) {
                alsg->anormal[i] = g_normalZ;
            }
            else {
                alsg->anormal[i] = glm::normalize(radial);
            }
        }

        alsg->lsgk = LSGK_EdgeToCylinder;
        return 1;
    }

    float edgeU = glm::dot(glm::cross(dpos, dposCylinder), normal) / sNormalSq;
    float cylU = glm::dot(glm::cross(dpos, dposEdge), normal) / sNormalSq;

    if (glm::dot(dpos, normal) < 0.0f)
        normal = -normal;

    normal = glm::normalize(normal);

    float sDist = glm::dot(dpos, normal);

    if (sRadiusCylinder < sDist)
        return 0;

    glm::vec3 normalRadial = glm::cross(normal, dposCylinder);

    if (glm::length(normalRadial) < 0.0001f)
        normalRadial = glm::vec3(0.0f, 1.0f, 0.0f);
    else
        normalRadial = glm::normalize(normalRadial);

    float edgeRadialDot = glm::dot(dposEdge, normalRadial);

    if (fabsf(edgeRadialDot) < 0.0001f)
        return 0;

    float halfWidth = sqrtf(sRadiusCylinder * sRadiusCylinder - sDist * sDist) / edgeRadialDot;
    float cylScale = glm::dot(dposEdge, dposCylinder) / glm::dot(dposCylinder, dposCylinder);

    alsg->au[0] = edgeU - halfWidth;
    alsg->au[1] = edgeU + halfWidth;
    alsg->data.cyl.au[0] = cylU - halfWidth * cylScale;
    alsg->data.cyl.au[1] = cylU + halfWidth * cylScale;

    if (alsg->data.cyl.au[1] < alsg->data.cyl.au[0]) {
        std::swap(alsg->au[0], alsg->au[1]);
        std::swap(alsg->data.cyl.au[0], alsg->data.cyl.au[1]);
    }

    if (alsg->data.cyl.au[1] < 0.0f || alsg->data.cyl.au[0] > 1.0f)
        return 0;

    if (alsg->data.cyl.au[1] - alsg->data.cyl.au[0] > 0.0001f) {
        float t0 = alsg->data.cyl.au[0] / (alsg->data.cyl.au[0] - alsg->data.cyl.au[1]);

        if (t0 > 0.0f && t0 < 1.0f) {
            alsg->au[0] = glm::mix(alsg->au[0], alsg->au[1], t0);
            alsg->data.cyl.au[0] = glm::mix(alsg->data.cyl.au[0], alsg->data.cyl.au[1], t0);
        }

        float t1 = (alsg->data.cyl.au[0] - 1.0f) / (alsg->data.cyl.au[0] - alsg->data.cyl.au[1]);

        if (t1 > 0.0f && t1 < 1.0f) {
            alsg->au[1] = glm::mix(alsg->au[0], alsg->au[1], t1);
            alsg->data.cyl.au[1] = glm::mix(alsg->data.cyl.au[0], alsg->data.cyl.au[1], t1);
        }
    }

    if (alsg->au[1] < alsg->au[0]) {
        std::swap(alsg->au[0], alsg->au[1]);
        std::swap(alsg->data.cyl.au[0], alsg->data.cyl.au[1]);
    }

    if (alsg->au[1] < 0.0f || alsg->au[0] > 1.0f)
        return 0;

    if (alsg->au[1] - alsg->au[0] > 0.0001f) {
        float t0 = alsg->au[0] / (alsg->au[0] - alsg->au[1]);

        if (t0 > 0.0f && t0 < 1.0f) {
            alsg->data.cyl.au[0] = glm::mix(alsg->data.cyl.au[0], alsg->data.cyl.au[1], t0);
            alsg->au[0] = glm::mix(alsg->au[0], alsg->au[1], t0);
        }

        float t1 = (alsg->au[0] - 1.0f) / (alsg->au[0] - alsg->au[1]);

        if (t1 > 0.0f && t1 < 1.0f) {
            alsg->data.cyl.au[1] = glm::mix(alsg->data.cyl.au[0], alsg->data.cyl.au[1], t1);
            alsg->au[1] = glm::mix(alsg->au[0], alsg->au[1], t1);
        }
    }

    glm::vec3 normalCylinder = glm::normalize(dposCylinder);

    for (int i = 0; i < 2; i++) {
        alsg->apos[i] = glm::mix(*pposEdge0, *pposEdge1, alsg->au[i]);

        if (alsg->data.cyl.au[i] == 0.0f) {
            alsg->anormal[i] = -normalCylinder;
        }
        else if (alsg->data.cyl.au[i] == 1.0f) {
            alsg->anormal[i] = normalCylinder;
        }
        else {
            glm::vec3 posCylinder = glm::mix(*pposCylinder0, *pposCylinder1, alsg->data.cyl.au[i]);
            glm::vec3 dposNormal = alsg->apos[i] - posCylinder;

            if (glm::length(dposNormal) < 0.0001f)
                alsg->anormal[i] = glm::vec3(0.0f, 0.0f, 1.0f);
            else
                alsg->anormal[i] = glm::normalize(dposNormal);
        }
    }

    alsg->lsgk = LSGK_EdgeToCylinder;
    return 1;
}

int ClsgMergeAlsg(int clsg, LSG* alsg)
{
    std::vector<LSG_ENDPOINT> endpoints;
    endpoints.reserve(clsg * 2);

    for (int i = 0; i < clsg; i++) {
        endpoints.push_back({ alsg[i].au[0], 0, alsg[i].apos[0], alsg[i].anormal[0] });
        endpoints.push_back({ alsg[i].au[1], 1, alsg[i].apos[1], alsg[i].anormal[1] });
    }

    std::sort(endpoints.begin(), endpoints.end(), [](const LSG_ENDPOINT& a, const LSG_ENDPOINT& b) {
        if (a.u != b.u)
            return a.u < b.u;

        return a.iEndpoint < b.iEndpoint;
        });

    int cOpen = 0;
    int clsgMerged = 0;

    for (const LSG_ENDPOINT& endpoint : endpoints) {
        if (endpoint.iEndpoint == 0) {
            cOpen++;

            if (cOpen == 1) {
                alsg[clsgMerged] = {};
                alsg[clsgMerged].au[0] = endpoint.u;
                alsg[clsgMerged].apos[0] = endpoint.pos;
                alsg[clsgMerged].anormal[0] = endpoint.normal;
                alsg[clsgMerged].lsgk = LSGK_Merge;
            }
        }
        else {
            cOpen--;

            if (cOpen == 0) {
                alsg[clsgMerged].au[1] = endpoint.u;
                alsg[clsgMerged].apos[1] = endpoint.pos;
                alsg[clsgMerged].anormal[1] = endpoint.normal;
                clsgMerged++;
            }
        }
    }

    return clsgMerged;
}

int ClsgClipEdgeToObjects(glm::vec3* ppos0, glm::vec3* ppos1, std::vector<SO*>* papso, int clsgMax, LSG* alsg)
{
    int clsg = 0;

    if (papso == nullptr || alsg == nullptr || clsgMax <= 0)
        return 0;

    for (SO* pso : *papso)
    {
        if (pso == nullptr)
            continue;

        if (pso->fNoXpsAll || pso->fNoXpsSelf)
            continue;

        int clsgAvailable = clsgMax - clsg;

        if (clsgAvailable <= 0)
            break;

        int clsgNew = ClsgClipEdgeToObject(pso, ppos0, ppos1, clsgAvailable, &alsg[clsg]);

        if (clsgNew <= 0)
            continue;

        if (clsg == 0)
            clsg = clsgNew;
        else
            clsg = ClsgMergeAlsg(clsg + clsgNew, alsg);
    }

    return clsg;
}

int ClsgClipEdgeToObject(SO* pso, glm::vec3* ppos0, glm::vec3* ppos1, int clsgMax, LSG* alsg)
{
    if (pso->bspc.absp.size() != 0)
        return ClsgClipEdgeToBsp(pso->bspc.absp.data(), ppos0, ppos1, nullptr, clsgMax, alsg);

    if (!pso->fSphere)
        return 0;

    return ClsgClipEdgeToSphere(&pso->xf.posWorld, pso->sRadiusSelf, ppos0, ppos1, clsgMax, alsg);
}

int ClsgClipLineToSphere(const glm::vec3* pposCenter, float radius, const glm::vec3* ppos0, const glm::vec3* ppos1, int clsgMax, LSG* plsg)
{
    if (clsgMax <= 0)
        return 0;

    const glm::vec3 dposLine = *ppos1 - *ppos0;
    const glm::vec3 dposCenter = *ppos0 - *pposCenter;

    const float a = glm::dot(dposLine, dposLine);
    if (a < 0.0001f)
        return 0;

    const float b = 2.0f * glm::dot(dposCenter, dposLine);
    const float c = glm::dot(dposCenter, dposCenter) - radius * radius;
    const float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0001f)
        return 0;

    const float sqrtDiscriminant = std::sqrt(discriminant);

    plsg->au[0] = (-b - sqrtDiscriminant) / (2.0f * a);
    plsg->au[1] = (-b + sqrtDiscriminant) / (2.0f * a);

    for (int i = 0; i < 2; ++i)
    {
        const float u = plsg->au[i];

        plsg->apos[i] = glm::mix(*ppos0, *ppos1, u);

        glm::vec3 normal = plsg->apos[i] - *pposCenter;

        if (glm::length(normal) < 0.0001f)
            normal = g_normalX;
        else
            normal = glm::normalize(normal);

        plsg->anormal[i] = normal;
    }

    plsg->lsgk = LSGK_EdgeToSphere;
    return 1;
}

BSP* PbspPointInBspQuick(glm::vec3* ppos, BSP* pbsp)
{
    while (true)
    {
        SURF* psurf = pbsp->psurf;
        float gSide = glm::dot(*ppos, psurf->normal) - psurf->gDot;

        if (gSide < 0.0001f)
        {
            BSP* pbspInside = pbsp;
            pbsp = pbsp->pbspNeg;

            if (pbsp == nullptr)
                return pbspInside;
        }
        else
        {
            pbsp = pbsp->pbspPos;

            if (pbsp == nullptr)
                return nullptr;
        }
    }
}

BSP s_bspAlwaysPos =
{
    &s_surfAlwaysPos, nullptr, nullptr
};

SURF s_surfAlwaysPos =
{
    glm::vec3(0.0, 0.0, 0.0), -1.0E+8, nullptr, nullptr, 0, 0
};

BSP s_bspAlwaysNeg =
{
    &s_surfAlwaysNeg, nullptr, nullptr
};

SURF s_surfAlwaysNeg =
{
    // With a zero normal, side = dot(pos, normal) - gDot.  A positive
    // gDot therefore makes every point classify on the negative side.
    // This must be the inverse of s_surfAlwaysPos; making both -1.0E+8
    // turns PRUNE_Neg sentinel branches into positive/empty space.
    glm::vec3(0.0, 0.0, 0.0), 1.0E+8, nullptr, nullptr, 0, 0
};

#include "mark.h"
#include "hide.h"
#include <cstdio>

XP* AllocSwXp(SW* psw)
{
    auto pxpOwned = std::make_unique<XP>();
    XP* pxp = pxpOwned.get();

    psw->axp.push_back(std::move(pxpOwned));

    *pxp = XP{};
    return pxp;
}

float MuFromAmtlk(MTLK* amtlk)
{
    const MTLKWA& a = s_mpmtlkwaMu[static_cast<int>(amtlk[0])];
    const MTLKWA& b = s_mpmtlkwaMu[static_cast<int>(amtlk[1])];

    float mu = (a.a + b.a) / (a.w + b.w);

    if (mu < 0.0f)
        mu = 0.0f;

    if ((g_grfcht & 8) != 0)
        mu *= 0.2f;

    return mu;
}

float ElasFromAmtlk(MTLK* amtlk)
{
    const MTLKWA& a = s_mpmtlkwaElas[static_cast<int>(amtlk[0])];
    const MTLKWA& b = s_mpmtlkwaElas[static_cast<int>(amtlk[1])];

    float elas = (a.a + b.a) / (a.w + b.w);

    if (elas < 0.0f)
        return 0.0f;

    if (elas > 1.0f)
        return 1.0f;

    return elas;
}

XP* PxpAdd(SO* pso0, SO* pso1, glm::vec3* ppos, glm::vec3* pnormal, float gDist, XPK xpk, void* pv0, void* pv1, XP** ppxpFirst)
{
    MTLK amtlk[2] = {};

    XP* pxp = AllocSwXp(pso0->psw);

    pxp->gDist = gDist;
    pxp->normal = *pnormal;
    pxp->xpk = xpk;

    pxp->axpd[0].psoLeaf = pso0;
    pxp->axpd[1].psoLeaf = pso1;

    for (int ixpd = 0; ixpd < 2; ixpd++)
    {
        SO* pso = pxp->axpd[ixpd].psoLeaf;

        if (pso != nullptr)
            pxp->axpd[ixpd].psoRoot = reinterpret_cast<SO*>(pso->paloRoot);
        else
        {
            pxp->axpd[ixpd].psoRoot = nullptr;
            pxp->axpd[ixpd].v = glm::vec3(0.0f);
        }
    }

    pxp->axpd[0].pos = *ppos;
    pxp->axpd[1].pos = *ppos + (*pnormal * -gDist);

    for (int ixpd = 0; ixpd < 2; ixpd++)
    {
        SO* pso = pxp->axpd[ixpd].psoLeaf;

        if (pso == nullptr)
        {
            pxp->axpd[ixpd].posLeaf = pxp->axpd[ixpd].pos;
            pxp->axpd[ixpd].mtlk = MTLK_Default;
            amtlk[ixpd] = MTLK_Default;
        }
        else
        {
            ConvertAloPos(nullptr, pso, &pxp->axpd[ixpd].pos, &pxp->axpd[ixpd].posLeaf);
            pxp->axpd[ixpd].mtlk = static_cast<MTLK>(pso->mtlk);
            amtlk[ixpd] = static_cast<MTLK>(pso->mtlk);
        }
    }

    pxp->mu = MuFromAmtlk(amtlk);
    pxp->elas = ElasFromAmtlk(amtlk);

    pxp->pv0 = pv0;
    pxp->pv1 = pv1;

    for (int ixpd = 0; ixpd < 2; ixpd++)
    {
        SO* psoHook = PsoFindSoPhysHook(pxp->axpd[ixpd].psoLeaf, offsetof(VTSO, pfnAdjustSoNewXp));

        if (psoHook != nullptr && psoHook->pvtso->pfnAdjustSoNewXp != nullptr)
            psoHook->pvtso->pfnAdjustSoNewXp(psoHook, pxp, ixpd);
    }

    XP* pxpTemp = pxp;
    RecalcSwXpMovement(pso0->psw, 1, &pxpTemp);

    pxp->pxpNext = *ppxpFirst;
    *ppxpFirst = pxp;

    return pxp;
}

void RecalcSwXpPos(SW* psw, int cpxp, XP** apxp)
{
    for (int ipxp = 0; ipxp < cpxp; ipxp++) {
        XP* pxp = apxp[ipxp];

        for (int ixpd = 0; ixpd < 2; ixpd++) {
            XPD* pxpd = &pxp->axpd[ixpd];
            ALO* paloFrom = pxpd->psoLeaf;

            if (paloFrom == nullptr) {
                continue;
            }

            bool fUseNormalOffset = false;

            if (ixpd == 0) {
                fUseNormalOffset = pxp->xpk > 3 && pxp->xpk < 7;
            }
            else {
                fUseNormalOffset = pxp->xpk == 7 || pxp->xpk == 9;
            }

            if (fUseNormalOffset) {
                // The decompiler expressed this SO field as an address through
                // paloFrom[1].  For sphere contact kinds the offset is the
                // collision sphere radius, not matrix data from a next ALO.
                SO* psoFrom = static_cast<SO*>(paloFrom);
                pxpd->pos = paloFrom->xf.posWorld - pxp->normal * psoFrom->sRadiusSelf;
            }
            else
                ConvertAloPos(paloFrom, nullptr, &pxpd->posLeaf, &pxpd->pos);

        }
    }
}

void RecalcSwXpMovement(SW* psw, int cpxp, XP** apxp)
{
    for (int ixp = 0; ixp < cpxp; ixp++)
    {
        XP* pxp = apxp[ixp];

        for (int ixpd = 0; ixpd < 2; ixpd++)
        {
            SO* psoLeaf = pxp->axpd[ixpd].psoLeaf;

            if (psoLeaf == nullptr)
                continue;

            ALO* const paloLeaf = reinterpret_cast<ALO*>(psoLeaf);
            CalculateAloMovement(paloLeaf, nullptr, pxp->axpd[ixpd].pos, &pxp->axpd[ixpd].v, nullptr, nullptr, nullptr);

            if (psoLeaf->pvtso->pfnAdjustSoXpLocal != nullptr)
                psoLeaf->pvtso->pfnAdjustSoXpLocal(psoLeaf, pxp, ixpd);

        }

        for (int ixpd = 0; ixpd < 2; ixpd++)
        {
            SO* psoHook = PsoFindSoPhysHook(pxp->axpd[ixpd].psoLeaf, offsetof(VTSO, pfnAdjustSoXpVelocity));

            if (psoHook != nullptr && psoHook->pvtso->pfnAdjustSoXpVelocity != nullptr)
                psoHook->pvtso->pfnAdjustSoXpVelocity(psoHook, pxp, ixpd);

        }

        pxp->vRelAdj = pxp->axpd[0].v - pxp->axpd[1].v;
    }
}

void FreeSwXpList(SW* psw, XP* pxp)
{
    while (pxp != nullptr)
    {
        XP* pxpNext = pxp->pxpNext;

        auto it = std::find_if(psw->axp.begin(), psw->axp.end(),
            [pxp](const std::unique_ptr<XP>& pxpOwned)
            {
                return pxpOwned.get() == pxp;
            });

        if (it != psw->axp.end())
            psw->axp.erase(it);

        pxp = pxpNext;
    }
}

void FindClosestPointOnLine(const glm::vec3* ppos, const glm::vec3* pposSeg0, const glm::vec3* pposSeg1, float* pu, float* ps)
{
    glm::vec3 dposSeg = *pposSeg1 - *pposSeg0;
    float sSegSquared = glm::dot(dposSeg, dposSeg);

    if (sSegSquared < 0.0001f)
    {
        *pu = 0.0f;
        *ps = glm::length(*ppos - *pposSeg0);
        return;
    }

    glm::vec3 dpos = *ppos - *pposSeg0;
    *pu = glm::dot(dposSeg, dpos) / sSegSquared;

    glm::vec3 posClosest = *pposSeg0 + dposSeg * *pu;
    *ps = glm::length(*ppos - posClosest);
}

void FindClosestPointOnLineSegment(const glm::vec3* ppos, const glm::vec3* pposSeg0, const glm::vec3* pposSeg1, float* pu, float* ps)
{
    const glm::vec3 segment = *pposSeg1 - *pposSeg0;
    const float segmentLengthSq = glm::dot(segment, segment);

    if (segmentLengthSq < 0.0001f)
    {
        *pu = 0.0f;
        *ps = glm::distance(*ppos, *pposSeg0);
        return;
    }

    const float u = glm::clamp(glm::dot(*ppos - *pposSeg0, segment) / segmentLengthSq, 0.0f, 1.0f);
    const glm::vec3 posClosest = glm::mix(*pposSeg0, *pposSeg1, u);

    *pu = u;
    *ps = glm::distance(*ppos, posClosest);
}

int ClsgClipSoAllEdges(SO* pso0, SO* pso1, BSP* pbsp, int clsgMax, LSG* alsg)
{
    int clsg = 0;
    int ilsgNew = 0;

    glm::vec3* apos = pso1->geomWorld.apos.data();
    EDGE* aedge = pso1->geomWorld.aedge.data();

    for (int iedge = pso1->geomWorld.cedge - 1; iedge >= 0; iedge--)
    {
        EDGE* pedge = &aedge[iedge];

        int clsgAdded = ClsgClipEdgeToBsp(pbsp, &apos[pedge->aipos[0]], &apos[pedge->aipos[1]], nullptr, clsgMax - clsg, &alsg[clsg]);
        clsg += clsgAdded;

        while (ilsgNew < clsg)
        {
            alsg[ilsgNew].pedge = pedge;
            ilsgNew++;
        }
    }

    return clsg;
}

int ClsgClipSoBspEdges(SO* pso0, SO* pso1, int cbspPruned, BSP* abspPruned, int clsgMax, LSG* alsg)
{
    int clsg = 0;
    int ilsgNew = 0;

    GEOM& geom = pso1->geomWorld;

    for (int ibsp = cbspPruned - 1; ibsp >= 0; --ibsp)
    {
        SURF* psurf = abspPruned[ibsp].psurf;
        int isurf = IsurfFromBspSurf(geom, psurf);

        if (isurf < 0)
            continue;

        if (isurf >= static_cast<int>(geom.mpisurfposCenter.size()) || isurf >= static_cast<int>(geom.mpisurfsRadius.size()))
            continue;

        glm::vec3 dpos = pso0->xf.posWorld - geom.mpisurfposCenter[isurf];
        float sRadius = pso0->sRadiusSelf + geom.mpisurfsRadius[isurf];

        if (glm::dot(dpos, dpos) < sRadius * sRadius)
            psurf->grfsurf |= 1;
    }

    for (int ibsp = cbspPruned - 1; ibsp >= 0; --ibsp)
    {
        SURF* psurf = abspPruned[ibsp].psurf;
        int isurf = IsurfFromBspSurf(geom, psurf);

        if (isurf < 0)
            continue;

        uint16_t grfsurf = psurf->grfsurf;

        if ((grfsurf & 1) == 0 || (grfsurf & 2) != 0)
            continue;

        psurf->grfsurf = grfsurf | 2;

        for (EDGE* pedge = psurf->pedge; pedge != nullptr; pedge = pedge->pedgeNext)
        {
            int isurfOther = static_cast<int16_t>(pedge->aisurf[1]);

            if (isurfOther < 0 || isurfOther >= static_cast<int>(geom.asurf.size()))
                continue;

            SURF* psurfOther = &geom.asurf[isurfOther];

            if ((psurfOther->grfsurf & 1) == 0)
                continue;

            int ipos0 = static_cast<int16_t>(pedge->aipos[0]);
            int ipos1 = static_cast<int16_t>(pedge->aipos[1]);

            if (ipos0 < 0 || ipos0 >= static_cast<int>(geom.apos.size()))
                continue;

            if (ipos1 < 0 || ipos1 >= static_cast<int>(geom.apos.size()))
                continue;

            if (clsg >= clsgMax)
                break;

            int clsgAdded = ClsgClipEdgeToBsp(pso0->bspc.absp.data(), &geom.apos[ipos0], &geom.apos[ipos1], nullptr, clsgMax - clsg, &alsg[clsg]);
            clsg += clsgAdded;

            while (ilsgNew < clsg)
            {
                alsg[ilsgNew].pedge = pedge;
                ++ilsgNew;
            }
        }
    }

    for (int ibsp = cbspPruned - 1; ibsp >= 0; --ibsp)
    {
        SURF* psurf = abspPruned[ibsp].psurf;

        if (IsurfFromBspSurf(geom, psurf) < 0)
            continue;

        psurf->grfsurf &= ~uint16_t(3);
    }

    return clsg;
}

void CategorizeAlsg(int clsg, LSG* alsg, int* pciposEmbed, int* aiposEmbed, LSG** aplsgEmbed, LSG** pplsgEdge)
{
    int ciposEmbed = 0;
    LSG* plsgFull = nullptr;

    *pplsgEdge = nullptr;

    for (int ilsg = 0; ilsg < clsg; ilsg++)
    {
        LSG* plsg = &alsg[ilsg];

        SURF* psurf0 = plsg->data.bsp.apsurf[0];
        SURF* psurf1 = plsg->data.bsp.apsurf[1];

        if (psurf0 == nullptr)
        {
            if (psurf1 == nullptr)
            {
                plsg->plsgNext = plsgFull;
                plsgFull = plsg;
                continue;
            }
        }
        else if (psurf1 == nullptr)
        {
            std::swap(plsg->apos[0], plsg->apos[1]);
            std::swap(plsg->anormal[0], plsg->anormal[1]);
            std::swap(plsg->data.bsp.apsurf[0], plsg->data.bsp.apsurf[1]);
            std::swap(plsg->au[0], plsg->au[1]);
            plsg->iiposSwap = 1;
        }
        else
        {
            plsg->plsgNext = *pplsgEdge;
            *pplsgEdge = plsg;
            continue;
        }

        EDGE* pedge = plsg->pedge;
        int iipos = plsg->iiposSwap;
        int ipos = pedge->aipos[iipos];

        int iembed = 0;

        while (iembed < ciposEmbed && aiposEmbed[iembed] != ipos)
            iembed++;

        if (iembed == ciposEmbed)
        {
            aiposEmbed[iembed] = ipos;
            aplsgEmbed[iembed] = nullptr;
            ciposEmbed++;
        }

        plsg->plsgNext = aplsgEmbed[iembed];
        aplsgEmbed[iembed] = plsg;
    }

    for (int iembed = 0; iembed < ciposEmbed; )
    {
        LSG* plsgList = aplsgEmbed[iembed];

        if (plsgList != nullptr && plsgList->plsgNext == nullptr)
        {
            bool keepEmbed = false;

            for (LSG* plsg = plsgFull; plsg != nullptr; plsg = plsg->plsgNext)
            {
                EDGE* pedge = plsg->pedge;

                if (pedge->aipos[0] == aiposEmbed[iembed] || pedge->aipos[1] == aiposEmbed[iembed])
                {
                    keepEmbed = true;
                    break;
                }
            }

            if (!keepEmbed)
            {
                plsgList->plsgNext = *pplsgEdge;
                *pplsgEdge = plsgList;

                for (int j = iembed; j < ciposEmbed - 1; j++)
                {
                    aiposEmbed[j] = aiposEmbed[j + 1];
                    aplsgEmbed[j] = aplsgEmbed[j + 1];
                }

                ciposEmbed--;
                continue;
            }
        }

        iembed++;
    }

    *pciposEmbed = ciposEmbed;
}

int CplsgFindEdgePath(SURF* asurf, int isurf, int isurfTarget, LSG* plsgEdge, LSG** aplsg)
{
    FEP afep[64] = {};

    int cfep = 1;
    int ifep = 0;

    afep[0].pfepSource = nullptr;
    afep[0].plsg = nullptr;
    afep[0].isurf = isurf;

    while (ifep < cfep)
    {
        FEP* pfep = &afep[ifep];

        for (LSG* plsg = plsgEdge; plsg != nullptr; plsg = plsg->plsgNext)
        {
            EDGE* pedge = plsg->pedge;
            int isurfNext = -1;

            if (pedge->aisurf[0] == pfep->isurf)
                isurfNext = pedge->aisurf[1];
            else if (pedge->aisurf[1] == pfep->isurf)
                isurfNext = pedge->aisurf[0];
            else
                continue;

            if (isurfNext == isurfTarget)
            {
                int clsg = 0;

                aplsg[clsg++] = plsg;

                for (FEP* pfepPath = pfep; pfepPath->plsg != nullptr; pfepPath = pfepPath->pfepSource)
                    aplsg[clsg++] = pfepPath->plsg;

                for (int i = 0; i < cfep; i++)
                    asurf[afep[i].isurf].grfsurf &= ~uint16_t(4);

                return clsg;
            }

            if ((asurf[isurfNext].grfsurf & 4) != 0)
                continue;

            asurf[isurfNext].grfsurf |= 4;

            if (cfep < 64)
            {
                afep[cfep].pfepSource = pfep;
                afep[cfep].plsg = plsg;
                afep[cfep].isurf = isurfNext;
                cfep++;
            }
        }

        ifep++;
    }

    for (int i = 0; i < cfep; i++)
        asurf[afep[i].isurf].grfsurf &= ~uint16_t(4);

    return 0;
}

void MarkSoContactsBspBsp(SO* pso0, SO* pso1, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst)
{
    int aclsg[2] = {};
    LSG aalsg[2][128] = {};
    int aciposEmbed[2] = {};
    int aaiposEmbed[2][64] = {};
    LSG* aaplsgEmbed[2][64] = {};
    LSG* aplsgEdge[2] = {};
    LSG* aplsgOther[32] = {};

    aclsg[0] = ClsgClipSoAllEdges(pso1, pso0, pbspPruned, 128, aalsg[0]);
    aclsg[1] = ClsgClipSoBspEdges(pso0, pso1, cbspPruned, abspPruned, 128, aalsg[1]);

    if (aclsg[0] == 0 && aclsg[1] == 0)
        return;

    for (int side = 0; side < 2; side++)
        CategorizeAlsg(aclsg[side], aalsg[side], &aciposEmbed[side], aaiposEmbed[side], aaplsgEmbed[side], &aplsgEdge[side]);

    SO* apsoSwap[2] = { pso0, pso1 };

    for (int side = 0; side < 2; side++)
    {
        SO* psoA = apsoSwap[0];
        SO* psoB = apsoSwap[1];

        for (int iembed = 0; iembed < aciposEmbed[side]; iembed++)
        {
            int iposEmbed = aaiposEmbed[side][iembed];
            LSG* plsgFirst = aaplsgEmbed[side][iembed];

            glm::vec3* apos = psoA->geomWorld.apos.data();
            glm::vec3* pposEmbed = &apos[iposEmbed];

            SURF* psurfBest = nullptr;
            float gBest = -FLT_MAX;
            float dotBest = 0.0f;

            for (LSG* plsg = plsgFirst; plsg != nullptr; plsg = plsg->plsgNext)
            {
                SURF* psurf = plsg->data.bsp.apsurf[1];

                if ((psurf->grfsurf & 8) != 0)
                    continue;

                psurf->grfsurf |= 8;

				// The original keeps two projections here: the embedded vertex
				// projection is used for the eventual XP depth, while the minimum
				// projection across the connected crossing edges is used only to
				// select the best separating surface.
				const float gEmbed = glm::dot(*pposEmbed, psurf->normal);
				float gMin = gEmbed;

                for (LSG* plsgOther = plsgFirst; plsgOther != nullptr; plsgOther = plsgOther->plsgNext)
                {
                    if (plsgOther == plsg)
                        continue;

                    int iposOther = plsgOther->pedge->aipos[1 - plsgOther->iiposSwap];
                    float g = glm::dot(apos[iposOther], psurf->normal);

                    if (g < gMin)
                        gMin = g;
                }

                float penetration = gMin - psurf->gDot;

                if (penetration > gBest)
                {
                    gBest = penetration;
					dotBest = gEmbed;
                    psurfBest = psurf;
                }
            }

            for (LSG* plsg = plsgFirst; plsg != nullptr; plsg = plsg->plsgNext)
                plsg->data.bsp.apsurf[1]->grfsurf &= ~8;

            if (psurfBest == nullptr)
                continue;

            const float gDistBest = dotBest - psurfBest->gDot;
            PxpAdd(psoA, psoB, pposEmbed, &psurfBest->normal, gDistBest, XPK_PointToSurface, &apos[iposEmbed], psurfBest, ppxpFirst);

            for (LSG* plsg = plsgFirst; plsg != nullptr; plsg = plsg->plsgNext)
            {
                SURF* psurf = plsg->data.bsp.apsurf[1];

                if (psurf == psurfBest)
                    continue;

                glm::vec3* pposEdge = &plsg->apos[1];
                float gEdge = glm::dot(*pposEdge, psurfBest->normal);

                if (gEdge < dotBest)
                    PxpAdd(psoA, psoB, pposEdge, &psurfBest->normal, gEdge - psurfBest->gDot, XPK_EdgeToSurface, plsg->pedge, psurfBest, ppxpFirst);
            }
        }

        std::swap(apsoSwap[0], apsoSwap[1]);
    }

    for (LSG* plsg0 = aplsgEdge[0]; plsg0 != nullptr; plsg0 = plsg0->plsgNext)
    {
        SURF* psurf0 = plsg0->data.bsp.apsurf[0];
        SURF* psurf1 = plsg0->data.bsp.apsurf[1];

        if (psurf0 == nullptr || psurf1 == nullptr)
            continue;

        SURF* asurf1 = pso1->geomWorld.asurf.data();
        int isurf0 = static_cast<int>(psurf0 - asurf1);
        int isurf1 = static_cast<int>(psurf1 - asurf1);
        int clsgOther = CplsgFindEdgePath(asurf1, isurf0, isurf1, aplsgEdge[1], aplsgOther);

        for (int ilsg = 0; ilsg < clsgOther; ilsg++)
        {
            LSG* plsg1 = aplsgOther[ilsg];

            EDGE* pedge0 = plsg0->pedge;
            EDGE* pedge1 = plsg1->pedge;

            glm::vec3* apos0 = pso0->geomWorld.apos.data();
            glm::vec3* apos1 = pso1->geomWorld.apos.data();

            glm::vec3* ppos00 = &apos0[pedge0->aipos[0]];
            glm::vec3* ppos01 = &apos0[pedge0->aipos[1]];
            glm::vec3* ppos10 = &apos1[pedge1->aipos[0]];
            glm::vec3* ppos11 = &apos1[pedge1->aipos[1]];

            float au[2] = {};
            float s = 0.0f;
            glm::vec3 normal = {};
            FindClosestPointBetweenLines(ppos00, ppos01, ppos10, ppos11, au, &s, &normal);

            glm::vec3 pos0 = (*ppos00 * (1.0f - au[0])) + (*ppos01 * au[0]);

            PxpAdd(pso0, pso1, &pos0, &normal, -s, XPK_EdgeToEdge, pedge0, pedge1, ppxpFirst);
        }
    }
}

int CsbiIntersectSphereBsp(glm::vec3* pposCenter, float sRadius, int cbsp, BSP* absp, BSP* pbspPrune, GEOM* pgeom, int csbiMax, SBI* asbi)
{
    if (csbiMax <= 0)
        return 0;

    int csbi = 0;
    int cvsxp = 0;
    VSXP avsxp[64] = {};

    std::vector<BSP> abspTemp;
    BSP* abspWork = absp;
    BSP* pbspWorkLast = pbspPrune;
    int cbspWork = cbsp;

    if (pbspPrune == nullptr)
    {
        abspTemp.resize(cbsp);
        abspWork = abspTemp.data();
        pbspWorkLast = abspWork;

        PRUNE prune = PruneBsp(absp, pposCenter, sRadius, &pbspWorkLast);

        if (prune != PRUNE_Mixed)
            return 0;

        cbspWork = static_cast<int>(pbspWorkLast - abspWork);
        pbspWorkLast = pbspWorkLast - 1;
    }

    for (int ibsp = cbspWork - 1; ibsp >= 0; ibsp--)
    {
        SURF* psurf = abspWork[ibsp].psurf;
        int isurf = static_cast<int>(psurf - pgeom->asurf.data());

        glm::vec3 dposSurf = *pposCenter - pgeom->mpisurfposCenter[isurf];
        float sBound = sRadius + pgeom->mpisurfsRadius[isurf];

        if (glm::dot(dposSurf, dposSurf) >= sBound * sBound)
            continue;

        float gPlane = glm::dot(*pposCenter, psurf->normal) - psurf->gDot;
        glm::vec3 posPlane = *pposCenter - psurf->normal * gPlane;
        glm::vec3 dposPlane = posPlane - pgeom->mpisurfposCenter[isurf];

        float sOutside = glm::length(dposPlane) - pgeom->mpisurfsRadius[isurf];
        if (sOutside < 0.0f)
            sOutside = 0.0f;

        if (sOutside * sOutside + gPlane * gPlane <= sRadius * sRadius)
            psurf->grfsurf |= 0x10;
    }

    for (int ibsp = cbspWork - 1; ibsp >= 0; ibsp--)
    {
        SURF* psurf = abspWork[ibsp].psurf;

        if ((psurf->grfsurf & 0x10) == 0 || (psurf->grfsurf & 0x40) != 0)
            continue;

        psurf->grfsurf |= 0x40;

        glm::vec3 pos0 = *pposCenter + psurf->normal * sRadius;
        glm::vec3 pos1 = *pposCenter - psurf->normal * sRadius;

        LSG alsg[16] = {};
        int clsg = ClsgClipEdgeToBsp(pbspWorkLast, &pos0, &pos1, nullptr, 16, alsg);

        for (int ilsg = 0; ilsg < clsg; ilsg++)
        {
            SURF* psurfHit = (alsg[ilsg].au[0] + alsg[ilsg].au[1] > 1.0f) ? alsg[ilsg].data.bsp.apsurf[0] : alsg[ilsg].data.bsp.apsurf[1];

            if (psurfHit != psurf)
                continue;

            if (csbi < csbiMax)
            {
                SBI* psbi = &asbi[csbi++];

                psbi->pos = *pposCenter - psurf->normal * sRadius;
                psbi->normal = psurf->normal;
                psbi->gDist = glm::dot(psbi->pos, psurf->normal) - psurf->gDot;
                psbi->xpk = XPK_SphereToSurface;
                psbi->psurf = psurf;
            }
            else
            {
                psurf->grfsurf |= 0x80;
            }

            break;
        }
    }

    for (int ibsp = cbspWork - 1; ibsp >= 0; ibsp--)
    {
        SURF* psurf = abspWork[ibsp].psurf;

        if ((psurf->grfsurf & 0x10) == 0 || (psurf->grfsurf & 0xa0) != 0)
            continue;

        psurf->grfsurf |= 0x20;

        for (EDGE* pedge = psurf->pedge; pedge != nullptr; pedge = pedge->pedgeNext)
        {
            SURF* psurfOther = &pgeom->asurf[pedge->aisurf[1]];

            if ((psurfOther->grfsurf & 0x10) == 0 || (psurfOther->grfsurf & 0x80) != 0)
                continue;

            glm::vec3* ppos0 = &pgeom->apos[pedge->aipos[0]];
            glm::vec3* ppos1 = &pgeom->apos[pedge->aipos[1]];

            glm::vec3 dposEdge = *ppos1 - *ppos0;
            float lenEdgeSq = glm::dot(dposEdge, dposEdge);

            if (lenEdgeSq <= 0.0f)
                continue;

            float u = glm::dot(*pposCenter - *ppos0, dposEdge) / lenEdgeSq;
            glm::vec3 dposClosest;
            int ipos = -1;

            if (u < 0.0001f)
            {
                ipos = pedge->aipos[0];
                dposClosest = *pposCenter - *ppos0;
            }
            else if (u > 0.9999f)
            {
                ipos = pedge->aipos[1];
                dposClosest = *pposCenter - *ppos1;
            }
            else
            {
                glm::vec3 posClosest = *ppos0 + dposEdge * u;
                dposClosest = *pposCenter - posClosest;
            }

            float distSq = glm::dot(dposClosest, dposClosest);

            if (distSq > sRadius * sRadius)
                continue;

            if (ipos >= 0)
            {
                bool fDuplicate = false;

                for (int ivsxp = 0; ivsxp < cvsxp; ivsxp++)
                {
                    if (avsxp[ivsxp].ipos == ipos)
                    {
                        fDuplicate = true;
                        break;
                    }
                }

                if (!fDuplicate && cvsxp < 64)
                {
                    float dist = std::sqrt(distSq);
                    glm::vec3 normal = (dist > 0.0001f) ? dposClosest * (1.0f / dist) : glm::vec3(0.0f, 0.0f, 1.0f);

                    avsxp[cvsxp].ipos = ipos;
                    avsxp[cvsxp].normal = normal;
                    avsxp[cvsxp].pos = *pposCenter - normal * sRadius;
                    avsxp[cvsxp].s = dist - sRadius;
                    cvsxp++;
                }

                continue;
            }

            if (csbi < csbiMax)
            {
                float dist = std::sqrt(distSq);
                glm::vec3 normal = (dist > 0.0001f) ? dposClosest * (1.0f / dist) : glm::vec3(0.0f, 0.0f, 1.0f);

                SBI* psbi = &asbi[csbi++];

                psbi->pos = *pposCenter - normal * sRadius;
                psbi->normal = normal;
                psbi->gDist = dist - sRadius;
                psbi->xpk = XPK_SphereToEdge;
                psbi->pedge = pedge;
            }
        }
    }

    for (int ivsxp = cvsxp - 1; ivsxp >= 0; ivsxp--)
    {
        int ipos = avsxp[ivsxp].ipos;
        bool fCoveredByEdge = false;

        for (int isbi = 0; isbi < csbi; isbi++)
        {
            if (asbi[isbi].xpk != XPK_SphereToEdge)
                continue;

            EDGE* pedge = asbi[isbi].pedge;

            if (pedge->aipos[0] == ipos || pedge->aipos[1] == ipos)
            {
                fCoveredByEdge = true;
                break;
            }
        }

        if (fCoveredByEdge)
            continue;

        if (csbi >= csbiMax)
            break;

        SBI* psbi = &asbi[csbi++];

        psbi->pos = avsxp[ivsxp].pos;
        psbi->normal = avsxp[ivsxp].normal;
        psbi->gDist = avsxp[ivsxp].s;
        psbi->xpk = XPK_SphereToPoint;
        psbi->ppos = &pgeom->apos[ipos];
    }

    for (int ibsp = cbspWork - 1; ibsp >= 0; ibsp--)
        abspWork[ibsp].psurf->grfsurf &= ~uint16_t(0xf0);

    return csbi;
}

void MarkSoContactsSphereBsp(SO* pso0, glm::vec3* pposCenter, float sRadius, SO* pso1, int cbsp, BSP* absp, BSP* pbspPrune, XP** ppxpFirst)
{
    SBI asbi[64] = {};

    int csbi = CsbiIntersectSphereBsp(pposCenter, sRadius, cbsp, absp, pbspPrune, &pso1->geomWorld, 64, asbi);

    for (int isbi = csbi - 1; isbi >= 0; isbi--)
        PxpAdd(pso0, pso1, &asbi[isbi].pos, &asbi[isbi].normal, asbi[isbi].gDist, asbi[isbi].xpk, nullptr, asbi[isbi].pv, ppxpFirst);
}

void MarkSoContactsSphereSphere(SO* pso0, SO* pso1, XP** ppxpFirst)
{
    glm::vec3 dpos = pso0->xf.posWorld - pso1->xf.posWorld;

    float distSq = glm::dot(dpos, dpos);
    float radiusSum = pso0->sRadiusSelf + pso1->sRadiusSelf;

    if (distSq >= 0.0001f && distSq <= radiusSum * radiusSum) {
        float dist = sqrtf(distSq);

        glm::vec3 normal = dpos * (1.0f / dist);
        glm::vec3 pos = pso0->xf.posWorld - normal * pso0->sRadiusSelf;

        PxpAdd(pso0, pso1, &pos, &normal, dist - radiusSum, XPK_SphereToSphere, nullptr, nullptr, ppxpFirst);
    }
}

void CalculateCenterContact(SO* pso, glm::vec3* pnormal, float gDot, glm::vec3* ppos, float* ps)
{
    float g = glm::dot(pso->xf.posWorld, *pnormal);

    if (pso->geomWorld.cpos < 1)
    {
        g -= pso->sRadiusSelf;
        *ppos = pso->xf.posWorld + (*pnormal * -pso->sRadiusSelf);
    }
    else
    {
        *ppos = pso->xf.posWorld;

        for (int ipos = pso->geomWorld.cpos - 1; ipos >= 0; ipos--)
        {
            glm::vec3* pos = &pso->geomWorld.apos[ipos];
            float gPos = glm::dot(*pos, *pnormal);

            if (gPos < g)
            {
                *ppos = *pos;
                g = gPos;
            }
        }
    }

    *ps = g - gDot;
}

int FMarkSoContactsCenterBsp(SO* pso0, SO* pso1, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst)
{
    LSG alsg[2] = {};
    glm::vec3 pos = {};
    float s = 0.0f;

    int clsg = ClsgClipEdgeToBsp(pbspPruned, &pso0->posWorldPrev, &pso0->xf.posWorld, nullptr, 2, alsg);

    int ilsg = 0;
    if (clsg > 0 && alsg[0].data.bsp.apsurf[0] == nullptr)
        ilsg = 1;

    if (ilsg < clsg)
    {
        SURF* psurf = alsg[ilsg].data.bsp.apsurf[0];

        CalculateCenterContact(pso0, &psurf->normal, psurf->gDot, &pos, &s);
        PxpAdd(pso0, pso1, &pos, &psurf->normal, s, XPK_CenterToSurface, nullptr, psurf, ppxpFirst);
    }

    return ilsg < clsg;
}

int FClipEdgeToSphere(glm::vec3* pposSphere, float sRadius, glm::vec3* ppos0, glm::vec3* ppos1, float* pu, glm::vec3* ppos, glm::vec3* pnormal, float* pgDot)
{
    glm::vec3 dposSphere = *ppos0 - *pposSphere;
    glm::vec3 dposEdge = *ppos1 - *ppos0;

    float a = glm::dot(dposEdge, dposEdge);
    float b = 2.0f * glm::dot(dposEdge, dposSphere);
    float c = glm::dot(dposSphere, dposSphere) - sRadius * sRadius;

    float discr = b * b - 4.0f * a * c;

    if (discr < 0.0f)
        return 0;

    float u = (-b - std::sqrt(discr)) / (2.0f * a);

    if (u < 0.0f || u > 1.0f)
        return 0;

    glm::vec3 pos = (*ppos0 * (1.0f - u)) + (*ppos1 * u);
    glm::vec3 normal = (pos - *pposSphere) * (1.0f / sRadius);

    if (pu != nullptr)
        *pu = u;

    if (ppos != nullptr)
        *ppos = pos;

    if (pnormal != nullptr)
        *pnormal = normal;

    if (pgDot != nullptr)
        *pgDot = glm::dot(normal, *pposSphere) + sRadius;

    return 1;
}

int FMarkSoContactsCenterSphere(SO* pso0, SO* pso1, XP** ppxpFirst)
{
    glm::vec3 pos = {};
    glm::vec3 normal = {};
    float gDot = 0.0f;
    float s = 0.0f;

    int fHit = FClipEdgeToSphere(&pso1->xf.posWorld, pso1->sRadiusSelf, &pso0->posWorldPrev, &pso0->xf.posWorld, nullptr, &pos, &normal, &gDot);

    if (fHit)
    {
        CalculateCenterContact(pso0, &normal, gDot, &pos, &s);
        PxpAdd(pso0, pso1, &pos, &normal, s, XPK_CenterToSphere, nullptr, nullptr, ppxpFirst);
    }

    return fHit != 0;
}

void AddSoXps(SO* psoRoot, SO* psoOther, int fCenterXp, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst)
{
    for (SO* pso0 = psoRoot->dlPhys.psoFirst; pso0 != nullptr; pso0 = pso0->dlePhys.psoNext)
    {
        if (pso0->fNoXpsSelf)
            continue;

        if (!pso0->fSphere && pso0->bspc.absp.data() == nullptr)
            continue;

        if ((pso0->fIgnoreLocked || pso0->fLockedAbove) &&
            (psoOther->fIgnoreLocked || psoOther->fLockedAbove))
        {
            if (pso0->fLockedAbove)
                continue;

            if (psoOther->fLockedAbove)
                continue;
        }

        if (fCenterXp)
        {
            bool handledCenterXp = false;

            if (!pso0->fNoXpsCenter)
            {
                glm::vec3 dpos = pso0->posWorldPrev - pso0->xf.posWorld;
                float dpos2 = glm::dot(dpos, dpos);

                if (dpos2 > 1.0f)
                {
                    int hit = 0;

                    if (!psoOther->fSphere)
                        hit = FMarkSoContactsCenterBsp(pso0, psoOther, cbspPruned, abspPruned, pbspPruned, ppxpFirst);
                    else
                        hit = FMarkSoContactsCenterSphere(pso0, psoOther, ppxpFirst);

                    if (hit)
                    {
                        pso0->fCenterXp = 1;
                        handledCenterXp = true;
                    }
                }
            }

            if (!handledCenterXp && !psoOther->fNoXpsCenter)
            {
                glm::vec3 dpos = psoOther->posWorldPrev - psoOther->xf.posWorld;
                float dpos2 = glm::dot(dpos, dpos);

                if (dpos2 > 1.0f)
                {
                    int hit = 0;

                    if (!pso0->fSphere)
                    {
                        BSP* absp = pso0->bspc.absp.data();
                        hit = FMarkSoContactsCenterBsp(psoOther, pso0, pso0->bspc.cbsp, absp, absp, ppxpFirst);
                    }
                    else
                        hit = FMarkSoContactsCenterSphere(psoOther, pso0, ppxpFirst);

                    if (hit)
                    {
                        psoOther->fCenterXp = 1;
                        continue;
                    }
                }
            }
        }

        if (!psoOther->fSphere)
        {
            if (!pso0->fSphere)
                MarkSoContactsBspBsp(pso0, psoOther, cbspPruned, abspPruned, pbspPruned, ppxpFirst);
            else
            {
                bool useUnpruned =
                    (pso0 != psoRoot) ||
                    (pso0->sRadiusSelf != pso0->sRadiusPrune);

                BSP* absp = useUnpruned ? pbspPruned : abspPruned;
                BSP* pbsp = useUnpruned ? nullptr : pbspPruned;

                MarkSoContactsSphereBsp(pso0, &pso0->xf.posWorld, pso0->sRadiusSelf, psoOther, cbspPruned, absp, pbsp, ppxpFirst);
            }
        }
        else
        {
            if (!pso0->fSphere)
                MarkSoContactsSphereBsp(psoOther, &psoOther->xf.posWorld, psoOther->sRadiusSelf, pso0, pso0->bspc.cbspFull, pso0->bspc.absp.data(), nullptr, ppxpFirst);
            else
                MarkSoContactsSphereSphere(pso0, psoOther, ppxpFirst);
        }

    }

}

XP* PxpMarkSwContactsRoot(SW* psw, SO** apso, int fCenterXp)
{
    SO* apsoRoot[2];

    apsoRoot[0] = apso[1];
    apsoRoot[1] = apso[0];

    if (apsoRoot[1]->sRadiusAll <= apsoRoot[0]->sRadiusAll)
    {
        apsoRoot[0] = apsoRoot[1];
        apsoRoot[1] = apso[1];
    }

    XP* pxpFirst = nullptr;

    int cbspFullMax = 0;

    for (SO* psoPhys = apsoRoot[1]->dlPhys.psoFirst; psoPhys != nullptr; psoPhys = psoPhys->dlePhys.psoNext)
        cbspFullMax = std::max(cbspFullMax, psoPhys->bspc.cbspFull);

    // The PS2 implementation obtains this as temporary stack scratch.  A
    // fresh std::vector here allocated and value-initialized the entire BSP
    // buffer once per overlap pair, twice per frame.  Break debris adds
    // several moving pairs, making that allocation dominate the physics pass.
    // Reuse one per-thread buffer; PruneBsp writes every entry it emits, so
    // retained contents do not need clearing between calls.
    thread_local std::vector<BSP> abspPruned;
    if (abspPruned.size() < static_cast<size_t>(cbspFullMax))
        abspPruned.resize(static_cast<size_t>(cbspFullMax));

    BSP* pbspBase = abspPruned.data();

    for (SO* psoPhys = apsoRoot[1]->dlPhys.psoFirst; psoPhys != nullptr; psoPhys = psoPhys->dlePhys.psoNext)
    {
        if (psoPhys->fNoXpsSelf)
            continue;

        int cbsp = 0;
        BSP* pbspFirst = nullptr;
        BSP* pbspLast = nullptr;

        if (psoPhys->fSphere)
        {
            pbspFirst = nullptr;
            pbspLast = nullptr;
        }
        else
        {
            BSP* pbsp = psoPhys->bspc.absp.data();

            if (pbsp == nullptr)
                continue;

            BSP* pbspPruned = pbspBase;

            PRUNE prune = PruneBsp(pbsp, &apsoRoot[0]->posPrune, apsoRoot[0]->sRadiusPrune, &pbspPruned);

            if (prune != PRUNE_Mixed)
                continue;

            cbsp = static_cast<int>(pbspPruned - pbspBase);
            pbspLast = pbspPruned - 1;
            pbspFirst = pbspBase;
        }

        apsoRoot[0]->pvtso->pfnAddSoXps(apsoRoot[0], psoPhys, fCenterXp, cbsp, pbspFirst, pbspLast, &pxpFirst);

        if (apsoRoot[0]->pvtso->pfnAddSoCustomXps != nullptr)
        {
            apsoRoot[0]->pvtso->pfnAddSoCustomXps(apsoRoot[0], psoPhys, cbsp, pbspFirst, pbspLast, &pxpFirst);
        }
    }

    for (SO* psoPhys = apsoRoot[0]->dlPhys.psoFirst; psoPhys != nullptr; psoPhys = psoPhys->dlePhys.psoNext)
    {
        if (psoPhys->fNoXpsSelf)
            continue;

        if (!psoPhys->fSphere && psoPhys->bspc.absp.data() == nullptr)
            continue;

        if (apsoRoot[1]->pvtso->pfnAddSoCustomXps != nullptr)
        {
            BSP* pbsp = psoPhys->bspc.absp.data();
            apsoRoot[1]->pvtso->pfnAddSoCustomXps(apsoRoot[1], psoPhys, psoPhys->bspc.cbsp, pbsp, pbsp, &pxpFirst);
        }
    }

    return pxpFirst;
}


int IsurfFromBspSurf(const GEOM& geom, const SURF* psurf)
{
    if (psurf == nullptr || geom.asurf.empty())
        return -1;

    const SURF* asurfBase = geom.asurf.data();
    const SURF* asurfEnd = asurfBase + geom.asurf.size();

    if (psurf < asurfBase || psurf >= asurfEnd)
        return -1;

    return static_cast<int>(psurf - asurfBase);
}

MTLKWA s_mpmtlkwaMu[12] = 
{
    { 1.0f, 1.0f  },
    { 3.0f, 2.4f  },
    { 1.0f, 0.4f  },
    { 1.0f, 0.5f  },
    { 1.0f, 0.4f  },
    { 1.0f, 1.0f  },
    { 5.0f, 0.5f  },
    { 1.0f, 1.5f  },
    { 3.0f, 1.5f  },
    { 1.0f, 0.5f  },
    { 1.0f, -10.0f },
    { 1.0f, -10.0f },
};

MTLKWA s_mpmtlkwaElas[12] = 
{
    { 1.0f, 0.5f },
    { 5.0f, 1.0f },
    { 1.0f, 0.8f },
    { 1.0f, 0.4f },
    { 1.0f, 0.6f },
    { 1.0f, 0.9f },
    { 1.0f, 0.1f },
    { 1.0f, -1.0f },
    { 1.0f, -1.0f },
    { 1.0f, -1.0f },
    { 1.0f, 0.5f },
    { 1.0f, 1.0f },
};

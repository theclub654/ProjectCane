#include "path.h"
#include "bsp.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>

PATHZONE* NewPathzone()
{
    return new PATHZONE{};
}

CBSP* PcbspExtract(CBSP* pcbspMod)
{
    if (((uintptr_t)pcbspMod & 1) != 0)
        return nullptr;

    return pcbspMod;
}

CGT* PcgtExtract(CGT* pcgtMod)
{
    if (((uintptr_t)pcgtMod & 1) == 0)
        return nullptr;

    return (CGT*)((uintptr_t)pcgtMod & ~1);
}

CGT* PcgtPointInCbspQuick(CBSP* pcbsp, const glm::vec3* ppos)
{
    while (pcbsp != nullptr)
    {
        const float gDot = glm::dot(*ppos, pcbsp->normal);

        if (gDot - pcbsp->gDot > 0.0001f)
        {
            if (CGT* pcgt = PcgtExtract(pcbsp->pcgtPos))
                return pcgt;

            pcbsp = pcbsp->pcbspPos;
        }
        else
        {
            if (CGT* pcgt = PcgtExtract(pcbsp->pcgtNeg))
                return pcgt;

            pcbsp = pcbsp->pcbspNeg;
        }
    }

    return nullptr;
}

CGT* PcgtPointInCbspSafe(CBSP* pcbsp, const glm::vec3* ppos)
{
    constexpr float kPlaneTolerance = 0.01f;

    while (pcbsp != nullptr)
    {
        const float gDistance = glm::dot(*ppos, pcbsp->normal) - pcbsp->gDot;

        if (gDistance < -kPlaneTolerance)
        {
            if (CGT* pcgt = PcgtExtract(pcbsp->pcgtNeg))
                return pcgt;

            pcbsp = pcbsp->pcbspNeg;
        }
        else if (gDistance > kPlaneTolerance)
        {
            if (CGT* pcgt = PcgtExtract(pcbsp->pcgtPos))
                return pcgt;

            pcbsp = pcbsp->pcbspPos;
        }
        else
        {
            // The point is close enough to the splitting plane that either
            // side may contain it. Test the positive side first.
            if (CGT* pcgt = PcgtExtract(pcbsp->pcgtPos))
                return pcgt;

            if (CGT* pcgt = PcgtPointInCbspSafe(pcbsp->pcbspPos, ppos))
                return pcgt;

            if (CGT* pcgt = PcgtExtract(pcbsp->pcgtNeg))
                return pcgt;

            pcbsp = pcbsp->pcbspNeg;
        }
    }

    return nullptr;
}

CBSK CbskFromG(float g)
{
    if (g > 0.01f)
        return CBSK_Pos;

    if (g < -0.01f)
        return CBSK_Neg;

    return CBSK_On;
}

int ClsgClipEdgeToCbsp(CBSP* pcbspRoot, const glm::vec3* ppos1, const glm::vec3* ppos2, int clsgMax, LSG* alsg)
{
    struct CECB
    {
        CBSP* pcbsp;
        CBSP* pcbspNeg;
        float u;
    };

    CECB cecb;
    CECB acecb[32];
    CECB* pececb = acecb;
    LSG* plsgLast = nullptr;
    int clsg = 0;

    cecb.pcbsp = pcbspRoot;
    cecb.pcbspNeg = nullptr;
    cecb.u = 0.0f;

    acecb[0].pcbsp = nullptr;
    acecb[0].pcbspNeg = nullptr;
    acecb[0].u = 1.0f;

    while (true) {
        while (cecb.pcbsp == nullptr) {
            if (cecb.pcbspNeg != nullptr) {
                float u0 = cecb.u;
                float u1 = pececb->u;

                if (plsgLast == nullptr || u0 != plsgLast->au[1]) {
                    if (clsg >= clsgMax)
                        return clsgMax;

                    LSG* plsg = &alsg[clsg++];

                    plsg->lsgk = LSGK_EdgeToCbsp;
                    plsg->apos[0] = glm::mix(*ppos1, *ppos2, u0);
                    plsg->apos[1] = glm::mix(*ppos1, *ppos2, u1);
                    plsg->au[0] = u0;
                    plsg->au[1] = u1;
                    plsg->id.n = 0;
                    plsg->data = {};
                    plsg->pedge = nullptr;
                    plsg->iiposSwap = 0;
                    plsg->plsgNext = nullptr;

                    plsgLast = plsg;
                }
                else {
                    plsgLast->apos[1] = glm::mix(*ppos1, *ppos2, u1);
                    plsgLast->au[1] = u1;
                }
            }

            if (pececb <= acecb)
                return clsg;

            cecb = *pececb--;
        }

        float g0 = glm::dot(*ppos1, cecb.pcbsp->normal) - cecb.pcbsp->gDot;
        float g1 = glm::dot(*ppos2, cecb.pcbsp->normal) - cecb.pcbsp->gDot;

        CBSK cbsk0 = CbskFromG(g0);
        CBSK cbsk1 = CbskFromG(g1);

        if (cbsk0 == CBSK_Neg && cbsk1 != CBSK_Pos) {
            cecb.pcbspNeg = cecb.pcbsp;
            cecb.pcbsp = PcbspExtract(cecb.pcbsp->pcbspNeg);
            continue;
        }

        if (cbsk0 == CBSK_Pos && cbsk1 != CBSK_Neg) {
            CGT* pcgt = PcgtExtract(cecb.pcbsp->pcgtPos);

            if (pcgt != nullptr) {
                cecb.pcbspNeg = cecb.pcbsp;
                cecb.pcbsp = nullptr;
            }
            else {
                cecb.pcbsp = PcbspExtract(cecb.pcbsp->pcbspPos);
                cecb.pcbspNeg = nullptr;
            }

            continue;
        }

        if (cbsk0 == CBSK_On && cbsk1 == CBSK_On) {
            cecb.pcbspNeg = cecb.pcbsp;
            cecb.pcbsp = PcbspExtract(cecb.pcbsp->pcbspNeg);
            continue;
        }

        // Match the original branch ordering for an edge whose first point
        // lies on the split plane. It follows the side containing the second
        // point; it is not treated as an edge crossing.
        if (cbsk0 == CBSK_On && cbsk1 == CBSK_Neg) {
            cecb.pcbspNeg = cecb.pcbsp;
            cecb.pcbsp = PcbspExtract(cecb.pcbsp->pcbspNeg);
            continue;
        }

        if (cbsk0 == CBSK_On && cbsk1 == CBSK_Pos) {
            CGT* pcgt = PcgtExtract(cecb.pcbsp->pcgtPos);

            if (pcgt != nullptr) {
                cecb.pcbspNeg = cecb.pcbsp;
                cecb.pcbsp = nullptr;
            }
            else {
                cecb.pcbsp = PcbspExtract(cecb.pcbsp->pcbspPos);
                cecb.pcbspNeg = nullptr;
            }

            continue;
        }

        float uSplit = g0 / (g0 - g1);
        float gStart = g1 * cecb.u + (1.0f - cecb.u) * g0;
        float gEnd = g1 * pececb->u + (1.0f - pececb->u) * g0;

        if (cecb.u < uSplit && uSplit < pececb->u) {
            CECB* pececbNew = pececb + 1;
            pececbNew->u = uSplit;

            if (gStart <= 0.0f) {
                CGT* pcgt = PcgtExtract(cecb.pcbsp->pcgtPos);

                if (pcgt == nullptr) {
                    pececbNew->pcbsp = PcbspExtract(cecb.pcbsp->pcbspPos);
                    pececbNew->pcbspNeg = nullptr;
                }
                else {
                    pececbNew->pcbsp = nullptr;
                    pececbNew->pcbspNeg = cecb.pcbsp;
                }
            }
            else {
                pececbNew->pcbspNeg = cecb.pcbsp;
                pececbNew->pcbsp = PcbspExtract(cecb.pcbsp->pcbspNeg);
            }

            pececb = pececbNew;
        }

        if (gStart < 0.0f || (gStart == 0.0f && gEnd <= 0.0f)) {
            cecb.pcbspNeg = cecb.pcbsp;
            cecb.pcbsp = PcbspExtract(cecb.pcbsp->pcbspNeg);
            continue;
        }

        CGT* pcgt = PcgtExtract(cecb.pcbsp->pcgtPos);

        if (pcgt != nullptr) {
            cecb.pcbspNeg = cecb.pcbsp;
            cecb.pcbsp = nullptr;
        }
        else {
            cecb.pcbsp = PcbspExtract(cecb.pcbsp->pcbspPos);
            cecb.pcbspNeg = nullptr;
        }
    }
}

bool FClipEdgeToCbsp(CBSP* pcbspRoot, const glm::vec3* ppos1, const glm::vec3* ppos2)
{
    LSG alsg[2];

    const int clsg = ClsgClipEdgeToCbsp(pcbspRoot, ppos1, ppos2, 2, alsg);

    // One segment covering the complete edge means no clipping occurred.
    if (clsg == 1 && alsg[0].au[0] == 0.0f && alsg[0].au[1] == 1.0f)
        return false;

    return true;
}

int IcgvFromPcgv(CG* pcg, CGV* pcgv)
{
    return static_cast<int>(pcgv - pcg->acgv.data());
}

void FindPathAStar(CG* pcg, CGV* pcgvStart, CGV* pcgvEnd, int cpcgvMax, int* pcpcgv, CGV** apcgv)
{
    *pcpcgv = 0;

    if (pcg == nullptr || pcgvStart == nullptr || pcgvEnd == nullptr || apcgv == nullptr || cpcgvMax <= 0)
        return;

    struct ASTARNODE
    {
        CGV* pcgv;
        float gCost;
        float fCost;
    };

    struct ASTARCOMPARE
    {
        bool operator()(const ASTARNODE& lhs, const ASTARNODE& rhs) const
        {
            return lhs.fCost > rhs.fCost;
        }
    };

    std::unordered_map<CGV*, float> pathDist;
    std::unordered_map<CGV*, CGV*> parent;
    std::unordered_set<CGV*> closed;
    std::priority_queue<ASTARNODE, std::vector<ASTARNODE>, ASTARCOMPARE> queue;

    pathDist[pcgvStart] = 0.0f;
    parent[pcgvStart] = nullptr;
    queue.push({ pcgvStart, 0.0f, glm::distance(pcgvStart->pos, pcgvEnd->pos) });

    bool fFound = false;

    while (!queue.empty())
    {
        const ASTARNODE node = queue.top();
        queue.pop();

        if (closed.find(node.pcgv) != closed.end())
            continue;

        closed.insert(node.pcgv);

        if (node.pcgv == pcgvEnd)
        {
            fFound = true;
            break;
        }

        CGV* pcgvCur = node.pcgv;
        const int cpcge = std::min(pcgvCur->cpcge, static_cast<int>(pcgvCur->apcge.size()));

        for (int ipcge = 0; ipcge < cpcge; ++ipcge)
        {
            CGE* pcge = pcgvCur->apcge[ipcge];
            if (pcge == nullptr)
                continue;

            CGV* pcgvNext = nullptr;
            if (pcge->apcgv[0] == pcgvCur)
                pcgvNext = pcge->apcgv[1];
            else if (pcge->apcgv[1] == pcgvCur)
                pcgvNext = pcge->apcgv[0];

            if (pcgvNext == nullptr || closed.find(pcgvNext) != closed.end())
                continue;

            const float gPathDist = node.gCost + pcge->s;
            const auto itDist = pathDist.find(pcgvNext);

            if (itDist != pathDist.end() && gPathDist >= itDist->second)
                continue;

            pathDist[pcgvNext] = gPathDist;
            parent[pcgvNext] = pcgvCur;

            const float gHeuristic = glm::distance(pcgvNext->pos, pcgvEnd->pos);
            queue.push({ pcgvNext, gPathDist, gPathDist + gHeuristic });
        }
    }

    if (!fFound)
        return;

    std::vector<CGV*> apcgvReverse;

    for (CGV* pcgv = pcgvEnd; pcgv != nullptr; pcgv = parent[pcgv])
    {
        apcgvReverse.push_back(pcgv);

        if (pcgv == pcgvStart)
            break;
    }

    if (apcgvReverse.empty() || apcgvReverse.back() != pcgvStart)
        return;

    if (static_cast<int>(apcgvReverse.size()) > cpcgvMax)
        return;

    *pcpcgv = static_cast<int>(apcgvReverse.size());

    for (int ipcgv = 0; ipcgv < *pcpcgv; ++ipcgv)
        apcgv[ipcgv] = apcgvReverse[*pcpcgv - ipcgv - 1];
}

void SimplifyApcgvNeighbor(CBSP* pcbsp, int* pcpcgv, CGV** apcgv)
{
    if (pcpcgv == nullptr || apcgv == nullptr || *pcpcgv <= 2)
        return;

    const int cpcgvOriginal = *pcpcgv;

    std::vector<CGV*> apcgvSimple;
    apcgvSimple.reserve(cpcgvOriginal);
    apcgvSimple.push_back(apcgv[0]);

    int ipcgvAnchor = 0;

    for (int ipcgvTest = 2; ipcgvTest < cpcgvOriginal; ++ipcgvTest)
    {
        const bool fBlocked = FClipEdgeToCbsp(pcbsp, &apcgv[ipcgvAnchor]->pos, &apcgv[ipcgvTest]->pos);

        if (fBlocked)
        {
            const int ipcgvKeep = ipcgvTest - 1;
            apcgvSimple.push_back(apcgv[ipcgvKeep]);
            ipcgvAnchor = ipcgvKeep;
        }
    }

    apcgvSimple.push_back(apcgv[cpcgvOriginal - 1]);

    int cpcgvSimple = static_cast<int>(apcgvSimple.size());

    // Run another pass when the previous pass removed points. Changing the
    // anchor points can make additional long-range connections possible.
    if (cpcgvSimple < cpcgvOriginal && cpcgvSimple > 2)
    {
        SimplifyApcgvNeighbor(pcbsp, &cpcgvSimple, apcgvSimple.data());
        apcgvSimple.resize(cpcgvSimple);
    }

    std::copy(apcgvSimple.begin(), apcgvSimple.end(), apcgv);
    *pcpcgv = cpcgvSimple;
}

int CposFindPath(CG* pcg, glm::vec3* pposStart, glm::vec3* pposEnd, int cposMax, glm::vec3* apos)
{
    if (pcg == nullptr || pposStart == nullptr || pposEnd == nullptr || apos == nullptr || cposMax <= 0)
        return 0;

    CBSP* pcbsp = pcg->acbsp.data();

    glm::vec3 posStart = *pposStart;
    glm::vec3 posEnd = *pposEnd;

    CGT* pcgtStart = PcgtPointInCbspQuick(pcbsp, &posStart);

    if (pcgtStart == nullptr)
    {
        FindClosestPointInCg(pcg, pposStart, &posStart);
        pcgtStart = PcgtPointInCbspSafe(pcbsp, &posStart);
    }

    CGT* pcgtEnd = PcgtPointInCbspQuick(pcbsp, &posEnd);

    if (pcgtEnd == nullptr)
    {
        FindClosestPointInCg(pcg, pposEnd, &posEnd);
        pcgtEnd = PcgtPointInCbspSafe(pcbsp, &posEnd);
    }

    if (pcgtStart == nullptr || pcgtEnd == nullptr)
        return 0;

    if (pcgtStart == pcgtEnd)
    {
        if (cposMax < 2)
            return 0;

        apos[0] = posStart;
        apos[1] = posEnd;

        return 2;
    }

    constexpr int kVerticesPerTriangle = 3;
    constexpr int kPathVertexMax = 128;

    CGV cgvStart{};
    CGV cgvEnd{};

    std::array<CGE, kVerticesPerTriangle> acgeStart{};
    std::array<CGE, kVerticesPerTriangle> acgeEnd{};

    cgvStart.pos = posStart;
    cgvStart.apcge.reserve(kVerticesPerTriangle);

    cgvEnd.pos = posEnd;
    cgvEnd.cpcge = 0;

    for (int ipcgv = 0; ipcgv < kVerticesPerTriangle; ++ipcgv)
    {
        CGV* pcgv = pcgtStart->apcgv[ipcgv];
        CGE& cge = acgeStart[ipcgv];

        cge.apcgv[0] = &cgvStart;
        cge.apcgv[1] = pcgv;
        cge.s = glm::distance(posStart, pcgv->pos);

        cgvStart.apcge.push_back(&cge);
    }

    cgvStart.cpcge = static_cast<int>(cgvStart.apcge.size());

    for (int ipcgv = 0; ipcgv < kVerticesPerTriangle; ++ipcgv)
    {
        CGV* pcgv = pcgtEnd->apcgv[ipcgv];
        CGE& cge = acgeEnd[ipcgv];

        cge.apcgv[0] = pcgv;
        cge.apcgv[1] = &cgvEnd;
        cge.s = glm::distance(pcgv->pos, posEnd);

        pcgv->apcge.push_back(&cge);
        pcgv->cpcge = static_cast<int>(pcgv->apcge.size());
    }

    std::array<CGV*, kPathVertexMax> apcgv{};
    int cpcgv = 0;

    FindPathAStar(pcg, &cgvStart, &cgvEnd, kPathVertexMax, &cpcgv, apcgv.data());

    if (cpcgv > 2)
        SimplifyApcgvNeighbor(pcbsp, &cpcgv, apcgv.data());

    const int cpos = std::min(cpcgv, cposMax);

    for (int ipos = 0; ipos < cpos; ++ipos)
        apos[ipos] = apcgv[ipos]->pos;

    // Remove the temporary edges that connect the graph to cgvEnd.
    for (int ipcgv = 0; ipcgv < kVerticesPerTriangle; ++ipcgv)
    {
        CGV* pcgv = pcgtEnd->apcgv[ipcgv];
        CGE* pcgeTemporary = &acgeEnd[ipcgv];

        const auto it = std::find(pcgv->apcge.begin(), pcgv->apcge.end(), pcgeTemporary);

        if (it != pcgv->apcge.end())
            pcgv->apcge.erase(it);

        pcgv->cpcge = static_cast<int>(pcgv->apcge.size());
    }

    return cpos;
}

void FindClosestPointInCg(CG* pcg, glm::vec3* ppos, glm::vec3* pposClosest)
{
    if (pcg == nullptr || ppos == nullptr || pposClosest == nullptr)
        return;

    if (PcgtPointInCbspSafe(pcg->acbsp.data(), ppos) != nullptr)
    {
        *pposClosest = *ppos;
        return;
    }

    float sClosest = std::numeric_limits<float>::max();
    float uClosest = 0.0f;
    CGE* pcgeClosest = nullptr;

    const int ccgeBoundary = std::min(pcg->ccgeBoundary, static_cast<int>(pcg->acge.size()));

    for (int icge = 0; icge < ccgeBoundary; ++icge)
    {
        CGE* pcge = &pcg->acge[icge];

        if (pcge->apcgv[0] == nullptr || pcge->apcgv[1] == nullptr)
            continue;

        glm::vec3 posEdge0 = pcge->apcgv[0]->pos;
        glm::vec3 posEdge1 = pcge->apcgv[1]->pos;

        // Measure against the boundary edge in the query point's XY plane.
        posEdge0.z = ppos->z;
        posEdge1.z = ppos->z;

        float u = 0.0f;
        float s = 0.0f;

        FindClosestPointOnLineSegment(ppos, &posEdge0, &posEdge1, &u, &s);

        if (s < sClosest)
        {
            sClosest = s;
            uClosest = u;
            pcgeClosest = pcge;
        }
    }

    if (pcgeClosest == nullptr)
    {
        *pposClosest = *ppos;
        return;
    }

    const glm::vec3& posEdge0 = pcgeClosest->apcgv[0]->pos;
    const glm::vec3& posEdge1 = pcgeClosest->apcgv[1]->pos;

    *pposClosest = glm::mix(posEdge0, posEdge1, uClosest);
}

void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis)
{
    CG* pcg = &ppathzone->cg;

    LoadLoFromBrx(ppathzone, pbis);

    pcg->ccgv = pbis->U16Read();
    pcg->acgv.resize(pcg->ccgv);

    pcg->ccgeBoundary = pbis->U16Read();
    pcg->ccge = pcg->ccgeBoundary + pbis->U16Read();
    pcg->acge.resize(pcg->ccge);

    pcg->ccgt = pbis->U16Read();
    pcg->acgt.resize(pcg->ccgt);

    for (int i = 0; i < pcg->ccgv; i++)
    {
        CGV& cgv = pcg->acgv[i];

        cgv.pos = pbis->ReadVector();

        cgv.cpcge = (int)(int8_t)pbis->U8Read();
        cgv.apcge.resize(cgv.cpcge);

        for (int j = 0; j < cgv.cpcge; j++)
        {
            u16 edgeIndex = pbis->U16Read();
            cgv.apcge[j] = &pcg->acge[edgeIndex];
        }
    }

    for (int i = 0; i < pcg->ccge; i++)
    {
        CGE& cge = pcg->acge[i];

        for (int j = 0; j < 2; j++)
        {
            u16 vertexIndex = pbis->U16Read();
            cge.apcgv[j] = &pcg->acgv[vertexIndex];
        }
    }

    for (int i = 0; i < pcg->ccgt; i++)
    {
        CGT& cgt = pcg->acgt[i];

        for (int j = 0; j < 3; j++)
        {
            u16 vertexIndex = pbis->U16Read();
            cgt.apcgv[j] = &pcg->acgv[vertexIndex];
        }
    }

    pcg->ccbsp = pbis->U16Read();
    pcg->acbsp.resize(pcg->ccbsp);

    for (int i = 0; i < pcg->ccbsp; i++)
    {
        CBSP& bsp = pcg->acbsp[i];

        bsp.normal = pbis->ReadVector();
        bsp.gDot = pbis->F32Read();

        s16 negIndex = pbis->S16Read();

        if (negIndex == -1)
            bsp.pcbspNeg = nullptr;
        else if (negIndex < 0)
        {
            int triangleIndex = -(negIndex + 2);
            bsp.pcbspNeg = (CBSP*)((uintptr_t)&pcg->acgt[triangleIndex] | 1);
        }
        else
            bsp.pcbspNeg = &pcg->acbsp[negIndex];

        s16 posIndex = pbis->S16Read();

        if (posIndex == -1)
            bsp.pcbspPos = nullptr;
        else if (posIndex < 0)
        {
            int triangleIndex = -(posIndex + 2);
            bsp.pcbspPos = (CBSP*)((uintptr_t)&pcg->acgt[triangleIndex] | 1);
        }
        else
            bsp.pcbspPos = &pcg->acbsp[posIndex];
    }

    HookupCg(pcg);
}

void ClonePathzone(PATHZONE* ppathzone, PATHZONE* ppathzoneBase)
{
    DLE savedDlePathzone = ppathzone->dlePathzone;

    CloneLo(ppathzone, ppathzoneBase);

    ppathzone->cg = ppathzoneBase->cg;

    CG& cgDst = ppathzone->cg;
    const CG& cgSrc = ppathzoneBase->cg;

    std::unordered_map<const CGV*, size_t> icgvFromPcgv;
    std::unordered_map<const CGE*, size_t> icgeFromPcge;
    std::unordered_map<const CGT*, size_t> icgtFromPcgt;
    std::unordered_map<const CBSP*, size_t> icbspFromPcbsp;

    for (size_t i = 0; i < cgSrc.acgv.size(); ++i) icgvFromPcgv[&cgSrc.acgv[i]] = i;
    for (size_t i = 0; i < cgSrc.acge.size(); ++i) icgeFromPcge[&cgSrc.acge[i]] = i;
    for (size_t i = 0; i < cgSrc.acgt.size(); ++i) icgtFromPcgt[&cgSrc.acgt[i]] = i;
    for (size_t i = 0; i < cgSrc.acbsp.size(); ++i) icbspFromPcbsp[&cgSrc.acbsp[i]] = i;

    const auto rebaseVertex = [&](CGV* pcgv) -> CGV* {
        const auto it = icgvFromPcgv.find(pcgv);
        return it == icgvFromPcgv.end() ? nullptr : &cgDst.acgv[it->second];
    };
    const auto rebaseEdge = [&](CGE* pcge) -> CGE* {
        const auto it = icgeFromPcge.find(pcge);
        return it == icgeFromPcge.end() ? nullptr : &cgDst.acge[it->second];
    };
    const auto rebaseTriangle = [&](CGT* pcgt) -> CGT* {
        const auto it = icgtFromPcgt.find(pcgt);
        return it == icgtFromPcgt.end() ? nullptr : &cgDst.acgt[it->second];
    };

    for (CGV& cgv : cgDst.acgv)
    {
        for (CGE*& pcge : cgv.apcge) pcge = rebaseEdge(pcge);
        cgv.cpcge = static_cast<int>(cgv.apcge.size());
    }
    for (CGE& cge : cgDst.acge)
    {
        cge.apcgv[0] = rebaseVertex(cge.apcgv[0]);
        cge.apcgv[1] = rebaseVertex(cge.apcgv[1]);
        cge.apcgt[0] = rebaseTriangle(cge.apcgt[0]);
        cge.apcgt[1] = rebaseTriangle(cge.apcgt[1]);
    }
    for (CGT& cgt : cgDst.acgt)
        for (CGV*& pcgv : cgt.apcgv) pcgv = rebaseVertex(pcgv);

    const auto rebaseCbspChild = [&](CBSP* pcbspChild) -> CBSP* {
        if (pcbspChild == nullptr) return nullptr;
        const uintptr_t child = reinterpret_cast<uintptr_t>(pcbspChild);
        if ((child & 1) != 0)
        {
            CGT* pcgtDst = rebaseTriangle(reinterpret_cast<CGT*>(child & ~uintptr_t(1)));
            return pcgtDst == nullptr ? nullptr : reinterpret_cast<CBSP*>(reinterpret_cast<uintptr_t>(pcgtDst) | 1);
        }
        const auto it = icbspFromPcbsp.find(pcbspChild);
        return it == icbspFromPcbsp.end() ? nullptr : &cgDst.acbsp[it->second];
    };

    for (CBSP& cbsp : cgDst.acbsp)
    {
        cbsp.pcbspNeg = rebaseCbspChild(cbsp.pcbspNeg);
        cbsp.pcbspPos = rebaseCbspChild(cbsp.pcbspPos);
    }

    cgDst.ccgv = static_cast<int>(cgDst.acgv.size());
    cgDst.ccge = static_cast<int>(cgDst.acge.size());
    cgDst.ccgt = static_cast<int>(cgDst.acgt.size());
    cgDst.ccbsp = static_cast<int>(cgDst.acbsp.size());
    HookupCg(&cgDst);
    ppathzone->dlePathzone = savedDlePathzone;
}

void HookupCg(CG* pcg)
{
    if (pcg == nullptr)
        return;

    for (CGE& cge : pcg->acge)
    {
        cge.apcgt[0] = nullptr;
        cge.apcgt[1] = nullptr;
        if (cge.apcgv[0] == nullptr || cge.apcgv[1] == nullptr)
        {
            cge.s = 0.0f;
            continue;
        }

        cge.s = glm::distance(cge.apcgv[0]->pos, cge.apcgv[1]->pos);
        for (CGT& cgt : pcg->acgt)
        {
            for (int icgv = 0; icgv < 3; ++icgv)
            {
                CGV* pcgv0 = cgt.apcgv[icgv];
                CGV* pcgv1 = cgt.apcgv[(icgv + 1) % 3];
                if (pcgv0 == cge.apcgv[0] && pcgv1 == cge.apcgv[1]) cge.apcgt[0] = &cgt;
                else if (pcgv0 == cge.apcgv[1] && pcgv1 == cge.apcgv[0]) cge.apcgt[1] = &cgt;
            }
        }
    }
}

int CposFindPathzonePath(PATHZONE* ppathzone, glm::vec3* pposStart, glm::vec3* pposEnd, int cposMax, glm::vec3* apos)
{
    return CposFindPath(&ppathzone->cg, pposStart, pposEnd, cposMax, apos);
}

void FindPathzoneClosestPoint(PATHZONE* ppathzone, glm::vec3* ppos, glm::vec3* pposClosest)
{
    FindClosestPointInCg(&ppathzone->cg, ppos, pposClosest);
}

int ClipPathzoneToBsp(PATHZONE* ppathzone, const glm::vec3* pposStart, const glm::vec3* pposEnd, glm::vec3* pposClip, float* pu)
{
    LSG lsg{};

    const int clsg = ClsgClipEdgeToCbsp(ppathzone->cg.acbsp.data(), pposStart, pposEnd, 1, &lsg);

    if (clsg <= 0)
    {
        if (pposClip != nullptr)
            *pposClip = *pposStart;

        if (pu != nullptr)
            *pu = 0.0f;

        return 0;
    }

    if (pposClip != nullptr)
        *pposClip = lsg.apos[1];

    if (pu != nullptr)
        *pu = lsg.au[1];

    return clsg;
}

void ClipPathzoneDirection(float s, PATHZONE* ppathzone, const glm::vec3* pposStart, glm::vec3* pdir)
{
    if (ppathzone == nullptr || pposStart == nullptr || pdir == nullptr)
        return;

    if (s == 0.0f)
    {
        *pdir = glm::vec3(0.0f);
        return;
    }

    const glm::vec3 dirOriginal = *pdir;

    glm::vec3 posBest = *pposStart;
    float uBest = -1.0f;

    for (int irad = 0; irad < 11; ++irad)
    {
        const glm::mat4 matRotate = glm::rotate(glm::mat4(1.0f), s_arad[irad], glm::vec3(0.0f, 0.0f, 1.0f));
        const glm::vec3 dirRotated = glm::mat3(matRotate) * dirOriginal;
        const glm::vec3 posEnd = *pposStart + dirRotated * s;

        glm::vec3 posClip{};
        float uClip = 0.0f;

        const int clsg = ClipPathzoneToBsp(ppathzone, pposStart, &posEnd, &posClip, &uClip);

        if (clsg > 0 && uClip > uBest)
        {
            posBest = posClip;
            uBest = uClip;

            if (uBest >= 1.0f)
                break;
        }
    }

    if (uBest < 0.0f)
    {
        *pdir = glm::vec3(0.0f);
        return;
    }

    *pdir = (posBest - *pposStart) / s;
}

void ChoosePathzoneRandomPoint(PATHZONE* ppathzone, glm::vec3* ppos)
{
    float totalArea2 = 0.0f;

    for (int icgt = 0; icgt < ppathzone->cg.ccgt; ++icgt) {
        CGT* pcgt = &ppathzone->cg.acgt[icgt];

        glm::vec3 dpos1 = pcgt->apcgv[1]->pos - pcgt->apcgv[0]->pos;
        glm::vec3 dpos2 = pcgt->apcgv[2]->pos - pcgt->apcgv[0]->pos;

        totalArea2 += dpos1.x * dpos2.y - dpos1.y * dpos2.x;
    }

    float areaPick = GRandInRange(0.0f, totalArea2);

    CGT* pcgtPick = ppathzone->cg.acgt.data();
    for (int icgt = 0; icgt < ppathzone->cg.ccgt; ++icgt) {
        CGT* pcgt = &ppathzone->cg.acgt[icgt];

        glm::vec3 dpos1 = pcgt->apcgv[1]->pos - pcgt->apcgv[0]->pos;
        glm::vec3 dpos2 = pcgt->apcgv[2]->pos - pcgt->apcgv[0]->pos;

        areaPick -= dpos1.x * dpos2.y - dpos1.y * dpos2.x;

        pcgtPick = pcgt;
        if (areaPick < 0.0f) {
            break;
        }
    }

    float u = GRandInRange(0.0f, 1.0f);
    float v = GRandInRange(0.0f, 1.0f);

    if (u + v > 1.0f) {
        u = 1.0f - u;
        v = 1.0f - v;
    }

    float w = 1.0f - u - v;

    *ppos =
        pcgtPick->apcgv[0]->pos * u +
        pcgtPick->apcgv[1]->pos * v +
        pcgtPick->apcgv[2]->pos * w;
}

void OnPathzoneAdd(PATHZONE* ppathzone)
{
    OnLoAdd(ppathzone);
    AppendDlEntry(&ppathzone->psw->dlPathzone, ppathzone);
}

void OnPathzoneRemove(PATHZONE* ppathzone)
{
    OnLoRemove(ppathzone);
    RemoveDlEntry(&ppathzone->psw->dlPathzone, ppathzone);
}

int GetPathzoneSize()
{
    return sizeof(PATHZONE);
}

void DeletePathzone(PATHZONE* ppathzone)
{
    delete ppathzone;
}

float s_arad[11] =
{
     0.0f,  0.5f, -0.5f,
     1.0f, -1.0f,
     1.5f, -1.5f,
     2.0f, -2.0f,
     2.5f, -2.5f
};

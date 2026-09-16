#include "geom.h"

void InitGeom(GEOM* pgeom)
{
	pgeom->aedge.clear();
	pgeom->cpos = 0;
	pgeom->apos.clear();
	pgeom->csurf = 0;
	pgeom->asurf.clear();
	pgeom->cedge = 0;
}

void ReadGeom(GEOM *pgeom, CBinaryInputStream *pbis)
{
    pgeom->sRadius = pbis->F32Read();

    pgeom->cpos = pbis->U16Read();
    pgeom->apos.resize(pgeom->cpos);

    for (int i = 0; i < pgeom->cpos; ++i)
        pgeom->apos[i] = pbis->ReadVector();

    pgeom->csurf = pbis->U16Read();

    pgeom->asurf.assign(pgeom->csurf, SURF{});
    pgeom->mpisurfposCenter.resize(pgeom->csurf);
    pgeom->mpisurfsRadius.resize(pgeom->csurf);

    pgeom->cedge = pbis->U16Read();
    pgeom->aedge.assign(pgeom->cedge, EDGE{});

    pgeom->indices.clear();

    int edgeIndex = 0;

    for (int isurf = 0; isurf < pgeom->csurf; ++isurf)
    {
        SURF& surf = pgeom->asurf[isurf];

        int ipos0 = static_cast<int16_t>(pbis->U16Read());
        int ipos1 = static_cast<int16_t>(pbis->U16Read());
        int ipos2 = static_cast<int16_t>(pbis->U16Read());

        if (ipos0 < 0 || ipos0 >= pgeom->cpos ||
            ipos1 < 0 || ipos1 >= pgeom->cpos ||
            ipos2 < 0 || ipos2 >= pgeom->cpos)
        {
            __debugbreak();
            return;
        }

        glm::vec3& p0 = pgeom->apos[ipos0];
        glm::vec3& p1 = pgeom->apos[ipos1];
        glm::vec3& p2 = pgeom->apos[ipos2];

        glm::vec3 dpos0 = p1 - p0;
        glm::vec3 dpos1 = p2 - p0;

        surf.normal = glm::normalize(glm::cross(dpos0, dpos1));
        surf.ipos = static_cast<int16_t>(ipos0);
        surf.gDot = glm::dot(surf.normal, p0);

        pgeom->mpisurfsRadius[isurf] = pbis->F32Read();
        pgeom->mpisurfposCenter[isurf] = pbis->ReadVector();

        int cedgeSurf = static_cast<int8_t>(pbis->U8Read());

        EDGE* prevEdge = nullptr;

        for (int iedge = 0; iedge < cedgeSurf; ++iedge)
        {
            if (edgeIndex >= pgeom->cedge)
            {
                __debugbreak();
                return;
            }

            EDGE& edge = pgeom->aedge[edgeIndex];

            int iposEdge0 = static_cast<int16_t>(pbis->U16Read());
            int iposEdge1 = static_cast<int16_t>(pbis->U16Read());
            int isurfOther = static_cast<int16_t>(pbis->U16Read());

            if (iposEdge0 < 0 || iposEdge0 >= pgeom->cpos ||
                iposEdge1 < 0 || iposEdge1 >= pgeom->cpos ||
                isurfOther < 0 || isurfOther >= pgeom->csurf)
            {
                __debugbreak();
                return;
            }

            edge.aipos[0] = static_cast<int16_t>(iposEdge0);
            edge.aisurf[0] = static_cast<int16_t>(isurf);
            edge.aipos[1] = static_cast<int16_t>(iposEdge1);
            edge.aisurf[1] = static_cast<int16_t>(isurfOther);

            if (iedge == 0)
                surf.pedge = &edge;
            else
                prevEdge->pedgeNext = &edge;

            SURF& otherSurf = pgeom->asurf[isurfOther];

            edge.pedgeOtherNext = otherSurf.pedgeOther;
            otherSurf.pedgeOther = &edge;

            prevEdge = &edge;
            ++edgeIndex;

            pgeom->indices.push_back(static_cast<uint16_t>(edge.aipos[0]));
            pgeom->indices.push_back(static_cast<uint16_t>(edge.aipos[1]));
        }
    }

    if (!pgeom->apos.empty())
    {
        glGenVertexArrays(1, &pgeom->VAO);
        glBindVertexArray(pgeom->VAO);

        glGenBuffers(1, &pgeom->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, pgeom->VBO);
        glBufferData(GL_ARRAY_BUFFER, pgeom->apos.size() * sizeof(glm::vec3), pgeom->apos.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &pgeom->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pgeom->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pgeom->indices.size() * sizeof(uint16_t), pgeom->indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
}

void CloneGeom(GEOM* pgeomSrc, glm::mat4* pdmat, GEOM* pgeomDst)
{
    pgeomDst->sRadius = pgeomSrc->sRadius;

    pgeomDst->cpos = pgeomSrc->cpos;
    pgeomDst->csurf = pgeomSrc->csurf;
    pgeomDst->cedge = pgeomSrc->cedge;

    pgeomDst->apos = pgeomSrc->apos;
    pgeomDst->asurf = pgeomSrc->asurf;
    pgeomDst->mpisurfposCenter = pgeomSrc->mpisurfposCenter;
    pgeomDst->mpisurfsRadius = pgeomSrc->mpisurfsRadius;
    pgeomDst->aedge = pgeomSrc->aedge;
    pgeomDst->indices = pgeomSrc->indices;

    RebaseGeomPointers(pgeomSrc, pgeomDst);

    pgeomDst->VAO = 0;
    pgeomDst->VBO = 0;
    pgeomDst->EBO = 0;

    if (pdmat != nullptr)
    {
        for (glm::vec3& pos : pgeomDst->apos)
            pos = glm::vec3((*pdmat) * glm::vec4(pos, 1.0f));

        for (glm::vec3& center : pgeomDst->mpisurfposCenter)
            center = glm::vec3((*pdmat) * glm::vec4(center, 1.0f));

        for (size_t i = 0; i < pgeomDst->asurf.size(); ++i)
        {
            // Retail CloneGeom transforms surface normals as vectors (w = 0)
            // and does not normalize the result before rebuilding gDot.
            glm::vec3 normal = glm::vec3((*pdmat) * glm::vec4(pgeomDst->asurf[i].normal, 0.0f));

            pgeomDst->asurf[i].normal = normal;

            int ipos = static_cast<int16_t>(pgeomDst->asurf[i].ipos);

            if (ipos >= 0 && ipos < static_cast<int>(pgeomDst->apos.size()))
                pgeomDst->asurf[i].gDot = glm::dot(normal, pgeomDst->apos[ipos]);
        }
    }

    if (g_fDebugMode > 0)
    {
        if (pgeomDst->cpos != 0)
        {
            glGenVertexArrays(1, &pgeomDst->VAO);
            glBindVertexArray(pgeomDst->VAO);

            glGenBuffers(1, &pgeomDst->VBO);
            glBindBuffer(GL_ARRAY_BUFFER, pgeomDst->VBO);
            glBufferData(GL_ARRAY_BUFFER, pgeomDst->apos.size() * sizeof(glm::vec3), pgeomDst->apos.data(), GL_STATIC_DRAW);

            glGenBuffers(1, &pgeomDst->EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pgeomDst->EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, pgeomDst->indices.size() * sizeof(uint16_t), pgeomDst->indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }
    }
}

static EDGE* RebaseEdgePtr(const GEOM* src, GEOM* dst, EDGE* edge)
{
    if (edge == nullptr)
        return nullptr;

    EDGE* srcBase = const_cast<EDGE*>(src->aedge.data());
    EDGE* srcEnd = srcBase + src->aedge.size();

    if (edge < srcBase || edge >= srcEnd)
        return nullptr;

    ptrdiff_t iedge = edge - srcBase;
    return &dst->aedge[iedge];
}

static SURF* RebaseSurfPtr(const GEOM* src, GEOM* dst, SURF* surf)
{
    if (surf == nullptr)
        return nullptr;

    SURF* srcBase = const_cast<SURF*>(src->asurf.data());
    SURF* srcEnd = srcBase + src->asurf.size();

    if (surf < srcBase || surf >= srcEnd)
        return nullptr;

    ptrdiff_t isurf = surf - srcBase;
    return &dst->asurf[isurf];
}

static void RebaseGeomPointers(const GEOM* src, GEOM* dst)
{
    for (SURF& surf : dst->asurf)
    {
        surf.pedge = RebaseEdgePtr(src, dst, surf.pedge);
        surf.pedgeOther = RebaseEdgePtr(src, dst, surf.pedgeOther);
    }

    for (EDGE& edge : dst->aedge)
    {
        edge.pedgeNext = RebaseEdgePtr(src, dst, edge.pedgeNext);
        edge.pedgeOtherNext = RebaseEdgePtr(src, dst, edge.pedgeOtherNext);
    }
}

void UpdateGeomWorld(GEOM* pgeomLocal, GEOM* pgeomWorld, glm::vec3& pos, glm::mat3& mat)
{
    // The original guard compared the raw position-buffer pointers.  GEOM owns
    // its arrays with std::vector here, so comparing vectors compares their
    // contents and incorrectly skips the first (and often every) world update.
    // Fixed physics is already excluded by UpdateSoXfWorldHierarchy.
    if (pgeomLocal == pgeomWorld)
        return;

    if (pgeomWorld->apos.size() < pgeomLocal->apos.size())
        pgeomWorld->apos.resize(pgeomLocal->apos.size());

    if (pgeomWorld->asurf.size() < pgeomLocal->asurf.size())
        pgeomWorld->asurf.resize(pgeomLocal->asurf.size());

    if (pgeomWorld->mpisurfposCenter.size() < pgeomLocal->asurf.size())
        pgeomWorld->mpisurfposCenter.resize(pgeomLocal->asurf.size());

    // Transform vertex positions: worldPos = mat * localPos + pos
    for (size_t i = 0; i < pgeomLocal->apos.size(); ++i)
    {
        const glm::vec3 localPos = pgeomLocal->apos[i];
        pgeomWorld->apos[i] = mat * localPos + pos;
    }

    // Transform surface normals and surface centers
    for (size_t i = 0; i < pgeomLocal->asurf.size(); ++i)
    {
        SURF &localSurf = pgeomLocal->asurf[i];
        SURF &worldSurf = pgeomWorld->asurf[i];

        // Normals get rotated only, no translation.
        glm::vec3 worldNormal = mat * localSurf.normal;

        worldSurf.normal = worldNormal;

        // gDot appears to be dot(normal, world position at surf ipos)
        const size_t posIndex = static_cast<unsigned short>(localSurf.ipos);

        if (posIndex < pgeomWorld->apos.size())
            worldSurf.gDot = glm::dot(worldNormal, pgeomWorld->apos[posIndex]);

        // Transform surface center position
        glm::vec3 localCenter(0.0f);

        if (i < pgeomLocal->mpisurfposCenter.size())
            localCenter = pgeomLocal->mpisurfposCenter[i];
        else if (posIndex < pgeomLocal->apos.size())
            localCenter = pgeomLocal->apos[posIndex];

        pgeomWorld->mpisurfposCenter[i] = mat * localCenter + pos;
    }
}

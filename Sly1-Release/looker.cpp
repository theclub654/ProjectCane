#include "looker.h"
#include "alo.h"
#include "pnt.h"
#include "render.h"
#include "util.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace
{
    glm::vec3 LookerVertexPosition(const VERTICE& vertex, const GLOB& glob,
        const GLOBSET& globset, size_t vertexIndex)
    {
        glm::vec4 position(vertex.pos, 1.0f);

        if (glob.poseCount > 0 && !glob.poseDpos.empty())
        {
            const size_t poseBase = vertexIndex * static_cast<size_t>(glob.poseCount);
            const int poseCount = std::min(glob.poseCount, globset.cpose);

            for (int ipose = 0; ipose < poseCount; ++ipose)
            {
                const size_t poseIndex = poseBase + static_cast<size_t>(ipose);

                if (poseIndex >= glob.poseDpos.size() || ipose >= static_cast<int>(globset.agPoses.size()))
                    break;

                position += glob.poseDpos[poseIndex] * globset.agPoses[ipose];
            }
        }

        glm::mat4 skin(0.0f);
        float totalWeight = 0.0f;

        for (int influence = 0; influence < 4; ++influence)
        {
            const float weight = vertex.boneWeights[influence];
            const uint32_t bone = vertex.boneIndices[influence];

            if (weight != 0.0f && bone < globset.boneMatrices.size())
            {
                skin += globset.boneMatrices[bone] * weight;
                totalWeight += weight;
            }
        }

        if (totalWeight > 0.0f)
            position = skin * position;

        return glm::vec3(position);
    }

    bool IntersectLookerTriangle(const glm::vec3& origin, const glm::vec3& segment,
        const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
        float* pt, glm::vec3* pbarycentric)
    {
        constexpr float epsilon = 1.0e-6f;
        const glm::vec3 edge1 = p1 - p0;
        const glm::vec3 edge2 = p2 - p0;
        const glm::vec3 p = glm::cross(segment, edge2);
        const float det = glm::dot(edge1, p);

        if (std::abs(det) <= epsilon)
            return false;

        const float inverseDet = 1.0f / det;
        const glm::vec3 fromP0 = origin - p0;
        const float u = glm::dot(fromP0, p) * inverseDet;

        if (u < 0.0f || u > 1.0f)
            return false;

        const glm::vec3 q = glm::cross(fromP0, edge1);
        const float v = glm::dot(segment, q) * inverseDet;

        if (v < 0.0f || u + v > 1.0f)
            return false;

        const float t = glm::dot(edge2, q) * inverseDet;

        if (t < 0.0f || t > 1.0f)
            return false;

        *pt = t;
        *pbarycentric = glm::vec3(1.0f - u - v, u, v);
        return true;
    }
}

std::shared_ptr<LOOKER> NewLooker()
{
    return std::make_shared<LOOKER>();
}

void LoadLookerFromBrx(LOOKER* plooker, CBinaryInputStream* pbis)
{
    float uCenter = pbis->F32Read();
    float vCenter = pbis->F32Read();
    float uMin = pbis->F32Read();
    float uMax = pbis->F32Read();
    float vMin = pbis->F32Read();
    float vMax = pbis->F32Read();

    plooker->uCenter = uCenter;
    plooker->vCenter = vCenter;

    plooker->duMin = uMin - uCenter;
    plooker->duMax = uMax - uCenter;

    plooker->dvMin = vMin - vCenter;
    plooker->dvMax = vMax - vCenter;
}

void InitLooker(LOOKER* plooker, SAAF* psaaf)
{
    InitSaa((SAA*)plooker, psaaf);

    plooker->sai.grfsai = (plooker->sai.grfsai & 0xfffffffe) | 2;
}

void NotifyLookerRender(LOOKER* plooker, ALO* palo, RPL* prpl)
{
    if (plooker == nullptr || palo == nullptr || prpl == nullptr || prpl->pglob == nullptr ||
        plooker->sai.pshd == nullptr || prpl->PFNDRAWRPL != DrawGlob || palo->palox == nullptr)
        return;

    // LOOKER is one of the original ALOX overlays.  These are the slots used
    // by the retail routine even though the decompiler names them IKJ/SCJ.
    PNT* ppntTarget = palo->palox->looker.ppntTarget;
    PNT* ppntFocus = palo->palox->looker.ppntFocus;

    if (ppntTarget == nullptr)
        ppntTarget = reinterpret_cast<PNT*>(palo->palox->ikj.paloIkh);

    if (ppntFocus == nullptr)
        ppntFocus = reinterpret_cast<PNT*>(palo->palox->scj.paloSchRot);

    if (ppntTarget == nullptr || ppntFocus == nullptr ||
        ppntTarget->paloParent == nullptr || ppntFocus->paloParent == nullptr)
        return;

    glm::vec3 posTarget;
    glm::vec3 posFocus;
    ConvertAloPos(ppntTarget->paloParent, palo, &ppntTarget->posLocal, &posTarget);
    ConvertAloPos(ppntFocus->paloParent, palo, &ppntFocus->posLocal, &posFocus);

    const glm::vec3 segment = posTarget - posFocus;

    if (glm::dot(segment, segment) <= 1.0e-8f)
        return;

    GLOB& glob = *prpl->pglob;
    GLOBSET& globset = palo->globset;
    float nearestT = FLT_MAX;
    glm::vec2 uvHit(0.0f);
    bool found = false;
    size_t vertexBase = 0;

    for (const SUBGLOB& subglob : glob.asubglob)
    {
        for (const INDICE& triangle : subglob.indices)
        {
            if (triangle.v1 >= subglob.vertices.size() || triangle.v2 >= subglob.vertices.size() ||
                triangle.v3 >= subglob.vertices.size())
                continue;

            const VERTICE& v0 = subglob.vertices[triangle.v1];
            const VERTICE& v1 = subglob.vertices[triangle.v2];
            const VERTICE& v2 = subglob.vertices[triangle.v3];
            const glm::vec3 p0 = LookerVertexPosition(v0, glob, globset, vertexBase + triangle.v1);
            const glm::vec3 p1 = LookerVertexPosition(v1, glob, globset, vertexBase + triangle.v2);
            const glm::vec3 p2 = LookerVertexPosition(v2, glob, globset, vertexBase + triangle.v3);
            float t;
            glm::vec3 barycentric;

            if (IntersectLookerTriangle(posFocus, segment, p0, p1, p2, &t, &barycentric) && t < nearestT)
            {
                nearestT = t;
                uvHit = v0.uv * barycentric.x + v1.uv * barycentric.y + v2.uv * barycentric.z;
                found = true;
            }
        }

        vertexBase += subglob.vertices.size();
    }

    float duTarget = 0.0f;
    float dvTarget = 0.0f;

    if (found)
    {
        duTarget = glm::clamp((1.0f - uvHit.x) - plooker->uCenter, plooker->duMin, plooker->duMax);
        dvTarget = glm::clamp((1.0f - uvHit.y) - plooker->vCenter, plooker->dvMin, plooker->dvMax);
    }

    SMP smpEye{ 1.0f, 0.1f, 0.1f };
    const float du = GSmooth(plooker->sai.tcx.du, duTarget, g_clock.dt, &smpEye, nullptr);
    const float dv = GSmooth(plooker->sai.tcx.dv, dvTarget, g_clock.dt, &smpEye, nullptr);

    SetSaiDuDv(&plooker->sai, du, dv);
}

void DeleteLooker(LOOKER* plooker)
{
    delete plooker;
}

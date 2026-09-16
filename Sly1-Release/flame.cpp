#include "flame.h"

int FFilterFlameObjects(void* pv, SO* pso)
{
    EMITB* pemitb = static_cast<EMITB*>(pv);

    if (pso->fNoXpsSelf)
        return 0;

    return pso != static_cast<SO*>(pemitb->emito.paloReference);
}

void PostFlameEmit(FLAME* pflame, EMITB* pemitb)
{
    if (pflame == nullptr || pemitb == nullptr)
        return;

    const CLQ& emitScale = pemitb->emitp.emitrip.clqScale;

    // Find the maximum value reached by the emitter's quadratic scale curve.
    float maxEmitScale = glm::max(emitScale.g0, emitScale.g0 + emitScale.g1 + emitScale.g2);

    if (std::abs(emitScale.g2) > 0.0001f)
    {
        const float uVertex = -emitScale.g1 / (2.0f * emitScale.g2);
        const float vertexScale = emitScale.g0 + uVertex * (emitScale.g1 + uVertex * emitScale.g2);

        maxEmitScale = glm::max(maxEmitScale, vertexScale);
    }

    const float initialScale = pflame->clqScale.g0;

    if (glm::length2(pflame->v) < 0.0001f)
        pflame->v = g_normalZ;
    else
        pflame->v = glm::normalize(pflame->v);

    glm::vec3 posRayStart = pflame->pos;
    glm::vec3 posRayEnd = posRayStart + pflame->v * (initialScale * maxEmitScale * 100.0f);

    std::vector <SO*> apso;
    IntersectSwBoundingBox(g_psw, nullptr, &posRayStart, &posRayEnd, (PFNFILTER)FFilterFlameObjects, pemitb, apso);

    LSG lsg{};
    float lengthScale = initialScale;

    SO* psoHit = PsoHitTestLineObjects(1, &posRayStart, &posRayEnd, apso, &lsg);

    if (psoHit != nullptr)
        lengthScale *= lsg.au[0];

    // Copy the emitter scale curve and scale every coefficient by the
    // available collision distance.
    pflame->clqScale.g0 = emitScale.g0 * lengthScale;
    pflame->clqScale.g1 = emitScale.g1 * lengthScale;
    pflame->clqScale.g2 = emitScale.g2 * lengthScale;
    pflame->clqScale.gUnused = emitScale.gUnused * lengthScale;

    LoadLookAtMatrix(&pflame->v, &pflame->mat);

    // The direction is now stored in the orientation matrix.
    pflame->v = glm::vec3(0.0f);
}

VTFLAME g_vtflame;

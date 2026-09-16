#include "stuck.h"

void UpdateStuck(STUCK* pstuck, float dt)
{
    UpdateRip(pstuck, dt);

    if (pstuck->paloParent != nullptr && !FIsLoInWorld(pstuck->paloParent))
        RemoveRip(pstuck);
}

void RenderStuck(STUCK* pstuck, CM* pcm)
{
    if (pstuck == nullptr || pcm == nullptr)
        return;

    // Without a parent, RIP::pos and RIP::mat are already in world space.
    if (pstuck->paloParent == nullptr)
    {
        FRenderRipPosMat(pstuck, pcm, &pstuck->pos, &pstuck->mat);
        return;
    }

    glm::vec3 posWorld;
    glm::mat3 matWorld;

    ConvertAloPos(pstuck->paloParent, nullptr, &pstuck->pos, &posWorld);
    ConvertAloMat(pstuck->paloParent, nullptr, &pstuck->mat, &matWorld);
    FRenderRipPosMat(pstuck, pcm, &posWorld, &matWorld);
}

void CreateStuck(RIP* pripSrc, ALO* paloRender, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal, STUCK** ppstuck)
{
    if (ppstuck != nullptr)
        *ppstuck = nullptr;

    if (pripSrc == nullptr || paloRender == nullptr || psoOther == nullptr || ppos == nullptr || pnormal == nullptr)
        return;

    STUCK* pstuck = static_cast<STUCK*>(PripNewRipg(RIPT_Stuck, nullptr));

    if (pstuck == nullptr)
        return;

    glm::vec3 posLocal;
    ConvertAloPos(nullptr, psoOther, ppos, &posLocal);

    float u = 0.0f;

    if (pripSrc->dtLifetime > 0.0f)
        u = (g_clock.t - pripSrc->tCreated) / pripSrc->dtLifetime;

    const float sourceScale = pripSrc->clqScale.g0 + u * (pripSrc->clqScale.g1 + u * pripSrc->clqScale.g2);
    const float stuckScale = sourceScale * GRandInRange(0.8f, 1.4f);

    pstuck->pvtrip->pfnInitRip(pstuck, &posLocal, stuckScale, nullptr);

    pstuck->paloRender = paloRender;
    pstuck->paloParent = static_cast<ALO*>(psoOther);
    pstuck->dtLifetime = 5.0f;

    // InitRip sets g0 to stuckScale. This produces:
    // scale(u) = stuckScale - stuckScale * u².
    pstuck->clqScale.g2 = -stuckScale;

    glm::mat3 matNormal;
    GetNormalVectors(pnormal, &matNormal[0], &matNormal[1], &g_normalX);
    matNormal[2] = *pnormal;

    glm::vec3 eul(0.0f, 0.0f, GRandInRange(0.0f, glm::two_pi<float>()));

    glm::mat3 matEul;
    LoadRotateMatrixEuler(&eul, &matEul);

    pstuck->mat = matNormal * matEul;

    // Give objects stuck to mostly vertical surfaces a downward velocity.
    if (std::abs(pnormal->z) < 0.7f)
    {
        const glm::vec3 normalNoZ(pnormal->x, pnormal->y, 0.0f);

        glm::mat3 matSurface;
        BuildRotateVectorsMatrix(&normalNoZ, pnormal, &matSurface);

        pstuck->v = matSurface * (g_normalZ * -50.0f);
    }

    if (ppstuck != nullptr)
        *ppstuck = pstuck;
}

VTSTUCK g_vtstuck;
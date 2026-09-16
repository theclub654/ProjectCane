#include "glint.h"

void InitGlint(GLINT* pglint, glm::vec3* ppos, float gScale, SO* psoTouch)
{
    InitRip(pglint, ppos, gScale, psoTouch);

    pglint->dtLifetime = 0.5f;
    pglint->paloRender = static_cast<ALO*>(g_psw->aploStock[18]);
    pglint->clqAlpha = s_clqGlintAlpha;
    pglint->clqScale = s_clqGlintScale;
}

void RenderGlint(GLINT* pglint, CM* pcm)
{
    if (pglint == nullptr || pcm == nullptr || pglint->paloRender == nullptr)
        return;

    glm::vec3 pos;
    ConvertAloPos(pglint->paloParent, nullptr, &pglint->pos, &pos);

    if (!SphereInFrustum(pcm->frustum, pos, pglint->paloRender->sRadiusRenderAll))
    {
        RemoveRip(pglint);
        return;
    }

    glm::vec3 dpos = pcm->pos - pos;
    const float distance = glm::length(dpos);
    const float uDistance = distance * 0.001f;

    glm::mat3 matCamera;
    BuildOrthonormalMatrixZ(dpos, g_normalZ, matCamera);

    const float uLifetime = pglint->dtLifetime > 0.0f ? (g_clock.t - pglint->tCreated) / pglint->dtLifetime : 0.0f;

    const float distanceScaleRaw = s_clqUDistToR.g0 + uDistance * (s_clqUDistToR.g1 + uDistance * s_clqUDistToR.g2);
    const float distanceScale = glm::clamp(distanceScaleRaw, s_lmR.gMin, s_lmR.gMax);

    const float lifetimeScale = pglint->clqScale.g0 + uLifetime * (pglint->clqScale.g1 + uLifetime * pglint->clqScale.g2);
    const float scale = lifetimeScale * distanceScale;

    const float lifetimeAlphaRaw = pglint->clqAlpha.g0 + uLifetime * (pglint->clqAlpha.g1 + uLifetime * pglint->clqAlpha.g2);
    const float lifetimeAlpha = glm::clamp(lifetimeAlphaRaw, 0.0f, 1.0f);

    const float distanceAlphaRaw = s_clqUDistToUAlpha.g0 + uDistance * (s_clqUDistToUAlpha.g1 + uDistance * s_clqUDistToUAlpha.g2);
    const float distanceAlpha = glm::clamp(distanceAlphaRaw, s_lmUAlpha.gMin, s_lmUAlpha.gMax);

    const float spin = s_clqUToRadSpin.g0 + uLifetime * (s_clqUToRadSpin.g1 + uLifetime * s_clqUToRadSpin.g2);

    glm::mat3 matSpin;
    LoadRotateMatrixRad(spin, &g_normalX, &matSpin);

    glm::mat3 mat = matCamera * matSpin;
    glm::vec3 vecScale(scale);

    RO ro{};
    ro.uAlpha = lifetimeAlpha * distanceAlpha;
    ro.uAlphaCelBorder = 1.0f;

    LoadMatrixFromPosRotScale(&pos, &mat, &vecScale, &ro.model);
    pglint->paloRender->pvtalo->pfnRenderAloGlobset(pglint->paloRender, pcm, &ro);
}

CLQ s_clqGlintScale{ 0.25, 3.0, -3.0 };
CLQ s_clqGlintAlpha{ 0.0, 4.0, -4.0 };
CLQ s_clqUDistToR{ 0.0, 0.75, 0.0 };
LM s_lmR{1.0, 5.0};
CLQ s_clqUDistToUAlpha{0.5, 0.0, 0.5};
LM s_lmUAlpha{0.5, 1.0};
CLQ s_clqUToRadSpin{ 0.0, 1.0, 0.0 };
VTGLINT g_vtglint;

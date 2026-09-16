#include "match.h"

void InitMatch(MATCH* pmatch, glm::vec3* ppos, float gScale, SO* psoTouch)
{
    InitRip(pmatch, ppos, gScale, psoTouch);

    pmatch->clqAlpha = CLQ{1.0, 0.0, 0.0};

    pmatch->clqScale.g0 = gScale;
    pmatch->clqScale.g1 = 1.0f - gScale;
    pmatch->clqScale.g2 = 0.0f;
    pmatch->clqScale.gUnused = 0.0f;
}

void LaunchMatch(MATCH* pmatch)
{
    if (pmatch == nullptr || pmatch->paloLaunch == nullptr || pmatch->paloRender == nullptr || pmatch->dtLifetime <= 0.0f)
        return;

    ALO* paloLaunch = pmatch->paloLaunch;
    const float dtArrival = pmatch->dtLifetime;

    pmatch->pos = paloLaunch->xf.posWorld;
    pmatch->mat = paloLaunch->xf.matWorld;

    glm::vec3 posTarget;
    glm::mat3 matTarget;

    PredictAloTransform(pmatch->paloRender, nullptr, dtArrival, &posTarget, &matTarget, nullptr, nullptr);

    // Initial ballistic velocity needed to reach the predicted target position.
    pmatch->v = (posTarget - pmatch->pos) / dtArrival - pmatch->emitdv.dv * (dtArrival * 0.5f);

    glm::mat3 dmat;
    CalculateDmat(&matTarget, &pmatch->mat, &dmat);
    DecomposeRotateMatrixRad(&dmat, &pmatch->radSmooth, &pmatch->normalSmooth);

    // Force the match to perform one additional full rotation while traveling.
    if (pmatch->radSmooth > 0.0f)
        pmatch->radSmooth += glm::two_pi<float>();
    else
        pmatch->radSmooth -= glm::two_pi<float>();

    paloLaunch->pvtlo->pfnRemoveLo(paloLaunch);
    pmatch->paloLaunch = nullptr;
}

void ProjectMatchTransform(MATCH* pmatch, float dt)
{
    if (pmatch == nullptr || pmatch->paloRender == nullptr || pmatch->dtLifetime <= 0.0f || g_clock.t < pmatch->tCreated)
        return;

    pmatch->posPrev = pmatch->pos;

    const float dtRemaining = pmatch->tCreated + pmatch->dtLifetime - g_clock.t;
    const float dtStep = glm::min(dt, dtRemaining);

    if (dtStep <= 0.0001f)
        return;

    glm::vec3 posTarget;
    glm::mat3 matTarget;

    PredictAloTransform(pmatch->paloRender, nullptr, dtStep, &posTarget, &matTarget, nullptr, nullptr);

    // Target velocity used by the Bezier integration.
    glm::vec3 vTarget = (posTarget - pmatch->pos) * (1.5f / dtStep) - pmatch->v * 0.5f + pmatch->emitdv.dv * (dtStep * 0.25f);

    glm::vec3 posNew;
    glm::vec3 vNew;

    EvaluateBezierPos(dtStep, dt, 1.0f, &pmatch->pos, &pmatch->v, &posTarget, &vTarget, &posNew, &vNew, nullptr);

    glm::mat3 matSpin;
    LoadRotateMatrixRad(pmatch->radSmooth * dtStep / pmatch->dtLifetime, &pmatch->normalSmooth, &matSpin);

    pmatch->pos = posNew;
    pmatch->v = vNew;
    pmatch->mat = matSpin * matTarget;
}

void UpdateMatch(MATCH* pmatch, float dt)
{
    if ((pmatch->tCreated <= g_clock.t) && (pmatch->paloLaunch != nullptr)) 
        LaunchMatch(pmatch);
    if (pmatch->dtLifetime < g_clock.t - pmatch->tCreated) {
        pmatch->paloRender->pvtlo->pfnAddLo(pmatch->paloRender);
        RemoveRip(pmatch);
    }
}

void RenderMatch(MATCH* pmatch, CM* pcm)
{
    if (pmatch->tCreated <= g_clock.t)
        RenderRip(pmatch, pcm);
}

VTMATCH g_vtmatch;
#include "spark.h"

void InitSpark(SPARK* pspark, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(pspark, ppos, scale, psoTouch);

    pspark->dtLifetime = 0.25f;
    pspark->dtTail = 0.05f;
    pspark->paloRender = static_cast<ALO*>(g_psw->aploStock[4]);
    pspark->clqAlpha = CLQ(1.0, 0.0, 0.0);
}

void RenderSpark(SPARK* pspark, CM* pcm)
{
    if (pspark == nullptr || pcm == nullptr)
        return;

    const float age = g_clock.t - pspark->tCreated;
    const float tailDuration = glm::min(age, pspark->dtTail);
    glm::vec3 posTail = pspark->pos - pspark->v * tailDuration;

    if (pspark->dtLifetime <= 0.0f)
    {
        RemoveRip(pspark);
        return;
    }

    const float lifetimeRatio = age / pspark->dtLifetime;

    const float alphaValue = pspark->clqAlpha.g0 + lifetimeRatio * (pspark->clqAlpha.g1 + lifetimeRatio * pspark->clqAlpha.g2);
    const float alpha = glm::clamp(alphaValue, 0.0f, 1.0f);

    const float scale = pspark->clqScale.g0 + lifetimeRatio * (pspark->clqScale.g1 + lifetimeRatio * pspark->clqScale.g2);

    if (alpha <= 0.0f || scale <= 0.0f)
    {
        RemoveRip(pspark);
        return;
    }

    RenderAloLine(pspark->paloRender, pcm, &posTail, &pspark->pos, scale, alpha);
}

VTSPARK g_vtspark;
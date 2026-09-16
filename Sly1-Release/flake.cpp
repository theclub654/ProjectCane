#include "flake.h"

void InitFlake(FLAKE* pflake, glm::vec3* ppos, float gScale, SO* psoTouch)
{
	InitRip(pflake, ppos, gScale, psoTouch);

	pflake->dtLifetime = 0.5;
	pflake->paloRender = (ALO*)g_psw->aploStock[6];
}

void UpdateFlake(FLAKE* pflake, float dt)
{
    if (pflake == nullptr)
        return;

    while (pflake->psoTouch != nullptr && !FIsLoInWorld(pflake->psoTouch))
        pflake->psoTouch = static_cast<SO*>(pflake->psoTouch->paloParent);

    if (g_clock.t - pflake->tCreated > 0.15f)
    {
        pflake->emitdv.rvDamping = -8.0f;
        pflake->emitdv.dv = g_psw->dvGravity * 0.5f;
    }

    UpdateRip(pflake, dt);
}

void RenderFlake(FLAKE* pflake, CM* pcm)
{
    if (pflake == nullptr || pcm == nullptr)
        return;

    RenderRip(pflake, pcm);

    const glm::vec3 movement = pflake->pos - pflake->posPrev;
    const float movementSquared = glm::dot(movement, movement);

    if (movementSquared <= 100.0f || pflake->dtLifetime <= 0.0f)
        return;

    const float lifetimeRatio = (g_clock.t - pflake->tCreated) / pflake->dtLifetime;
    const float alphaValue = pflake->clqAlpha.g0 + lifetimeRatio * (pflake->clqAlpha.g1 + lifetimeRatio * pflake->clqAlpha.g2);
    const float alpha = glm::clamp(alphaValue, 0.0f, 1.0f);

    RenderAloLine(static_cast<ALO*>(g_psw->aploStock[7]), pcm, &pflake->posPrev, &pflake->pos, 0.1f, alpha);
}

void TouchFlake(FLAKE* pflake, int fTouching)
{
	if (fTouching != 0)
		RemoveRip(pflake);
}

VTFLAKE g_vtflake;
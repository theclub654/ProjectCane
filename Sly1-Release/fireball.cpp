#include "fireball.h"

void InitFireball(FIREBALL* pfireball, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(pfireball, ppos, scale, psoTouch);

    pfireball->dtLifetime = 0.25f;
    pfireball->paloRender = static_cast<ALO*>(g_psw->aploStock[8]);

    pfireball->clqScale.g0 = g_clqFireballScale.g0 * scale;
    pfireball->clqScale.g1 = g_clqFireballScale.g1 * scale;
    pfireball->clqScale.g2 = g_clqFireballScale.g2 * scale;
    pfireball->clqScale.gUnused = g_clqFireballScale.gUnused * scale;

    pfireball->clqAlpha = g_clqFireballAlpha;
}

void UpdateFireball(FIREBALL* pfireball, float dt)
{
    UpdateRip(pfireball, dt);

    const float timeSinceSmokeSpawn = (g_clock.t - pfireball->tCreated) - 0.15f;

    if (timeSinceSmokeSpawn < 0.0f || timeSinceSmokeSpawn >= dt)
        return;

    const float smokeScale = -pfireball->clqScale.g2;
    RIP* psmoke = PripNewRipg(RIPT_SmokeCloud, nullptr);

    if (psmoke != nullptr && psmoke->pvtrip->pfnInitRip != nullptr)
        psmoke->pvtrip->pfnInitRip(psmoke, &pfireball->pos, smokeScale, nullptr);
}

CLQ g_clqFireballScale = { 0.0, 2.0, -1.0 };
CLQ g_clqFireballAlpha = { 4.0, -4.0, 0.0 };
VTFIREBALL g_vtfireball;

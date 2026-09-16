#include "burst.h"

void InitBurst(BURST* pburst, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(pburst, ppos, scale, psoTouch);

    pburst->dtLifetime = 0.5f;
    pburst->paloRender = static_cast<ALO*>(g_psw->aploStock[5]);

    pburst->clqScale.g0 = g_clqBurstScale.g0 * scale;
    pburst->clqScale.g1 = g_clqBurstScale.g1 * scale;
    pburst->clqScale.g2 = g_clqBurstScale.g2 * scale;
    pburst->clqScale.gUnused = g_clqBurstScale.gUnused * scale;
}

VTBURST g_vtburst;
CLQ g_clqBurstScale = {0.5, 2.0, -2.0, 0.0};
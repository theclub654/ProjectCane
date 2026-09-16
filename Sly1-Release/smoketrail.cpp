#include "smoketrail.h"

void InitSmoketrail(SMOKETRAIL* psmoketrail, glm::vec3* ppos, float scale, SO* psoTouch)
{
    if (psmoketrail == nullptr || ppos == nullptr)
        return;

    InitTrail(psmoketrail, ppos, scale, psoTouch);

    psmoketrail->paloRender = static_cast<ALO*>(g_psw->aploStock[10]);
    psmoketrail->emitdv.rvDamping = -15.0f;
    psmoketrail->clqAlpha = g_clqSmoketrailAlpha;

    psmoketrail->clqScale.g0 = g_clqSmoketrailScale.g0 * scale;
    psmoketrail->clqScale.g1 = g_clqSmoketrailScale.g1 * scale;
    psmoketrail->clqScale.g2 = g_clqSmoketrailScale.g2 * scale;
    psmoketrail->clqScale.gUnused = g_clqSmoketrailScale.gUnused * scale;
}

CLQ g_clqSmoketrailScale{1.0, 2.0, -3.0};
CLQ g_clqSmoketrailAlpha{0.75, -0.75, -0.0};
VTSMOKETRAIL g_vtsmoketrail;
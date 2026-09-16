#include "smokecloud.h"

void InitSmokecloud(SMOKECLOUD* psmokecloud, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(psmokecloud, ppos, scale, psoTouch);

    psmokecloud->dtLifetime = 3.0f;
    psmokecloud->paloRender = static_cast<ALO*>(g_psw->aploStock[9]);

    psmokecloud->clqScale.g1 = psmokecloud->clqScale.g0 * 0.25f;
    psmokecloud->clqAlpha = g_clqSmokecloudAlpha;

    psmokecloud->emitdv.rvDamping = -1.0f;
    psmokecloud->emitdv.dv = g_dvSmokecloud;
}

CLQ g_clqSmokecloudAlpha{ 0.69999999, 0.0, -0.69999999 };
glm::vec3 g_dvSmokecloud{ 0.0, 0.0, 50.0 };
VTSMOKECLOUD g_vtsmokecloud;
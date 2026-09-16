#include "debris.h"

void InitDebris(DEBRIS* pdebris, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(pdebris, ppos, scale, psoTouch);

    pdebris->dtLifetime = 0.5f;
    pdebris->paloRender = static_cast<ALO*>(g_psw->aploStock[1]);
    pdebris->emitdv.dv = g_psw->dvGravity;
}

VTDEBRIS g_vtdebris;
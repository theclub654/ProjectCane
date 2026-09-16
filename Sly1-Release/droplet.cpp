#include "droplet.h"

void InitDroplet(DROPLET* pdroplet, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(pdroplet, ppos, scale, psoTouch);

    pdroplet->dtLifetime = 5.0f;
    pdroplet->paloRender = static_cast<ALO*>(g_psw->aploStock[1]);
    pdroplet->emitdv.dv = g_psw->dvGravity;
}

void TouchDroplet(DROPLET* pdroplet, int fTouching)
{
    if (pdroplet == nullptr || fTouching == 0 || pdroplet->psoTouch == nullptr)
        return;

    SO* psoTouch = pdroplet->psoTouch;

    LSG alsg[16]{};
    alsg[0].apos[0] = pdroplet->pos;

    if (!psoTouch->bspc.absp.empty())
        ClsgClipEdgeToBsp(psoTouch->bspc.absp.data(), &pdroplet->posPrev, &pdroplet->pos, nullptr, 16, alsg);

    RIP* prip = PripNewRipg(RIPT_Ripple, nullptr);

    if (prip != nullptr)
    {
        if (prip->pvtrip->pfnInitRip != nullptr)
            prip->pvtrip->pfnInitRip(prip, &alsg[0].apos[0], 40.0f, nullptr);

        prip->dtLifetime = 0.75f;
        prip->clqScale.g1 = 30.0f;

        WRBG* pwrbg = psoTouch->globset.pwrbgFirst.get();

        if (pwrbg != nullptr)
            prip->pwr = pwrbg->pwr;
    }

    RemoveRip(pdroplet);
}

VTDROPLET g_vtdroplet;
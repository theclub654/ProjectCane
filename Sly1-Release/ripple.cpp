#include "ripple.h"

void InitRipple(RIPPLE* pripple, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(pripple, ppos, scale, psoTouch);

    const float clampedScale = glm::min(scale, 200.0f);

    pripple->clqScale.g0 = clampedScale;
    pripple->clqScale.g1 = clampedScale * 0.6f;
    pripple->paloRender = static_cast<ALO*>(g_psw->aploStock[2]);
}

void StockSplashSmall(glm::vec3* ppos, float scale, SO* psoTouch)
{
	RIP* const prip = PripNewRipg(RIPT_Ripple, nullptr);

	if (prip == nullptr)
		return;

	if (prip->pvtrip->pfnInitRip != nullptr)
		prip->pvtrip->pfnInitRip(prip, ppos, scale, nullptr);

	prip->dtLifetime = 0.75f;

	if (psoTouch != nullptr && psoTouch->globset.pwrbgFirst != nullptr)
		prip->pwr = psoTouch->globset.pwrbgFirst->pwr;
}


VTRIPPLE g_vtripple;
#include "dablet.h"

void InitDablet(DABLET* pdablet, glm::vec3* ppos, float gScale, SO* psoTouch)
{
	InitRip(pdablet, ppos, gScale, psoTouch);

	pdablet->dtLifetime = 2.0;
	pdablet->clqScale.g2 = -1.0;
	pdablet->emitdv.dv = g_psw->dvGravity;
}

VTDABLET g_vtdablet;
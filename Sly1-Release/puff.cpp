#include "puff.h"

void InitPuff(PUFF* ppuff, glm::vec3* ppos, float gScale, SO* psoTouch)
{
	InitRip(ppuff, ppos, gScale, psoTouch);

	ppuff->dtLifetime = 3.0;
	ppuff->clqScale.g1 = 0.6;
	ppuff->paloRender = (ALO*)g_psw->aploStock[3];
}


VTPUFF g_vtpuff;
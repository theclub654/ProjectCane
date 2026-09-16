#include "ray.h"

void InitRay(RAY* pray, glm::vec3* ppos, float gScale, SO* psoTouch)
{
	InitSpark(pray, ppos, gScale, psoTouch);

	pray->dtTail = 0.15;
	pray->paloRender = (ALO*)g_psw->aploStock[16];
}

VTRAY g_vtray;
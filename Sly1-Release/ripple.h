#pragma once
#include "rip.h"

struct RIPPLE : public RIP
{

};

void InitRipple(RIPPLE* pripple, glm::vec3* ppos, float scale, SO* psoTouch);
void StockSplashSmall(glm::vec3* ppos, float scale, SO* psoTouch);

struct VTRIPPLE
{
	void (*pfnInitRipple)(RIPPLE*, glm::vec3*, float, SO*) = InitRipple;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTRIPPLE g_vtripple;
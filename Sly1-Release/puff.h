#pragma once
#include "rip.h"

struct PUFF : RIP
{

};

void InitPuff(PUFF* ppuff, glm::vec3* ppos, float gScale, SO* psoTouch);

struct VTPUFF
{
    void (*pfnInitPuff)(PUFF*, glm::vec3*, float, SO*) = InitPuff;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTPUFF g_vtpuff;
#pragma once
#include "rip.h"

struct DEBRIS : public RIP
{

};

void InitDebris(DEBRIS* pdebris, glm::vec3* ppos, float scale, SO* psoTouch);

struct VTDEBRIS
{
    void (*pfnInitDebris)(DEBRIS*, glm::vec3*, float, SO*) = InitDebris;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTDEBRIS g_vtdebris;
#pragma once
#include "rip.h"

struct DABLET : public RIP
{

};

void InitDablet(DABLET* pdablet, glm::vec3* ppos, float gScale, SO* psoTouch);

struct VTDABLET
{
    void (*pfnInitDablet)(DABLET*, glm::vec3*, float, SO*) = InitDablet;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTDABLET g_vtdablet;
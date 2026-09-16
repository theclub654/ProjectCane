#pragma once
#include "rip.h"

struct BURST : public RIP
{

};

void InitBurst(BURST* pburst, glm::vec3* ppos, float scale, SO* psoTouch);

struct VTBURST
{
    void (*pfnInitBurst)(BURST*, glm::vec3*, float, SO*) = InitBurst;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTBURST g_vtburst;
extern CLQ g_clqBurstScale;
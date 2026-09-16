#pragma once
#include "rip.h"

struct SMOKECLOUD : public RIP
{

};

void InitSmokecloud(SMOKECLOUD* psmokecloud, glm::vec3* ppos, float scale, SO* psoTouch);

struct VTSMOKECLOUD
{
    void (*pfnInitSmokecloud)(SMOKECLOUD*, glm::vec3*, float, SO*) = InitSmokecloud;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern CLQ g_clqSmokecloudAlpha;
extern glm::vec3 g_dvSmokecloud;
extern VTSMOKECLOUD g_vtsmokecloud;
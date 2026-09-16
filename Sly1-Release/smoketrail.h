#pragma once
#include "trail.h"

struct SMOKETRAIL : public TRAIL
{

};

void InitSmoketrail(SMOKETRAIL* psmoketrail, glm::vec3* ppos, float scale, SO* psoTouch);

struct VTSMOKETRAIL
{
    void (*pfnInitTrail)(SMOKETRAIL*, glm::vec3*, float, SO*) = InitSmoketrail;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnTrailRemove)(TRAIL*) = OnTrailRemove;
    void (*pfnProjectTrailTransform)(TRAIL*, float) = ProjectTrailTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderTrail)(TRAIL*, CM*) = RenderTrail;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern CLQ g_clqSmoketrailScale;
extern CLQ g_clqSmoketrailAlpha;
extern VTSMOKETRAIL g_vtsmoketrail;
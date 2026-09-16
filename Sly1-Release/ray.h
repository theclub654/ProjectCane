#pragma once
#include "spark.h"

struct RAY : public SPARK
{

};

void InitRay(RAY* pray, glm::vec3* ppos, float gScale, SO* psoTouch);

struct VTRAY
{
    void (*pfnInitRay)(RAY*, glm::vec3*, float, SO*) = InitRay;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderSpark)(SPARK*, CM*) = RenderSpark;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTRAY g_vtray;
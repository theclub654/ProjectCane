#pragma once
#include "rip.h"

struct SPARK : public RIP
{
	float dtTail;
};

void InitSpark(SPARK* prip, glm::vec3* ppos, float scale, SO* psoTouch);
void RenderSpark(SPARK* prip, CM* pcm);

struct VTSPARK
{
    void (*pfnInitSpark)(SPARK*, glm::vec3*, float, SO*) = InitSpark;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderSpark)(SPARK*, CM*) = RenderSpark;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTSPARK g_vtspark;
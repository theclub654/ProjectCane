#pragma once
#include "rip.h"

struct FLAKE : public RIP
{

};

void InitFlake(FLAKE* pflake, glm::vec3* ppos, float gScale, SO* psoTouch);
void UpdateFlake(FLAKE* pflake, float dt);
void RenderFlake(FLAKE* pflake, CM* pcm);
void TouchFlake(FLAKE* pflake, int fTouching);

struct VTFLAKE
{
    void (*pfnInitFlake)(FLAKE*, glm::vec3*, float, SO*) = InitFlake;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateFlake)(FLAKE*, float) = UpdateFlake;
    void (*pfnRenderFlake)(FLAKE*, CM*) = RenderFlake;
    void (*pfnTouchFlake)(FLAKE*, int) = TouchFlake;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTFLAKE g_vtflake;
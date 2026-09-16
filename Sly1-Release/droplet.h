#pragma once
#include "rip.h"

struct DROPLET : public RIP
{

};

void InitDroplet(DROPLET* pdroplet, glm::vec3* ppos, float scale, SO* psoTouch);
void TouchDroplet(DROPLET* pdroplet, int fTouching);

struct VTDROPLET
{
    void (*pfnInitDroplet)(DROPLET*, glm::vec3*, float, SO*) = InitDroplet;
    void (*pfnPostRipEmit)(RIP*);
    void (*pfnOnRipRemove)(RIP*);
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchDroplet)(DROPLET*, int) = TouchDroplet;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTDROPLET g_vtdroplet;
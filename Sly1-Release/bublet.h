#pragma once
#include "droplet.h"

struct BUBLET : public DROPLET
{

};

void InitBublet(BUBLET* pbublet, glm::vec3* ppos, float scale, SO* psoTouch);
void ProjectBubletTransform(BUBLET* pbublet, float dt);
void TouchBublet(BUBLET* pbublet, int fTouching);

struct VTBUBLET
{
    void (*pfnInitBublet)(BUBLET*, glm::vec3*, float, SO*) = InitBublet;
    void (*pfnPostRipEmit)(RIP*);
    void (*pfnOnRipRemove)(RIP*);
    void (*pfnProjectBubletTransform)(BUBLET*, float) = ProjectBubletTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchBublet)(BUBLET*, int) = TouchBublet;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTBUBLET g_vtbublet;
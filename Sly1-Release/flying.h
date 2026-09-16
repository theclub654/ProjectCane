#pragma once
#include "rip.h"

struct FLYING : public RIP
{
	class ALO* paloStuck;
};

void PostFlyingEmit(FLYING* pflying, EMITB* pemitb);
void RenderFlying(FLYING* pflying, CM* pcm);
int FBounceFlying(FLYING* pflying, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal);

struct VTFLYING
{
    void (*pfnInitRip)(RIP*, glm::vec3*, float, SO*) = InitRip;
    void (*pfnPostFlyingEmit)(FLYING*, EMITB*) = PostFlyingEmit;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderFlying)(FLYING*, CM*) = RenderFlying;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceFlying)(FLYING*, SO*, glm::vec3*, glm::vec3*) = FBounceFlying;
};

extern VTFLYING g_vtflying;

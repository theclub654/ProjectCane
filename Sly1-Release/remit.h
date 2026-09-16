#pragma once
#include "rip.h"

struct REMIT : public RIP
{
	class EXPLS* pexpls;
	float svcParticle;
	float cParticle;
};

void PostRemitEmit(REMIT* premit, EMITB* pemitb);
void UpdateRemit(REMIT* premit, float dt);

struct VTREMIT
{
    void (*pfnInitRip)(RIP*, glm::vec3*, float, SO*) = InitRip;
    void (*pfnPostRemitEmit)(REMIT*, EMITB*) = PostRemitEmit;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRemit)(REMIT*, float) = UpdateRemit;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTREMIT g_vtremit;
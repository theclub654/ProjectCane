#pragma once
#include "rip.h"

struct FLAME : public RIP
{

};

int  FFilterFlameObjects(void* pv, SO* pso);
void PostFlameEmit(FLAME* pflame, EMITB* pemitb);

struct VTFLAME
{
    void (*pfnInitRip)(RIP*, glm::vec3*, float, SO*) = InitRip;
    void (*pfnPostFlameEmit)(FLAME*, EMITB*) = PostFlameEmit;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTFLAME g_vtflame;
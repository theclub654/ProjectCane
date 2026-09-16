#pragma once
#include "rip.h"

struct SHRAPNEL : public RIP
{
	float sRadius;
	float elas;
	float mu;
};

void PostShrapnelEmit(SHRAPNEL* pshrapnel, EMITB* pemitb);
int  FBounceShrapnel(SHRAPNEL* pshrapnel, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal);
void RenderShrapnel(SHRAPNEL* pshrapnel, CM* pcm);

struct VTSHRAPNEL
{
    void (*pfnInitRip)(RIP*, glm::vec3*, float, SO*) = InitRip;
    void (*pfnPostShrapnelEmit)(SHRAPNEL*, EMITB*) = PostShrapnelEmit;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderShrapnel)(SHRAPNEL*, CM*) = RenderShrapnel;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceShrapnel)(SHRAPNEL*, SO*, glm::vec3*, glm::vec3*) = FBounceShrapnel;
};

extern LM s_lmRdtLifetime;
extern CLQ s_clqSvToU;
extern VTSHRAPNEL g_vtshrapnel;
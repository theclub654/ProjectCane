#pragma once
#include "rip.h"

struct STUCK : public RIP
{
	class ALO* paloParent;
};

void UpdateStuck(STUCK* pstuck, float dt);
void RenderStuck(STUCK* pstuck, CM* pcm);

void CreateStuck(RIP* pripSrc, ALO* paloRender, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal, STUCK** ppstuck);

struct VTSTUCK
{
    void (*pfnInitRip)(RIP*, glm::vec3*, float, SO*) = InitRip;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateStuck)(STUCK*, float) = UpdateStuck;
    void (*pfnRenderStuck)(STUCK*, CM*) = RenderStuck;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTSTUCK g_vtstuck;
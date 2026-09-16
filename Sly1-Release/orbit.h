#pragma once
#include "rip.h"

struct ORBIT : public RIP
{
	class ALO* palo;
	int ctknv;
};

void InitOrbit(ORBIT* porbit, glm::vec3* ppos, float scale, SO* psoTouch);
void ProjectOrbitTransform(ORBIT* porbit, float dt);
void UpdateOrbit(ORBIT* porbit, float dt);

struct VTORBIT
{
    void (*pfnInitOrbit)(ORBIT*, glm::vec3*, float, SO*) = InitOrbit;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectOrbitTransform)(ORBIT*, float) = ProjectOrbitTransform;
    void (*pfnUpdateOrbit)(ORBIT*, float) = UpdateOrbit;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTORBIT g_vtorbit;
#pragma once
#include "rip.h"

struct FIREBALL : public RIP
{

};

void InitFireball(FIREBALL* pfireball, glm::vec3* ppos, float scale, SO* psoTouch);
void UpdateFireball(FIREBALL* pfireball, float dt);

struct VTFIREBALL
{
    void (*pfnInitRip)(FIREBALL*, glm::vec3*, float, SO*) = InitFireball;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateFireball)(FIREBALL*, float) = UpdateFireball;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern CLQ g_clqFireballScale;
extern CLQ g_clqFireballAlpha;
extern VTFIREBALL g_vtfireball;
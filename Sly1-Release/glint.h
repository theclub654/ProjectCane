#pragma once
#include "rip.h"

struct GLINT : public RIP
{
	class ALO* paloParent;
};

void InitGlint(GLINT* pglint, glm::vec3* ppos, float gScale, SO* psoTouch);
void RenderGlint(GLINT* pglint, CM* pcm);

struct VTGLINT
{
    void (*pfnInitGlint)(GLINT*, glm::vec3*, float, SO*) = InitGlint;
    void (*pfnPostRipEmit)(RIP*, EMITB*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderGlint)(GLINT*, CM*) = RenderGlint;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern CLQ s_clqGlintScale;
extern CLQ s_clqGlintAlpha;
extern CLQ s_clqUDistToR;
extern LM s_lmR;
extern CLQ s_clqUDistToUAlpha;
extern LM s_lmUAlpha;
extern CLQ s_clqUToRadSpin;

extern VTGLINT g_vtglint;
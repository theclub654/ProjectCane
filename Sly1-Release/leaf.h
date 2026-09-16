#pragma once
#include "rip.h"

struct LEAF : public RIP
{

};

void PostLeafEmit(LEAF* pleaf, EMITB* pemitb);
void ProjectLeafTransform(LEAF* pleaf, float dt);
int  FBounceLeaf(LEAF* pleaf, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal);

struct VTLEAF
{
    void (*pfnInitRip)(RIP*, glm::vec3*, float, SO*) = InitRip;
    void (*pfnPostRipEmit)(LEAF*, EMITB*) = PostLeafEmit;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectLeafTransform)(LEAF*, float) = ProjectLeafTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(RIP*, CM*) = RenderRip;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceLeaf)(LEAF*, SO*, glm::vec3*, glm::vec3*) = FBounceLeaf;
};

extern CLQ s_clqSvToSw;
extern LM s_lmSw;
extern float s_svLeafTarget;
extern float s_rvDampingX;
extern VTLEAF g_vtleaf;
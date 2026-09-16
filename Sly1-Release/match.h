#pragma once
#include "rip.h"

struct MATCH : public RIP
{
	glm::vec3 normalSmooth;
	float radSmooth;
	class ALO* paloLaunch;
};

void InitMatch(MATCH* pmatch, glm::vec3* ppos, float gScale, SO* psoTouch);
void LaunchMatch(MATCH* pmatch);
void ProjectMatchTransform(MATCH* pmatch, float dt);
void UpdateMatch(MATCH* pmatch, float dt);
void RenderMatch(MATCH* pmatch, CM* pcm);

struct VTMATCH
{
    void (*pfnInitMatch)(MATCH*, glm::vec3*, float, SO*) = InitMatch;
    void (*pfnPostRipEmit)(RIP*, EMITB*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectMatchTransform)(MATCH*, float) = ProjectMatchTransform;
    void (*pfnUpdateMatch)(MATCH*, float) = UpdateMatch;
    void (*pfnRenderMatch)(MATCH*, CM*) = RenderMatch;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern VTMATCH g_vtmatch;
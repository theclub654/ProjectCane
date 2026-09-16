#pragma once
#include "rip.h"

struct BULLET : public RIP
{
	int fRichochet;
	int fDamage;
};

void PostBulletEmit(BULLET* pbullet, EMITB* pemitb);
void RenderBullet(BULLET* pbullet, CM* pcm);
int  FBounceBullet(BULLET* pbullet, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal);

struct VTBULLET
{
    void (*pfnInitRip)(RIP*, glm::vec3*, float, SO*) = InitRip;
    void (*pfnPostBulletEmit)(BULLET*, EMITB*) = PostBulletEmit;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRipTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderRip)(BULLET*, CM*) = RenderBullet;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceBullet)(BULLET*, SO*, glm::vec3*, glm::vec3*) = FBounceBullet;
};

extern VTBULLET g_vtbullet;
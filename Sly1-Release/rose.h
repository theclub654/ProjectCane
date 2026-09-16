#pragma once
#include "rip.h"

enum ROSES 
{
    ROSES_Nil = -1,
    ROSES_Closing = 0,
    ROSES_Spinning = 1,
    ROSES_Opening = 2,
    ROSES_Max = 3
};

struct HP 
{
    // Original PS2 HP layout is 0x30 bytes. Both vectors are quadwords;
    // using vec3 here shifts every following field and corrupts the hull.
    glm::vec4 pos;
    glm::vec4 posHull;
    float radHull;
    float sEdge;
    float sCorner;
    int xhpKey;
};

static_assert(sizeof(HP) == 0x30);

using PFNROSE = int (*)(void*, int, HP*);

struct ROSE : public RIP
{
    ROSES roses;
    float uSpin;
    int c;
    int xhpKey;
    PFNROSE pfnrose;
    void* pv;
};

void InitRose(ROSE* prose, glm::vec3* ppos, float gScale, SO* psoTouch);
void ProjectRoseTransform(RIP* prip, float dt);
void UpdateRose(ROSE* prose, float dt);
void RenderRose(ROSE* prose, CM* pcm);
void SetRoseRoses(ROSE* prose, ROSES roses);

int SgnCmpHp(const void* pv0, const void* pv1);
int ChpBuildConvexHullScreen(const glm::vec3* pposCenter, int chp, HP* ahp);
int ChpBuildConvexHullXY(const glm::mat4* view, int chp, HP* ahp);

struct VTROSE
{
    void (*pfnInitRose)(ROSE*, glm::vec3*, float, SO*) = InitRose;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectRipTransform)(RIP*, float) = ProjectRoseTransform;
    void (*pfnUpdateRose)(ROSE*, float) = UpdateRose;
    void (*pfnRenderRose)(ROSE*, CM*) = RenderRose;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern CLQ s_clqUToDuOpening;
extern CLQ s_clqUToDuClosing;
extern CLQ s_clqUToSOpening;
extern CLQ s_clqUToSClosing;
extern CLQ s_clqSToUAlpha;
extern CLQ s_clqSToRScale;
extern LM s_lmRScale;
extern CLQ s_clqSToC;
extern LM s_lmC;
extern LM s_lmUAlphaRose;
extern VTROSE g_vtrose;

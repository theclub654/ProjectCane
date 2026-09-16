#pragma once
#include "rip.h"

enum TRLS 
{
    TRLS_Nil = -1,
    TRLS_None = 0,
    TRLS_TrackingRip = 1,
    TRLS_TrackingAlo = 2,
    TRLS_ChainingRip = 3,
    TRLS_Max = 4
};

struct TRAIL : public RIP
{
	TRLS trls;
	class RIP* pripTrack;
	class ALO* paloTrack;
};


void InitTrail(TRAIL* ptrail, glm::vec3* ppos, float scale, SO* psoTouch);
void OnTrailRemove(TRAIL* ptrail);
void SetTrailTrls(TRAIL* ptrail, TRLS trls, void* pv);
void DetachTrail(TRAIL* ptrail);
void ProjectTrailTransform(TRAIL* ptrail, float dt);
void RenderTrail(TRAIL* ptrail, CM* pcm);
void HandleTrailMessage(TRAIL* ptrail, MSGID msgid, void* pv);

struct VTTRAIL
{
    void (*pfnInitTrail)(TRAIL*, glm::vec3*, float, SO*) = InitTrail;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnTrailRemove)(TRAIL*) = OnTrailRemove;
    void (*pfnProjectTrailTransform)(TRAIL*, float) = ProjectTrailTransform;
    void (*pfnUpdateRip)(RIP*, float) = UpdateRip;
    void (*pfnRenderTrail)(TRAIL*, CM*) = RenderTrail;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern CLQ g_clqTrailScale;
extern VTTRAIL g_vttrail;
#pragma once
#include "xform.h"

class TARGET : public XFM
{
	public:
	DLE dleTarget;
	int grftak;
	float sRadiusTarget;
	int fHitTest;
};

void StartupTarget();
void ResetTargetList();

TARGET* NewTarget();
void InitTarget(TARGET* ptarget);
void*GetTargetGrftak(TARGET* ptarget);
void SetTargetGrftak(TARGET* ptarget, GRFTAK grftak);
void*GetTargetSRadiusTarget(TARGET* ptarget);
void SetTargetSRadiusTarget(TARGET* ptarget, float sRadiusTarget);
void*GetTargetFHitTest(TARGET* ptarget);
void SetTargetFHitTest(TARGET* ptarget, int fHitTest);
int  GetTargetSize();
void OnTargetAdd(TARGET* ptarget);
void OnTargetRemove(TARGET* ptarget);
void CloneTarget(TARGET* ptarget, TARGET* ptargetBase);
void DeleteTarget(TARGET* ptarget);

extern DL g_dlTarget;
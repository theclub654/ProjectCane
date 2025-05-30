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

TARGET* NewTarget();
void InitTarget(TARGET* ptarget);
int  GetTargetSize();
void OnTargetAdd(TARGET* ptarget);
void OnTargetRemove(TARGET* ptarget);
void CloneTarget(TARGET* ptarget, TARGET* ptargetBase);
void RenderTarget(TARGET* ptarget, CM* pcm);
TARGET* PtargetEnsureAlo(ALO* palo);
void SetAloTargetHitTest(ALO* palo, int fHitTest);
void DeleteTarget(TARGET* ptarget);
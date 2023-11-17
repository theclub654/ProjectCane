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

void InitTarget(TARGET* ptarget);
void OnTargetAdd(TARGET* ptarget);
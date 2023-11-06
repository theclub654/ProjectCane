#pragma once
#include "step.h"

class STEPGUARD : public STEP
{
	public:
};

class PUFFC : public STEPGUARD
{
	public:
};

class SGG : public LO
{
	public:
};

void InitSgg(SGG* psgg);
void InitStepGuard(STEPGUARD* pstepguard);
void OnStepguardAdd(STEPGUARD* pstepguard);
void LoadStepGuardFromBrx(STEPGUARD *pstepguard, CBinaryInputStream *pbis);
void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro);
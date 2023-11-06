#pragma once
#include "step.h"

class JT : public STEP
{
	public:

};

void InitJt(JT *pjt); // NOT FINSHED
void LoadJtFromBrx(JT *pjt, CBinaryInputStream *pbis); // NOT FINSHED
void RenderJtAll(JT* pjt, CM* pcm, RO* pro);
void RenderJtSelf(JT* pjt, CM* pcm, RO* pro);
static JT *g_pjt;
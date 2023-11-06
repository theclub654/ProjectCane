#pragma once
#include "step.h"

class JT : public STEP
{
	public:

};

void InitJt(JT *pjt); // NOT FINSHED
void LoadJtFromBrx(JT *pjt, CBinaryInputStream *pbis); // NOT FINSHED

static JT *g_pjt;
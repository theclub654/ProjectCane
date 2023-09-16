#pragma once
#include "lo.h"

class ASEG : public LO
{
	public:
};

static int LoadAsegaCount;
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);
void LoadAsegEventsFromBrx(CBinaryInputStream* pbis, int fFrame);
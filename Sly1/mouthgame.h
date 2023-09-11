#pragma once
#include "alo.h"

class MGC : public ALO
{
	public:
};

void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);

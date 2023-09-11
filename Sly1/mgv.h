#pragma once
#include "po.h"

class MGV :public PO
{
	public:
};

void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis);
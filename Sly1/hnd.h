#pragma once
#include "target.h"

class HND : public TARGET
{
	public:
};

void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);
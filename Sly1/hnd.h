#pragma once
#include "target.h"

class HND : public TARGET
{
	public:
};

void InitHnd(HND* phnd);
void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);
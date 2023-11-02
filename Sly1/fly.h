#pragma once
#include "so.h"

class FLY : public SO
{
	public:
};

void InitFly(FLY* pfly);
void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis);
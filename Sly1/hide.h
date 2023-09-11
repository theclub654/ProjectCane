#pragma once
#include "so.h"

class HBSK : public SO
{
	public:
};

void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);

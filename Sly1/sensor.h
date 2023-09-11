#pragma once
#include "so.h"

class SENSOR : public SO
{
	public:
};

class LASEN : public SENSOR
{
	public:
};

void LoadLasenFromBrx(LASEN *plasen, CBinaryInputStream *pbis);
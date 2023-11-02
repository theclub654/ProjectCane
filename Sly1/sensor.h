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

class CAMSEN
{
	public:
};

class PRSEN : public SENSOR
{
	public:
};

void InitSensor(SENSOR* psensor);
void InitLasen(LASEN* plasen);
void InitCamsen(CAMSEN* pcamsen);
void InitPrsen(PRSEN* pprsen);
void LoadLasenFromBrx(LASEN *plasen, CBinaryInputStream *pbis);

#include "sensor.h"

void InitSensor(SENSOR* psensor)
{
	InitSo(psensor);
}

void InitLasen(LASEN* plasen)
{
	InitSensor(plasen);
}

void InitCamsen(CAMSEN* pcamsen)
{
	InitSensor(pcamsen);
}

void InitPrsen(PRSEN* pprsen)
{
	InitSensor(pprsen);
}

void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(plasen, pbis);
}

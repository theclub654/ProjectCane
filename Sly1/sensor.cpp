#include "sensor.h"

void InitSensor(SENSOR* psensor)
{
	InitSo(psensor);
}

void DeleteSensor(LO* plo)
{
	delete (SENSOR*)plo;
}

void InitLasen(LASEN* plasen)
{
	InitSensor(plasen);
}

void DeleteLasen(LO* plo)
{
	delete(LASEN*)plo;
}

void InitCamsen(CAMSEN* pcamsen)
{
	InitSensor(pcamsen);
}

void DeleteCamsen(LO* plo)
{
	delete (CAMSEN*)plo;
}

void InitPrsen(PRSEN* pprsen)
{
	InitSensor(pprsen);
}

void DeletePrsen(LO* plo)
{
	delete (PRSEN*)plo;
}

void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(plasen, pbis);
}

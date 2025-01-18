#include "sensor.h"

SENSOR* NewSensor()
{
	return new SENSOR{};
}

void InitSensor(SENSOR* psensor)
{
	InitSo(psensor);
	psensor->sensm = SENSM_SenseOnly;
	psensor->sensors = SENSORS_Nil;
}

int GetSensorSize()
{
	return sizeof(SENSOR);
}

void CloneSensor(SENSOR* psensor, SENSOR* psensorBase)
{
	LO lo = *psensor;
	*psensor = *psensorBase;
	memcpy(psensor, &lo, sizeof(LO));

	CloneLo(psensor, psensorBase);

	ClearDl(&psensor->dlChild);

	psensor->pxa = nullptr;
	psensor->grfpvaXpValid = 0;
	psensor->pstso = nullptr;
}

void DeleteSensor(SENSOR *psensor)
{
	delete psensor;
}

LASEN* NewLasen()
{
	return new LASEN{};
}

void InitSwLasenDl(SW* psw)
{
	InitDl(&psw->dlBusyLasen, offsetof(LASEN, dleBusyLasen));
}

void InitLasen(LASEN* plasen)
{
	InitSensor(plasen);
	plasen->uDrawMax = 1.0;
}

int GetLasenSize()
{
	return sizeof(LASEN);
}

void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(plasen, pbis);
}

void CloneLasen(LASEN* plasen, LASEN* plasenBase)
{
	LO lo = *plasen;
	*plasen = *plasenBase;
	memcpy(plasen, &lo, sizeof(LO));

	CloneLo(plasen, plasenBase);

	ClearDl(&plasen->dlChild);

	plasen->pxa = nullptr;
	plasen->grfpvaXpValid = 0;
	plasen->pstso = nullptr;
}

void RenderLasenSelf(LASEN* plasen, CM* pcm, RO* pro)
{
	RenderSoSelf(plasen, pcm, pro);
}

void DeleteLasen(LASEN *plasen)
{
	delete plasen;
}

CAMSEN* NewCamsen()
{
	return new CAMSEN{};
}

void InitCamsen(CAMSEN* pcamsen)
{
	InitSensor(pcamsen);
	pcamsen->csdts = CSDTS_Nil;
}

int GetCamsenSize()
{
	return sizeof(CAMSEN);
}

void CloneCamsen(CAMSEN* pcamsen, CAMSEN* pcamsenBase)
{
	LO lo = *pcamsen;
	*pcamsen = *pcamsenBase;
	memcpy(pcamsen, &lo, sizeof(LO));

	CloneLo(pcamsen, pcamsenBase);

	ClearDl(&pcamsen->dlChild);

	pcamsen->pxa = nullptr;
	pcamsen->grfpvaXpValid = 0;
	pcamsen->pstso = nullptr;
}

void RenderCamsenSelf(CAMSEN* pcamsen, CM* pcm, RO* pro)
{
	pcamsen->pvtalo->pfnRenderAloGlobset(pcamsen, pcm, pro);
}

void DeleteCamsen(CAMSEN* pcamsen)
{
	delete pcamsen;
}

PRSEN* NewPrsen()
{
	return new PRSEN{};
}

void InitPrsen(PRSEN* pprsen)
{
	InitSensor(pprsen);
}

int GetPrsenSize()
{
	return sizeof(PRSEN);
}

void ClonePrsen(PRSEN* pprsen, PRSEN* pprsenBase)
{
	LO lo = *pprsen;
	*pprsen = *pprsenBase;
	memcpy(pprsen, &lo, sizeof(LO));

	CloneLo(pprsen, pprsenBase);

	ClearDl(&pprsen->dlChild);

	pprsen->pxa = nullptr;
	pprsen->grfpvaXpValid = 0;
	pprsen->pstso = nullptr;
}

void DeletePrsen(PRSEN *ppprsen)
{
	delete ppprsen;
}
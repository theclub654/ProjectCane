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
	CloneSo(psensor, psensorBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	psensor->tSensors = psensorBase->tSensors;
	psensor->fTriggerAll = psensorBase->fTriggerAll;
	psensor->dtEnabling = psensorBase->dtEnabling;
	psensor->dtDisabling = psensorBase->dtDisabling;
	psensor->fRemainDisabledIndefinite = psensorBase->fRemainDisabledIndefinite;
	psensor->svtRestore = psensorBase->svtRestore;

	// Shallow copy of pointer members
	psensor->palarm = psensorBase->palarm;
	psensor->pamb = psensorBase->pamb;
	psensor->pasegaPause = psensorBase->pasegaPause;

	// Shallow copy of arrays
	for (int i = 0; i < 4; ++i)
	{
		psensor->aoidTrigger[i] = psensorBase->aoidTrigger[i];
		psensor->aoidNoTrigger[i] = psensorBase->aoidNoTrigger[i];
		psensor->acidTrigger[i] = psensorBase->acidTrigger[i];
		psensor->acidNoTrigger[i] = psensorBase->acidNoTrigger[i];
	}

	// Shallow copy of other struct members
	psensor->sensm = psensorBase->sensm;
	psensor->sensors = psensorBase->sensors;
	psensor->sensorsInitial = psensorBase->sensorsInitial;
	psensor->coidTrigger = psensorBase->coidTrigger;
	psensor->coidNoTrigger = psensorBase->coidNoTrigger;
	psensor->ccidTrigger = psensorBase->ccidTrigger;
	psensor->ccidNoTrigger = psensorBase->ccidNoTrigger;
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
	SnipAloObjects(plasen, 2, s_asnipLasen);
}

void CloneLasen(LASEN* plasen, LASEN* plasenBase)
{
	CloneSensor(plasen, plasenBase); // Assuming CloneSensor handles copying the base class (SENSOR) members

	// Shallow copy of value members
	plasen->cposBeamShapeMax = plasenBase->cposBeamShapeMax;
	plasen->dtDamageDisabling = plasenBase->dtDamageDisabling;
	plasen->fBusyLasen = plasenBase->fBusyLasen;
	plasen->tSenseNext = plasenBase->tSenseNext;
	plasen->fJtOnlyTriggerObject = plasenBase->fJtOnlyTriggerObject;
	plasen->uDrawMax = plasenBase->uDrawMax;
	plasen->svuDrawMax = plasenBase->svuDrawMax;

	// Shallow copy of pointer members
	plasen->paloRenderSense = plasenBase->paloRenderSense;
	plasen->paloRenderDamage = plasenBase->paloRenderDamage;

	// Shallow copy of array members
	for (int i = 0; i < 16; ++i)
	{
		plasen->albeam[i] = plasenBase->albeam[i];
	}

	// Shallow copy of other struct members
	plasen->dleBusyLasen = plasenBase->dleBusyLasen;
}

void BindLasen(LASEN* plasen)
{
	BindAlo(plasen);
}

void PostLasenLoad(LASEN* plasen)
{
	PostAloLoad(plasen);

	for (int i = 0; i < plasen->clbeam; ++i) {
		LBEAM* pbeam = &plasen->albeam[i];

		// Remove the shape's LO
		if (pbeam->pshape && pbeam->pshape->pvtlo->pfnRemoveLo) {
			pbeam->pshape->pvtlo->pfnRemoveLo((LO*)pbeam);
		}
		
		if (i == 0) {
			// Skip cloning damage emitters for beam 0 (used as source template)
			continue;
		}

		int clemitDamage = plasen->albeam[0].clemitDamage;
		pbeam->clemitDamage = clemitDamage;

		if (clemitDamage > 0) {
			LEMIT *dstEmit = pbeam->alemitDamage;
			LEMIT *srcEmit = plasen->albeam[0].alemitDamage;

			for (int j = 0; j < clemitDamage; ++j) {
				EMITTER* pemitter = (EMITTER*)PloCloneLo((LO*)srcEmit->pemitter, plasen->psw, (ALO*)plasen);
				dstEmit->pemitter = pemitter;

				if (pemitter && pemitter->pvtalo->pfnBindAlo) {
					pemitter->pvtalo->pfnBindAlo(pemitter);
				}

				if (pemitter && pemitter->pvtlo->pfnPostLoLoad) {
					pemitter->pvtlo->pfnPostLoLoad(pemitter);
				}

				dstEmit->fScorch = srcEmit->fScorch;

				++dstEmit;
				++srcEmit;
			}
		}
	}

	// Set jtOnlyTriggerObject flag based on g_pjt
	//plasen->fJtOnlyTriggerObject = FOnlySensorTriggerObject(plasen, g_pjt);

	//// Initialize damage handling
	//if (plasen->pvtpo && plasen->pvtpo->pfnFTakePoDamage) {
	//	plasen->pvtpo->pfnFTakePoDamage(plasen, plasen->sensorsInitial);
	//}
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
	CloneSensor(pcamsen, pcamsenBase); // Assuming CloneSensor handles copying the base class (SENSOR) members

	// Shallow copy of value members
	pcamsen->tCsdts = pcamsenBase->tCsdts;
	pcamsen->dtDamageFocus = pcamsenBase->dtDamageFocus;
	pcamsen->dtDamageZap = pcamsenBase->dtDamageZap;
	pcamsen->dtDamageUnfocus = pcamsenBase->dtDamageUnfocus;

	// Shallow copy of pointer members
	pcamsen->paloRenderDamage = pcamsenBase->paloRenderDamage;
	pcamsen->paloRenderZap = pcamsenBase->paloRenderZap;

	// Shallow copy of struct member
	pcamsen->csdts = pcamsenBase->csdts;
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
	CloneSensor(pprsen, pprsenBase); // Assuming CloneSensor handles copying the base class (SENSOR) members

	// Shallow copy of value members
	pprsen->iframeSenseStart = pprsenBase->iframeSenseStart;
	pprsen->iframeSenseEnd = pprsenBase->iframeSenseEnd;
	pprsen->dtSenseAnim = pprsenBase->dtSenseAnim;
	pprsen->pssatSense = pprsenBase->pssatSense;
	pprsen->iframeDamageStart = pprsenBase->iframeDamageStart;
	pprsen->iframeDamageEnd = pprsenBase->iframeDamageEnd;
	pprsen->dtDamageAnim = pprsenBase->dtDamageAnim;
	pprsen->pssatDamage = pprsenBase->pssatDamage;
	pprsen->iframeDisabledStart = pprsenBase->iframeDisabledStart;
	pprsen->iframeDisabledEnd = pprsenBase->iframeDisabledEnd;
	pprsen->dtDisabledAnim = pprsenBase->dtDisabledAnim;
	pprsen->pssatDisabled = pprsenBase->pssatDisabled;
	pprsen->iframeDisablingFlash = pprsenBase->iframeDisablingFlash;
	pprsen->svtDisablingFlash = pprsenBase->svtDisablingFlash;
	pprsen->dtRemainEnabled = pprsenBase->dtRemainEnabled;
	pprsen->dtRemainDisabled = pprsenBase->dtRemainDisabled;
	pprsen->tSensePrev = pprsenBase->tSensePrev;
	pprsen->fTriggered = pprsenBase->fTriggered;

	// Shallow copy of pointer members
	pprsen->ploop = pprsenBase->ploop;
}

void DeletePrsen(PRSEN *ppprsen)
{
	delete ppprsen;
}

SNIP s_asnipLasen[2] =
{
	2, OID_laser_sensor_render, offsetof(LASEN, paloRenderSense),
	2, OID_laser_damage_render, offsetof(LASEN, paloRenderDamage)
};
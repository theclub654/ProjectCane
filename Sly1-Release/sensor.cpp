#include "sensor.h"
#include "emitter.h"
#include "shape.h"
#include "alarm.h"
#include "actla.h"
#include "loop.h"


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

void* GetSensorFTriggerAll(SENSOR* psensor)
{
	return &psensor->fTriggerAll;
}

void SetSensorFTriggerAll(SENSOR* psensor, int fTriggerAll)
{
	psensor->fTriggerAll = fTriggerAll;
}

void* GetSensorDtEnabling(SENSOR* psensor)
{
	return &psensor->dtEnabling;
}

void SetSensorDtEnabling(SENSOR* psensor, float dtEnabling)
{
	psensor->dtEnabling = dtEnabling;
}

void* GetSensorDtDisabling(SENSOR* psensor)
{
	return &psensor->dtDisabling;
}

void SetSensorDtDisabling(SENSOR* psensor, float dtDisabling)
{
	psensor->dtDisabling = dtDisabling;
}

void* GetSensorSensorsInitial(SENSOR* psensor)
{
	return &psensor->sensorsInitial;
}

void SetSensorSensorsInitial(SENSOR* psensor, SENSORS sensorsInitial)
{
	psensor->sensorsInitial = sensorsInitial;
}

void* GetSensorPalarm(SENSOR* psensor)
{
	return &psensor->palarm;
}

void* GetSensorSensors(SENSOR* psensor)
{
	return &psensor->sensors;
}

void* GetSensorSensm(SENSOR* psensor)
{
	return &psensor->sensm;
}

void* GetSensorFRemainDisabledIndefinite(SENSOR* psensor)
{
	return &psensor->fRemainDisabledIndefinite;
}

void SetSensorFRemainDisabledIndefinite(SENSOR* psensor, int fRemainDisabledIndefinite)
{
	psensor->fRemainDisabledIndefinite = fRemainDisabledIndefinite;
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

void SetSensorAlarm(SENSOR* psensor, ALARM* palarm)
{
	psensor->palarm = palarm;
}

void SetSensorSensors(SENSOR* psensor, SENSORS sensors)
{
	if (!psensor) return;

	const SENSORS oldState = psensor->sensors;
	if (oldState == sensors)
		return;

	// Special case: going from Enabled -> Triggered
	if (oldState == SENSORS_SenseEnabled && sensors == SENSORS_SenseTriggered)
	{
		// Default to triggered, but the alarm callback may change psensor->sensors.
		SENSORS finalState = SENSORS_SenseTriggered;

		if (psensor->palarm != NULL)
		{
			TriggerAlarm(psensor->palarm, ALTK_Trigger);

			// Preserve original behavior: if TriggerAlarm changed psensor->sensors,
			// we keep that updated state instead of forcing Triggered.
			if (psensor->sensors != SENSORS_SenseEnabled)
				finalState = psensor->sensors;
		}

		HandleLoSpliceEvent(psensor, 2, 0, NULL);
		psensor->sensors = finalState;
	}
	else
		psensor->sensors = sensors;

	psensor->tSensors = g_clock.t;
}

int FCheckSensorObject(SENSOR* psensor, SO* psoOther)
{
	if (psensor->fTriggerAll != 0) {
		return 1;
	}

	if (FIgnoreSensorObject(psensor, psoOther) != 0) {
		return 0;
	}

	for (int i = 0; i < psensor->coidTrigger; ++i) {
		if (FMatchesLoName(psoOther, psensor->aoidTrigger[i]) != 0) {
			return 1;
		}
	}

	for (int i = 0; i < psensor->ccidTrigger; ++i) {
		if (FIsBasicDerivedFrom(psoOther, psensor->acidTrigger[i]) != 0) {
			return 1;
		}
	}

	return 0;
}

int FIgnoreSensorObject(SENSOR* psensor, SO* psoOther)
{
	for (int i = 0; i < psensor->coidNoTrigger; ++i) {
		if (FMatchesLoName(psoOther, psensor->aoidNoTrigger[i]) != 0) {
			return 1;
		}
	}

	for (int i = 0; i < psensor->ccidNoTrigger; ++i) {
		if (FIsBasicDerivedFrom(psoOther, psensor->acidNoTrigger[i]) != 0) {
			return 1;
		}
	}

	return 0;
}

int FOnlySensorTriggerObject(SENSOR* psensor, SO* psoOther)
{
	if (psoOther == nullptr || psensor->fTriggerAll != 0) {
		return 0;
	}

	if (psensor->coidTrigger != 1 || psensor->ccidTrigger != 0) {
		return 0;
	}

	return FMatchesLoName(psoOther, psensor->aoidTrigger[0]) != 0;
}

void PauseSensor(SENSOR* psensor)
{
	ASEGA* pasega = PasegaFindAloNearest(psensor);

	if (pasega != nullptr) 
	{
		psensor->svtRestore = pasega->svtLocal;
		SetAsegaSpeed(pasega, 0.0f);
		psensor->pasegaPause = pasega;
	}
}

void UpdateSensor(SENSOR* psensor, float dt)
{
	UpdateSo(psensor, dt);

	if (psensor->pasegaPause != nullptr && g_pjt != nullptr && g_pjt->jts != JTS_Zap) {
		psensor->pasegaPause->svtLocal = psensor->svtRestore;
		psensor->svtRestore = 0.0f;
		psensor->pasegaPause = nullptr;
	}
}

void AddSensorTriggerObject(SENSOR* psensor, int oid)
{
	if (psensor->coidTrigger < 4) {
		psensor->aoidTrigger[psensor->coidTrigger++] = (OID)oid;
	}
}

void AddSensorNoTriggerObject(SENSOR* psensor, int oid)
{
	if (psensor->coidNoTrigger < 4) {
		psensor->aoidNoTrigger[psensor->coidNoTrigger++] = (OID)oid;
	}
}

void AddSensorTriggerClass(SENSOR* psensor, int cid)
{
	if (psensor->ccidTrigger < 4) {
		psensor->acidTrigger[psensor->ccidTrigger++] = (CID)cid;
	}
}

void AddSensorNoTriggerClass(SENSOR* psensor, int cid)
{
	if (psensor->ccidNoTrigger < 4) {
		psensor->acidNoTrigger[psensor->ccidNoTrigger++] = (CID)cid;
	}
}

void DeleteSensor(SENSOR* psensor)
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

void* GetLasenLask(LASEN* plasen)
{
	return &plasen->lask;
}

void SetLasenLask(LASEN* plasen, LASK lask)
{
	plasen->lask = lask;
}

void* GetLasenDtDamageDisabling(LASEN* plasen)
{
	return &plasen->dtDamageDisabling;
}

void SetLasenDtDamageDisabling(LASEN* plasen, float dtDamageDisabling)
{
	plasen->dtDamageDisabling = dtDamageDisabling;
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
	CloneSensor(plasen, plasenBase);

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
		plasen->albeam[i] = plasenBase->albeam[i];

	// Shallow copy of other struct members
	plasen->dleBusyLasen = plasenBase->dleBusyLasen;
}

void BindLasen(LASEN* plasen)
{
	BindAlo(plasen);

	// DL iteration safety
	DLI dli{};
	dli.m_pdl = &plasen->dlChild;
	dli.m_ibDle = plasen->dlChild.ibDle;

	EMITTER* pemitter = (EMITTER*)plasen->dlChild.paloFirst;

	dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pemitter) + dli.m_ibDle);

	dli.m_pdliNext = s_pdliFirst;
	s_pdliFirst = &dli;

	while (pemitter)
	{
		if (!FIsBasicDerivedFrom((BASIC*)pemitter, CID_SHAPE))
		{
			// Non-shape children: track damage scorch emitters (up to 4)
			if (FIsBasicDerivedFrom((BASIC*)pemitter, CID_EMITTER))
			{
				uint32_t idx = plasen->albeam[0].clemitDamage;
				if (idx < 4)
				{
					plasen->albeam[0].clemitDamage = idx + 1;

					LEMIT* plemit = plasen->albeam[0].alemitDamage + idx;
					plemit->pemitter = pemitter;
					plemit->fScorch = (short)(pemitter->oid == 649);

					PauseEmitterIndefinite(pemitter);
				}
			}
		}
		else
		{
			// Shape children: bind beam shapes (up to 16) that have a linear CRVL child
			uint32_t iBeam = plasen->clbeam;
			if (iBeam < 16)
			{
				SHAPE* pshape = (SHAPE*)pemitter;
				CRV* pcrv = pshape->pcrv.get();

				if (pcrv != nullptr && pcrv->crvk == CRVK_Linear)
				{
					CRV* pcrvl = pcrv;

					if (pcrvl->ccv > 0)
					{
						LBEAM* pbeam = &plasen->albeam[iBeam];

						plasen->clbeam = iBeam + 1;

						pbeam->pshape = pshape;
						pbeam->sShape = pcrvl->mpicvs[pcrvl->ccv - 1];

						if (plasen->cposBeamShapeMax < pcrvl->ccv)
							plasen->cposBeamShapeMax = pcrvl->ccv;

						for (int i = 0; i < pcrvl->ccv; ++i)
						{
							glm::vec3 posCvWorld{};

							ConvertAloPos(plasen, nullptr, &pcrvl->mpicvpos[i], &posCvWorld);

							const float dx = plasen->xf.posWorld.x - posCvWorld.x;
							const float dy = plasen->xf.posWorld.y - posCvWorld.y;
							const float dz = plasen->xf.posWorld.z - posCvWorld.z;
							const float distance = sqrtf(dx * dx + dy * dy + dz * dz);

							if (plasen->sRadiusRenderAll < distance)
								plasen->sRadiusRenderAll = distance;
						}
					}
				}
			}
		}

		// Advance using the same DLI pointer-walk the decomp does
		pemitter = reinterpret_cast<EMITTER*>(*dli.m_ppv);
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pemitter) + dli.m_ibDle);
	}

	if (plasen->zons != 2)
		ClipVismapSphereOneHop(plasen->psw->pvismap, &(plasen->xf).posWorld, plasen->sRadiusRenderAll, &plasen->grfzon);

	// Propagate expanded radius up the parent chain based on local offset length + child radius
	if (plasen->paloParent != nullptr)
	{
		LASEN* cur = plasen;

		for (;;)
		{
			// |cur->xf.pos| + cur->sRadiusRenderAll
			float px = (cur->xf).pos.x;
			float py = (cur->xf).pos.y;
			float pz = (cur->xf).pos.z;
			float localLen = sqrtf(px * px + py * py + pz * pz);

			ALO* parent = cur->paloParent;
			float needed = localLen + cur->sRadiusRenderAll;

			if (needed <= parent->sRadiusRenderAll)
				break;

			parent->sRadiusRenderAll = needed;

			if (parent->zons != 2)
				ClipVismapSphereOneHop(parent->psw->pvismap, &(parent->xf).posWorld, needed, &parent->grfzon);

			// Walk up
			cur = (LASEN*)parent;
			if (!cur->paloParent)
				break;
		}
	}

	// Pop DLI
	s_pdliFirst = dli.m_pdliNext;
}

void PostLasenLoad(LASEN* plasen)
{
	PostAloLoad(plasen);

	for (int i = 0; i < plasen->clbeam; ++i) 
	{
		LBEAM* pbeam = &plasen->albeam[i];

		// Remove the shape's LO
		if (pbeam->pshape && pbeam->pshape->pvtlo->pfnRemoveLo) {
			pbeam->pshape->pvtlo->pfnRemoveLo(pbeam->pshape);
		}

		if (i == 0) {
			// Skip cloning damage emitters for beam 0 (used as source template)
			continue;
		}

		int clemitDamage = plasen->albeam[0].clemitDamage;
		pbeam->clemitDamage = clemitDamage;

		if (clemitDamage > 0) {
			LEMIT* dstEmit = pbeam->alemitDamage;
			LEMIT* srcEmit = plasen->albeam[0].alemitDamage;

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

	 //Set jtOnlyTriggerObject flag based on g_pjt
	plasen->fJtOnlyTriggerObject = FOnlySensorTriggerObject(plasen, g_pjt);
	plasen->pvtlasen->pfnSetLasenSensors(plasen, plasen->sensorsInitial);
}

void UpdateBusyLasenSenseTimes()
{
	LASEN* plasen;
	int cEnabled = 0;
	int iEnabled = 0;

	// Count enabled busy LASEN sensors
	for (plasen = g_psw->dlBusyLasen.plasenFirst; plasen != nullptr; plasen = plasen->dleBusyLasen.plasenNext)
	{
		if (plasen->sensors != SENSORS_Disabled)
			cEnabled++;
	}

	// Spread their next sense times evenly across one frame
	if (cEnabled > 0)
	{
		float dtSense = 0.033333335f / (float)cEnabled;

		for (plasen = g_psw->dlBusyLasen.plasenFirst; plasen != nullptr; plasen = plasen->dleBusyLasen.plasenNext)
		{
			if (plasen->sensors == SENSORS_Disabled)
				continue;

			plasen->tSenseNext = g_clock.t + dtSense * (float)iEnabled;
			iEnabled++;
		}
	}

	g_fLasenBusyListChange = 0;
}

void UpdateLasen(LASEN* plasen, float dt)
{
	UpdateSensor(plasen, dt);

	plasen->uDrawMax = glm::clamp(plasen->uDrawMax + dt * plasen->svuDrawMax, 0.0f, 1.0f);

	if (g_fLasenBusyListChange != 0)
		UpdateBusyLasenSenseTimes();

	SENSORS sensors = plasen->sensors;
	bool fSense = false;
	float dtSensors = g_clock.t - plasen->tSensors;

	switch (sensors) 
	{
		case SENSORS_SenseEnabling:
		if (plasen->dtEnabling < dtSensors) {
			sensors = SENSORS_SenseEnabled;
		}

		fSense = true;
		break;

		case SENSORS_SenseEnabled:
		case SENSORS_DamageEnabled:
		case SENSORS_DamageTriggered:
		fSense = true;
		break;

		case SENSORS_SenseTriggered:
		sensors = SENSORS_SenseDisabling;
		fSense = true;
		break;

		case SENSORS_SenseDisabling:
		if (plasen->dtDisabling < dtSensors) {
			if (plasen->sensm == SENSM_SenseOnly) {
				sensors = SENSORS_Disabled;
				fSense = true;
			}
			else if (plasen->sensm == SENSM_DamageTarget) {
				sensors = SENSORS_DamageEnabling;
				fSense = true;
			}
		}
		else {
			fSense = true;
		}
		break;

		case SENSORS_DamageEnabling:
		if (plasen->dtEnabling < dtSensors) {
			sensors = SENSORS_DamageEnabled;
		}

		fSense = true;
		break;

		case SENSORS_DamageDisabling:
		if (plasen->dtDamageDisabling < dtSensors) {
			sensors = SENSORS_Disabled;
		}

		fSense = true;
		break;

		default:
		break;
	}

	if (fSense && plasen->pvtlasen->pfnSenseLasen != nullptr)
		plasen->pvtlasen->pfnSenseLasen(plasen, &sensors);

	plasen->pvtlasen->pfnSetLasenSensors(plasen, sensors);
}

void FreezeLasen(LASEN* plasen, int fFreeze)
{
	FreezeSo(plasen, fFreeze);

	if (fFreeze == 0)
	{
		AppendDlEntry(&plasen->psw->dlBusyLasen, plasen);
		plasen->fBusyLasen = 1;
	}
	else
	{
		if (plasen->fBusyLasen != 0)
		{
			RemoveDlEntry(&plasen->psw->dlBusyLasen, plasen);
			plasen->fBusyLasen = 0;
		}
	}

	g_fLasenBusyListChange = 1;
}

void RenderLasenSelf(LASEN* plasen, CM* pcm, RO* pro)
{
	RenderSoSelf(plasen, pcm, pro);

	SENSORS sensors = plasen->sensors;
	if (sensors == SENSORS_Disabled || sensors <= SENSORS_Nil)
		return;

	// Choose which ALO to render the beam lines with (matches original state gates)
	ALO* paloRender = nullptr;
	if (sensors < SENSORS_DamageEnabling) {
		paloRender = plasen->paloRenderSense;
	}
	else {
		if (sensors > SENSORS_DamageDisabling)
			return;
		paloRender = plasen->paloRenderDamage;
	}

	// Fallback to stock[0x0B] like original
	if (!paloRender) {
		paloRender = reinterpret_cast<ALO*>(plasen->psw->aploStock[0x0B]);
		if (!paloRender)
			return;
	}

	// Alpha is fetched per-segment in the original; value is constant for this call
	float alpha = 1.0;

	if (pro != nullptr)
		alpha = pro->uAlpha;

	// Temp buffer (original uses stack allocator sized by cposBeamShapeMax * 16 bytes)
	std::vector<glm::vec3> posBuf;
	posBuf.resize(static_cast<size_t>(plasen->cposBeamShapeMax));

	const int clbeam = plasen->clbeam;
	for (int ibeam = 0; ibeam < clbeam; ++ibeam)
	{
		LBEAM* plbeam = &plasen->albeam[ibeam];

		// --- Build positions from the curve ---
		CRV* crv = plbeam->pshape->pcrv.get();
		int ccv = crv->ccv;

		const float extent = SCalcLasenShapeExtent(plasen, plbeam);

		// Clamp CV count if we have a hit: min(ccv, iposHit + 2)
		if (plbeam->psoHit != nullptr) {
			const int clampCount = plbeam->iposHit + 2;
			if (clampCount < ccv)
				ccv = clampCount;
		}

		// Convert curve CVs to world space into posBuf[0..ccv-1]
		for (int i = 0; i < ccv; ++i) {
			ConvertAloPos(plasen, nullptr, &crv->mpicvpos[i], &posBuf[i]);
		}

		// Overwrite posBuf[iposHit+1] with exact hit point (if valid)
		if (plbeam->psoHit != nullptr) {
			const int idx = plbeam->iposHit + 1;
			if (0 <= idx && idx < ccv) {
				posBuf[idx] = glm::vec3(plbeam->posHit.x, plbeam->posHit.y, plbeam->posHit.z);
			}
		}

		// --- Render segments up to the extent ---
		float drawnLen = 0.0f;

		if (ccv > 1)
		{
			for (int i = 0; i < ccv - 1; ++i)
			{
				glm::vec3& p0 = posBuf[i];
				glm::vec3& p1 = posBuf[i + 1];

				const float segLenFull = glm::length(p1 - p0);
				if (segLenFull <= 0.0f)
					continue;

				const float remaining = extent - drawnLen;
				if (remaining <= 0.0f)
					break;

				float segLen = segLenFull;

				// If extent ends inside this segment, truncate p1 in-place (like original)
				if (remaining < segLenFull) {
					const float t = remaining / segLenFull; // 0..1
					p1 = p0 * (1.0f - t) + p1 * t;
					segLen = remaining;
				}

				RenderAloLine(paloRender, pcm, &p0, &p1, 1.0f, alpha);

				drawnLen += segLen;

				// Original breaks when fVar13 < fVar15 after rendering
				if (extent < drawnLen)
					break;
			}
		}

		// --- Update sShapeLast with the same state-machine behavior as original ---
		sensors = plasen->sensors;

		if (sensors < SENSORS_SenseEnabling) {
			// no update
		}
		else if (sensors < SENSORS_SenseDisabling) {
			plbeam->sShapeLast = drawnLen;
		}
		else {
			if (sensors > SENSORS_DamageTriggered) {
				// no update
			}
			else if (sensors > SENSORS_SenseDisabling) {
				plbeam->sShapeLast = drawnLen;
			}
			else {
				// sensors == SENSORS_SenseDisabling -> no update
			}
		}
	}
}

int FFilterLasen(void* pv, JT* pjt)
{
	LASEN* plasen = (LASEN*)pv;

	if (pjt->fNoXpsSelf != 0) {
		return 0;
	}

	if (pjt->paloRoot == plasen->paloRoot) {
		return 0;
	}

	if (FIgnoreSensorObject(plasen, pjt) != 0) {
		return 0;
	}

	if (pjt == g_pjt && pjt->jts == JTS_Hide && pjt->jthk == JTHK_Nonchalant) {
		if ((GetAvailableVaultFlags() & 0x12000) != 0)
			return 0;
	}

	return pjt->fHidden == 0;


}

void SenseLasen(LASEN* plasen, SENSORS* psensors)
{
	if (g_clock.t < plasen->tSenseNext)
		return;

	plasen->tSenseNext = g_clock.t + (1.0f / 30.0f);

	std::vector <glm::vec3> aposWorld(plasen->cposBeamShapeMax);
	std::vector <SO*> apso;

	for (int ilbeam = 0; ilbeam < plasen->clbeam; ++ilbeam) 
	{
		LBEAM* plbeam = &plasen->albeam[ilbeam];
		CRV* pcrv = plbeam->pshape->pcrv.get();
		int cpos = pcrv->ccv;
		float sExtent = SCalcLasenShapeExtent(plasen, plbeam);
		float sTraveled = 0.0f;
		bool fUseEmitter = false;
		LSG lsg{};

		plbeam->psoHit = nullptr;

		for (int ipos = 0; ipos < cpos; ++ipos) 
			ConvertAloPos(plasen, nullptr, &pcrv->mpicvpos[ipos], &aposWorld[ipos]);

		for (int ipos = 0; ipos < cpos - 1; ++ipos) 
		{
			glm::vec3 pos1 = aposWorld[ipos];
			glm::vec3 pos2 = aposWorld[ipos + 1];
			glm::vec3 posCenter = (pos1 + pos2) * 0.5f;
			float sSegment = glm::length(pos2 - pos1);
			float sRadius = sSegment * 0.5f;
			SO* psoOther = nullptr;
			bool fTestWorld = plasen->fJtOnlyTriggerObject == 0;

			if (plasen->fJtOnlyTriggerObject != 0 && SphereInFrustum(g_pcm->frustum, posCenter, sRadius))
			{
				fTestWorld = true;

				if (g_pjt != nullptr && plasen->lask == LASK_Static)
				{
					bool fIgnoreJtOnly = g_pjt->jts == JTS_Hide && g_pjt->jthk == JTHK_Nonchalant && (GetAvailableVaultFlags() & 0x12000) != 0;

					if (!fIgnoreJtOnly)
					{
						std::vector<SO*> apsoJt = { static_cast<SO*>(g_pjt) };

						psoOther = PsoHitTestLineObjects(0, &pos1, &pos2, apsoJt, &lsg);
						fTestWorld = false;
					}
				}
			}

			if (fTestWorld)
			{
				apso.clear();

				IntersectSwBoundingSphere(plasen->psw, nullptr, &posCenter, sRadius, reinterpret_cast<PFNFILTER>(FFilterLasen), plasen, apso);
				psoOther = PsoHitTestLineObjects(0, &pos1, &pos2, apso, &lsg);
			}

			if (psoOther != nullptr) 
			{
				float sHit = glm::length(lsg.apos[0] - pos1);

				if (sTraveled + sHit < sExtent) {
					plbeam->posHit = lsg.apos[0];
					plbeam->iposHit = ipos;
					plbeam->psoHit = psoOther;

					fUseEmitter = *psensors >= SENSORS_DamageEnabling && *psensors < SENSORS_Disabled;

					if (FCheckSensorObject((SENSOR*)plasen, psoOther) != 0) {
						switch (*psensors) 
						{
							case SENSORS_SenseEnabled:
							*psensors = SENSORS_SenseTriggered;
							break;

							case SENSORS_DamageEnabled:
							*psensors = SENSORS_DamageTriggered;
							break;

							default:
							break;
						}

						if (*psensors == SENSORS_DamageTriggered && psoOther == (SO*)g_pjt) 
						{
							ZPR zpr;

							InitZpr(&zpr, ZPK_Fire, (LO*)plasen);

							if (g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zpr) != 0) {
								PauseSensor((SENSOR*)plasen);
							}
						}

						plasen->pvtlasen->pfnSetLasenSensors(plasen, *psensors);
					}

					break;
				}
			}

			sTraveled += sSegment;

			if (sExtent < sTraveled) {
				break;
			}
		}

		for (int ilemit = 0; ilemit < plbeam->clemitDamage; ++ilemit) 
		{
			LEMIT* plemit = &plbeam->alemitDamage[ilemit];

			if (!fUseEmitter || plbeam->psoHit == nullptr) {
				PauseEmitterIndefinite(plemit->pemitter);
				continue;
			}

			if (plbeam->psoHit->fCpsoBuildContactGroup == 0 && plemit->fScorch != 0) {
				PauseEmitterIndefinite(plemit->pemitter);
				continue;
			}

			EMITB* pemitb = PemitbEnsureEmitter(plemit->pemitter, ENSK_Set);
			pemitb->emito.vec = lsg.anormal[0];

			glm::vec3 posEmitter;
			ConvertAloPos(nullptr, plasen, &lsg.apos[0], &posEmitter);
			plemit->pemitter->pvtalo->pfnTranslateAloToPos(plemit->pemitter, &posEmitter);
			UnpauseEmitter(plemit->pemitter);
		}
	}
}

void EnableLasen(LASEN* plasen, SENSM sensm)
{
	plasen->sensm = sensm;

	SENSORS sensors = sensm == SENSM_SenseOnly ? SENSORS_SenseEnabled : SENSORS_DamageEnabled;
	plasen->pvtlasen->pfnSetLasenSensors(plasen, sensors);
}

void DisableLasen(LASEN* plasen)
{
	if (plasen->sensors == SENSORS_Disabled) {
		return;
	}

	SENSORS sensors = plasen->sensm == SENSM_SenseOnly ? SENSORS_SenseDisabling : SENSORS_DamageDisabling;
	plasen->pvtlasen->pfnSetLasenSensors(plasen, sensors);
}

void OnLasenAlarmTriggered(LASEN* plasen)
{
	if (plasen->sensors > SENSORS_Nil && plasen->sensors < SENSORS_SenseDisabling) {
		plasen->pvtlasen->pfnSetLasenSensors(plasen, SENSORS_SenseDisabling);
	}

	plasen->sensm = SENSM_DamageTarget;
}

void SetLasenSensors(LASEN* plasen, SENSORS sensors)
{
	SENSORS sensorsPrev = plasen->sensors;

	if (sensorsPrev == sensors) {
		return;
	}

	if (sensorsPrev == SENSORS_Nil || sensorsPrev == SENSORS_Disabled) 
		StartSound((SFXID)26, &plasen->pamb, plasen, nullptr, 1000.0f, 100.0f, 0.4f, 0.0f, 0.0f, nullptr, nullptr);

	if (sensors == SENSORS_SenseDisabling || sensors == SENSORS_DamageDisabling) {
		for (int ilbeam = 0; ilbeam < plasen->clbeam; ++ilbeam) {
			LBEAM* plbeam = &plasen->albeam[ilbeam];

			for (int ilemit = 0; ilemit < plbeam->clemitDamage; ++ilemit)
				PauseEmitterIndefinite(plbeam->alemitDamage[ilemit].pemitter);
		}
	}
	else if (sensors == SENSORS_Disabled) 
	{
		float dtFade = plasen->sensm == SENSM_SenseOnly ? plasen->dtDisabling : plasen->dtDamageDisabling;

		StopSound(plasen->pamb, static_cast<int>(dtFade * 1000.0f));
		plasen->pvtlo->pfnRemoveLo(plasen);
		g_fLasenBusyListChange = 1;
	}

	SetSensorSensors(plasen, sensors);
}

float SCalcLasenShapeExtent(LASEN* plasen, LBEAM* plbeam)
{
	float sShape = plbeam->sShape;
	float uDraw = 0.0f;
	float dtSensors = g_clock.t - plasen->tSensors;

	switch (plasen->sensors) 
	{
		case SENSORS_SenseEnabling:
		case SENSORS_DamageEnabling:
		if (plasen->dtEnabling == 0.0f)
			uDraw = plasen->uDrawMax;
		else
			uDraw = glm::clamp(dtSensors / plasen->dtEnabling, 0.0f, plasen->uDrawMax);
		break;

		case SENSORS_SenseEnabled:
		case SENSORS_SenseTriggered:
		case SENSORS_DamageEnabled:
		case SENSORS_DamageTriggered:
		uDraw = plasen->uDrawMax;
		break;

		case SENSORS_SenseDisabling:
		case SENSORS_DamageDisabling:
		{
			float dtDisabling = plasen->sensm == SENSM_SenseOnly ? plasen->dtDisabling : plasen->dtDamageDisabling;

			if (dtDisabling != 0.0f)
				uDraw = glm::clamp(1.0f - dtSensors / dtDisabling, 0.0f, plasen->uDrawMax);

			sShape = plbeam->sShapeLast;
			break;
		}

		case SENSORS_Disabled:
		default:
		break;
	}

	return sShape * uDraw;
}

void RetractLasen(LASEN* plasen, float dtRetract)
{
	plasen->svuDrawMax = -1.0 / dtRetract;
}

void ExtendLasen(LASEN* plasen, float dtExpand)
{
	plasen->svuDrawMax = 1.0 / dtExpand;
}

void DeleteLasen(LASEN* plasen)
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

void* GetCamsenDtDamageFocus(CAMSEN* pcamsen)
{
	return &pcamsen->dtDamageFocus;
}

void SetCamsenDtDamageFocus(CAMSEN* pcamsen, float dtDamageFocus)
{
	pcamsen->dtDamageFocus = dtDamageFocus;
}

void* GetCamsenDtDamageZap(CAMSEN* pcamsen)
{
	return &pcamsen->dtDamageZap;
}

void SetCamsenDtDamageZap(CAMSEN* pcamsen, float dtDamageZap)
{
	pcamsen->dtDamageZap = dtDamageZap;
}

void* GetCamsenDtDamageUnfocus(CAMSEN* pcamsen)
{
	return &pcamsen->dtDamageUnfocus;
}

void SetCamsenDtDamageUnfocus(CAMSEN* pcamsen, float dtDamageUnfocus)
{
	pcamsen->dtDamageUnfocus = dtDamageUnfocus;
}

void* GetCamsenCsdts(CAMSEN* pcamsen)
{
	return &pcamsen->csdts;
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

void PostCamsenLoad(CAMSEN* pcamsen)
{
	PostAloLoad(pcamsen);
	SnipAloObjects(pcamsen, 2, s_asnipCamsen);

	if (pcamsen->paloRenderDamage == nullptr) {
		pcamsen->paloRenderDamage = pcamsen;
	}

	if (pcamsen->paloRenderZap == nullptr) {
		pcamsen->paloRenderZap = (ALO*)pcamsen->psw->aploStock[11];
	}

	if (pcamsen->pactla != nullptr)
		pcamsen->pactla->nPriorityEnabled = 0;

	// Retail directly sets inherited SO flag bit 43 here (fNoXpsSelf).
	pcamsen->fNoXpsSelf = 1;

	SetSoConstraints(pcamsen, CT_Locked, nullptr, CT_Locked, nullptr);
	pcamsen->pvtcamsen->pfnSetCamsenSensors(pcamsen, pcamsen->sensorsInitial);

}

void UpdateCamsen(CAMSEN* pcamsen, float dt)
{
	UpdateSensor((SENSOR*)pcamsen, dt);

	SENSORS sensors = pcamsen->sensors;
	float dtSensors = g_clock.t - pcamsen->tSensors;

	switch (sensors) {
	case SENSORS_SenseEnabling:
		if (pcamsen->dtEnabling < dtSensors) {
			sensors = SENSORS_SenseEnabled;
		}
		break;

	case SENSORS_SenseEnabled:
	case SENSORS_DamageEnabled:
		if (pcamsen->pvtcamsen->pfnSenseCamsen != nullptr) {
			pcamsen->pvtcamsen->pfnSenseCamsen(pcamsen, &sensors);
		}
		break;

	case SENSORS_SenseTriggered:
		sensors = SENSORS_SenseDisabling;
		break;

	case SENSORS_SenseDisabling:
		if (pcamsen->dtDisabling < dtSensors) {
			sensors = SENSORS_Disabled;
		}
		break;

	case SENSORS_DamageEnabling:
		if (pcamsen->dtEnabling < dtSensors) {
			sensors = SENSORS_DamageEnabled;
		}
		break;

	case SENSORS_DamageTriggered:
	{
		CSDTS csdts = pcamsen->csdts;
		float dtCsdts = g_clock.t - pcamsen->tCsdts;

		switch (csdts) {
		case CSDTS_Focus:
			if (g_pjt != nullptr && g_pjt->pvtpo->pfnFInvulnerablePo(g_pjt, ZPK_Fire) != 0) {
				csdts = CSDTS_Unfocus;
			}
			else if (pcamsen->dtDamageFocus < dtCsdts) {
				if (g_pjt == nullptr) {
					csdts = CSDTS_Zap;
				}
				else {
					ZPR zpr;

					InitZpr(&zpr, ZPK_Fire, (LO*)pcamsen);

					if (g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zpr) != 0) {
						csdts = CSDTS_Zap;
						PauseSensor((SENSOR*)pcamsen);
					}
				}
			}
			break;

		case CSDTS_Zap:
			if (pcamsen->dtDamageZap < dtCsdts) {
				csdts = CSDTS_Unfocus;
			}
			break;

		case CSDTS_Unfocus:
			if (pcamsen->dtDamageUnfocus < dtCsdts) {
				csdts = CSDTS_Nil;
				sensors = SENSORS_DamageEnabled;
			}
			break;

		default:
			break;
		}

		SetCamsenCsdts(pcamsen, csdts);
		break;
	}

	case SENSORS_DamageDisabling:
		if (pcamsen->dtDisabling < dtSensors) {
			sensors = SENSORS_Disabled;
		}
		break;

	case SENSORS_Disabled:
		if (pcamsen->fRemainDisabledIndefinite == 0) {
			if (pcamsen->sensm == SENSM_SenseOnly) {
				sensors = SENSORS_SenseEnabling;
			}
			else if (pcamsen->sensm == SENSM_DamageTarget) {
				sensors = SENSORS_DamageEnabling;
			}
		}
		break;

	default:
		break;
	}

	pcamsen->pvtcamsen->pfnSetCamsenSensors(pcamsen, sensors);
}

void RenderCamsenSelf(CAMSEN* pcamsen, CM* pcm, RO* pro)
{
	if (pcamsen->sensors == SENSORS_Disabled)
		return;

	RO ro{};
	DupAloRo((ALO*)pcamsen, pro, &ro);

	// Default scale
	glm::vec3 scale(1.0f, 1.0f, 1.0f);

	const float now = g_clock.t;

	switch (pcamsen->sensors)
	{
	case SENSORS_SenseEnabling:
	case SENSORS_DamageEnabling:
	{
		float t = 1.0f;
		if (pcamsen->dtEnabling != 0.0f)
		{
			t = (now - pcamsen->tSensors) / pcamsen->dtEnabling;
			if (t < 0.0f) t = 0.0f;
			else if (t > 1.0f) t = 1.0f;
		}

		// X fixed at 1, Y/Z ramp
		scale.x = 1.0f;
		scale.y = t;
		scale.z = t;
		break;
	}

	case SENSORS_SenseEnabled:
	case SENSORS_SenseTriggered:
	case SENSORS_DamageEnabled:
	{
		scale.x = 1.0f;
		scale.y = 1.0f;
		scale.z = 1.0f;
		break;
	}

	case SENSORS_SenseDisabling:
	case SENSORS_DamageDisabling:
	{
		float t = 0.0f;
		if (pcamsen->dtDisabling != 0.0f)
		{
			t = 1.0f - (now - pcamsen->tSensors) / pcamsen->dtDisabling;
			if (t < 0.0f) t = 0.0f;
			else if (t > 1.0f) t = 1.0f;
		}

		scale.x = 1.0f;
		scale.y = t;
		scale.z = t;
		break;
	}

	case SENSORS_DamageTriggered:
	{
		scale.x = 1.0f;

		if (pcamsen->csdts == CSDTS_Zap)
		{
			if (g_pjt && pcamsen->paloRenderZap)
			{
				float t = 1.0f;
				if (pcamsen->dtDamageZap != 0.0f)
				{
					t = (now - pcamsen->tCsdts) / pcamsen->dtDamageZap;
					if (t < 0.0f) t = 0.0f;
					else if (t > 1.0f) t = 1.0f;
				}

				// posZap = pcamsenPos*(1-t) + pjtPos*t
				const glm::vec3 camsenPos = pcamsen->xf.posWorld;     // adjust if your type differs
				const glm::vec3 pjtPos = g_pjt->xf.posWorld;

				glm::vec3 posZap;
				posZap.x = camsenPos.x * (1.0f - t) + pjtPos.x * t;
				posZap.y = camsenPos.y * (1.0f - t) + pjtPos.y * t;
				posZap.z = camsenPos.z * (1.0f - t) + pjtPos.z * t;

				RenderAloLine(pcamsen->paloRenderZap, pcm, &posZap, (glm::vec3*)&(g_pjt->xf.posWorld), 10.0f, ro.uAlpha);
			}
		}
		else if (pcamsen->csdts == CSDTS_Focus)
		{
			float v = 0.1f;

			if (pcamsen->dtDamageFocus != 0.0f)
			{
				v = 1.0f - (now - pcamsen->tCsdts) / pcamsen->dtDamageFocus;
				if (v < 0.0f) v = 0.0f;
				else if (v > 1.0f) v = 1.0f;

				if (v < 0.1f) v = 0.1f;
			}

			scale.y = v;
			scale.z = v;
		}
		else if (pcamsen->csdts == CSDTS_Unfocus)
		{
			float v = 0.1f;

			if (pcamsen->dtDamageUnfocus != 0.0f)
			{
				v = (now - pcamsen->tCsdts) / pcamsen->dtDamageUnfocus;
				if (v < 0.0f) v = 0.0f;
				else if (v > 1.0f) v = 1.0f;

				if (v < 0.1f) v = 0.1f;
			}

			scale.y = v;
			scale.z = v;
		}
		// else: unknown csdts -> leave scale as default (1,1,1) except X forced to 1
		break;
	}

	default:
		// leave scale at (1,1,1)
		break;
	}

	// Choose which ALO to render (matches the decompiled state-band check)
	CAMSEN* renderAlo = pcamsen;
	if ((SENSORS_Nil < pcamsen->sensors) &&
		(SENSORS_SenseDisabling < pcamsen->sensors) &&
		(pcamsen->sensors < SENSORS_Disabled) &&
		pcamsen->paloRenderDamage)
	{
		renderAlo = (CAMSEN*)pcamsen->paloRenderDamage;
	}

	LoadMatrixFromPosRotScale(&pcamsen->xf.posWorld, &pcamsen->xf.matWorld, &scale, &ro.model);
	renderAlo->pvtalo->pfnRenderAloGlobset(renderAlo, pcm, &ro);
}

int FIgnoreCamsenIntersection(CAMSEN* pcamsen, SO* psoOther)
{
	return 1;
}

int FFilterCamsen(void* pv, SO* pso)
{
	CAMSEN* pcamsen = (CAMSEN*)pv;
	PO* ppo = PpoCur();

	if (pso->fNoXpsSelf != 0) {
		return 0;
	}

	if (pso->paloRoot == (ALO*)ppo) {
		return 0;
	}

	if (pso->paloRoot == pcamsen->paloRoot) {
		return 0;
	}

	if (FIgnoreSensorObject((SENSOR*)pcamsen, pso) != 0) {
		return 0;
	}

	return pso->fHidden == 0;
}

void SenseCamsen(CAMSEN* pcamsen, SENSORS* psensors)
{
	if (g_pjt == nullptr) {
		return;
	}

	if (g_pjt->pvtpo->pfnFInvulnerablePo(g_pjt, ZPK_Fire) != 0) {
		return;
	}

	bool fInsideSensor = false;
	glm::vec3 posJt = g_pjt->xf.posWorld;
	const float sDistance = glm::length(posJt - pcamsen->xf.posWorld);
	bool fInsideBsp = false;

	if (pcamsen->fSphere != 0) {
		if (sDistance < pcamsen->sRadiusSelf) {
			fInsideSensor = true;
		}
	}

	if (!fInsideSensor && pcamsen->bspc.absp.size() != 0) {
		fInsideBsp = PbspPointInBspQuick(&posJt, pcamsen->bspc.absp.data()) != nullptr;
		fInsideSensor = fInsideBsp;
	}

	if (!fInsideSensor) {
		return;
	}

	if (g_pjt->jts == JTS_Hide) {
		glm::vec3 dpos = posJt - pcamsen->xf.posWorld;
		dpos.z = 0.0f;

		float sDistance = glm::length(dpos);

		if (sDistance < 0.0001f) {
			dpos = glm::vec3(g_normalX);
		}
		else {
			dpos /= sDistance;
		}

		float gFacing = glm::dot(dpos, glm::vec3(g_pjt->xf.matWorld[0]));

		if (g_pjt->jthk == JTHK_Duck && gFacing < 0.0f) {
			return;
		}

		if (g_pjt->jthk == JTHK_Flatten && gFacing > 0.0f) {
			return;
		}

		// Release values: jthk 2 is the basket and jtbs 16 is its stationary
		// hide state. The proto-named JTBS_Hide_Stand value is shifted here.
		if ((int)g_pjt->jthk == 2 && (int)g_pjt->jtbs == 16) {
			return;
		}
	}

	std::vector <SO*> apso;

	IntersectSwBoundingBox(pcamsen->psw, nullptr, &posJt, &pcamsen->paloRoot->xf.posWorld, (PFNFILTER)FFilterCamsen, pcamsen, apso);

	SO* psoHit = PsoHitTestLineObjects((GRFHTL)1, &pcamsen->xf.posWorld, &posJt, apso, nullptr);

	if (psoHit == nullptr) {
		*psensors = pcamsen->sensm == SENSM_SenseOnly ? SENSORS_SenseTriggered : SENSORS_DamageTriggered;
	}
}

void EnableCamsen(CAMSEN* pcamsen, SENSM sensm)
{
	pcamsen->sensm = sensm;

	SENSORS sensors = sensm == SENSM_SenseOnly ? SENSORS_SenseEnabled : SENSORS_DamageEnabled;
	pcamsen->pvtcamsen->pfnSetCamsenSensors(pcamsen, sensors);
}

void DisableCamsen(CAMSEN* pcamsen)
{
	if (pcamsen->sensors != SENSORS_Disabled) {
		SENSORS sensors = pcamsen->sensm == SENSM_SenseOnly ? SENSORS_SenseDisabling : SENSORS_DamageDisabling;
		pcamsen->pvtcamsen->pfnSetCamsenSensors(pcamsen, sensors);
	}

	ALARM* palarm = pcamsen->palarm;

	if (palarm != nullptr && palarm->alarms == ALARMS_Disabled && std::fabs(g_clock.t - palarm->tAlarms) < 0.2f) {
		pcamsen->fRemainDisabledIndefinite = 1;
	}
}

void OnCamsenAlarmTriggered(CAMSEN* pcamsen)
{
	if (pcamsen->sensm != SENSM_SenseOnly) {
		return;
	}

	pcamsen->sensm = SENSM_DamageTarget;

	if (pcamsen->sensors != SENSORS_Disabled) {
		pcamsen->pvtcamsen->pfnSetCamsenSensors(pcamsen, SENSORS_SenseDisabling);
	}
}

void SetCamsenSensors(CAMSEN* pcamsen, SENSORS sensors)
{
	SENSORS sensorsPrev = pcamsen->sensors;

	if (sensorsPrev == sensors) {
		return;
	}

	if (sensorsPrev == SENSORS_DamageTriggered) {
		SetCamsenCsdts(pcamsen, CSDTS_Nil);
	}
	else if (sensorsPrev == SENSORS_Disabled) {
		pcamsen->fRemainDisabledIndefinite = 0;
	}

	if (sensors == SENSORS_DamageTriggered) {
		SetCamsenCsdts(pcamsen, CSDTS_Focus);
	}

	SetSensorSensors(pcamsen, sensors);
}

void SetCamsenCsdts(CAMSEN* pcamsen, CSDTS csdts)
{
	CSDTS csdtsPrev = pcamsen->csdts;

	if (csdtsPrev == csdts)
		return;

	if (csdtsPrev == CSDTS_Zap)
		StopSound(pcamsen->pamb, 0);

	switch (csdts) 
	{
		case CSDTS_Focus:
		if (pcamsen->pactla != nullptr) {
			pcamsen->pactla->nPriorityEnabled = 4;
		}
		break;

		case CSDTS_Zap:
		StartSound((SFXID)27, &pcamsen->pamb, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
		break;

		case CSDTS_Unfocus:
		if (pcamsen->pactla != nullptr) 
			pcamsen->pactla->nPriorityEnabled = 0;
		break;

		default:
		break;
	}

	pcamsen->csdts = csdts;
	pcamsen->tCsdts = g_clock.t;
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

	pprsen->iframeDisablingFlash = -1;
	pprsen->iframeSenseStart = -1;
	pprsen->iframeSenseEnd = -1;
	pprsen->iframeDamageStart = -1;
	pprsen->iframeDamageEnd = -1;
	pprsen->iframeDisabledStart = -1;
	pprsen->iframeDisabledEnd = -1;
}

void* GetPrsenIframeSenseStart(PRSEN* pprsen)
{
	return &pprsen->iframeSenseStart;
}

void SetPrsenIframeSenseStart(PRSEN* pprsen, int iframeSenseStart)
{
	pprsen->iframeSenseStart = iframeSenseStart;
}

void* GetPrsenIframeSenseEnd(PRSEN* pprsen)
{
	return &pprsen->iframeSenseEnd;
}

void SetPrsenIframeSenseEnd(PRSEN* pprsen, int iframeSenseEnd)
{
	pprsen->iframeSenseEnd = iframeSenseEnd;
}

void* GetPrsenDtSenseAnim(PRSEN* pprsen)
{
	return &pprsen->dtSenseAnim;
}

void SetPrsenDtSenseAnim(PRSEN* pprsen, float dtSenseAnim)
{
	pprsen->dtSenseAnim = dtSenseAnim;
}

void* GetPrsenPssatSense(PRSEN* pprsen)
{
	return &pprsen->pssatSense;
}

void SetPrsenPssatSense(PRSEN* pprsen, PSSAT pssatSense)
{
	pprsen->pssatSense = pssatSense;
}

void* GetPrsenIframeDamageStart(PRSEN* pprsen)
{
	return &pprsen->iframeDamageStart;
}

void SetPrsenIframeDamageStart(PRSEN* pprsen, int iframeDamageStart)
{
	pprsen->iframeDamageStart = iframeDamageStart;
}

void* GetPrsenIframeDamageEnd(PRSEN* pprsen)
{
	return &pprsen->iframeDamageEnd;
}

void SetPrsenIframeDamageEnd(PRSEN* pprsen, int iframeDamageEnd)
{
	pprsen->iframeDamageEnd = iframeDamageEnd;
}

void* GetPrsenDtDamageAnim(PRSEN* pprsen)
{
	return &pprsen->dtDamageAnim;
}

void SetPrsenDtDamageAnim(PRSEN* pprsen, float dtDamageAnim)
{
	pprsen->dtDamageAnim = dtDamageAnim;
}

void* GetPrsenPssatDamage(PRSEN* pprsen)
{
	return &pprsen->pssatDamage;
}

void SetPrsenPssatDamage(PRSEN* pprsen, PSSAT pssatDamage)
{
	pprsen->pssatDamage = pssatDamage;
}

void* GetPrsenIframeDisabledStart(PRSEN* pprsen)
{
	return &pprsen->iframeDisabledStart;
}

void SetPrsenIframeDisabledStart(PRSEN* pprsen, int iframeDisabledStart)
{
	pprsen->iframeDisabledStart = iframeDisabledStart;
}

void* GetPrsenIframeDisabledEnd(PRSEN* pprsen)
{
	return &pprsen->iframeDisabledEnd;
}

void SetPrsenIframeDisabledEnd(PRSEN* pprsen, int iframeDisabledEnd)
{
	pprsen->iframeDisabledEnd = iframeDisabledEnd;
}

void* GetPrsenDtDisabledAnim(PRSEN* pprsen)
{
	return &pprsen->dtDisabledAnim;
}

void SetPrsenDtDisabledAnim(PRSEN* pprsen, float dtDisabledAnim)
{
	pprsen->dtDisabledAnim = dtDisabledAnim;
}

void* GetPrsenPssatDisabled(PRSEN* pprsen)
{
	return &pprsen->pssatDisabled;
}

void SetPrsenPssatDisabled(PRSEN* pprsen, PSSAT pssatDisabled)
{
	pprsen->pssatDisabled = pssatDisabled;
}

void* GetPrsenIframeDisablingFlash(PRSEN* pprsen)
{
	return &pprsen->iframeDisablingFlash;
}

void SetPrsenIframeDisablingFlash(PRSEN* pprsen, int iframeDisablingFlash)
{
	pprsen->iframeDisablingFlash = iframeDisablingFlash;
}

void* GetPrsenSvtDisablingFlash(PRSEN* pprsen)
{
	return &pprsen->svtDisablingFlash;
}

void SetPrsenSvtDisablingFlash(PRSEN* pprsen, float svtDisablingFlash)
{
	pprsen->svtDisablingFlash = svtDisablingFlash;
}

void* GetPrsenDtRemainEnabled(PRSEN* pprsen)
{
	return &pprsen->dtRemainEnabled;
}

void SetPrsenDtRemainEnabled(PRSEN* pprsen, float dtRemainEnabled)
{
	pprsen->dtRemainEnabled = dtRemainEnabled;
}

void* GetPrsenDtRemainDisabled(PRSEN* pprsen)
{
	return &pprsen->dtRemainDisabled;
}

void SetPrsenDtRemainDisabled(PRSEN* pprsen, float dtRemainDisabled)
{
	pprsen->dtRemainDisabled = dtRemainDisabled;
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

void PostPrsenLoad(PRSEN* pprsen)
{
	PostAloLoad(pprsen);

	for (int i = 0; i < pprsen->globset.cpsaa; ++i) {
		SAA* psaa = pprsen->globset.apsaa[i];

		if (psaa->saak == SAAK_Loop && psaa->sai.pshd != nullptr) {
			pprsen->ploop = (LOOP*)psaa;
			break;
		}
	}

	pprsen->pvtprsen->pfnSetPrsenSensors(pprsen, pprsen->sensorsInitial);
}

void UpdatePrsen(PRSEN* pprsen, float dt)
{
	UpdateSensor(pprsen, dt);

	SENSORS sensors = pprsen->sensors;
	float dtSensors = g_clock.t - pprsen->tSensors;
	bool fSense = false;

	switch (sensors) 
	{
		case SENSORS_SenseEnabling:
		if (pprsen->dtEnabling < dtSensors) {
			sensors = SENSORS_SenseEnabled;
		}
		break;

		case SENSORS_SenseEnabled:
		if (pprsen->dtRemainEnabled >= 0.0f && pprsen->dtRemainEnabled < dtSensors) {
			sensors = SENSORS_SenseDisabling;
		}
		else {
			fSense = true;
		}
		break;

		case SENSORS_SenseDisabling:
		if (pprsen->dtDisabling < dtSensors) {
			sensors = SENSORS_Disabled;
		}
		break;

		case SENSORS_DamageEnabling:
		if (pprsen->dtEnabling < dtSensors) {
			sensors = SENSORS_DamageEnabled;
		}
		break;

		case SENSORS_DamageEnabled:
		if (pprsen->dtRemainEnabled >= 0.0f && pprsen->dtRemainEnabled < dtSensors) {
			sensors = SENSORS_DamageDisabling;
		}
		else {
			fSense = true;
		}
		break;

		case SENSORS_DamageDisabling:
		if (pprsen->dtDisabling < dtSensors) {
			sensors = SENSORS_Disabled;
		}
		break;

		case SENSORS_Disabled:
		if (pprsen->fRemainDisabledIndefinite == 0 && pprsen->dtRemainDisabled < dtSensors) {
			if (pprsen->sensm == SENSM_SenseOnly) {
				sensors = SENSORS_SenseEnabling;
			}
			else if (pprsen->sensm == SENSM_DamageTarget) {
				sensors = SENSORS_DamageEnabling;
			}
		}
		break;

		default:
		break;
	}

	if (fSense && pprsen->pvtprsen->pfnSensePrsen != nullptr) {
		pprsen->pvtprsen->pfnSensePrsen(pprsen, &sensors);
	}

	pprsen->pvtprsen->pfnSetPrsenSensors(pprsen, sensors);
	UpdatePrsenLoopShader(pprsen);
}

void SensePrsen(PRSEN* pprsen, SENSORS* psensors)
{
	SO* root = (SO*)pprsen->paloRoot;

	for (OX* ox = root->poxa->pox; ox != nullptr; ox = ox->poxNext) {
		for (XP* xp = ox->pxp; xp != nullptr; xp = xp->pxpNext) {
			int sensorSide = root != xp->axpd[0].psoRoot;
			JT* other = (JT*)xp->axpd[1 - sensorSide].psoRoot;

			if ((PRSEN*)xp->axpd[sensorSide].psoLeaf != pprsen) {
				continue;
			}

			if (!FCheckSensorObject((SENSOR*)pprsen, (SO*)other)) {
				continue;
			}

			if (!pprsen->fTriggered) {
				*psensors = pprsen->sensm == SENSM_SenseOnly ? SENSORS_SenseTriggered : SENSORS_DamageTriggered;
			}

			if (*psensors != SENSORS_DamageEnabled &&
				*psensors != SENSORS_DamageTriggered) {
				continue;
			}

			if (other == g_pjt) {
				ZPR zpr;

				InitZpr(&zpr, ZPK_Electric, (LO*)pprsen);

				if (g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zpr)) {
					PauseSensor(pprsen);
				}
			}
			else 
			{
				WKR wkr{};

				wkr.pos = other->xf.posWorld;
				wkr.v = g_normalZ;
				wkr.ploSource = (LO*)pprsen;
				wkr.ploTarget = (LO*)other;

				PaloAbsorbWkr(&wkr, 0, nullptr);
			}
		}
	}
}

void EnablePrsen(PRSEN* pprsen, SENSM sensm)
{
	pprsen->sensm = sensm;

	SENSORS sensors = sensm == SENSM_SenseOnly ? SENSORS_SenseEnabling : SENSORS_DamageEnabling;
	pprsen->pvtprsen->pfnSetPrsenSensors(pprsen, sensors);
}

void DisablePrsen(PRSEN* pprsen)
{
	if (pprsen->sensors != SENSORS_Disabled) {
		SENSORS sensors = pprsen->sensm == SENSM_SenseOnly ? SENSORS_SenseDisabling : SENSORS_DamageDisabling;
		pprsen->pvtprsen->pfnSetPrsenSensors(pprsen, sensors);
	}

	ALARM* alarm = pprsen->palarm;

	if (alarm != nullptr && alarm->alarms == ALARMS_Disabled && glm::abs(g_clock.t - alarm->tAlarms) < 0.2f) {
		pprsen->fRemainDisabledIndefinite = true;
	}
}

void OnPrsenAlarmTriggered(PRSEN* pprsen)
{
	pprsen->sensm = SENSM_DamageTarget;

	if (pprsen->dtRemainEnabled >= 0.0f) {
		return;
	}

	pprsen->fTriggered = false;

	if (pprsen->sensors == SENSORS_SenseEnabled) {
		pprsen->pvtprsen->pfnSetPrsenSensors(pprsen, SENSORS_SenseDisabling);
	}
	else if (pprsen->sensors == SENSORS_DamageEnabled) {
		pprsen->pvtprsen->pfnSetPrsenSensors(pprsen, SENSORS_DamageDisabling);
	}
}

void SetPrsenSensors(PRSEN* pprsen, SENSORS sensors)
{
	if (pprsen->sensors == sensors) {
		return;
	}

	switch (pprsen->sensors) 
	{
		case SENSORS_SenseEnabled:
		case SENSORS_DamageEnabled:
		StopSound(pprsen->pamb, 0);
		pprsen->tSensePrev = pprsen->tSensors;
		break;

		case SENSORS_Disabled:
		pprsen->fRemainDisabledIndefinite = false;
		pprsen->fTriggered = false;
		break;

		default:
		break;
	}

	switch (sensors) 
	{
		case SENSORS_SenseEnabled:
		StartSound((SFXID)15, &pprsen->pamb, (ALO*)pprsen, nullptr, 2000.0f, 500.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
		break;

		case SENSORS_SenseTriggered:
		case SENSORS_DamageTriggered:
		pprsen->fTriggered = true;
		break;

		case SENSORS_DamageEnabled:
		StartSound((SFXID)27, &pprsen->pamb, (ALO*)pprsen, nullptr, 2000.0f, 500.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
		break;

		default:
		break;
	}

	SetSensorSensors(pprsen, sensors);

	if (!pprsen->fTriggered) {
		return;
	}

	switch (sensors) 
	{
		case SENSORS_SenseTriggered:
		pprsen->tSensors = pprsen->tSensePrev;
		pprsen->sensors = SENSORS_SenseEnabled;
		break;

		case SENSORS_DamageTriggered:
		pprsen->tSensors = pprsen->tSensePrev;
		pprsen->sensors = SENSORS_DamageEnabled;
		break;

		default:
		break;
	}
}

void UpdatePrsenLoopShader(PRSEN* pprsen)
{
	int frameStart;
	int frameEnd;
	int frameCount;
	int frameOffset = 0;
	int cycleLength;
	float animDuration;
	float animStartTime;
	PSSAT saturation;

	switch (pprsen->sensors) 
	{
		case SENSORS_SenseEnabling:
		case SENSORS_DamageEnabling:
		case SENSORS_Disabled:
		frameStart = pprsen->iframeDisabledStart;
		frameEnd = pprsen->iframeDisabledEnd;
		animDuration = pprsen->dtDisabledAnim;
		saturation = pprsen->pssatDisabled;
		animStartTime = pprsen->tSensors;
		break;

		case SENSORS_SenseEnabled:
		frameStart = pprsen->iframeSenseStart;
		frameEnd = pprsen->iframeSenseEnd;
		animDuration = pprsen->dtSenseAnim;
		saturation = pprsen->pssatSense;
		animStartTime = pprsen->tSensors;
		break;

		case SENSORS_SenseDisabling:
		frameOffset =
			(int)((g_clock.t - pprsen->tSensors) /
				(1.0f / pprsen->svtDisablingFlash)) % 2;

		if (frameOffset == 0) {
			frameStart = pprsen->iframeDisablingFlash;
			frameEnd = frameStart;
		}
		else {
			frameStart = pprsen->iframeSenseStart;
			frameEnd = pprsen->iframeSenseEnd;
			animDuration = pprsen->dtSenseAnim;
			saturation = pprsen->pssatSense;
			animStartTime = pprsen->tSensePrev;
		}
		break;

		case SENSORS_DamageEnabled:
		frameStart = pprsen->iframeDamageStart;
		frameEnd = pprsen->iframeDamageEnd;
		animDuration = pprsen->dtDamageAnim;
		saturation = pprsen->pssatDamage;
		animStartTime = pprsen->tSensors;
		break;

		case SENSORS_DamageDisabling:
		frameOffset =
			(int)((g_clock.t - pprsen->tSensors) /
				(1.0f / pprsen->svtDisablingFlash)) % 2;

		if (frameOffset == 0) {
			frameStart = pprsen->iframeDisablingFlash;
			frameEnd = frameStart;
		}
		else {
			frameStart = pprsen->iframeDamageStart;
			frameEnd = pprsen->iframeDamageEnd;
			animDuration = pprsen->dtDamageAnim;
			saturation = pprsen->pssatDamage;
			animStartTime = pprsen->tSensePrev;
		}
		break;
	}

	frameCount = frameEnd - frameStart + 1;

	if (frameCount != 1 && animDuration > 0.0f) {
		if (saturation == PSSAT_Loop) {
			if (frameCount == 0) {
				//trap(7);
			}

			frameOffset =
				(int)((g_clock.t - animStartTime) /
					(animDuration / (float)frameCount)) %
				frameCount;

			frameStart += frameOffset;
		}
		else if (saturation == PSSAT_PingPong) {
			cycleLength = frameCount * 2 - 2;

			if (cycleLength == 0) {
				//trap(7);
			}

			frameOffset =
				(int)((g_clock.t - animStartTime) /
					(animDuration / (float)cycleLength)) %
				cycleLength;

			if (frameOffset < frameCount) {
				frameStart += frameOffset;
			}
			else {
				frameStart += frameCount * 2 - frameOffset - 2;
			}
		}
		else {
			/*
			 * The decompilation uses in_a1_lo here. Its source is unclear
			 * outside the disabling states, so this preserves that behavior.
			 */
			frameStart += frameOffset;
		}
	}

	SetSaiIframe(&pprsen->ploop->sai, frameStart);
}

void DeletePrsen(PRSEN* ppprsen)
{
	delete ppprsen;
}

SNIP s_asnipLasen[2] =
{
	2, (OID)650, offsetof(LASEN, paloRenderSense),
	2, (OID)651, offsetof(LASEN, paloRenderDamage)
};

int g_fLasenBusyListChange = 0;

SNIP s_asnipCamsen[2] = 
{
	{ 0x02, (OID)0x28C, offsetof(CAMSEN, paloRenderDamage) },
	{ 0x02, (OID)0x28D, offsetof(CAMSEN, paloRenderZap) },
};

#include "sensor.h"
#include "emitter.h"
#include "shape.h"

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
			//TriggerAlarm(psensor->palarm, ALTK_Trigger);

			// Preserve original behavior: if TriggerAlarm changed psensor->sensors,
			// we keep that updated state instead of forcing Triggered.
			if (psensor->sensors != SENSORS_SenseEnabled)
				finalState = psensor->sensors;
		}

		//HandleLoSpliceEvent(psensor, 2, 0, NULL);
		psensor->sensors = finalState;
	}
	else
	{
		psensor->sensors = sensors;
	}

	psensor->tSensors = g_clock.t;
}

void UpdateSensor(SENSOR* psensor, float dt)
{
	UpdateSo(psensor, dt);
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

	plasen->sensorsInitial = SENSORS_SenseEnabled;
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
			if (iBeam < 0x10)
			{
				CRVL* pcrvl = (CRVL*)pemitter->dlChild.ploFirst;
				if (pcrvl && pcrvl->crvk == CRVK_Linear)
				{
					plasen->clbeam = iBeam + 1;
					plasen->albeam[iBeam].pshape = (SHAPE*)pemitter;
					plasen->albeam[iBeam].sShape = pcrvl->mpicvs[pcrvl->ccv + -1];

					// cposBeamShapeMax = max(cposBeamShapeMax, pcrvl->ccv)
					if (plasen->cposBeamShapeMax < pcrvl->ccv)
						plasen->cposBeamShapeMax = pcrvl->ccv;

					// Expand plasen->sRadiusRenderAll to include all control points in world space
					for (int i = 0; i < pcrvl->ccv; ++i)
					{
						glm::vec3 posCvWorld{};

						ConvertAloPos(plasen, nullptr, pcrvl->mpicvpos[i], posCvWorld);

						// distance = |plasen->xf.posWorld - posCvWorld|
						// (spelled out to avoid extra helpers)
						float dx = ((plasen->xf).posWorld.x - posCvWorld.x);
						float dy = ((plasen->xf).posWorld.y - posCvWorld.y);
						float dz = ((plasen->xf).posWorld.z - posCvWorld.z);
						float dist = sqrtf(dx * dx + dy * dy + dz * dz);

						if (plasen->sRadiusRenderAll < dist)
							plasen->sRadiusRenderAll = dist;
					}
				}
			}
		}

		// Advance using the same DLI pointer-walk the decomp does
		pemitter = reinterpret_cast<EMITTER*>(*dli.m_ppv);
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pemitter) + dli.m_ibDle);
	}

	// Clip current lasen sphere unless flagged as "no vismap clip" (matches the mask test)
	if ((*(unsigned long*)&plasen->bitfield & 0x03000000) != 0x02000000)
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

			if ((*(unsigned long*)&parent->bitfield & 0x03000000) != 0x02000000)
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

	//for (int i = 0; i < plasen->clbeam; ++i) {
	//	LBEAM* pbeam = &plasen->albeam[i];

	//	// Remove the shape's LO
	//	if (pbeam->pshape && pbeam->pshape->pvtlo->pfnRemoveLo) {
	//		pbeam->pshape->pvtlo->pfnRemoveLo((LO*)pbeam);
	//	}

	//	if (i == 0) {
	//		// Skip cloning damage emitters for beam 0 (used as source template)
	//		continue;
	//	}

	//	int clemitDamage = plasen->albeam[0].clemitDamage;
	//	pbeam->clemitDamage = clemitDamage;

	//	if (clemitDamage > 0) {
	//		LEMIT* dstEmit = pbeam->alemitDamage;
	//		LEMIT* srcEmit = plasen->albeam[0].alemitDamage;

	//		for (int j = 0; j < clemitDamage; ++j) {
	//			EMITTER* pemitter = (EMITTER*)PloCloneLo((LO*)srcEmit->pemitter, plasen->psw, (ALO*)plasen);
	//			dstEmit->pemitter = pemitter;

	//			if (pemitter && pemitter->pvtalo->pfnBindAlo) {
	//				pemitter->pvtalo->pfnBindAlo(pemitter);
	//			}

	//			if (pemitter && pemitter->pvtlo->pfnPostLoLoad) {
	//				pemitter->pvtlo->pfnPostLoLoad(pemitter);
	//			}

	//			dstEmit->fScorch = srcEmit->fScorch;

	//			++dstEmit;
	//			++srcEmit;
	//		}
	//	}
	//}

	// Set jtOnlyTriggerObject flag based on g_pjt
	//plasen->fJtOnlyTriggerObject = FOnlySensorTriggerObject(plasen, g_pjt);

	plasen->pvtlasen->pfnSetLasenSensors(plasen, plasen->sensorsInitial);
}

void UpdateLasen(LASEN* plasen, float dt)
{
	// Base sensor update
	UpdateSensor(plasen, dt);

	// uDrawMax = clamp01(uDrawMax + dt * svuDrawMax)
	float u = plasen->uDrawMax + dt * plasen->svuDrawMax;
	if (u < 0.0f) u = 0.0f;
	else if (u > 1.0f) u = 1.0f;

	plasen->uDrawMax = u;

	/*if (g_fLasenBusyListChange != 0) {
		UpdateBusyLasenSenseTimes();
	}*/

	SENSORS sensors = plasen->sensors;

	// State machine
	switch (sensors)
	{
	case SENSORS_SenseEnabling:
	{
		if (plasen->dtEnabling < (g_clock.t - plasen->tSensors)) {
			sensors = SENSORS_SenseEnabled;
		}

		// Always call OnPoActive (if any) after this case in the original flow
		/*code* onActive = (code*)plasen->field0_0x0.pvtpo->pfnOnPoActive;
		if (onActive != nullptr) {
			(*onActive)(plasen, &sensors);
		}*/
		break;
	}

	case SENSORS_SenseEnabled:
	case SENSORS_DamageEnabled:
	case SENSORS_DamageTriggered:
	{
		// Falls through to OnPoActive call in original
		/*code* onActive = (code*)plasen->field0_0x0.pvtpo->pfnOnPoActive;
		if (onActive != nullptr) {
			(*onActive)(plasen, &sensors);
		}*/
		break;
	}

	case SENSORS_SenseTriggered:
	{
		sensors = SENSORS_SenseDisabling;

		/*code* onActive = (code*)plasen->field0_0x0.pvtpo->pfnOnPoActive;
		if (onActive != nullptr) {
			(*onActive)(plasen, &sensors);
		}*/
		break;
	}

	case SENSORS_SenseDisabling:
	{
		if (plasen->dtDisabling < (g_clock.t - plasen->tSensors))
		{
			// If SenseOnly, it goes straight to Disabled
			if (plasen->sensm == SENSM_SenseOnly)
			{
				sensors = SENSORS_Disabled;
			}
			else
			{
				// If DamageTarget, transition to DamageEnabling and call OnPoActive immediately
				if (plasen->sensm == SENSM_DamageTarget)
				{
					sensors = SENSORS_DamageEnabling;
					/*code* onActive = (code*)plasen->field0_0x0.pvtpo->pfnOnPoActive;
					if (onActive != nullptr) {
						(*onActive)(plasen, &sensors);
					}*/
				}
				// else: sensors stays SenseDisabling, no special transition here
			}
		}
		else
		{
			// Not finished disabling yet: call OnPoActive
			/*code* onActive = (code*)plasen->field0_0x0.pvtpo->pfnOnPoActive;
			if (onActive != nullptr) {
				(*onActive)(plasen, &sensors);
			}*/
		}

		// If we didn't do the "DamageTarget -> DamageEnabling" immediate call above,
		// the original still calls OnPoActive via the common tail path.
		// We preserve that by calling it again only when we didn't already call it
		// in the DamageTarget transition.
		if (!((plasen->dtDisabling < (g_clock.t - plasen->tSensors)) &&
			(plasen->sensm != SENSM_SenseOnly) &&
			(plasen->sensm == SENSM_DamageTarget)))
		{
			/*code* onActive = (code*)plasen->field0_0x0.pvtpo->pfnOnPoActive;
			if (onActive != nullptr) {
				(*onActive)(plasen, &sensors);
			}*/
		}
		break;
	}

	case SENSORS_DamageEnabling:
	{
		if (plasen->dtEnabling < (g_clock.t - plasen->tSensors)) {
			sensors = SENSORS_DamageEnabled;
		}

		/*code* onActive = (code*)plasen->field0_0x0.pvtpo->pfnOnPoActive;
		if (onActive != nullptr) {
			(*onActive)(plasen, &sensors);
		}*/
		break;
	}

	case SENSORS_DamageDisabling:
	{
		if (plasen->dtDamageDisabling < (g_clock.t - plasen->tSensors))
		{
			sensors = SENSORS_Disabled;
			// Note: original does NOT call OnPoActive after forcing Disabled (it jumps to default tail)
		}
		else
		{
			// In this state, original calls OnPoActive only while not finished disabling
			/*code* onActive = (code*)plasen->field0_0x0.pvtpo->pfnOnPoActive;
			if (onActive != nullptr) {
				(*onActive)(plasen, &sensors);
			}*/
		}
		break;
	}

	default:
	{
		// Original jumps to a shared tail; effectively just skips the per-case call.
		break;
	}
	}

	// Always call pfnFTakePoDamage(plasen, sensors) at the end
	//((code*)plasen->field0_0x0.pvtpo->pfnFTakePoDamage)(plasen, sensors);
}

void SetLasenSensors(LASEN* plasen, SENSORS sensors)
{
	if (plasen->sensors == sensors)
		return;

	SetSensorSensors(plasen, sensors);
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
			ConvertAloPos(plasen, nullptr, crv->mpicvpos[i], posBuf[i]);
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

void PostCamsenLoad(CAMSEN* pcamsen)
{
	PostAloLoad(pcamsen);
}

void SetCamsenCsdts(CAMSEN* pcamsen, CSDTS csdts)
{
	if (!pcamsen)
		return;

	// No change
	if (pcamsen->csdts == csdts)
		return;

	// Leaving Zap -> stop looping zap sound
	//if (pcamsen->csdts == CSDTS_Zap)
	//	StopSound(pcamsen->pamb, 0);

	//// Entering Focus -> raise priority
	//if (csdts == CSDTS_Focus)
	//{
	//	if (pcamsen->pactla)
	//		pcamsen->pactla->nPriorityEnabled = 4;

	//	pcamsen->csdts = csdts;
	//	pcamsen->tCsdts = g_clock.t;
	//	return;
	//}

	//// Entering Zap -> start zap sound and force state to Zap
	//if (csdts == CSDTS_Zap)
	//{
	//	StartSound(SFXID_EnvEzap, &pcamsen->pamb, (ALO*)0x0, (VECTOR*)0x0,
	//		3000.0f,
	//		300.0f,
	//		1.0f,
	//		0.0f,
	//		0.0f,
	//		(LM*)0x0,
	//		(LM*)0x0
	//	);

	//	pcamsen->csdts = CSDTS_Zap;
	//	pcamsen->tCsdts = g_clock.t;
	//	return;
	//}

	//// Entering Unfocus -> drop priority
	//if (csdts == CSDTS_Unfocus)
	//{
	//	if (pcamsen->pactla)
	//		pcamsen->pactla->nPriorityEnabled = 0;

	//	pcamsen->csdts = csdts;
	//	pcamsen->tCsdts = g_clock.t;
	//	return;
	//}

	// Any other state
	pcamsen->csdts = csdts;
	pcamsen->tCsdts = g_clock.t;
}

void UpdateCamsen(CAMSEN* pcamsen, float dt)
{
	if (!pcamsen)
		return;

	UpdateSensor((SENSOR*)pcamsen, dt);

	SENSORS sensors = pcamsen->sensors;

	switch (sensors)
	{
	case SENSORS_SenseEnabling:
	{
		if (pcamsen->dtEnabling < (g_clock.t - pcamsen->tSensors))
			sensors = SENSORS_SenseEnabled;
		break;
	}

	case SENSORS_SenseEnabled:
	case SENSORS_DamageEnabled:
	{
		// If the PO has an "active" callback, let it modify the sensor state.
		/*auto pfnOnPoActive = pcamsen->pvtpo ? pcamsen->pvtpo->pfnOnPoActive : nullptr;
		if (pfnOnPoActive)
			pfnOnPoActive(pcamsen, &sensors);*/

		break;
	}

	case SENSORS_SenseTriggered:
	{
		sensors = SENSORS_SenseDisabling;
		break;
	}

	case SENSORS_SenseDisabling:
	{
		if (pcamsen->dtDisabling < (g_clock.t - pcamsen->tSensors))
			sensors = SENSORS_Disabled;
		break;
	}

	case SENSORS_DamageEnabling:
	{
		if (pcamsen->dtEnabling < (g_clock.t - pcamsen->tSensors))
			sensors = SENSORS_DamageEnabled;
		break;
	}

	case SENSORS_DamageTriggered:
	{
		CSDTS csdts = pcamsen->csdts;

		if (csdts == CSDTS_Zap)
		{
			if (pcamsen->dtDamageZap < (g_clock.t - pcamsen->tCsdts))
				csdts = CSDTS_Unfocus;
		}
		else if (csdts == CSDTS_Focus)
		{
			bool invulnerable = false;

			if (g_pjt)
			{
				// pfnFInvulnerablePo(g_pjt, 2) != 0 means invulnerable
				//invulnerable = (g_pjt->pvtpo && g_pjt->pvtpo->pfnFInvulnerablePo) && (g_pjt->pvtpo->pfnFInvulnerablePo(g_pjt, 2) != 0);
			}

			if (invulnerable)
			{
				csdts = CSDTS_Unfocus;
			}
			else
			{
				if (pcamsen->dtDamageFocus < (g_clock.t - pcamsen->tCsdts))
				{
					if (!g_pjt)
					{
						csdts = CSDTS_Zap;
					}
					else
					{
						ZPR zpr{};
						/*InitZpr(&zpr, ZPK_Fire, (LO*)pcamsen);

						long tookDamage = 0;
						if (g_pjt->pvtpo && g_pjt->pvtpo->pfnFTakePoDamage)
							tookDamage = g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zpr);

						if (tookDamage != 0)
						{
							csdts = CSDTS_Zap;
							PauseSensor((SENSOR*)pcamsen);
						}*/
					}
				}
			}
		}
		else if (csdts == CSDTS_Unfocus)
		{
			if (pcamsen->dtDamageUnfocus < (g_clock.t - pcamsen->tCsdts))
			{
				csdts = CSDTS_Nil;
				sensors = SENSORS_DamageEnabled;
			}
		}

		//SetCamsenCsdts(pcamsen, csdts);
		break;
	}

	case SENSORS_DamageDisabling:
	{
		if (pcamsen->dtDisabling < (g_clock.t - pcamsen->tSensors))
			sensors = SENSORS_Disabled;
		break;
	}

	case SENSORS_Disabled:
	{
		if (pcamsen->fRemainDisabledIndefinite != 0)
		{
			// remain disabled; no state change
			break;
		}

		if (pcamsen->sensm == SENSM_SenseOnly)
		{
			sensors = SENSORS_SenseEnabling;
		}
		else if (pcamsen->sensm == SENSM_DamageTarget)
		{
			sensors = SENSORS_DamageEnabling;
		}
		// else: leave disabled
		break;
	}

	default:
		break;
	}

}

void RenderCamsenSelf(CAMSEN* pcamsen, CM* pcm, RO* pro)
{
	if (!pcamsen || !pcm || !pro)
		return;

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

	LoadMatrixFromPosRotScale(pcamsen->xf.posWorld, pcamsen->xf.matWorld, scale, ro.model);
	renderAlo->pvtalo->pfnRenderAloGlobset(renderAlo, pcm, &ro);
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

void PostPrsenLoad(PRSEN* pprsen)
{
	PostAloLoad(pprsen);
}

void UpdatePrsen(PRSEN* pprsen, float dt)
{
	UpdateSensor(pprsen, dt);
}

void DeletePrsen(PRSEN *ppprsen)
{
	delete ppprsen;
}

float SCalcLasenShapeExtent(LASEN* plasen, LBEAM* plbeam)
{
	const float now = g_clock.t;
	const float shape = plbeam->sShape;
	const float shapeLast = plbeam->sShapeLast;
	const float drawMax = plasen->uDrawMax;

	float extentFactor = 0.0f;

	switch (plasen->sensors)
	{
		// --- Enabling (ramping up) ---
	case SENSORS_SenseEnabling:
	case SENSORS_DamageEnabling:
		if (plasen->dtEnabling == 0.0f)
		{
			// Instant on if no enable duration
			extentFactor = drawMax;
			break;
		}
		else
		{
			const float t = (now - plasen->tSensors) / plasen->dtEnabling;
			const float max = drawMax;

			if (t < 0.0f)
			{
				// Not started yet
				extentFactor = 0.0f;
			}
			else if (t <= max)
			{
				// In ramp-up phase: directly use t as factor
				return shape * t;
			}
			else
			{
				// Past ramp: clamp to max
				extentFactor = max;
			}
		}
		break;

		// --- Fully off ---
	case SENSORS_Disabled:
		extentFactor = 0.0f;
		break;

		// --- Fully on / triggered ---
	case SENSORS_SenseEnabled:
	case SENSORS_SenseTriggered:
	case SENSORS_DamageEnabled:
	case SENSORS_DamageTriggered:
		extentFactor = drawMax;
		break;

		// --- Disabling (ramping down) ---
	case SENSORS_SenseDisabling:
	case SENSORS_DamageDisabling:
	{
		float dt = (plasen->sensm == SENSM_SenseOnly)
			? plasen->dtDisabling
			: plasen->dtDamageDisabling;

		float factor = 0.0f;

		if (dt != 0.0f)
		{
			factor = 1.0f - (now - plasen->tSensors) / dt;

			if (factor < 0.0f)
				factor = 0.0f;
			else if (factor > drawMax)
				factor = drawMax;
		}

		// During disabling we use the *previous* shape value
		return shapeLast * factor;
	}

	default:
		// Unknown state: treat as off
		extentFactor = 0.0f;
		break;
	}

	// All non-disabling paths use the current shape
	return shape * extentFactor;
}

SNIP s_asnipLasen[2] =
{
	2, (OID)650, offsetof(LASEN, paloRenderSense),
	2, (OID)651, offsetof(LASEN, paloRenderDamage)
};
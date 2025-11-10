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

	// --- DLI iterator setup ---
	DLI dli{};
	dli.m_pdl   = &plasen->dlChild;
	dli.m_ibDle = plasen->dlChild.ibDle;

	EMITTER* pemitter = (EMITTER*)plasen->dlChild.paloFirst;

	dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pemitter) + dli.m_ibDle);

	dli.m_pdliNext = s_pdliFirst;
	s_pdliFirst = &dli;

	while (pemitter != nullptr)
	{
		const bool isShape   = FIsBasicDerivedFrom(pemitter, CID_SHAPE)   != 0;
		const bool isEmitter = FIsBasicDerivedFrom(pemitter, CID_EMITTER) != 0;

		if (!isShape)
		{
			if (isEmitter)
			{
				uint32_t n = plasen->albeam[0].clemitDamage;
				if (n < 4)
				{
					plasen->albeam[0].clemitDamage = n + 1;
					LEMIT* le = plasen->albeam[0].alemitDamage + n;
					le->pemitter = pemitter;
					le->fScorch =(pemitter->oid == 649) ? 1u : 0u;
					PauseEmitterIndefinite(pemitter);
				}
			}
		}
		else
		{
			uint32_t idx = plasen->clbeam;
			SHAPE *pshape = pemitter->dlChild.pshapeFirst;

			if (idx < 0x10 && pshape->psidebag == nullptr)
			{
				plasen->clbeam = idx + 1;
				plasen->albeam[idx].pshape = (SHAPE*)pshape;

				//plasen->albeam[idx].sShape = ;

				//int posCount = pshape->pcrv.get()->ccv;
				int posCount = 0;

				plasen->cposBeamShapeMax = std::max(plasen->cposBeamShapeMax, posCount);

				if (posCount > 0)
				{
					ALO* parent = pshape->paloParent;
					for (int i = 0; i < posCount; ++i)
					{
						glm::vec3 cvWorld;
						//glm::vec3 *src = (glm::vec3*)pshape->pcrv.get()->mpicvpos;

						//ConvertAloPos(plasen, nullptr, *src, cvWorld);

						float d = glm::length(plasen->xf.posWorld - cvWorld);
						plasen->sRadiusRenderAll = std::max(plasen->sRadiusRenderAll, d);
					}
				}
			}
		}

		pemitter = reinterpret_cast<EMITTER*>(*dli.m_ppv);
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pemitter) + dli.m_ibDle);
	}

	if ((reinterpret_cast<unsigned long&>(plasen->bitfield) & 0x3000000UL) != 0x2000000UL)
	{
		//ClipVismapSphereOneHop(plasen->psw->pvismap, &plasen->xf.posWorld, plasen->sRadiusRenderAll, &plasen->grfzon);
	}

	while (plasen->paloParent != nullptr)
	{
		float sRadius = glm::length(plasen->xf.pos) + plasen->sRadiusRenderAll;
		ALO* parent = plasen->paloParent;

		if (sRadius <= parent->sRadiusRenderAll)
			break;

		parent->sRadiusRenderAll = sRadius;

		if ((reinterpret_cast<unsigned long&>(parent->bitfield) & 0x3000000UL) != 0x2000000UL)
		{
			//ClipVismapSphereOneHop(parent->psw->pvismap, &parent->xf.posWorld, sRadius, &parent->grfzon);
		}

		plasen = (LASEN*)parent;
	}

	s_pdliFirst = dli.m_pdliNext;
}

void RenderLasenSelf(LASEN* plasen, CM* pcm, RO* pro)
{
	RenderSoSelf(plasen, pcm, pro);

	//SENSORS sensors = plasen->sensors;
	//if (sensors == SENSORS_Disabled || sensors <= SENSORS_Nil) return;

	//// Choose which ALO to render the lines with
	//ALO* paloRender = nullptr;
	//if (sensors < SENSORS_DamageEnabling) {
	//	paloRender = plasen->paloRenderSense;
	//}
	//else {
	//	if (sensors > SENSORS_DamageDisabling) return;
	//	paloRender = plasen->paloRenderDamage;
	//}
	//if (!paloRender) {
	//	paloRender = (ALO*)plasen->psw->aploStock[0x0B];
	//	if (!paloRender) return;
	//}

	//std::vector <glm::vec3> posBuf;
	//posBuf.resize(plasen->cposBeamShapeMax);

	//if (plasen->clbeam > 0)
	//{
	//	for (int ibeam = 0; ibeam < plasen->clbeam; ++ibeam)
	//	{
	//		float drawnLen = 0.0f;

	//		LBEAM* plbeam = &plasen->albeam[ibeam];
	//		CRV* crv = plbeam->pshape->pcrv.get();

	//		int ccv = crv->ccv;
	//		const float extent = SCalcLasenShapeExtent(plasen, plbeam);

	//		// If we had a collision, clamp the max CV processed like original (+2 past hit index)
	//		if (plbeam->psoHit && (plbeam->iposHit + 2) < ccv) {
	//			ccv = plbeam->iposHit + 2;
	//		}

	//		// Convert curve CVs to world space into posBuf[0..ccv-1]
	//		for (int i = 0; i < ccv; ++i) {
	//			// ConvertAloPos(ALO* dstAlo, ALO* srcAloOrNull, VECTOR* src, VECTOR* dst)
	//			ConvertAloPos(plasen, nullptr, crv->mpicvpos[i], posBuf[i]);
	//		}

	//		// If a hit point exists, overwrite posBuf[iposHit+1] with the exact hit
	//		if (plbeam->psoHit) {
	//			const int ip = plbeam->iposHit;
	//			if (ip + 1 < ccv) {
	//				// posHit is assumed to have .x/.y/.z; if different, adjust field names.
	//				posBuf[ip + 1] = glm::vec3(plbeam->posHit.x, plbeam->posHit.y, plbeam->posHit.z);
	//			}
	//		}

	//		// Draw line segments along the curve up to 'extent'
	//		if (ccv > 1)
	//		{
	//			for (int i = 0; i < ccv - 1; ++i)
	//			{
	//				glm::vec3& p0 = posBuf[i];
	//				glm::vec3& p1 = posBuf[i + 1];

	//				float segLen = glm::length(p1 - p0);
	//				if (segLen > 0.0f)
	//				{
	//					const float remaining = extent - drawnLen;
	//					if (remaining < segLen)
	//					{
	//						// Clamp the segment endpoint so we don't overdraw beyond 'extent'
	//						const float t = glm::clamp(remaining / segLen, 0.0f, 1.0f);
	//						p1 = p0 * (1.0f - t) + p1 * t;
	//						segLen = remaining;
	//					}
	//				}

	//				const float alpha = pro ? pro->uAlpha : 1.0f;

	//				// Render this segment
	//				RenderAloLine(paloRender, pcm, &p0, &p1, 1.0f, alpha);

	//				drawnLen += segLen;

	//				// Stop if we finished desired extent
	//				if (extent < drawnLen) break;
	//			}
	//		}

	//		// Update sShapeLast depending on sensor state (mirrors original conditions):
	//		// set if sensors [SenseEnabling, DamageTriggered] and sensors != SenseDisabling
	//		sensors = plasen->sensors;
	//		if (sensors >= SENSORS_SenseEnabling &&
	//			sensors <= SENSORS_DamageTriggered &&
	//			sensors != SENSORS_SenseDisabling)
	//		{
	//			plbeam->sShapeLast = drawnLen;
	//		}
	//	}
	//}
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
	2, (OID)650, offsetof(LASEN, paloRenderSense),
	2, (OID)651, offsetof(LASEN, paloRenderDamage)
};
#include "light.h"

LIGHT* NewLight()
{
	return new LIGHT{};
}

void InitSwLightDl(SW* psw)
{
	InitDl(&psw->dlLight, offsetof(LIGHT, dleLight));
}

void InitLight(LIGHT* plight)
{
	plight->lightk = LIGHTK_Direction;
	*(unsigned long*)&plight->bitfield = *(unsigned long*)&plight->bitfield | 0x10000000000;
	plight->normalLocal.x = 0.0;
	plight->normalLocal.y = 0.0;
	plight->normalLocal.z = 1.0;
	plight->vecUpLocal.x = 0.0;
	plight->vecUpLocal.y = 0.0;
	plight->vecUpLocal.z = 1.0;
	plight->lmFallOffS.gMin = 200.0;
	plight->lmFallOffS.gMax = 2000.0;
	plight->degMidtone = 240.0;
	plight->degShadow = 180.0;
	plight->degCone = 60.0;
	plight->degHighlight = 180.0;

	RebuildLightFrustrum(plight);
	InitAlo(plight);

	numRl++;
}

int GetLightSize()
{
	return sizeof(LIGHT);
}

void OnLightAdd(LIGHT* plight)
{
	OnAloAdd(plight);
	AddLightToSw(plight);
}

void OnLightRemove(LIGHT* plight)
{
	OnAloRemove(plight);
	RemoveLightFromSw(plight);
}

void RebuildLightFrustrum(LIGHT* plight)
{
	float xfov = 0.0;
	if ((0.0 <= plight->degCone) && (xfov = plight->degCone, 175.0 < plight->degCone))
		xfov = 175.0;

	float yfov = 0.0;
	if ((0.0 <= plight->degHotSpot) && (yfov = plight->degHotSpot, plight->degHotSpot <= xfov))
		yfov = plight->degHotSpot;

	plight->rx = tanf(xfov * 0.008726647);
	plight->ry = plight->rx;

	plight->lmFallOffAbsX.gMax = 1.0;
	plight->lmFallOffAbsY.gMax = 1.0;

	plight->lmFallOffAbsX.gMin = tanf(yfov * 0.008726647) / plight->rx;
	plight->lmFallOffAbsY.gMin = tanf(yfov * 0.008726647) / plight->ry;

	plight->lmFallOffPenumbra.gMin = sqrt(tanf(yfov * 0.008726647) / plight->rx);
	plight->lmFallOffPenumbra.gMax = 1.0;
}
void UpdateLightXfWorldHierarchy(LIGHT* plight)
{
	UpdateAloXfWorldHierarchy(plight);
	RebuildLight(plight);
}

void CloneLight(LIGHT* plight, LIGHT* plightBase)
{
	CloneAlo(plight, plightBase);

	// Clone the LIGHTK structure (direct copy)
	plight->lightk = plightBase->lightk;

	// Clone the TWPS structure (direct copy)
	plight->twps = plightBase->twps;

	// Clone the color vectors (HSV and RGBA)
	plight->vecHighlight = plightBase->vecHighlight;
	plight->rgbaColor = plightBase->rgbaColor;

	// Clone the LTFN structure (direct copy)
	plight->ltfn = plightBase->ltfn;

	// Clone the falloff vectors
	plight->agFallOff = plightBase->agFallOff;
	plight->falloff0Frustum = plightBase->falloff0Frustum;
	plight->falloff1Frustum = plightBase->falloff1Frustum;

	// Clone the matrix values (frustum and matLookAt)
	plight->frustum = plightBase->frustum;
	plight->matLookAt = plightBase->matLookAt;

	// Clone the float values
	plight->gMidtone = plightBase->gMidtone;
	plight->gShadow = plightBase->gShadow;
	plight->degHighlight = plightBase->degHighlight;
	plight->degMidtone = plightBase->degMidtone;
	plight->degShadow = plightBase->degShadow;
	plight->degCone = plightBase->degCone;
	plight->degHotSpot = plightBase->degHotSpot;
	plight->rx = plightBase->rx;
	plight->ry = plightBase->ry;

	// Clone the vectors for local direction and normals
	plight->vecDirectionOrig = plightBase->vecDirectionOrig;
	plight->normalLocal = plightBase->normalLocal;
	plight->vecUpLocal = plightBase->vecUpLocal;

	// Clone the LM structures (light falloff and penumbra)
	plight->lmFallOffS = plightBase->lmFallOffS;
	plight->lmFallOffPenumbra = plightBase->lmFallOffPenumbra;
	plight->lmFallOffAbsX = plightBase->lmFallOffAbsX;
	plight->lmFallOffAbsY = plightBase->lmFallOffAbsY;

	// Clone the frustrum vector array (size 6)
	for (int i = 0; i < 6; ++i)
	{
		plight->avecFrustrum[i] = plightBase->avecFrustrum[i];
	}

	// Clone the grfzonBeam (int value)
	plight->grfzonBeam = plightBase->grfzonBeam;

	// Clone the dynamic flags (int values)
	plight->fDynamic = plightBase->fDynamic;
	plight->fExcludeDynamicObjects = plightBase->fExcludeDynamicObjects;

	// Clone the DLE structure (direct copy)
	plight->dleLight = plightBase->dleLight;

	numRl++;
}

void AddLightToSw(LIGHT* plight)
{
	AppendDlEntry(&plight->psw->dlLight, plight);

	plight->pvtalo->pfnUpdateAloXfWorld(plight);
}

void FitLinearFunction(float x0, float y0, float x1, float y1, float &pdu, float &pru)
{
	bool abs = FFloatsNear(x0, x1, 0.0001);
	
	if (abs == 0)
	{
		float fVar2 = (y1 - y0) / (x1 - x0);
		pru = fVar2;
		pdu = y0 - fVar2 * x0;
	}

	else 
	{
		pru = 0.0;
		pdu = y0;
	}
}

void FitRecipFunction(float x0, float y0, float x1, float y1, float* pdu, float* pru)
{
	float fVar2 = 0.0001;
	bool ABS = FFloatsNear(x0, x1, 0.0001);

	if (ABS == 0) {
		if (fVar2 <= abs(x0)) 
		{
			if (fVar2 <= abs(x1)) 
			{
				fVar2 = (x0 * x1 * (y1 - y0)) / (x0 - x1);
				*pru = fVar2;
				*pdu = y0 - fVar2 / x0;
				return;
			}

			*pru = 0.0;
		}

		else
			*pru = 0.0;
	}

	else
		*pru = 0.0;

	*pdu = y0;
}

void ConvertFallOff(LM* plm, float* pdu, float* pru)
{
	FitLinearFunction(plm->gMin, 1.0, plm->gMax, 0.0, *pdu, *pru);
}

void ConvertAngleStrength(float deg0, float g0, float deg1, float g1, float &pdu, float &pru)
{
	float cos0 = cosf(deg0 * 0.008726647);
	float cos1 = cosf(deg1 * 0.008726647);

	FitLinearFunction(cos0 + cos0 * cos0 * cos0, g0 * 0.003921569, cos1 + cos1 * cos1 * cos1, g1 * 0.003921569, pdu, pru);
}

void RebuildLight(LIGHT* plight)
{
	if (plight->degHighlight > 0.0001)
	{
		if (plight->vecHighlight.z <= 0.0001)
		{
			if (0.0001 < plight->degMidtone)
			{
				if (0.0001 < plight->gMidtone)
					plight->twps = TWPS_ShadowMidtone;

				else
					plight->twps = TWPS_Shadow;
			}

			else 
				plight->twps = TWPS_Shadow;
		}

		plight->twps = TWPS_ShadowMidtoneSaturate;
	}

	else
	{
		if (plight->degMidtone > 0.0001)
		{
			if (0.0001 < plight->gMidtone)
				plight->twps = TWPS_ShadowMidtone;

			else
				plight->twps = TWPS_Shadow;
		}

		else
			plight->twps = TWPS_Shadow;
	}
	
	glm::vec3 rgba{};

	if (plight->vecHighlight.z > 0.0001)
	{
		ConvertUserHsvToUserRgb(plight->vecHighlight, rgba);

		plight->agFallOff = rgba;
		plight->agFallOff.x = 1.0 / plight->vecHighlight.z;
		plight->agFallOff.y = 0.0;

		rgba = rgba * plight->agFallOff.x;
	}

	else
		rgba = glm::vec3(0.0f);

	plight->rgbaColor = rgba;

	ConvertAngleStrength(plight->degHighlight, 0.0, 0.0, plight->vecHighlight.z, plight->ltfn.duHighlight, plight->ltfn.ruHighlight);
	ConvertAngleStrength(plight->degMidtone, 0.0, 0.0, plight->gMidtone, plight->ltfn.duMidtone, plight->ltfn.ruMidtone);
	ConvertAngleStrength(360.0 - plight->degShadow, 0.0, 360.0, plight->gShadow, plight->ltfn.duShadow, plight->ltfn.ruShadow);

	if (plight->lightk == LIGHTK_Position)
		FitRecipFunction(plight->lmFallOffS.gMin, 1.0, plight->lmFallOffS.gMax, 0.0, &plight->agFallOff.x, &plight->agFallOff.y);

	else if (plight->lightk == LIGHTK_Frustrum || plight->lightk == LIGHTK_Spot)
	{
		// Compute falloff vector in local space
		glm::vec3 agFallOff;
		ConvertAloVec(plight, nullptr, &plight->normalLocal, &agFallOff);

		agFallOff *= -1.0f;

		// Compute local up vector
		glm::vec3 vecUp;
		ConvertAloVec(plight, nullptr, &plight->vecUpLocal, &vecUp);

		BuildOrthonormalMatrixZ(agFallOff, vecUp, plight->matLookAt);
		BuildFrustrum(plight->matLookAt, plight->rx, plight->ry, (glm::vec3*)plight->avecFrustrum);

		// Add the distance component to the frustum planes
		for (int i = 0; i < 4; ++i)
		{
			glm::vec4 plane = plight->avecFrustrum[i];
			float d = glm::dot(plight->xf.posWorld, glm::vec3(plane));
			plight->avecFrustrum[i].w = -d;
		}

		float falloffMax = plight->lmFallOffS.gMax;
		glm::vec4 nearPlane = glm::vec4(plight->matLookAt[0]);
		float nearDist = glm::dot(plight->xf.posWorld, glm::vec3(nearPlane));
		plight->avecFrustrum[4] = nearPlane;
		plight->avecFrustrum[4].w = -falloffMax - nearDist;

		glm::vec4 flippedNear = nearPlane * glm::vec4(-1, -1, -1, 1);
		float farDist = glm::dot(plight->xf.posWorld, glm::vec3(flippedNear));
		plight->avecFrustrum[5] = flippedNear;
		plight->avecFrustrum[5].w = 50.0f - farDist;

		glm::mat4 matProj;
		BuildSimpleProjectionMatrix(1.0f / plight->rx, 1.0f / plight->ry, 0.0f, 0.0f, 50.0f, falloffMax, matProj);
		CombineEyeLookAtProj(plight->xf.posWorld, plight->matLookAt, matProj, plight->frustum);

		agFallOff *= glm::vec3(-1.0f, 1.0f, 1.0f);

		if (plight->lightk == LIGHTK_Frustrum)
		{
			ConvertFallOff(&plight->lmFallOffAbsX, &plight->falloff0Frustum.x, &plight->falloff1Frustum.x);
			ConvertFallOff(&plight->lmFallOffAbsY, &plight->falloff0Frustum.y, &plight->falloff1Frustum.y);
			plight->falloff0Frustum.z = 1.0;
			plight->falloff1Frustum.z = 1.0;
		}
		else
		{
			plight->falloff0Frustum.x = 1.0;
			plight->falloff0Frustum.y = 1.0;
			plight->falloff1Frustum.x = 0.0;
			plight->falloff1Frustum.y = 0.0;
			ConvertFallOff(&plight->lmFallOffPenumbra, &plight->falloff0Frustum.z, &plight->falloff1Frustum.z);
		}
	}
}

void* GetLightKind(LIGHT* plight)
{
	return &plight->lightk;
}

void SetLightKind(LIGHT* plight, LIGHTK lightk)
{
	plight->lightk = lightk;
	*(unsigned long*)&plight->bitfield = *(unsigned long*)&plight->bitfield & 0xfffffeffffffffff | (unsigned long)(lightk == LIGHTK_Direction) << 0x28;
	RebuildLight(plight);
}

void* GetLightHighlightColor(LIGHT* plight)
{
	return &plight->vecHighlight;
}

void SetLightHighlightColor(LIGHT* plight, glm::vec3 &pvecHighlight)
{
	plight->vecHighlight = pvecHighlight;
	RebuildLight(plight);
}

void* GetLightMidtoneStrength(LIGHT* plight)
{
	return &plight->gMidtone;
}

void SetLightMidtoneStrength(LIGHT* plight, float gMidtone)
{
	plight->gMidtone = gMidtone;
	RebuildLight(plight);
}

void* GetLightShadowStrength(LIGHT* plight)
{
	return &plight->gShadow;
}

void SetLightShadowStrength(LIGHT* plight, float gShadow)
{
	plight->gShadow = gShadow;
	RebuildLight(plight);
}

void* GetLightHighlightAngle(LIGHT* plight)
{
	return &plight->degHighlight;
}

void SetLightHighlightAngle(LIGHT* plight, float degHighlight)
{
	plight->degHighlight = degHighlight;
	RebuildLight(plight);
}

void* GetLightMidtoneAngle(LIGHT* plight)
{
	return &plight->degMidtone;
}

void SetLightMidtoneAngle(LIGHT* plight, float degMidtone)
{
	plight->degMidtone = degMidtone;
	RebuildLight(plight);
}

void* GetLightShadowAngle(LIGHT* plight)
{
	return &plight->degShadow;
}

void SetLightShadowAngle(LIGHT* plight, float degShadow)
{
	plight->degShadow = degShadow;
	RebuildLight(plight);
}

void* GetLightDirection(LIGHT* plight)
{
	return &plight->vecDirectionOrig;
}

void SetLightDirection(LIGHT* plight, glm::vec3& pvecDirection)
{
	plight->vecDirectionOrig = pvecDirection;
	RebuildLight(plight);
}

void* GetLightDynamic(LIGHT* plight)
{
	return &plight->fDynamic;
}

void SetLightDynamic(LIGHT* plight, int fDynamic)
{
	if (fDynamic != plight->fDynamic)
	{
		plight->fDynamic = fDynamic;
		RebuildLight(plight);
	}
}

void* GetLightFallOff(LIGHT* plight)
{
	return &plight->lmFallOffS;
}

void SetLightFallOff(LIGHT* plight, LM* plm)
{
	plight->lmFallOffS = *plm;
	RebuildLight(plight);
}

void* GetLightConeAngle(LIGHT* plight)
{
	return &plight->degCone;
}

void SetLightConeAngle(LIGHT* plight, float degCone)
{
	plight->degCone = degCone;
	RebuildLightFrustrum(plight);
	RebuildLight(plight);
}

void* GetLightHotSpotAngle(LIGHT* plight)
{
	return &plight->degHotSpot;
}

void SetLightHotSpotAngle(LIGHT* plight, float degHotSpot)
{
	plight->degHotSpot = degHotSpot;
	RebuildLightFrustrum(plight);
	RebuildLight(plight);
}

void* GetLightFrustrumUp(LIGHT* plight)
{
	return &plight->vecUpLocal;
}

void SetLightFrustrumUp(LIGHT* plight, glm::vec3& pvecUpLocal)
{
	plight->vecUpLocal = pvecUpLocal;
	RebuildLight(plight);
}

void RemoveLightFromSw(LIGHT* plight)
{
	RemoveDlEntry(&plight->psw->dlLight, plight);
}

void AllocateLightBlkList()
{
	lightBlk.resize(numRl);
	numRl = 0;
}

bool SphereInFrustumLight(const FRUSTUM& frustum, const glm::vec3& position, float radius)
{
	for (int i = 0; i < 6; i++)
	{
		float distance = glm::dot(glm::vec3(frustum.planes[i]), position) + frustum.planes[i].w;

		if (distance <= -radius * 3.0)
			return false;
	}
	return true;
}

void PrepareSwLights(SW* psw, CM* pcm)
{
	LIGHT* plight = psw->dlLight.plightFirst;
	int numLights = 0;

	while (plight != nullptr)
	{
		switch (plight->lightk)
		{
		case LIGHTK_Direction:
		{
			LIGHTBLK dirlight;
			dirlight.lightk = plight->lightk;
			dirlight.dir = glm::vec4(plight->xf.matWorld[2], 0.0f);
			dirlight.color = glm::vec4(plight->rgbaColor, 0.0f);
			dirlight.ru = glm::vec4(plight->ltfn.ruShadow, plight->ltfn.ruMidtone, plight->ltfn.ruHighlight, 0.0f);
			dirlight.du = glm::vec4(plight->ltfn.duShadow, plight->ltfn.duMidtone, plight->ltfn.duHighlight, 0.0f);
			lightBlk[numRl] = dirlight;
			numRl++;

			numLights++;
			plight = plight->dleLight.plightNext;
			break;
		}

		case LIGHTK_Position:
		{
			if (g_fBsp != 0)
			{
				if ((plight->grfzon & pcm->grfzon) != pcm->grfzon)
				{
					plight = plight->dleLight.plightNext;
					continue;
				}
			}

			if (SphereInFrustumLight(pcm->frustum, plight->xf.posWorld, plight->lmFallOffS.gMax) == 0)
			{
				plight = plight->dleLight.plightNext;
				continue;
			}

			LIGHTBLK poslight;
			poslight.lightk = plight->lightk;
			poslight.pos = glm::vec4(plight->xf.posWorld, 0.0f);
			poslight.color = glm::vec4(plight->rgbaColor, 0.0f);
			poslight.falloff = glm::vec4(plight->agFallOff, 0.0f);
			poslight.ru = glm::vec4(plight->ltfn.ruShadow, plight->ltfn.ruMidtone, plight->ltfn.ruHighlight, 0.0f);
			poslight.du = glm::vec4(plight->ltfn.duShadow, plight->ltfn.duMidtone, plight->ltfn.duHighlight, 0.0f);
			lightBlk[numRl] = poslight;
			numRl++;

			numLights++;
			plight = plight->dleLight.plightNext;
			break;
		}
		}
	}

	GLuint lightsUBO;
	glGenBuffers(1, &lightsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LIGHTBLK) * numRl, lightBlk.data(), GL_DYNAMIC_DRAW);

	// Bind UBO to binding point 0
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO);

	// Send numLights separately (as it's a uniform, not in the UBO)
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "numLights"), numRl);

	numRl = 0;
}

void DeallocateLightBlkList()
{
	lightBlk.clear();
	lightBlk.shrink_to_fit();

	numRl = 0;
}

void DeleteLight(LIGHT *plight)
{
	delete plight;
}

int numRl;
std::vector <LIGHTBLK> lightBlk;
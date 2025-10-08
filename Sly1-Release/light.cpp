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
	//plight->falloff0Frustum = plightBase->falloff0Frustum;
	//plight->falloff1Frustum = plightBase->falloff1Frustum;

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
		// aliases to match your original names
		glm::mat4* pmat = &plight->matLookAt;
		glm::vec4* anormalFrustrum = plight->avecFrustrum;
		glm::vec3  agFallOff; // from normalLocal (forward hint)
		glm::vec3  vecUp;     // up hint

		// Convert forward / up from local to world
		ConvertAloVec(plight, nullptr, &plight->normalLocal, &agFallOff);
		ConvertAloVec(plight, nullptr, &plight->vecUpLocal, &vecUp);

		// --- match first VU broadcast -1 multiply (uniform flip)
		agFallOff = -agFallOff;

		// --- BuildOrthonormalMatrixZ via GLM (Z = forward, X = right, Y = up)
		glm::vec3 f = glm::normalize(agFallOff);
		glm::vec3 r = glm::normalize(glm::cross(vecUp, f));
		if (!glm::all(glm::isfinite(r)) || glm::length2(r) < 1e-10f) {
			glm::vec3 alt = (std::abs(f.z) < 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
			r = glm::normalize(glm::cross(alt, f));
		}
		glm::vec3 u = glm::normalize(glm::cross(f, r));
		*pmat = glm::mat4(1.0f);
		(*pmat)[0] = glm::vec4(r, 0.0f);  // X (right)
		(*pmat)[1] = glm::vec4(u, 0.0f);  // Y (up)
		(*pmat)[2] = glm::vec4(f, 0.0f);  // Z (forward)

		// --- BuildFrustrum via GLM (side plane normals)
		const glm::vec3 X = glm::vec3((*pmat)[0]);
		const glm::vec3 Y = glm::vec3((*pmat)[1]);
		const glm::vec3 Z = glm::vec3((*pmat)[2]);

		anormalFrustrum[0] = glm::vec4(glm::normalize(Z + plight->rx * X), 0.0f); // Left
		anormalFrustrum[1] = glm::vec4(glm::normalize(Z - plight->rx * X), 0.0f); // Right
		anormalFrustrum[2] = glm::vec4(glm::normalize(Z + plight->ry * Y), 0.0f); // Top
		anormalFrustrum[3] = glm::vec4(glm::normalize(Z - plight->ry * Y), 0.0f); // Bottom

		// Add distance term: w = -dot(posWorld, N)
		const glm::vec3 eye = plight->xf.posWorld;
		for (int i = 0; i < 4; ++i) {
			const glm::vec3 N = glm::vec3(anormalFrustrum[i]);
			anormalFrustrum[i].w = -glm::dot(eye, N);
		}

		// --- Near / far planes
		const float gMax = plight->lmFallOffS.gMax;
		glm::vec4 nearN = (*pmat)[2]; // forward
		float nearDot = glm::dot(eye, glm::vec3(nearN));
		plight->avecFrustrum[4] = nearN;
		plight->avecFrustrum[4].w = -(gMax + nearDot);

		glm::vec4 farN = nearN * glm::vec4(-1, -1, -1, 1);
		float farDot = glm::dot(eye, glm::vec3(farN));
		plight->avecFrustrum[5] = farN;
		plight->avecFrustrum[5].w = 50.0f - farDot;

		// --- Projection via GLM (equivalent to scales 1/rx, 1/ry)
		const float fovy = 2.0f * std::atan(plight->ry);
		const float aspect = plight->rx / plight->ry;
		const float zNear = 50.0f;
		const float zFar = gMax;

		glm::mat4 matProj = glm::perspectiveRH_ZO(fovy, aspect, zNear, zFar);

		// --- CombineEyeLookAtProj via GLM
		glm::mat4 matView = glm::lookAtRH(eye, eye + Z, u);
		glm::mat4 matFrustrum = matProj * matView;

		// --- restore agFallOff sign (VU does a second -1 broadcast before upload)
		agFallOff = -agFallOff;

		// --- Write directly into LIGHT::falloffBias / falloffScale (no aqwFallOff temp)
		// Mapping follows the original: bias from the “0” slot, scale from the “1” slot.
		// Channels: X=|x/w|, Y=|y/w|, Z=r^2 (or disabled), W=w

		if (plight->lightk == LIGHTK_Frustrum) {
			float bx, sx; ConvertFallOff(&plight->lmFallOffAbsX, &bx, &sx);
			float by, sy; ConvertFallOff(&plight->lmFallOffAbsY, &by, &sy);
			float bs, ss; ConvertFallOff(&plight->lmFallOffS, &bs, &ss);

			plight->falloffBias = glm::vec4(bx, by, 1.0f, bs); // Z channel forced to 1.0
			plight->falloffScale = glm::vec4(sx, sy, 0.0f, ss); // Z channel disabled (0.0)
		}
		else {
			// Spotlight-like path: X/Y disabled, Penumbra mapped to Z, S to W
			float bp, sp; ConvertFallOff(&plight->lmFallOffPenumbra, &bp, &sp);
			float bs, ss; ConvertFallOff(&plight->lmFallOffS, &bs, &ss);

			plight->falloffBias  = glm::vec4(1.0f, 1.0f, bp, bs);
			plight->falloffScale = glm::vec4(0.0f, 0.0f, sp, ss);
		}

		// --- store frustum matrix for GLSL
		plight->frustum = matFrustrum;

		// (optional) if your GLSL uses light direction from LIGHT.dir, set it here, too:
		plight->agFallOff = agFallOff;
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
	lightIndices.resize(numRl);

	glGenBuffers(1, &g_lightUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, g_lightUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LIGHTBLK) * numRl, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_lightUbo);
	
	numRl = 0;
	LIGHT* plight = g_psw->dlLight.plightFirst;

	while (plight != nullptr)
	{
		switch (plight->lightk)
		{
			case LIGHTK_Direction:
			lightBlk[numRl].lightk = plight->lightk;
			lightBlk[numRl].dir    = glm::vec4(plight->xf.matWorld[2], 0.0f);
			lightBlk[numRl].color  = glm::vec4(plight->rgbaColor, 0.0f);
			lightBlk[numRl].ru     = glm::vec4(plight->ltfn.ruShadow, plight->ltfn.ruMidtone, plight->ltfn.ruHighlight, 0.0f);
			lightBlk[numRl].du     = glm::vec4(plight->ltfn.duShadow, plight->ltfn.duMidtone, plight->ltfn.duHighlight, 0.0f);
			numRl++;
			break;

			case LIGHTK_Position:
			lightBlk[numRl].lightk  = plight->lightk;
			lightBlk[numRl].pos     = glm::vec4(plight->xf.posWorld, 1.0f);
			lightBlk[numRl].color   = glm::vec4(plight->rgbaColor, 1.0f);
			lightBlk[numRl].falloff = plight->agFallOff;
			lightBlk[numRl].maxDst  = plight->lmFallOffS.gMax * plight->lmFallOffS.gMax;
			lightBlk[numRl].ru	    = glm::vec4(plight->ltfn.ruShadow, plight->ltfn.ruMidtone, plight->ltfn.ruHighlight, 0.0f);
			lightBlk[numRl].du	    = glm::vec4(plight->ltfn.duShadow, plight->ltfn.duMidtone, plight->ltfn.duHighlight, 0.0f);
			numRl++;
			break;

			case LIGHTK_Frustrum:
			case LIGHTK_Spot:
			lightBlk[numRl].lightk       = plight->lightk;
			lightBlk[numRl].color        = glm::vec4(plight->rgbaColor, 1.0f);
			lightBlk[numRl].matFrustrum  = plight->frustum;
			lightBlk[numRl].falloffScale = plight->falloffScale;
			lightBlk[numRl].falloffBias  = plight->falloffBias;
			lightBlk[numRl].ru           = glm::vec4(plight->ltfn.ruShadow, plight->ltfn.ruMidtone, plight->ltfn.ruHighlight, 0.0f);
			lightBlk[numRl].du           = glm::vec4(plight->ltfn.duShadow, plight->ltfn.duMidtone, plight->ltfn.duHighlight, 0.0f);
			break;
		}

		plight = plight->dleLight.plightNext;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, g_lightUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LIGHTBLK) * numRl, lightBlk.data());
}

void PrepareSwLights(SW* psw, CM* pcm)
{
	numRl = 0;
	int idx = 0;
	const bool useZones = (g_fBsp != 0);
	bool include = false;

	for (LIGHT* plight = psw->dlLight.plightFirst; plight && numRl < 255; plight = plight->dleLight.plightNext, ++idx)
	{
		if (useZones && ((plight->grfzon & pcm->grfzon) != pcm->grfzon))
			continue;

		switch (plight->lightk)
		{
			case LIGHTK_Direction:
			lightIndices[numRl] = idx;
			numRl++;
			break;

			case LIGHTK_Position:
			if (!SphereInFrustum(pcm->frustum, plight->xf.posWorld, plight->lmFallOffS.gMax * 3.0))
				break;

			lightIndices[numRl] = idx;
			numRl++;
			break;

			case LIGHTK_Frustrum:
			case LIGHTK_Spot:
			lightIndices[numRl] = idx;
			numRl++;
			break;
		}
	}

	// Upload only the used indices
	//std::cout << numRl << "\n";
	glUniform1i(glslNumLights, numRl);

	if (numRl > 0)
		glUniform1iv(glslLightIndices, numRl, lightIndices.data());
}

void FindSwLights(SW *psw, CM *pcm, glm::vec3 posCenter, float sRadius)
{
	numRl = 0;
	int idx = 0;
	const bool useZones = (g_fBsp != 0);
	bool include = false;

	for (LIGHT* plight = psw->dlLight.plightFirst; plight && numRl < 255; plight = plight->dleLight.plightNext, ++idx)
	{
		if (useZones && ((plight->grfzon & pcm->grfzon) != pcm->grfzon))
			continue;

		switch (plight->lightk)
		{
			case LIGHTK_Direction:
			lightIndices[numRl] = idx;
			numRl++;
			break;

			case LIGHTK_Position:
			{
				// Sphere–sphere test: |Lp - C|^2 <= (r + R)^2
				const glm::vec3 Lpos = *(glm::vec3*)&plight->xf.posWorld;
				const float r = plight->lmFallOffS.gMax;
				const glm::vec3 d = Lpos - posCenter;
				const float Rsum = r + sRadius;

				if (glm::dot(d, d) > Rsum * Rsum)
					break;

				lightIndices[numRl] = idx;
				numRl++;
				break;
			}

			case LIGHTK_Frustrum:
			case LIGHTK_Spot:
			lightIndices[numRl] = idx;
			numRl++;
			break;
		}
	}

	// Upload only the used indices
	//std::cout << numRl << "\n";
	glUniform1i(glslNumLights, numRl);

	if (numRl > 0)
		glUniform1iv(glslLightIndices, numRl, lightIndices.data());
}

void DeallocateLightBlkList()
{
	lightBlk.clear();
	lightBlk.shrink_to_fit();

	lightIndices.clear();
	lightIndices.shrink_to_fit();

	glDeleteBuffers(1, &g_lightUbo);
	g_lightUbo = 0;

	numRl = 0;
}

void DeleteLight(LIGHT *plight)
{
	delete plight;
}

void DeallocateLightVector()
{
	allSwLights.clear();
	allSwLights.shrink_to_fit();
}

std::vector <LIGHT*> allSwLights;

GLuint g_lightUbo;
int numRl;
std::vector <LIGHTBLK> lightBlk;
std::vector <int> lightIndices;
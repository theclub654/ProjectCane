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

	allSwLights.push_back(plight);
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
	LO lo = *plight;
	*plight = *plightBase;
	memcpy(plight, &lo, sizeof(LO));

	CloneLo(plight, plightBase);

	ClearDl(&plight->dlChild);
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
		ConvertUserHsvToUserRgb(plight->vecHighlight, rgba, plight->agFallOff);
		
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
		ConvertAloVec(plight, nullptr, &plight->normalLocal, &plight->agFallOff);

		
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

void PrepareSwLightsForDraw(SW *psw)
{
	int numDirLights = 0;
	int numPointLights = 0;
	int numFrustumLights = 0;

	for (int i = 0; i < allSwLights.size(); i++)
	{
		switch (allSwLights[i]->lightk)
		{
			case LIGHTK_Position:
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].pos"),     1, glm::value_ptr(allSwLights[i]->xf.posWorld));
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].color"),   1, glm::value_ptr(allSwLights[i]->rgbaColor));
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].falloff"), 1, glm::value_ptr(allSwLights[i]->agFallOff));

			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ltfn.ruShadow"),    allSwLights[i]->ltfn.ruShadow);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ltfn.ruMidtone"),   allSwLights[i]->ltfn.ruMidtone);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ltfn.ruHighlight"), allSwLights[i]->ltfn.ruHighlight);

			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ltfn.duShadow"),    allSwLights[i]->ltfn.duShadow);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ltfn.duMidtone"),   allSwLights[i]->ltfn.duMidtone);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ltfn.duHighlight"), allSwLights[i]->ltfn.duHighlight);

			numPointLights++;
			break;

			case LIGHTK_Direction:
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].dir"),   1, glm::value_ptr(allSwLights[i]->xf.matWorld[2]));
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].color"), 1, glm::value_ptr(allSwLights[i]->rgbaColor));

			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ltfn.ruShadow"),    allSwLights[i]->ltfn.ruShadow);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ltfn.ruMidtone"),   allSwLights[i]->ltfn.ruMidtone);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ltfn.ruHighlight"), allSwLights[i]->ltfn.ruHighlight);

			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ltfn.duShadow"),    allSwLights[i]->ltfn.duShadow);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ltfn.duMidtone"),   allSwLights[i]->ltfn.duMidtone);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ltfn.duHighlight"), allSwLights[i]->ltfn.duHighlight);

			numDirLights++;

			case LIGHTK_Frustrum:
			case LIGHTK_Spot:
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "frustumlights[" + std::to_string(numFrustumLights) + "].dir"),     1, glm::value_ptr(allSwLights[i]->xf.matWorld[2]));
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "frustumlights[" + std::to_string(numFrustumLights) + "].color"),   1, glm::value_ptr(allSwLights[i]->rgbaColor));
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "frustumlights[" + std::to_string(numFrustumLights) + "].falloff"), 1, glm::value_ptr(allSwLights[i]->agFallOff));

			numFrustumLights++;
			break;
		}
	}

	glUniform1i(glGetUniformLocation(glGlobShader.ID, "numDirLights"),   numDirLights);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "numPointLights"), numPointLights);
}

TWPS TwpsFindSwLights(SW *psw, glm::vec3 &ppos, float sRadius, int grffindlight, int cplightMax, int *pcplightStatic, int *pcplightAll, LIGHT **aplight, char *pchzTarget)
{
	for (int i = 0; i < allSwLights.size(); i++)
	{
		if (allSwLights[i]->lightk == LIGHTK_Position)
		{
			
		}
	}

	return TWPS();
}

void DeleteLight(LIGHT *plight)
{
	delete plight;
}
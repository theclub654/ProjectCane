#include "light.h"

void* NewLight()
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
	plight->lmFallOffS.gMin = 200.0;
	plight->lmFallOffS.gMax = 2000.0;
	plight->degMidtone = 240.0;
	plight->degShadow = 180.0;
	plight->degCone = 60.0;
	plight->degHighlight = 180.0;

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

void UpdateLightXfWorldHierarchy(LIGHT* plight)
{
	UpdateAloXfWorldHierarchy(plight);
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

void SetLightKind(LIGHT* plight, LIGHTK lightk)
{
	plight->lightk = lightk;

}

void SetLightHighlightColor(LIGHT* plight, glm::vec3 &pvecHighlight)
{
	plight->vecHighlight = pvecHighlight;
}

void SetLightMidtoneStrength(LIGHT* plight, float gMidtone)
{
	plight->gMidtone = gMidtone;
}

void SetLightShadowStrength(LIGHT* plight, float gShadow)
{
	plight->gShadow = gShadow;
}

void SetLightHighlightAngle(LIGHT* plight, float degHighlight)
{
	plight->degHighlight = degHighlight;
}

void SetLightMidtoneAngle(LIGHT* plight, float degMidtone)
{
	plight->degMidtone = degMidtone;
}

void SetLightShadowAngle(LIGHT* plight, float degShadow)
{
	plight->degShadow = degShadow;
}

void SetLightDirection(LIGHT* plight, glm::vec3& pvecDirection)
{
	plight->vecDirectionOrig = pvecDirection;
}

void SetLightDynamic(LIGHT* plight, int fDynamic)
{
	if (fDynamic != plight->fDynamic)
		plight->fDynamic = fDynamic;
}

void SetLightFallOff(LIGHT* plight, LM* plm)
{
	plight->lmFallOffS = *plm;
}

void SetLightConeAngle(LIGHT* plight, float degCone)
{
	plight->degCone = degCone;
}

void SetLightHotSpotAngle(LIGHT* plight, float degHotSpot)
{
	plight->degHotSpot = degHotSpot;
}

void SetLightFrustrumUp(LIGHT* plight, glm::vec3& pvecUpLocal)
{
	plight->vecUpLocal = pvecUpLocal;
}

void RemoveLightFromSw(LIGHT* plight)
{
	RemoveDlEntry(&plight->psw->dlLight, plight);
}

void DeleteLight(LO* plo)
{
	delete(LIGHT*)plo;
}
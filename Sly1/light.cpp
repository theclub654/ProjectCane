#include "light.h"

void* NewLight()
{
	return new LIGHT;
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
}

void OnLightAdd(LIGHT* plight)
{
	OnAloAdd(plight);
	AddLightToSw(plight);
}

void AddLightToSw(LIGHT* plight)
{
	AppendDlEntry(&plight->psw->dlLight, plight);
}

void RemoveLightFromSw(LIGHT* plight)
{
	RemoveDlEntry(&plight->psw->dlLight, plight);
}

void DeleteLight(LO* plo)
{
	delete(LIGHT*)plo;
}

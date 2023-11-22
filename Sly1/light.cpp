#include "light.h"

void* NewLight()
{
	return new LIGHT;
}

void InitLight(LIGHT* plight)
{
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

void DeleteLight(LO* plo)
{
	delete(LIGHT*)plo;
}

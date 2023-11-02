#include "light.h"

void InitLight(LIGHT* plight)
{
	InitAlo(plight);
}

void OnLightAdd(LIGHT* plight)
{
	OnAloAdd(plight);
}

#include "sky.h"

void* NewSky()
{
	return new SKY;
}

int GetSkySize()
{
	return sizeof(SKY);
}

void CloneSky(SKY* psky, SKY* pskyBase)
{
	LO lo = *psky;
	*psky = *pskyBase;
	memcpy(psky, &lo, sizeof(LO));

	CloneLo(psky, pskyBase);

	ClearDl(&psky->dlChild);
}

void DeleteSky(LO* plo)
{
	delete(SKY*)plo;
}

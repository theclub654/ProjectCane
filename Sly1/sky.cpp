#include "sky.h"

void* NewSky()
{
	return new SKY;
}

void DeleteSky(LO* plo)
{
	delete(SKY*)plo;
}

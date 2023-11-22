#include "flash.h"

void* NewFlash()
{
	return new FLASH;
}

void InitFlash(FLASH* pflash)
{
	InitAlo(pflash);
}

void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pflash, pbis);
	LoadTbspFromBrx(pbis);
}

void DeleteFlash(LO* plo)
{
	delete(FLASH*)plo;
}

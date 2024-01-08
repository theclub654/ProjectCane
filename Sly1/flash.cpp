#include "flash.h"

void* NewFlash()
{
	return new FLASH;
}

void InitFlash(FLASH* pflash)
{
	InitAlo(pflash);
}

int GetFlashSize()
{
	return sizeof(FLASH);
}

void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pflash, pbis);
	LoadTbspFromBrx(pbis);
}

void CloneFlash(FLASH* pflash, FLASH* pflashBase)
{
	LO lo = *pflash;
	*pflash = *pflashBase;
	memcpy(pflash, &lo, sizeof(LO));

	CloneLo(pflash, pflashBase);

	ClearDl(&pflash->dlChild);
}

void DeleteFlash(LO* plo)
{
	delete(FLASH*)plo;
}

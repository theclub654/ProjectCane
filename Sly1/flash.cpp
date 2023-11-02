#include "flash.h"

void InitFlash(FLASH* pflash)
{
	InitAlo(pflash);
}

void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pflash, pbis);
	LoadTbspFromBrx(pbis);
}

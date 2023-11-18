#include "flash.h"

void InitFlash(FLASH* pflash)
{
	//std::cout << "FLASH Size: " << sizeof(FLASH) << "\n";
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

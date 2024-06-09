#pragma once
#include "alo.h"

void LoadTbspFromBrx(CBinaryInputStream *pbis);

class FLASH : public ALO
{
	public:
        int ctsurf;
        struct TSURF* atsurf;
        int ctbsp;
        struct TBSP* atbsp;
        struct SMP smpScale;
        float gScaleCur;
        float gScaleTarget;
};

FLASH*NewFlash();
void InitFlash(FLASH* pflash);
int  GetFlashSize();
void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);
void CloneFlash(FLASH* pflash, FLASH* pflashBase);
void DeleteFlash(LO* plo);
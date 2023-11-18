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

void InitFlash(FLASH* pflash);
void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);
void DeleteFlash(LO* plo);
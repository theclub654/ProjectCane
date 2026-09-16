#pragma once
#include "alo.h"
#include "tn.h"

class FLASH : public ALO
{
	public:
    int ctsurf;
    std::vector <TSURF> atsurf;
    int ctbsp;
    std::vector <TBSP> atbsp;
    SMP smpScale;
    float gScaleCur;
    float gScaleTarget;
};

FLASH*NewFlash();
void InitFlash(FLASH* pflash);
int  GetFlashSize();
void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);
void CloneFlash(FLASH* pflash, FLASH* pflashBase);
void UpdateFlash(FLASH* pflash, float dt);
void RenderFlashSelf(FLASH* pflash, CM* pcm, RO* pro);
int  FPosFlashWithin(FLASH* pflash, glm::vec3* ppos);
void DeleteFlash(FLASH* pflash);
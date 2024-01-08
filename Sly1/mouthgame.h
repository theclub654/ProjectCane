#pragma once
#include "po.h"

class MGC : public ALO
{
	public:
        struct SM* psmGenerate;
        struct SMA* psmaGenerate;
        struct SM* psmCollect;
        struct SMA* psmaCollect;
        struct ASEG* pasegPath;
        struct VOL* pvolSmash;
        struct VOL* pvolHole;
        DL dlMgcoFree;
        DL dlMgco;
        int fDamaging;
        struct EXPL* pexplGood;
        struct EXPL* pexplBad;
        struct MGV* pmgv;
};

void*NewMgc();
void InitMgc(MGC* pmgc);
int  GetMgcSize();
void CloneMgc(MGC* pmgc, MGC* pmgcBase);
void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);
void DeleteMgc(LO* plo);
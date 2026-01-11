#pragma once
#include "stepguard.h"

struct SGF 
{
    OID oidFlash;
    struct FLASH* pflash;
};
struct SGFT 
{
    OID oidTarget;
    struct ALO* paloTarget;
};

class SMARTGUARD : public STEPGUARD
{
	public:
        SGF sgf;
        float sDetectNear;
        float zDetectNear;
        float sSneak;
        float zSneak;
        OID oidSneakVol;
        struct VOL* pvolSneak;
        struct ALO* paloFlashTarget;
        SGFT mpsgssgft[17];
        struct EXC* pexcSneak;
        int cidleSearch;
};

SMARTGUARD*NewSmartguard();
void InitSmartGuard(SMARTGUARD* psmartguard);
int  GetSmartguardSize();
void CloneSmartguard(SMARTGUARD* psmartguard, SMARTGUARD* psmartguardBase);
void PostSmartguardLoad(SMARTGUARD* psmartguard);
void UpdateSmartguard(SMARTGUARD* psmartguard, float dt);
void DeleteSmartGuard(SMARTGUARD* psmartguard);
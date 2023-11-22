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

void* NewSmartguard();
void InitSmartGuard(SMARTGUARD* psmartguard);
void DeleteSmartGuard(LO* plo);
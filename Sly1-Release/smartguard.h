#pragma once
#include "stepguard.h"

struct SGF 
{
    OID oidFlash;
    class FLASH* pflash;
};
struct SGFT 
{
    OID oidTarget;
    class ALO* paloTarget;
};

struct SMARTGUARDBARRIER
{
    OID oidBarrier;
    class BARRIER *pbarrier;
};

class SMARTGUARD : public STEPGUARD
{
	public:
    SGF sgf;
    float sDetectNear;
    float zDetectNear;
    float sSneak;
    float zSneak;
    short oidSneakVol;
    struct VOL* pvolSneak;
    class ALO* paloFlashTarget;
    std::shared_ptr <ACTLA> pactlaSmartguard;
    SGFT mpsgssgft[17];
    struct EXC* pexcSneak;
    int cidleSearch;
    int unk0;
    int fNoDetect;
    int grfDetection;
    int cbarrier;
    SMARTGUARDBARRIER smartguardBarrier[4];
};

SMARTGUARD*NewSmartguard();
void InitSmartGuard(SMARTGUARD* psmartguard);
void*GetSmartguardSgf(SMARTGUARD* psmartguard);
void SetSmartguardSgf(SMARTGUARD* psmartguard, const SGF& sgf);
void*GetSmartguardSgsFlash(SMARTGUARD* psmartguard);
void*GetSmartguardSDetectNear(SMARTGUARD* psmartguard);
void SetSmartguardSDetectNear(SMARTGUARD* psmartguard, float sDetectNear);
void*GetSmartguardZDetectNear(SMARTGUARD* psmartguard);
void SetSmartguardZDetectNear(SMARTGUARD* psmartguard, float zDetectNear);
void*GetSmartguardSSneak(SMARTGUARD* psmartguard);
void SetSmartguardSSneak(SMARTGUARD* psmartguard, float sSneak);
void*GetSmartguardZSneak(SMARTGUARD* psmartguard);
void SetSmartguardZSneak(SMARTGUARD* psmartguard, float zSneak);
void*GetSmartguardOidSneakVol(SMARTGUARD* psmartguard);
void SetSmartguardOidSneakVol(SMARTGUARD* psmartguard, OID oidSneakVol);
void*GetSmartguardUnk0(SMARTGUARD* psmartguard);
void SetSmartguardUnk0(SMARTGUARD* psmartguard, int Unk0);
void*GetSmartguardfNoDetect(SMARTGUARD* psmartguard);
void SetSmartguardfNoDetect(SMARTGUARD* psmartguard, int fNoDetect);
void*GetSmartguardgrfDetection(SMARTGUARD* psmartguard);
void SetSmartguardgrfDetection(SMARTGUARD* psmartguard, int grfDetection);
int  GetSmartguardSize();
void CloneSmartguard(SMARTGUARD* psmartguard, SMARTGUARD* psmartguardBase);
void AddSmartguardBarrier(SMARTGUARD* psmartguard, OID oidBarrier);
void PostSmartguardLoad(SMARTGUARD* psmartguard);
void PostSmartguardLoadFlashlight(SMARTGUARD* psmartguard);
int  FFilterSmartguardDetect(SMARTGUARD* psmartguard, SO* pso);
void UseSmartguardFlashlightTarget(SMARTGUARD* psmartguard, SGS sgs, OID oidTarget);
void HandleSmartguardMessage(SMARTGUARD* psmartguard, MSGID msgid, void* pv);
int  FDetectSmartguard(SMARTGUARD* psmartguard);
void UpdateSmartguard(SMARTGUARD* psmartguard, float dt);
void UpdateSmartguardFlashlight(SMARTGUARD* psmartguard);
void OnSmartguardEnteringSgs(SMARTGUARD* psmartguard, SGS sgsPrev, ASEG* pasegOverride);
int  FCanSmartguardAttack(SMARTGUARD* psmartguard);
int  SgasGetSmartguard(SMARTGUARD* psmartguard);
void FreezeSmartguard(SMARTGUARD* psmartguard, int fFreeze);
void DeleteSmartGuard(SMARTGUARD* psmartguard);

extern glm::vec3 s_posFlashSelf;
extern glm::vec3 s_posFlashTarget;
static glm::vec3 s_aposTestAttack[4] = 
{
    glm::vec3(0.0f,   0.0f, -80.0f),
    glm::vec3(0.0f,   0.0f, 100.0f),
    glm::vec3(0.0f, -70.0f,  40.0f),
    glm::vec3(0.0f,  70.0f,  40.0f),
};

static glm::vec3 s_aposTestPatrol[4] = 
{
    glm::vec3(0.0f,   0.0f, -20.0f),
    glm::vec3(0.0f,   0.0f,  70.0f),
    glm::vec3(0.0f, -30.0f,  30.0f),
    glm::vec3(0.0f,  30.0f,  30.0f),
};
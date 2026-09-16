#pragma once
#include "cplcy.h"

enum LOOKK
{
    LOOKK_Nil = -1,
    LOOKK_User = 0,
    LOOKK_Sniper = 1,
    LOOKK_SniperTrack = 2,
    LOOKK_Dialog = 3,
    LOOKK_Confront = 4,
    LOOKK_Max = 5
};

struct CPLOOK : public CPLCY
{
    float radPan;
    float swPan;
    float radTilt;
    float swTilt;
    float uZoom;
    float svuZoom;
    float rZoomMax;
    LM lmFOV;
    LOOKK alookk[4];
    int clookk;
    int fSoundPaused;
    struct AMB* pambBinoc;
    struct AMB* pambAmbient;
    class ALO* paloFocusSniper;
    float rScreenSniper;
    float sRadiusSniper;
    class PNT* ppntAnchor;
    // Release stores this in a slot Ghidra misidentified as alookk[0].
    // Keep it separate so activating the policy cannot corrupt the look stack.
    float sNearClipRestore;
};

void InitCplook(CPLOOK* pcplook, CM* pcm);
void PosCplookAnchor(CPLOOK* pcplook, glm::vec3* pposAnchor);
void PosCplookEye(CPLOOK* pcplook, glm::vec3* pposEye);
void StartCplookSound(CPLOOK* pcplook);
void PushCplookLookk(CPLOOK* pcplook, LOOKK lookk);
LOOKK LookkPopCplook(CPLOOK* pcplook);
LOOKK LookkCurCplook(CPLOOK* pcplook);
void ActivateCplook(CPLOOK* pcplook, void* pv);
//GOTTA COME BACK TO THIS WHEN DONE SOUND
void UpdateCplook(CPLOOK* pcplook, CPDEFI* pcpdefi, JOY* pjoy, float dt);
void DeactivateCplook(CPLOOK* pcplook);

struct VTCPLOOK
{
    void (*pfnActivateCplook)(CPLOOK*, void*) = ActivateCplook;
    void (*pfnDeactivateCplook)(CPLOOK*) = DeactivateCplook;
    void (*pfnSetCplcy)() = nullptr;
    void (*pfnRevokeCplcy)() = nullptr;
    void (*pfnUpdateCplook)(CPLOOK*, CPDEFI*, JOY*, float) = UpdateCplook;
};

inline VTCPLOOK g_vtcplook;
extern float DZ_CplookAnchor;
extern glm::vec3 s_dposLookFar;
extern glm::vec3 s_dposLookNear;
extern float RAD_CplookTiltDefault;
extern CLQ s_clqUZoomToRsvu;
extern SMP s_smpSvu;
extern SMPA s_smpaSniperStretch;
extern SMPA s_smpaSniperZoom;
extern SMP s_smpSwCplook;
extern SMP s_smpDialogZoom;
extern float DT_CplookSniperTrack;

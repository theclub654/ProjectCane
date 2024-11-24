#pragma once
#include "lo.h"

enum CPMT
{
    CPMT_Truck = 0,
    CPMT_Pan = 1,
    CPMT_Slide = 2,
    CPMT_Orbit = 3,
    CPMT_Align = 4,
    CPMT_Walkthrough = 5,
    CPMT_Max = 6
};
enum CPP
{
    CPP_Nil = -1,
    CPP_Base = 0,
    CPP_Default = 1,
    CPP_Tunnel_Pri0 = 2,
    CPP_Tunnel_Pri1 = 3,
    CPP_Tunnel_Pri2 = 4,
    CPP_Tunnel_Pri3 = 5,
    CPP_LookAround = 6,
    CPP_Animated = 7,
    CPP_ManualOverride = 8,
    CPP_UserTestWalkthrough = 9,
    CPP_Max = 10
};
enum CFK
{
    CFK_Nil = -1,
    CFK_Behind = 0,
    CFK_Side = 1,
    CFK_Max = 2
};
enum LOOKK
{
    LOOKK_Nil = -1,
    LOOKK_User = 0,
    LOOKK_Sniper = 1,
    LOOKK_Dialog = 2,
    LOOKK_Confront = 3,
    LOOKK_Max = 4
};
enum FTND
{
    FTND_Nil = -1,
    FTND_Forward = 1,
    FTND_Reverse = 2,
    FTND_Manual = 4,
    FTND_Max = 5
};
struct CPDEFI
{
    int grfcpd;
    glm::vec3 posBase;
    glm::vec3 vBase;
    float radHome;
    float sAdjust;
    float tMoveLast;
    CFK cfk;
    glm::vec3 posAdjust;
};
struct CPR
{
    CPP cpp;
    struct CPLCY* pcplcy;
    struct SO* psoFocus;
    void* pv;
};

struct CPLCY
{
    union
    {
        struct VTCPLCY* pvtcplcy;
        struct VTCPMAN* pvtcpman;
        struct VTCPLOOK* pvtcplook;
        struct VTCPALIGN* pvtcpalign;
        struct VTCPASEG* pvtcpaseg;
        struct VTCPTN* pvtcptn;
    };

    struct CM* pcm;
};

struct CPMAN : public CPLCY
{
    CPMT cpmt;
    struct ALO* paloOrbit;
    int cframeStatus;
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
    struct ALO* paloFocusSniper;
    float rScreenSniper;
    float sRadiusSniper;
    struct PNT* ppntAnchor;
};

struct CPALIGN : public CPLCY
{
    glm::vec3 posLocal;
    glm::mat3 matLocal;
};
struct CPASEG : public CPLCY
{
    struct CAMERA* pcamera;
};
struct CPTN : public CPLCY
{
    struct TN* ptn;
    float radManual;
    float xyManual;
    float zManual;
    int fHome;
    float radHome;
    FTND ftnd;
    int fActivate;
    int fPanOnEntryDone;
    float swOrbit;
    float tLastOrbit;
    float radCur;
    float radFwdPrev;
    float radRevPrev;
    float tMoveLast;
    float uFollowCur;
    float vxy;
    float vz;
    glm::vec3 posEyePrev;
};

struct VTCPMAN
{
    void (*pfnActivateCplcy)() = nullptr;
    void (*pfnDeactivateCplcy)() = nullptr;
    void (*pfnSetCplcy)() = nullptr;
    void (*pfnRevokeCplcy)() = nullptr;
    void (*UpdateCpman)(CPMAN* pcpman, CPDEFI* pcpdefi, float dt) = UpdateCpman;
};
struct VTCPLOOK
{
    void (*pfnActivateCplook)() = nullptr;
    void (*pfnDeactivateCplook)() = nullptr;
    void (*pfnSetCplcy)() = nullptr;
    void (*pfnRevokeCplcy)() = nullptr;
    void (*pfnUpdateCplook)() = nullptr;
};
struct VTCPALIGN
{
    void (*pfnActivateCpalign)() = nullptr;
    void (*pfnDeactivateCplcy)() = nullptr;
    void (*pfnSetCplcy)() = nullptr;
    void (*pfnRevokeCplcy)() = nullptr;
    void (*pfnUpdateCpalign)() = nullptr;
};
struct VTCPASEG
{
    void (*pfnActivateCpaseg)() = nullptr;
    void (*pfnDeactivateCpaseg)() = nullptr;
    void (*pfnSetCplcy)() = nullptr;
    void (*pfnRevokeCplcy)() = nullptr;
    void (*pfnUpdateCpaseg)() = nullptr;
};
struct VTCPTN
{
    void (*pfnActivateCptn)() = nullptr;
    void (*pfnDeactivateCptn)() = nullptr;
    void (*pfnSetCptn)() = nullptr;
    void (*pfnRevokeCptn)() = nullptr;
    void (*pfnUpdateCptn)() = nullptr;
};

// Camera Object
class CM : public LO
{
public:

    // Camera psoition
    glm::vec3 pos;
    glm::vec4 anormalFrustrumTranspose[3];
    // Camera lookAt
    glm::mat3 mat;
    float rMRDAdjust;
    // Camera projection 
    glm::mat4 matProj;
    // This is the projection and eye matrix combined
    glm::mat4 matWorldToClip;
    glm::mat4 matClipToWorld;
    glm::vec3 anormalFrustrum[4];
    float rMRD;
    // Camera field of view
    float radFOV;
    // Camera Aspect Ratio
    float rAspect;
    float sNearClip;
    float sFarClip;
    float sRadiusNearClip;
    float xScreenRange;
    float yScreenRange;
    float sNearFog;
    float sFarFog;
    float uFogMax;
    RGBA rgbaFog;
    FGFN fgfn;
    float tJolt;
    int grfzon;
    int fCutNext;
    int fCut;
    int fRadCut;
    float radCut;
    int fDisplaced;
    float uPanProgress;
    float uTiltProgress;
    float uSProgress;
    glm::vec3 dposCenter;
    glm::vec3 vCenter;
    glm::vec3 dposAdjust;
    glm::vec3 vAdjust;
    glm::vec3 dposFocus;
    glm::vec3 vFocus;
    glm::vec3 posScreen;
    glm::vec3 vScreen;
    float swPanPos;
    float swTiltPos;
    float sv;
    float swPanMat;
    float swTiltMat;
    glm::vec3 posCenterPrev;
    glm::vec3 posClear;
    glm::mat3 matClear;
    struct SO* psoFocusPrev;
    CPDEFI cpdefiPrev;
    int cpaloFade;
    struct ALO* apaloFade[8];
    float tActivateCplcy;
    glm::mat3 matRotateToCam;
    glm::mat3 matRotateTiltToCam;
    int ccpr;
    CPR acpr[8];
    // Manual camera
    CPMAN cpman;
    CPLOOK cplook;
    CPALIGN cpalign;
    CPASEG cpaseg;
    CPTN cptn;
};

void InitCplcy(CPLCY* pcplcy, CM* pcm);
void InitCplook(CPLOOK* pcplook, CM* pcm);
void InitCpalign(CPALIGN* pcpalign, CM* pcm);
// Update manual camera
void UpdateCpman(GLFWwindow* window, CPMAN* pcpman, CPDEFI* pcpdefi, float dt);

inline VTCPMAN   g_vtcpman;
inline VTCPLOOK  g_vtcplook;
inline VTCPALIGN g_vtcpalign;
inline VTCPASEG  g_vtcpaseg;
inline VTCPTN    g_vtcptn;
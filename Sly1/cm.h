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
struct FGFN 
{
    float duFogBias;
    float ruFog;
    float sNearFog;
    float duFogPlusClipBias;
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
// Camera Object
class CM : public LO
{
public:
    glm::mat4 matProj;
    glm::mat4 matWorldToClip;
    glm::mat4 matClipToWorld;
    glm::vec3 anormalFrustrum[4];
    float rMRD;
    float radFOV;
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
    CPMAN cpman;
    CPLOOK cplook;
    CPALIGN cpalign;
    CPASEG cpaseg;
    CPTN cptn;
};

// Initialize camera object
void InitCm(CM* pcm); // NOT FINISHED

// Global pointer to parent camera object
extern inline CM *g_pcm = nullptr;
#pragma once
#define CPLCY_BUILDING
#include "lo.h"
#include "glob.h"
#undef CPLCY_BUILDING
#include <array>

extern bool g_fDisableInput;

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
    GRFCPD grfcpd;
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
    class SO* psoFocus;
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

void InitCplcy(CPLCY* pcplcy, CM* pcm);
int  FActiveCplcy(CPLCY* pcplcy);

struct VTCPLCY
{
    void (*pfnActivateCplcy)(CPLCY*, void*) = nullptr;
    void (*pfnDeactivateCplcy)(CPLCY*, void*) = nullptr;
    void (*pfnSetCplcy)(CPLCY*, void*) = nullptr;
    void (*pfnRevokeCplcy)(CPLCY*, void*) = nullptr;
    void (*pfnUpdateCplcy)(CPLCY*, CPDEFI*, JOY*, float) = nullptr;
};

#pragma once
#include "lo.h"

enum CSPK 
{
    CSPK_Nil = -1,
    CSPK_AutoCycle = 0,
    CSPK_Manual = 1,
    CSPK_Max = 2
};
enum RWMTK 
{
    RWMTK_Nil = -1,
    RWMTK_Current = 0,
    RWMTK_Predict = 1,
    RWMTK_Homing = 2,
    RWMTK_Max = 3
};
struct RWC
{
    OID oid;
    int cpso;
    struct SO** apso;
    int fReuse;
    int fEnabled;
    int cpsoCloned;
};
struct RWFI 
{
    struct PNT* ppnt;
    struct XFM* pxfm;
    struct ALO* palo;
    glm::vec3 pos;
    glm::mat3 mat;
    glm::vec3 v;
    glm::vec3 w;
    LM lmSw;
    float tMax;
};
struct RWTI 
{
    struct ALO* palo;
    struct TARGET* ptarget;
    glm::vec3 pos;
    RWMTK rwmtk;
    float dtPredict;
    CLQ clqDistance;
    LM lmDistance;
    CLQ clqSToDtPredict;
    LM lmDtPredict;
};
struct RWAC 
{
    int grfask;
    float radTilt;
    float radPan;
    float sv;
    struct XFM* pxfm;
    struct ALO* palo;
};

class RWM : public LO
{
	public:
        int crwc;
        RWC arwc[4];
        CSPK cspk;
        int irwc;
        struct SO* psoAmmo;
        OID oidFire;
        OID oidTarget;
        OID oidAim;
        RWFI rwfi;
        RWTI rwti;
        RWAC rwac;
        int fPostLoad;
};

RWM* NewRwm();
void InitRwm(RWM* prwm);
void CloneRwm(RWM* prwm, RWM* prwmBase);
int  GetRwmSize();
void DeleteRwm(RWM* prwm);
#pragma once
#include "so.h"

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

enum RWMCLONEK
{
    RWMCLONEK_Nil = 0,
    RWMCLONEK_Base = 1,
    RWMCLONEK_Clone = 2
};

struct RWMAMMOXF
{
    glm::vec3 posOrig;
    glm::vec3 eulOrig;
};

struct RWMCLONE
{
    RWM*prwm;

    bool fSharedAmmo;
    RWMCLONEK clonek;

    std::vector<RWMAMMOXF> arwmammoxf;
    int irwmammoxfCur;

    DL dlClone;

    RWM*prwmBase;
    DLE dleClone;
};

struct RWC
{
    short oid;
    int cpsoMax;
    int cpso;
    std::vector <SO*> apso;
    uint32_t grfrwc;
};
struct RWFI 
{
    struct PNT*ppnt;
    struct XFM*pxfm;
    class ALO*palo;
    glm::vec3 pos;
    glm::mat3 mat;
    glm::vec3 v;
    glm::vec3 w;
    LM lmSw;
    float tMax;
};
struct RWTI 
{
    class ALO*palo;
    struct TARGET*ptarget;
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
    struct XFM*pxfm;
    class ALO*palo;
};

class RWM : public LO
{
	public:
    int crwc;
    std::vector <RWC> arwc;
    std::shared_ptr <RWMCLONE> prwmclone;
    CSPK cspk;
    int irwc;
    class SO *psoAmmo;
    short oidFire;
    short oidTarget;
    short oidAim;
    int oidAimXfm;
    RWFI rwfi;
    RWTI rwti;
    RWAC rwac;
    int fPostLoad;
    ALO*paloFireContext;
};

RWM*NewRwm();
void InitRwm(RWM*prwm);
bool*PrwmcloneEnsureRwmFSharedAmmo(RWM*prwm, ENSK ensk);
void*GetRwmCspk(RWM*prwm);
void SetRwmCspk(RWM*prwm, CSPK cspk);
void*GetRwmIrwc(RWM*prwm);
void SetRwmIrwc(RWM*prwm, int irwc);
void*GetRwmPsoAmmo(RWM*prwm);
void SetRwmPsoAmmo(RWM* prwm, SO* psoAmmo);
void*GetRwmOidFire(RWM* prwm);
void SetRwmOidFire(RWM*prwm, int oidFire);
void*GetRwmRwfiPpnt(RWM*prwm);
void SetRwmRwfiPpnt(RWM*prwm, PNT*ppnt);
void*GetRwmRwfiPxfm(RWM*prwm);
void SetRwmRwfiPxfm(RWM*prwm, XFM*pxfm);
void*GetRwmRwfiPalo(RWM*prwm);
void SetRwmRwfiPalo(RWM*prwm, ALO*palo);
void*GetRwmRwfiPos(RWM*prwm);
void SetRwmRwfiPos(RWM*prwm, glm::vec3 pos);
void*GetRwmRwfiMat(RWM*prwm);
void SetRwmRwfiMat(RWM*prwm, glm::mat3 mat);
void*GetRwmRwfiV(RWM*prwm);
void SetRwmRwfiV(RWM*prwm, glm::vec3 v);
void*GetRwmRwfiW(RWM*prwm);
void SetRwmRwfiW(RWM*prwm, glm::vec3 w);
void*GetRwmRwfiLmSw(RWM*prwm);
void SetRwmRwfiLmSw(RWM*prwm, LM lmSw);
void*GetRwmRwfiTMax(RWM*prwm);
void SetRwmRwfiTMax(RWM*prwm, float tMax);
void*GetRwmOidTarget(RWM*prwm);
void SetRwmOidTarget(RWM*prwm, int oidTarget);
void*GetRwmRwtiPalo(RWM*prwm);
void SetRwmRwtiPalo(RWM*prwm, ALO*palo);
void*GetRwmRwtiPtarget(RWM*prwm);
void SetRwmRwtiPtarget(RWM*prwm, TARGET*ptarget);
void*GetRwmRwtiPos(RWM*prwm);
void SetRwmRwtiPos(RWM*prwm, glm::vec3 pos);
void*GetRwmRwtiRwmtk(RWM*prwm);
void SetRwmRwtiRwmtk(RWM*prwm, RWMTK rwmtk);
void*GetRwmRwtiClqDistance(RWM*prwm);
void SetRwmRwtiClqDistance(RWM*prwm, CLQ clqDistance);
void*GetRwmRwtiLmDistance(RWM*prwm);
void SetRwmRwtiLmDistance(RWM*prwm, LM lmDistance);
void*GetRwmRwtiClqSToDtPredict(RWM*prwm);
void SetRwmRwtiClqSToDtPredict(RWM*prwm, CLQ clqSToDtPredict);
void*GetRwmRwtiLmDtPredict(RWM*prwm);
void SetRwmRwtiLmDtPredict(RWM*prwm, LM lmDtPredict);
void*GetRwmRwacGrfask(RWM*prwm);
void SetRwmRwacGrfask(RWM*prwm, GRFASK grfask);
void*GetRwmOidAimXfm(RWM* prwm);
void SetRwmOidAimXfm(RWM*prwm, int oidAimXfm);
void*GetRwmRwacPxfm(RWM*prwm);
void SetRwmRwacPxfm(RWM*prwm, XFM*pxfm);
void*GetRwmRwacRadTilt(RWM*prwm);
void SetRwmRwacRadTilt(RWM*prwm, float radTilt);
void*GetRwmRwacRadPan(RWM*prwm);
void SetRwmRwacRadPan(RWM*prwm, float radPan);
void*GetRwmRwacSv(RWM*prwm);
void SetRwmRwacSv(RWM*prwm, float sv);
void*GetRwmOidAim(RWM* prwm);
void SetRwmOidAim(RWM*prwm, int oidAim);
void*GetRwmPaloFireContext(RWM*prwm);
void SetRwmPaloFireContext(RWM* prwm, ALO* paloContext);
void OnRwmRemove(RWM*prwm);
RWMCLONE* PrwmcloneEnsureRwm(RWM* prwm, ENSK ensk);
void AddRwmCache(RWM*prwm, OID oid, int cpso, bool fReuse, bool fEnabled);
void BindRwmAmmo(RWM*prwm);
void CloneRwm(RWM*prwm, RWM*prwmBase);
void PostRwmLoad(RWM*prwm);
void FreezeRwm(RWM*prwm, int fFreeze);
RWC*PrwcFindRwmCache(RWM*prwm, OID oidCache);
void EnableRwmCache(RWM*prwm, OID oidCache);
void DisableRwmCache(RWM*prwm, OID oidCache);
void ResizeRwmCache(RWM*prwm, OID oidCache, int cpso);
int  FIsRwmAmmo(RWM*prwm, SO*psoAmmo);
bool FAvailableRwmAmmo(RWM*prwm, SO*psoAmmo);
void CycleRwmCache(RWM*prwm);
void ReloadRwm(RWM*prwm);
int  FEnsureRwmLoaded(RWM*prwm);
int  FFireRwm(RWM*prwm, bool fAim);
void ClearRwmAmmo(RWM*prwm);
void ClearRwmFireInfo(RWM*prwm);
void ClearRwmTargetInfo(RWM*prwm);
void ClearRwmAimControl(RWM*prwm);
void GetRwfiPosMat(const RWFI*prwfi, glm::vec3*ppos, glm::mat3*pmat, ALO*palo);
void GetRwtiPos(RWTI*prwti, const glm::vec3*pposSource, glm::vec3*ppos, ALO*paloReference);
void GetRwacPan(const RWAC*prwac, float*pradPan, ALO*paloReference);
void GetRwacTilt(const RWAC*prwac, float*pradTilt, ALO*paloReference);
int  GetRwmSize();
void DeleteRwm(RWM*prwm);

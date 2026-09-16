#pragma once
#include "ac.h"

struct TRANS;

#include "transition.h"

enum SEGRPT
{
    SEGRPT_Nil = -1,
    SEGRPT_Once = 0,
    SEGRPT_Loop = 1,
    SEGRPT_PingPong = 2,
    SEGRPT_Manual = 3,
    SEGRPT_Max = 4
};
enum FAPL 
{
    FAPL_Nil = -1,
    FAPL_Smooth = 1,
    FAPL_HandleEvents = 2,
    FAPL_Snap = 4,
    FAPL_Max = 5
};

enum SEEK 
{
    SEEK_Nil = -1,
    SEEK_Start = 0,
    SEEK_Current = 1,
    SEEK_End = 2,
    SEEK_EndDoEvents = 3,
    SEEK_Max = 4
};
enum EATYP 
{
    EATYP_Nil = -1,
    EATYP_SoundEffect = 0,
    EATYP_SoundStop = 1,
    EATYP_MusicStart = 2,
    EATYP_MusicStop = 3,
    EATYP_Seek = 4,
    EATYP_SetSpeed = 5,
    EATYP_ShowDialog = 6,
    EATYP_SeekIfMessage = 7,
    EATYP_Retract = 8,
    EATYP_ShowObject = 9,
    EATYP_HideObject = 10,
    EATYP_Pause = 11,
    EATYP_EnableCamera = 12,
    EATYP_DisableCamera = 13,
    EATYP_EnableHandsOff = 14,
    EATYP_DisableHandsOff = 15,
    EATYP_SetObjectInteract = 16,
    EATYP_EnableObjectDrive = 17,
    EATYP_DisableObjectDrive = 18,
    EATYP_EnableObjectPuppet = 19,
    EATYP_DisableObjectPuppet = 20,
    EATYP_EnableObjectPosErrorFunc = 21,
    EATYP_DisableObjectPosErrorFunc = 22,
    EATYP_EnableObjectRotErrorFunc = 23,
    EATYP_DisableObjectRotErrorFunc = 24,
    EATYP_Wipe = 25,
    EATYP_WipeOnButton = 26,
    EATYP_TriggerDefaultExit = 27,
    EATYP_StartAnimation = 28,
    EATYP_EnableObjectScale = 29,
    EATYP_DisableObjectScale = 30,
    EATYP_EnableObjectMesh = 31,
    EATYP_DisableObjectMesh = 32,
    EATYP_StartTimer = 33,
    EATYP_StopTimer = 34,
    EATYP_CutObjectPos = 35,
    EATYP_CutObjectRot = 36,
    EATYP_StartRumble = 37,
    EATYP_SendUserMessage = 38,
    EATYP_Label = 39,
    EATYP_Splice = 40,
    EATYP_Waypoint = 41,
    EATYP_PauseForVag = 42,
    EATYP_StopVag = 43,
    EATYP_PreloadVag = 44,
    EATYP_PoseCombo = 45,
    EATYP_SetSmGoal = 46,
    EATYP_RetractSm = 47,
    EATYP_Explode = 48,
    EATYP_ShowLetterbox = 49,
    EATYP_HideLetterbox = 50,
    EATYP_ShowMovie = 51,
    EATYP_PlaceCredits = 52,
    EATYP_VacateCredits = 53,
    EATYP_SetCredit = 54,
    EATYP_StartSplash = 55,
    EATYP_StartAttract = 56,
    EATYP_SetDark = 57,
    EATYP_SetDarkSmooth = 58,
    EATYP_StartScript = 59,
    EATYP_Max = 60
};

struct CHN
{
    OID oid;
    std::shared_ptr <ACP> pacp;
    std::shared_ptr <ACR> pacr;
    std::shared_ptr <ACS> pacs;
    std::shared_ptr <ACG> pacgTwist;
    std::vector <std::shared_ptr<ACG>> apacgPose;
    int cpacgPose;
};

struct OVR
{
    OID oid;
    class ALO* palo;
};

struct EASND
{
    s16 csfxid;
    SFXID asfxid[6];
    s16 isfxidLast;
    OID oid;
    s32 sFull;
    s32 sStop;
    float gPitch;
    float gDoppler;
    float gVolume;
};

struct EAEF
{
    OID oid;
    float sFull;
    float sStop;
};

struct EAA
{
    OID oidAseg;
    OID oidRoot;
    float tLocal;
    float svtLocal;
};

struct EAEX
{
    OID oidExpl;
    OID oidRef;
    OID oidTouch;
    float gScale;
};

struct EA
{
    float tLocal;
    EATYP eatyp;

    struct { std::shared_ptr <char[]> pchz; } string;
    struct { std::shared_ptr <EASND> peasnd; } sound;
    struct { float tToLocal; float svtLocal; } seek;
    struct { float svtLocal; } setspeed;
    struct { OID oid; } object;
    struct { OID oid; s16 ctForce; s16 ctTorque; } setinteract;
    struct { std::shared_ptr <EAEF> peaef; } errorfunc;
    struct { s16 wipek; std::shared_ptr <TRANS> ptrans; } wipe;
    struct { int fInhibitAnims; int wipek; } defexit;
    struct { std::shared_ptr <EAA> peaa; } animation;
    struct { float dt; } timer;
    struct { s16 dframe; s16 rumk; } rumble;
    struct { OID oid; OID oidLabel; } sendusermessage;
    struct { OID oidLabel; } label;
    struct { OID oid; int fSnap; } puppet;
    struct { int frame; } splice;
    struct { OID oidTarget; OID oidCombo; } posecombo;
    struct { s16 oidSm; s16 oidRoot; s16 oidState; } setsm;
    struct { OID oidSm; OID oidRoot; } retractsm;
    struct { std::shared_ptr <EAEX> peaex; } explode;
    struct { OID oid; float dt; } showhide;
    struct { s16 x; s16 y; s16 cline; } place;
    struct { s16 iline; s16 dtVisible; std::shared_ptr<char[]> pchz; } credit;
    struct { float rDark; } dark;
};

class ASEG : public LO
{
    public:
    float tMax;
    int cchn;
    std::vector <CHN> achn;
    short oidRoot;
    SEGRPT segrpt;
    int fDefault;
    int fHandsOff;
    int fRealClock;
    int ceaApply;
    std::vector <EA> aeaApply;
    int ceaFrame;
    std::vector <EA> aeaFrame;
    int ceaRetract;
    std::vector <EA> aeaRetract;
    DL dlAsega;
    int nPriority;
    float svtMaster;
    CLQ clqMasterSuck;
    LM lmMasterSuck;
    float svtMasterSuck;
    struct CHN* pchnStrip;
    int coidSearchRoot;
    std::vector <short> aoidSearchRoot;
};

//GOTTA COME BACK HERE
ASEG* NewAseg();
void InitAseg(ASEG* paseg);
int  GetAsegSize();
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);
void LoadAsegEventsFromBrx(ASEG* paseg, CBinaryInputStream* pbis, int fFrame, int* pcea, std::vector<EA>& aea);
void*GetAsegSegrpt(ASEG* paseg);
void SetAsegSegrpt(ASEG* paseg, SEGRPT segrpt);
void*GetAsegfDefault(ASEG* paseg);
void SetAsegfDefault(ASEG* paseg, int fDefault);
void*GetAsegfHandsOff(ASEG* paseg);
void SetAsegfHandsOff(ASEG* paseg, int fHandsOff);
void*GetAsegnPriority(ASEG* paseg);
void SetAsegnPriority(ASEG* paseg, int nPriority);
void*GetAsegsvtMaster(ASEG* paseg);
void SetAsegsvtMaster(ASEG* paseg, float svtMaster);
void*GetAsegclqMasterSuck(ASEG* paseg);
void SetAsegclqMasterSuck(ASEG* paseg, const CLQ& clqMasterSuck);
void*GetAseglmMasterSuck(ASEG* paseg);
void SetAseglmMasterSuck(ASEG* paseg, const LM& lmMasterSuck);
void*GetAsegtMax(ASEG* paseg);
void SetAsegtMax(ASEG* paseg, float tMax);
void*GetAsegfRealClock(ASEG* paseg);
void SetAsegfRealClock(ASEG* paseg, int fRealClock);
void CloneAseg(ASEG* paseg, ASEG* pasegBase);
void PostAsegLoad(ASEG* paseg);
void PostAsegLoadCallback(ASEG* paseg, MSGID msgid, void* pvData);
void ApplyAsegOvr(ASEG* paseg, ALO* paloAsegRoot, int covr, OVR* aovr, float tLocal, float svtLocal, GRFAPL grfapl, ASEGA** ppasega);
void ApplyAseg(ASEG* paseg, ALO* paloAsegRoot, float tLocal, float svtLocal, int grfapl, ASEGA** ppasega);
ASEGA* PasegaApplyAseg(ASEG* paseg, ALO* paloAsegRoot, float tLocal, float svtLocal, GRFAPL grfapl);
ASEGA* PasegaFindAseg(ASEG* paseg, ALO* paloRoot);
void EnsureAseg(ASEG* paseg, ALO* paloRoot, SEEK seek, float t, float svt, GRFAPL grfapl, ASEGA** ppasega);
ASEGA* PasegaEnsureAseg(ASEG* paseg, ALO* paloRoot, SEEK seek, float t, float svt, GRFAPL grfapl);
void ApplyAsegCur(ASEG* paseg, ALO* paloRoot, float t, float svt, GRFAPL grfapl, ASEGA** ppasega);
int  FWipingAseg(ASEG* paseg);
void FindAsegClosestPoint(ASEG* paseg, ALO* palo, const glm::vec3* ppos, float t, float* ptClosest, glm::vec3* pposClosest, glm::vec3* pvClosest);
EA* PeaFindAsegLabel(ASEG* paseg, OID oidLabel);
float TFindAsegLabel(ASEG* paseg, OID oidLabel);
CHN* PchnFindAseg(ASEG* paseg, ALO* palo);
void PredictAsegEffect(ASEG* paseg, float t, float svt, ALO* paloLeaf, ALO* paloBasis, int cpaloPredict, glm::vec3* apos, glm::mat3* amat, glm::vec3* av, glm::vec3* aw);
void GetAsegWaypoints(ASEG* paseg, ALO* paloContext, int* pcpwaypoint, std::vector<WAYPOINT*>& waypoints);
void StripAsegChn(ASEG* paseg, CHN* pchn);
void StripAsegAlo(ASEG* paseg, ALO* palo);
void DeleteAseg(ASEG *paseg);

CHN* PchnFromAsegIchn(ASEG* paseg, int ichn);
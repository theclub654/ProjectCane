#pragma once
#include "jt.h"

enum FTAK 
{
    FTAK_Nil = -1,
    FTAK_None = 0,
    FTAK_Reach = 1,
    FTAK_Sweep = 2,
    FTAK_Whack = 2,
    FTAK_Smash = 4,
    FTAK_Rush = 8,
    FTAK_Bomb = 16,
    FTAK_Gun = 16,
    FTAK_Max = 17,
    FTAK_Shock = 32
};

enum JSGEK
{
    JSGEK_Context = 0,
    JSGEK_Cut = 1,
    JSGEK_Label = 2,
    JSGEK_Tunnel = 3,
    JSGEK_Pause = 4,
    JSGEK_Jump = 5,
    JSGEK_Run = 6,
    JSGEK_Climb = 7,
    JSGEK_Attack = 8,
    JSGEK_Aseg = 9,
    JSGEK_Focus = 10,
    JSGEK_Clock = 11,
    JSGEK_Hide = 12,
    JSGEK_Max = 13
};

enum JSGJK 
{
    JSGJK_Nil = -1,
    JSGJK_Fall = 0,
    JSGJK_Jump = 1,
    JSGJK_DoubleJump = 2,
    JSGJK_Max = 3
};

struct JSGE
{
    JSGEK jsgek;
    short oid;
    int fAsync;

    union
    {
        struct
        {
            OID oid;
        } label;

        struct
        {
            float dtPause;
        } pause;

        struct
        {
            JSGJK jsgjk;
            JSGLK jsglk;
        } jump;

        struct
        {
            float uRun;
        } run;

        struct
        {
            float uClimb;
        } climb;

        struct
        {
            FTAK ftak;
        } attack;

        struct
        {
            float rdt;
        } clock;
    } data;
};

class JSG : public LO
{
	public:
    int cjsge;
    std::vector <JSGE> ajsge;
    class JT* pjt;
    int ijsgeCur;
    float tCur;
    struct JSGE* pjsgeJoy;
    struct LO* ploContext;
    struct ASEGA* pasegaCur;
    struct LO* ploFocus;
    struct TN* ptnCur;
    int fHideTn;
    float uRail; 
    float sPipe;
    int fHandsOff;
};

JSG* NewJsg();
void InitJsg(JSG* pjsg);
void*GetJsgFHandsOff(JSG* pjsg);
void SetJsgFHandsOff(JSG* pjsg, int fHandsOff);
void CloneJsg(JSG* pjsg, JSG* pjsgBase);
void GetJsgLocation(JSG* jsg, LO* plo, glm::vec3* outPos, float* outRot);
void SetJsgFocus(JSG* jsg, LO* focus);
void SetJsgTn(JSG* jsg, TN* tn);
void NextJsgJsge(JSG* jsg);
bool FIsJsgJsgeComplete(JSG* pjsg, JSGE* pjsge);
void UpdateJsgJsge(JSG* pjsg);
void ReadJsgJoystick(JSG* pjsg, JOY* pjoy);
bool FIsJsgActive(JSG* pjsg);
void ClearJsg(JSG* pjsg);
void ApplyJsg(JSG* pjsg, JT* pjt);
void RetractJsg(JSG* pjsg);
JSGE* PjsgeAllocJsg(JSG* pjsg, JSGEK jsgek, OID oid, int fAsync);
void AddJsgContext(JSG* pjsg, OID oid);
void AddJsgCut(JSG* pjsg, OID oid);
void AddJsgFocus(JSG* pjsg, OID oid);
void AddJsgLabel(JSG* pjsg, OID oid);
void AddJsgTunnel(JSG* pjsg, OID oid);
void AddJsgPause(JSG* pjsg, OID oid, float dtPause);
void AddJsgAseg(JSG* pjsg, OID oid, int fAsync);
void AddJsgJump(JSG* pjsg, OID oid, JSGJK jsgjk, JSGLK jsglk, int fAsync);
void AddJsgRun(JSG* pjsg, OID oid, float uRun, int fAsync);
void AddJsgClimb(JSG* pjsg, OID oid, float uClimb, int fAsync);
void AddJsgAttack(JSG* pjsg, OID oid, FTAK ftak, int fAsync);
void AddJsgClock(JSG* pjsg, float rdt);
void AddJsgHide(JSG* pjsg);
int  GetJsgSize();
void DeleteJsg(JSG* pjsg);

static float DZ_JsgHndTarget = -200;
static float SD_JsgRunComplete = 25.0;   
static float DS_JsgClimbComplete = 15.0;
inline float DT_JsgTargetFall = 0.2;
inline float DT_JsgTargetBoost = 0.5;
inline float DS_JsgClimbSample = 50.0;
static JSGE s_ajsge[128];

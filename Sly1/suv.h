#pragma once
#include "po.h"

enum SVEK 
{
    SVEK_Nil = -1,
    SVEK_BoostCollect = 0,
    SVEK_BoostOpportunity = 1,
    SVEK_CheckPoint = 2,
    SVEK_Feature = 3,
    SVEK_Max = 4
};
enum SUVGK 
{
    SUVGK_Nil = -1,
    SUVGK_Race = 0,
    SUVGK_Chase = 1,
    SUVGK_Max = 2
};
enum SUVS 
{
    SUVS_Nil = -1,
    SUVS_Stop = 0,
    SUVS_Auto = 1,
    SUVS_Manual = 2,
    SUVS_Chase = 3,
    SUVS_Max = 4
};
struct SVRB
{
    CLQ clqDuToRsvAhead;
    CLQ clqDuToRsvBehind;
    LM lmRsvRandom;
    LM lmRsvBalance;
    float mpcpsuvuBoost[4];
};
struct SXP
{
    struct ALO* paloWheel;
    struct ALO* paloHub;
    struct PNT* ppntTop;
    int xsxp;
    glm::vec3 posCur;
    glm::vec3 posGround;
    glm::vec3 normalGround;
    glm::vec3 vGround;
    float swSpin;
    float radSpin;
    float cParticleDirt;
    float cParticleDust;
};
struct SVE 
{
    SVEK svek;
    struct VOL* pvol;
    int f;
    int isveCheck;
    float sTrack;
    struct SVE* psveCheckPrev;
    float mu;
    float svMax;
    struct EXPL* pexplDirt;
    struct EXPL* pexplDust;
};

class SUV : public PO
{
	public:
        float sRadiusFrontWheel;
        float sRadiusRearWheel;
        float svMax;
        float dyMax;
        CLQ clqTune;
        SVRB asvrb[2];
        SVRB svrb;
        struct ALO* paloShadow;
        float radTarget;
        float svTarget;
        float radFront;
        int xsxp;
        SXP asxp[4];
        struct SHAPE* pshapeTrack;
        float sTrackMax;
        float uTrack;
        float sTrack;
        float dyTrack;
        float dyTarget;
        float dsTrackFinish;
        struct SHAPE* pshapeLine;
        float uLine;
        float tUpright;
        float tBoost;
        int cBoost;
        float rsvBalance;
        float rsvGoal;
        float tBalance;
        float muSxp;
        float tPunched;
        int csve;
        SVE asve[16];
        struct SVE* psveCheckFirst;
        struct SVE* psveCheckCur;
        struct SVE* psveFeatureCur;
        int cLap;
        int cLapMax;
        int nPlace;
        int nPlaceMax;
        float tPlace;
        struct EMITTER* pemitterBoost;
        struct EXPL* pexplDirt;
        struct EXPL* pexplDust;
        float cParticleDirt;
        float cParticleDust;
        struct ALO* paloFrontAxle;
        struct ALO* paloRearAxle;
        SUVGK suvgk;
        SUVS suvs;
        float tSuvs;
        struct PATHZONE* ppathzone;
        struct SO* psoPrizeCur;
        struct SM* psmPuncher;
        struct SMA* psmaPuncher;
        struct SO* psoPuncher;
        int cpsoIgnore;
        struct SO* apsoIgnore[8];
        struct XP* pxpPuncher;
        struct MURRAY* pmurray;
        int fFreeWheeling;
        struct AMB* pambSkid;
        struct AMB* pambRunning;
        struct AMB* pambIdle;
        struct AMB* pambBoost;
        float volBoostTarget;
        float volBoostCur;
        float volTarget;
        float volCur;
        float dvolInc;
        float frqTarget;
        float frqCur;
        float dfrqInc;
        float dfrqTop;
};

void InitSuv(SUV *psuv);
void DeleteSUV(LO* plo);
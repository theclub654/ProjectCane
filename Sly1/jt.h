#pragma once
#include "step.h"

struct XMG
{
    struct ASEG* paseg;
    int fValid;
    float sSphere;
    glm::vec3 posExtrem;
    glm::vec3 posSphere;
    glm::vec3 posUp;
    glm::vec3 posDown;
};
enum SSLF
{
    SSLF_Nil = -1,
    SSLF_Right = 0,
    SSLF_Left = 1
};
enum JTS 
{
    JTS_Nil = -1,
    JTS_Stand = 0,
    JTS_Run = 1,
    JTS_Jump = 2,
    JTS_Hang = 3,
    JTS_Zap = 4,
    JTS_Sidestep = 5,
    JTS_Hide = 6,
    JTS_Pipe = 7,
    JTS_Edge = 8,
    JTS_Celebrate = 9,
    JTS_Rush = 10,
    JTS_Puppet = 11,
    JTS_Peek = 12,
    JTS_Ski = 13,
    JTS_Ball = 14,
    JTS_Max = 15
};
enum JTBS 
{
    JTBS_Nil = -1,
    JTBS_Jump_Init = 0,
    JTBS_Jump_Rise = 1,
    JTBS_Jump_Fall = 2,
    JTBS_Jump_Boost = 3,
    JTBS_Jump_In = 4,
    JTBS_Jump_Out = 5,
    JTBS_Jump_Thrown = 6,
    JTBS_Jump_Smash = 7,
    JTBS_Jump_Dive = 8,
    JTBS_Jump_Target = 9,
    JTBS_Jump_Spire = 10,
    JTBS_Jump_Rail = 11,
    JTBS_Jump_Hshape = 12,
    JTBS_Jump_Hpnt = 13,
    JTBS_Hide_Stand = 14,
    JTBS_Hide_Sidestep = 15,
    JTBS_Hide_PeekLeft = 16,
    JTBS_Hide_PeekRight = 17,
    JTBS_Hide_Run = 18,
    JTBS_Hide_Spin = 19,
    JTBS_Pipe_Init = 20,
    JTBS_Pipe_Stay = 21,
    JTBS_Pipe_Up = 22,
    JTBS_Pipe_Down = 23,
    JTBS_Pipe_Spin = 24,
    JTBS_Celebrate_Key = 25,
    JTBS_Celebrate_Timed = 26,
    JTBS_Celebrate_UseKey = 27,
    JTBS_Rush_Attack = 28,
    JTBS_Rush_Bounce = 29,
    JTBS_Peek_Enter = 30,
    JTBS_Peek_Peek = 31,
    JTBS_Peek_Exit = 32,
    JTBS_Zap_Blunt = 33,
    JTBS_Zap_Electric = 34,
    JTBS_Zap_Fire = 35,
    JTBS_Zap_Water = 36,
    JTBS_Zap_Crush = 37,
    JTBS_Zap_Pit = 38,
    JTBS_Zap_Thrown = 39,
    JTBS_Zap_Dead = 40,
    JTBS_Zap_DeadFront = 41,
    JTBS_Zap_DeadInWater = 42,
    JTBS_Zap_DeadInPit = 43,
    JTBS_Zap_DeadCrush = 44,
    JTBS_Ski_Stand = 45,
    JTBS_Ski_Jump = 46,
    JTBS_Ski_Fall = 47,
    JTBS_Ski_Boost = 48,
    JTBS_Ski_Attack = 49,
    JTBS_Hang_Forward = 50,
    JTBS_Hang_Neutral = 51,
    JTBS_Hang_Backward = 52,
    JTBS_Hang_Idle = 53,
    JTBS_Ball_Fall = 54,
    JTBS_Ball_Roll = 55,
    JTBS_Ball_Recover = 56,
    JTBS_Max = 57
};
enum JTCS
{
    JTCS_Nil = -1,
    JTCS_Reach = 0,
    JTCS_Hang = 1,
    JTCS_Pipe = 2,
    JTCS_SweepInit = 3,
    JTCS_SweepFore = 4,
    JTCS_SweepBack = 5,
    JTCS_StandingSweepFore = 6,
    JTCS_StandingSweepBack = 7,
    JTCS_Max = 8
};
enum JTJK 
{
    JTJK_Nil = -1,
    JTJK_Standing = 0,
    JTJK_Swinging = 1,
    JTJK_Running = 2,
    JTJK_FromWater = 3,
    JTJK_Edge = 4,
    JTJK_Max = 5
};
enum JSGLK
{
    JSGLK_Nil = -1,
    JSGLK_Normal = 0,
    JSGLK_Big = 1,
    JSGLK_Max = 2
};
enum JTJJ
{
    JTJJ_Nil = -1,
    JTJJ_Init = 0,
    JTJJ_FullDeflection = 1,
    JTJJ_PartialDeflection = 2,
    JTJJ_Default = 3,
    JTJJ_Max = 4
};
struct FX 
{
    glm::vec3 dv;
    glm::vec3 dw;
};
enum JTHK 
{
    JTHK_Nil = -1,
    JTHK_Flatten = 0,
    JTHK_Duck = 1,
    JTHK_Basket = 2,
    JTHK_Nonchalant = 3,
    JTHK_Rail = 4,
    JTHK_Spire = 5,
    JTHK_Reach = 6,
    JTHK_Vault = 7,
    JTHK_Max = 8
};
enum XPK 
{
    XPK_Nil = -1,
    XPK_Illegal = 0,
    XPK_PointToSurface = 1,
    XPK_EdgeToSurface = 2,
    XPK_EdgeToEdge = 3,
    XPK_SphereToPoint = 4,
    XPK_SphereToEdge = 5,
    XPK_SphereToSurface = 6,
    XPK_SphereToSphere = 7,
    XPK_CenterToSurface = 8,
    XPK_CenterToSphere = 9,
    XPK_Internal = 10,
    XPK_Equality = 11,
    XPK_Limited = 12,
    XPK_Max = 13
};
enum HFDK 
{
    HFDK_Nil = -1,
    HFDK_Match = 0,
    HFDK_Left = 1,
    HFDK_Right = 2,
    HFDK_Stand = 3,
    HFDK_Max = 4
};
enum JTFOCK
{
    JTFOCK_Nil = -1,
    JTFOCK_Relax = 0,
    JTFOCK_Clamp = 1,
    JTFOCK_Focus = 2,
    JTFOCK_Max = 3
};
struct ZQE {
    JTBS jtbs;
    int grfzqe;
    glm::vec3 v;
    glm::vec3 w;
};
struct ZPR 
{
    ZPK zpk;
    struct LO* ploSource;
    void* pv;
    void* pfnzap;
};
enum JTPDK 
{
    JTPDK_Nil = -1,
    JTPDK_Up = 0,
    JTPDK_Down = 1,
    JTPDK_Max = 2
};
struct WEXPL 
{
    struct EXPL* pexpl;
    float tExplodeLast;
};

class JT : public STEP
{
	public:
		BLRUN ablrunRun[6];
		struct ASEGBL* pasegblRun;
		char ablssSidestep[256];
		struct ASEGBL* pasegblSidestep;
		SSLF sslfCur;
		char ablssFlatten[256];
        struct ASEGBL* pasegblFlatten;
        struct ASEG* pasegFlattenStand;
        struct ASEG* pasegFlattenPeekLeft;
        struct ASEG* pasegFlattenPeekRight;
        BLSS ablssDuck[8];
        struct ASEGBL* pasegblDuck;
        struct ASEG* pasegDuckStand;
        struct ASEG* pasegDuckPeekLeft;
        struct ASEG* pasegDuckPeekRight;
        BLRUN ablrunBasket[6];
        struct ASEGBL* pasegblBasket;
        struct ASEG* pasegBasketStand;
        struct ASEG* pasegBasketJumpIn;
        struct ASEG* pasegBasketJumpOut;
        BLRUN ablrunRail[6];
        struct ASEGBL* pasegblRail;
        struct ASEG* pasegReachStand;
        struct ASEG* pasegJumpStanding;
        struct ASEG* pasegJumpRunning;
        struct ASEG* pasegJumpSwinging;
        struct ASEG* pasegJumpSmash;
        struct ASEG* pasegJumpTarget;
        struct ASEG* pasegJumpBoost;
        struct ASEG* pasegEdge;
        struct BLCL ablcl[2];
        struct ASEGBL* pasegblClimb;
        struct ASEG* pasegPipeInit;
        struct ASEG* pasegPipeStay;
        struct ASEG* pasegPipeSpin;
        struct ASEG* mpjtbspaseg[57];
        struct SM* psmIdle;
        int fStandTight;
        XMG xmgLeftBall;
        XMG xmgRightBall;
        struct SO* mpjtspso[15];
        struct SO* mpjthkpso[15];
        struct SO* mpjtbspso[57];
        struct XP* pxpZap;
        glm::vec3 posSafeLast;
        SQTRM sqtrmCane;
        int fRenderCaneTrail;
        struct RCHM* prchmReach;
        float tSweepPending;
        int cposHook;
        glm::vec3 aposHook[3];
        struct TARGET* ptargetCur;
        struct HND* phndCur;
        struct PIPE* ppipeCur;
        float sPipeCur;
        struct HND* phndUnhook;
        struct PIPE* ppipeUnhook;
        float sPipeUnhook;
        int fPipeReverse;
        JTS jts;
        float tJts;
        JTBS jtbs;
        float tJtbs;
        struct ASEGA* pasegaCur;
        struct SMA* psmaCur;
        JTCS jtcs;
        float tJtcs;
        struct ASEGA* pasegaJtcs;
        int fCaneInterruptible;
        JTJK jtjk;
        JSGLK jsglk;
        int grfjtsc;
        JTS jtsNext;
        JTBS jtbsNext;
        JTCS jtcsNext;
        JTJK jtjkNext;
        int fShortenJump;
        float svxyAirSteer;
        JTJJ jtjj;
        float mpjtjjt[4];
        float tJumpPending;
        glm::vec3 vTargetCoast;
        float tHangDeflectLast;
        JTBS jtbsHangLatch;
        int fHangReverse;
        struct XA* pxaHang;
        FX afxHang[2];
        BL ablSwing[3];
        struct ASEGBL* pasegblSwing;
        float uSwingForward;
        float uSwingActive;
        struct ALO* paloSkinStandard;
        struct ALO* paloSkinElectric;
        struct ALO* paloSkinFire;
        JTHK jthk;
        struct HSHAPE* phshape;
        float sParamHshape;
        struct HPNT* phpnt;
        struct HBSK* phbsk;
        struct JMT* pjmtCur;
        struct SPIRE* pspireCur;
        struct RAIL* prailCur;
        float uRailCur;
        float sRailCur;
        int fRailReverse;
        struct VAULT* pvaultCur;
        struct ALO* paloTool;
        struct ALO* paloHook;
        struct ALO* paloKey;
        struct ALO* paloKeyCane;
        struct ALO* paloKeyFree;
        struct EXPLS* pexplsKeyTrail;
        struct MATCH* apmatchKey[9];
        int cpmatchKey;
        glm::vec3 posKeyLast;
        ZPK zpk;
        glm::vec3 posPipeHandLocal;
        glm::vec3 posPipeFootLocal;
        glm::vec3 dposPipeXp;
        float sPipeHand;
        float dsPipeHand;
        LM lmPipeHandWindow;
        float sRadiusHook;
        float tContact;
        struct EXPL* pexplHook;
        float swToolSpinTarget;
        float swToolSpin;
        float radToolSpin;
        struct WATER* pwater;
        struct ALO* paloShadow;
        SFF sffRun;
        SFF sffSidestep;
        SFF sffStand;
        SFF sffFlatten;
        SFF sffDuck;
        struct TAIL* ptailTail;
        struct TAIL* ptailMask;
        int fCanBoost;
        float tCancelJump;
        struct SO* psoEdgeCur;
        glm::vec3 posEdgeLocal;
        glm::vec3 normalEdgeCur;
        glm::vec3 dposEdge;
        glm::vec3 posEdgeTarget;
        XPK xpkEdge;
        void* pvEdge;
        struct ALO* paloGut;
        struct ACTADD* pactaddGut;
        struct ACTADJ* pactadjGut;
        struct ACTADJ* pactadjLeftHand;
        struct ACTADJ* pactadjRightHand;
        struct ACTADJ* pactadjLeftFoot;
        struct ACTADJ* pactadjRightFoot;
        struct ACTVAL* pactvalJump;
        glm::vec3 dposSmoothCpdefi;
        struct BL ablSweepAttackFore[2];
        struct ASEGBL* pasegblSweepAttackFore;
        struct BL ablSweepAttackBack[2];
        struct ASEGBL* pasegblSweepAttackBack;
        struct BL ablSweepBounceFore[2];
        struct ASEGBL* pasegblSweepBounceFore;
        struct BL ablSweepBounceBack[2];
        struct ASEGBL* pasegblSweepBounceBack;
        struct BL ablStandingSweepAttackFore[2];
        struct ASEGBL* pasegblStandingSweepAttackFore;
        struct BL ablStandingSweepAttackBack[2];
        struct ASEGBL* pasegblStandingSweepAttackBack;
        struct BL ablStandingSweepBounceFore[2];
        struct ASEGBL* pasegblStandingSweepBounceFore;
        struct BL ablStandingSweepBounceBack[2];
        struct ASEGBL* pasegblStandingSweepBounceBack;
        struct ASEG* pasegRushAttack;
        struct ASEG* pasegRushBounce;
        struct ASEG* pasegSmashAttack;
        struct ASEG* pasegSmashBounce;
        struct ASEG* pasegDiveAttack;
        struct ASEG* pasegPeekEnter;
        struct ASEG* pasegPeekExit;
        struct ASEG* pasegBall;
        struct SO* psoSweepEffect;
        struct SO* psoSmashEffect;
        struct SO* psoRushEffect;
        struct SO* psoDiveEffect;
        int cpaloIgnoreEffect;
        struct ALO* apaloIgnoreEffect[16];
        struct XP* pxpEffect;
        struct TN* ptnCur;
        struct TN* ptnHideDynamic;
        struct TN* ptnKeyCelebration;
        float tMoveLast;
        HFDK hfdk;
        float tHfdk;
        JTFOCK jtfock;
        glm::vec3 posFocus;
        float uFlattenClamp;
        struct EXC* pexc;
        int cspks;
        struct SPKS* aspks;
        int czqe;
        ZQE azqe[4];
        ZPR zpr;
        int fFlash;
        float tFlash;
        float tCharm;
        struct LOCKG* plockgCur;
        WEXPL awexpl[8];
        struct PNT* ppntTow;
        struct ROPE* propeTow;
        float sTow;
        glm::vec3 posDrag;
        float xSteer;
        struct BL ablSki[6];
        struct ASEGBL* pasegblSki;
        struct BL ablSkiJump[3];
        struct ASEGBL* pasegblSkiJump;
        struct BL ablSkiBoost[3];
        struct ASEGBL* pasegblSkiBoost;
        struct BL ablSkiAttack[3];
        struct ASEGBL* pasegblSkiAttack;
        struct ALO* apaloSki[2];
        struct SHAPE* apshapeSki[2];
        float cripSkiSplash;
        float dtSpeed;
        glm::vec3 posNext;
        glm::vec3 vNext;
        JTPDK jtpdk;
        struct EXPL* pexplJumpLanding;
        struct EXPL* pexplSpireLanding;
        float uAlpha;
        float uAlphaCelBorder;
        struct EMITTER* pemitterShadow;
        float tCharmPending;
        int fCharmEnabled;
        struct LVO* plvoSave;
        struct JSG* pjsgCur;
        int fSlowDown;
        float uFsp;
};

void* CreateJt();
void InitJt(JT *pjt); // NOT FINSHED
void LoadJtFromBrx(JT *pjt, CBinaryInputStream *pbis); // NOT FINSHED
void RenderJtAll(JT* pjt, CM* pcm, RO* pro);
void RenderJtSelf(JT* pjt, CM* pcm, RO* pro);
void DeleteJt(LO* plo);

extern inline JT *g_pjt = nullptr;
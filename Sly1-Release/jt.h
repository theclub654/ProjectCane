#pragma once
#include "step.h"
#include "sqtr.h"
#include "hide.h"
#include "blot.h"

struct JTICON : public BLOT
{
};

void StartupJtIcon(JTICON* pjticon);
void PostJtIconLoad(JTICON* pjticon);
void DrawJtIcon(JTICON* pjticon);

struct VTJTICON
{
    void (*pfnInitBlot)(BLOT*, BLOTK) = InitBlot;
    void (*pfnPostJtIconLoad)(JTICON*) = PostJtIconLoad;
    void (*pfnUpdateBlot)(BLOT*) = UpdateBlot;
    void (*pfnOnBlotActive) = nullptr;
    void (*pfnUpdateBlotActive) = nullptr;
    void (*pfnOnBlotReset)(BLOT*) = OnBlotReset;
    void (*pfnOnBlotPush) = nullptr;
    void (*pfnOnBlotPop) = nullptr;
    void (*pfnSetBlotAchzDraw)(BLOT*, char*) = SetBlotAchzDraw;
    void (*pfnDrawJtIcon)(JTICON*) = DrawJtIcon;
    void (*pfnRenderBlot) = nullptr;
    float (*pfnDtAppearBlot)(BLOT*) = DtAppearBlot;
    float (*pfnDtVisibleBlot)(BLOT*) = DtVisibleBlot;
    float (*pfnDtDisappearBlot)(BLOT*) = DtDisappearBlot;
    void (*pfnShowBlot)(BLOT*) = ShowBlot;
    void (*pfnHideBlot)(BLOT*) = HideBlot;
    void (*pfnSetBlotBlots)(BLOT*, BLOTS) = SetBlotBlots;
    void (*pfnSetBlotClock)(BLOT*, float*) = SetBlotClock;
    int (*pfnFIncludeBlotForPeg)(BLOT*, BLOT*) = FIncludeBlotForPeg;
};

extern VTJTICON g_vtjticon;
extern JTICON g_rubyicon;
extern JTICON g_jticon;

enum SSLF
{
    SSLF_Nil = -1,
    SSLF_Right = 0,
    SSLF_Left = 1
};
enum : GRFTAK 
{
    GRFTAK_Sweep = 2,
    GRFTAK_Smash = 4,
    GRFTAK_Rush = 8,
    GRFTAK_Bomb = 16,
    GRFTAK_Shock = 32,
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
    // Retail inserted/reassigned these contextual slots relative to the
    // prototype. The celebration states moved to 28-30.
    JTBS_Release_25 = 25,
    JTBS_Release_26 = 26,
    JTBS_Release_27 = 27,
    JTBS_Celebrate_Key = 28,
    JTBS_Celebrate_Timed = 29,
    JTBS_Celebrate_UseKey = 30,
    JTBS_Rush_Attack = 31,
    JTBS_Rush_Bounce = 32,
    JTBS_Peek_Enter = 33,
    JTBS_Peek_Peek = 34,
    JTBS_Peek_Exit = 35,
    JTBS_Zap_Blunt = 36,
    JTBS_Zap_Electric = 37,
    JTBS_Zap_Fire = 38,
    JTBS_Zap_Water = 39,
    JTBS_Zap_Crush = 40,
    JTBS_Zap_Pit = 41,
    JTBS_Zap_Thrown = 42,
    JTBS_Zap_Dead = 43,
    JTBS_Zap_DeadFront = 44,
    JTBS_Zap_DeadInWater = 45,
    JTBS_Zap_DeadInPit = 46,
    JTBS_Zap_DeadCrush = 47,
    JTBS_Ski_Stand = 48,
    JTBS_Ski_Jump = 49,
    JTBS_Ski_Fall = 50,
    JTBS_Ski_Boost = 51,
    JTBS_Ski_Attack = 52,
    // These state values are contextual to JTS and intentionally overlap
    // some Ski values in the retail table.
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
enum JTFOCK
{
    JTFOCK_Nil = -1,
    JTFOCK_Relax = 0,
    JTFOCK_Clamp = 1,
    JTFOCK_Focus = 2,
    JTFOCK_Max = 3
};

struct WATERMSG
{
    WATER* pwater;
    ALO* palo;
};

struct JTWATERCONTACT
{
    EXPL* pexpl;
    float tContact;
};

struct WATER_EVENT_MESSAGE
{
    WATER* pwater;
    JT* pjt;
};

struct ZQE 
{
    JTBS jtbs;
    int grfzqe;
    glm::vec3 v;
    glm::vec3 w;
};
enum JTPDK 
{
    JTPDK_Nil = -1,
    JTPDK_Up = 0,
    JTPDK_Down = 1,
    JTPDK_Max = 2
};

struct JTJUMPTARGET
{
    LO* plo;
    float s;         
    float uTarget;
    float vTarget;
    glm::vec3 pos;   
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
    ASEGBL* pasegblRun;

    BLSS ablssSidestep[8];
    ASEGBL* pasegblSidestep;
    SSLF sslfCur;

    BLSS ablssFlatten[8];
    ASEGBL* pasegblFlatten;
    ASEG* pasegFlattenStand;
    ASEG* pasegFlattenPeekLeft;
    ASEG* pasegFlattenPeekRight;

    BLSS ablssDuck[8];
    ASEGBL* pasegblDuck;
    ASEG* pasegDuckStand;
    ASEG* pasegDuckPeekLeft;
    ASEG* pasegDuckPeekRight;

    BLRUN ablrunBasket[6];
    ASEGBL* pasegblBasket;
    ASEG* pasegBasketStand;
    ASEG* pasegBasketJumpIn;
    ASEG* pasegBasketJumpOut;

    BLRUN ablrunRail[6];
    ASEGBL* pasegblRail;

    ASEG* pasegReachStand;
    ASEGBL* pasegblPipeEnter;
    ASEGBL* pasegblPipeExit;

    ASEG* pasegJumpStanding;
    ASEG* pasegJumpRunning;
    ASEG* pasegJumpSwinging;
    ASEG* pasegJumpTarget;
    ASEG* pasegJumpBoost;
    ASEG* pasegEdge;

    BLCL ablcl[2];
    ASEGBL* pasegblClimb;

    ASEG* pasegPipeInit;
    ASEG* pasegPipeStay;
    ASEG* pasegPipeSpin;

    BL ablRailBalance[3];
    ASEGBL* pasegblRailBalance;
    float uRailBalance;
    float duRailBalance;

    ASEG* mpjtbspaseg[57];

    SM* psmIdle;
    int fStandTight;

    XMG xmgLeftBall;
    XMG xmgRightBall;

    SO* mpjtspso[15];
    SO* mpjthkpso[15];
    SO* mpjtbspso[57];

    LBONE* plboneMineOrigin;
    ALO* apaloMineHat[4];
    MINE* pmine;

    XP* pxpZap;

    SQTRM sqtrmCane;
    int fRenderCaneTrail;

    RCHM* prchmReach;
    float tSweepPending;

    int cposHook;
    glm::vec3 aposHook[3];

    TARGET* ptargetCur;
    HND* phndCur;
    PIPE* ppipeCur;
    float sPipeCur;

    HND* phndUnhook;
    PIPE* ppipeUnhook;
    float sPipeUnhook;
    int fPipeReverse;

    int jts;
    float tJts;
    int jtbs;
    float tJtbs;

    ASEGA* pasegaCur;
    SMA* psmaCur;

    float tStuckCheck;

    int jtcs;
    float tJtcs;
    ASEGA* pasegaJtcs;

    int fCaneInterruptible;

    int jtjk;
    int jsglk;
    int grfjtsc;

    JTS jtsNext;
    JTBS jtbsNext;
    JTCS jtcsNext;
    JTJK jtjkNext;

    int fShortenJump;
    float svxyAirSteer;

    int jtjj;
    float mpjtjjt[4];
    float tJumpPending;

    glm::vec3 vTargetCoast;

    float tHangDeflectLast;
    int  jtbsHangLatch;
    int fHangReverse;

    std::shared_ptr <XA> pxaHang;
    FX afxHang[2];

    glm::vec3 dposHangHand;
    glm::mat3 dmatHangHand;
    glm::mat3 dmatHangTool;

    BL ablSwing[3];
    ASEGBL* pasegblSwing;
    float uSwingForward;
    float uSwingActive;

    ALO* paloSkinStandard;
    ALO* paloSkinElectric;
    ALO* paloSkinFire;

    int jthk;

    HSHAPE* phshape;
    float sParamHshape;

    HPNT* phpnt;
    float radHpnt;

    HBSK* phbsk;
    JMT* pjmtCur;
    SPIRE* pspireCur;

    RAIL* prailCur;
    float uRailCur;
    float sRailCur;
    int fRailReverse;

    VAULT* pvaultCur;

    ALO* paloTool;
    ALO* paloHook;
    ALO* paloKey;

    int zpk;

    glm::vec3 posPipeHandLocal;
    glm::vec3 posPipeFootLocal;
    glm::vec3 dposPipeXp;

    float sPipeHand;
    float dsPipeHand;
    float uPipeClimbSmooth;

    int jtMoveDir;
    float sRadiusHook;
    float tContact;

    EXPL* pexplHook;

    float swToolSpinTarget;
    float swToolSpin;
    float radToolSpin;

    WATER* pwater;
    ALO* paloShadow;

    SFF sffRun;
    SFF sffSidestep;
    SFF sffStand;
    SFF sffFlatten;
    SFF sffDuck;

    TAIL* ptailTail;
    TAIL* ptailMask;

    int fCanBoost;
    float tCancelJump;

    SO* psoEdgeCur;
    glm::vec3 posEdgeLocal;
    glm::vec3 normalEdgeCur;
    glm::vec3 dposEdge;
    glm::vec3 posEdgeTarget;

    XPK xpkEdge;
    void* pvEdge;

    ALO* paloGut;

    ACTADD* pactaddGut;
    ACTADJ* pactadjGut;
    ACTADJ* pactadjLeftHand;
    ACTADJ* pactadjRightHand;
    ACTADJ* pactadjLeftFoot;
    ACTADJ* pactadjRightFoot;

    ACTVAL* pactvalJump;
    ACTVAL* pactvalGut;
    ACTVAL* pactvalHangHand;
    ACTVAL* pactvalHangTool;

    glm::vec3 dposSmoothCpdefi;

    BL ablSweepAttackFore[2];
    ASEGBL* pasegblSweepAttackFore;

    BL ablSweepAttackBack[2];
    ASEGBL* pasegblSweepAttackBack;

    BL ablStandingSweepAttackFore[2];
    ASEGBL* pasegblStandingSweepAttackFore;

    BL ablStandingSweepAttackBack[2];
    ASEGBL* pasegblStandingSweepAttackBack;

    ASEGBL* pasegblStandingSweepForeReturn;
    ASEGBL* pasegblStandingSweepBackReturn;

    ASEG* pasegRushAttack;
    ASEG* pasegRushBounce;
    ASEG* pasegSmashAttack;
    ASEG* pasegSmashBounce;
    ASEG* pasegDiveAttack;
    ASEG* pasegPeekEnter;
    ASEG* pasegPeekExit;
    ASEG* pasegBall;

    SO* psoSweepEffect;
    SO* psoSmashEffect;
    SO* psoRushEffect;
    SO* psoDiveEffect;
    SO* psoBallEffect;

    int cpaloIgnoreEffect;
    ALO* apaloIgnoreEffect[16];

    XP* pxpEffect;

    TN* ptnCur;
    TN* ptnHideDynamic;
    TN* ptnKeyCelebration;

    float tMoveLast;

    int hfdk;
    float tHfdk;

    JTFOCK jtfock;
    glm::vec3 posFocus;

    float uFlattenClamp;
    EXC* pexcHide;

    int cspks;
    std::vector <SPKS> aspks;

    PFNZQE pfnZqe;
    ZPR zpr;

    int fFlash;
    float tFlashPulse;
    float tCharm;
    float dtCharmFlash;

    LOCKG* plockgCur;

    JTWATERCONTACT jtWaterContact[8];

    int jtpdk;

    EXPL* pexplJumpLand;
    EXPL* pexplHideJumpLand;

    float uAlpha;
    float uAlphaCelBorder;

    EMITTER* pemitterShadow;

    int czqe;
    float tCharmPending;
    int fCharmEnabled;

    std::shared_ptr <LVO> plvoSave;
    JSG* pjsgCur;

    int fSlowDownClock;
    float uSpeedUpClock;

    ASEG* pasegDecoy;
    DECOY* pdecoy;

    ASEG* pasegStun;
    float tStun;
    int fStun;

    ASEG* pasegMinePlace;
    ASEG* pasegMineActivate;

    int cjumpTargetHistory;
    JTJUMPTARGET ajtJumpTargetHistory[32];
    JTJUMPTARGET jtJumpTarget;

    glm::vec3 vDamagePrev;
    glm::vec3 wDamagePrev;

    DIALOG* pdialogPeekPending;
    int fCancelPeek;
    int fDisableStepCustomXps;

    LO* ploJumpSource;
    LOCKG* plockg;

    SO* psoDisplacePending;
    SO* psoDisplacePendingSecondary;
};

JT*  NewJt();
void InitJt(JT *pjt);
void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis);
void*GetJtFRenderCaneTrail(JT* pjt);
void SetJtFRenderCaneTrail(JT* pjt, int fRenderCaneTrail);
void*GetJtJtpdk(JT* pjt);
void*GetJtFCancelPeek(JT* pjt);
void SetJtFCancelPeek(JT* pjt, int fCancelPeek);
void*GetJtFDisableStepCustomXps(JT* pjt);
void SetJtFDisableStepCustomXps(JT* pjt, int fDisableStepCustomXps);
void CloneJt(JT* pjt, JT* pjtBase);
void PostJtLoad(JT* pjt);
void PresetJtAccelBase(JT* pjt);
void PresetJtAccel(JT* pjt, float dt);
void AdjustJtNewXp(JT* pjt, XP* pxp, int ixpd);
void AdjustJtDz(JT* pjt, GRFADJ grfadj, DZ* pdz, int ixpd, float dt);
void HandleJtGrfjtsc(JT* pjt);
void UpdateJtInternalXps(JT* pjt);
int  FCheckJtXpBase(JT* pjt, XP* pxp, int ixpd);
void AdjustJtXpVelocity(JT* pjt, XP* pxp, int ixpd);
void HandleJtMessage(JT* pjt, MSGID msgid, void* pv);
int  FTurnJtToTarget(JT* pjt);
void UpdateJtBalance(JT* pjt);
void RebuildJtXmg(JT* pjt, ALO* paloExtrem, float sRadius, ALO* paloIkh, ACTADJ* pactadj, XMG* pxmg);
int  FMatchJtXmg(JT* pjt, XMG* pxmg, ACTADJ* pactadj);
void UpdateJtStand(JT* pjt);
void ThrowJt(JT* pjt, glm::vec3* pposTarget, float angVert, float uSteer);
void UnhookJt(JT* pjt);
void CalculateJtToolSpin(JT* pjt, float u, glm::mat3* pmat);
void RenderJtAll(JT* pjt, CM* pcm, RO* pro);
void UpdateJtXfWorld(JT* pjt);
void RenderJtSelf(JT* pjt, CM* pcm, RO* pro);
void UpdateJtTool(JT* pjt);
void AdjustJtXps(JT* pjt);
void UpdateJtPosWorldPrev(JT* pjt);
void ProjectJtTransform(JT* pjt);
void UpdateJtBounds(JT* pjt);
SO*  PsoGetJtEffect(JT* pjt, int* pigrfic);
void AddJtCustomXps(JT* pjt, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst);
CT   CtTorqueJt(JT* pjt);
void GetJtCpdefi(JT* pjt, float dt, CPDEFI* pcpdefi);
void GetJtCpdefiFlatten(JT* pjt, CPDEFI* pcpdefi);
void OnJtActive(JT* pjt, int fActive);
int  FCheckJtBumperPath(JT* pjt, const glm::vec3* pvec);
void UpdateJtActive(JT* pjt, JOY* pjoy, float dt);
void UpdateJt(JT* pjt, float dt);
void UpdateJtDrive(JT* pjt);
void ChooseJtPhys(JT* pjt, SO* pso);
void EnableJtActadj(JT* pjt, int grfjta);
void SetJtJts(JT* pjt, int jts, int jtbs);
void SetJtPuppet(JT* pjt, EA* pea);
void SetJtDisableStepCustomXps(JT* pjt, int fDisable);
ALO* PaloAbsorbWkr(WKR* pwkr, int cpaloIgnore, ALO** apaloIgnore);
int  NCmpWkr(WKR* pwkr1, WKR* pwkr2);
void UpdateJtEffect(JT* pjt);
bool FIsJtSoundBase(JT* pjt);
void CollectJtPrize(JT* pjt, PCK pck, ALO* paloOther);
void GetJtDiapi(JT* pjt, DIALOG* pdialog, DIAPI* pdiapi);
void PlayJtDialog(JT* pjt, DIALOG* pdialog);
void TranslateJtToPos(JT* pjt, glm::vec3* pvector);
void OnJtAdd(JT* pjt);
void UpdateJtPendingInteractions(JT* pjt);
void TriggerJtLockg(JT* pjt, LOCKG* plockg);
void TriggerJtDisplace(JT* pjt, SO* pso);
void TriggerJtDisplaceSecondary(JT* pjt, SO* pso);
void CalculateJtJumpTrajectory(JT* pjt, float dt, JTJUMPTARGET* ptarget, glm::vec3* pvLaunch, glm::vec3* pposGoal, glm::vec3* pvGoal);
void StartJtZapJumpToTarget(JT* pjt, float dtJump, const JTJUMPTARGET* ptarget);
void FindBestJtJumpDuration(JT* pjt, JTJUMPTARGET* ptarget, std::vector<SO*>* papso, float* pdtBest);
float ScoreJtJumpTarget(JT* pjt, JTJUMPTARGET* ptarget, std::vector<STEPGUARD*>& stepguards, JTJUMPTARGET* ptargetAdjusted, float* pdtJump);
int  CompareJtJumpTargetsByDistance(const void* pvA, const void* pvB);
void FindBestJtJumpTarget(JT* pjt, std::vector<LO*>& jumpObjects, JTJUMPTARGET* ptargetBest, float* pdtBest);
void UpdateJtJumpTargetHistory(JT* pjt);
int  GetJtSize();
void DeleteJt(JT* pjt);

extern JT *g_pjt;
extern glm::vec3 s_posEdgeTargetDefault;
extern SFF s_sffDuck;
extern SFF s_sffFlatten;
extern SFF s_sffStand;
extern SFF s_sffSidestep;
extern SFF s_sffRun;
extern glm::vec3 s_posThrowJt;
static SMPA s_smpaSpin{40, 0.0, 0.25, 200};
static CLQ s_clqUDeflectToUSpin{ 0.2, 0.0, 0.0 };
static SMPA s_smpaSpinRun{ 15, 0.0, 0.25, 100 };
static SMPA s_smpaTilt{ 10, 0.0, 0.5, 50 };
static CLQ s_clqSvToUSpin{ -0.2, 0.002, 0.0 };
static CLQ s_clqJtState9Spring{ 0.0, 5.0, 0.0 };
static float SV_JtBallMax = 1000.0;
static SMP s_smpRecover{ 40, 0.0, 0.1 };
static float MU_JtBall = 0.8;
static float ELAS_JtBall = 0.5;
static float MU_JtZap = 1.0;
static float ELAS_JtZap = 0.3;
static float MU_JtBase = 1.5;
static float DT_JtLimitImpactFrictionMax = 0.1;
static float R_JtLimitPivotFrictionMax = 2.0;
static float DZ_StepMatchGroundUp = 50;
static float DZ_StepMatchGroundDown = -50;
static float U_JtAlphaNonchalant = 0.34999999;
static float U_JtAlphaCelBorderNonchalant = 0.0;
static SMP s_smpAlpha{ 5.0, 0.0, 0.1 };
static SMP s_smpRad{ 50, 0.0, 0.25 };
static SMP s_smpPeek{ 600, 300, 0.1};
static const JTBS s_mpjtbsjtbsNext[14] =
{
    static_cast<JTBS>(-1),
    static_cast<JTBS>(36),
    static_cast<JTBS>(36),
    static_cast<JTBS>(36),
    static_cast<JTBS>(-1),
    static_cast<JTBS>(36),
    static_cast<JTBS>(36),
    static_cast<JTBS>(36),
    static_cast<JTBS>(36),
    static_cast<JTBS>(-1),
    static_cast<JTBS>(45),
    static_cast<JTBS>(45),
    static_cast<JTBS>(41),
    static_cast<JTBS>(40)
};

constexpr float DZ_JtBaseMax = 75.0f;
constexpr float DZ_JtBaseMin = -325.0f;
constexpr float DZ_JtBaseTargetMax = -50.0f;
constexpr float DZ_JtBaseTargetMin = -350.0f;
constexpr float DZ_JtBaseTargetHang = -300.0f;

static CLQ s_clqSwzToUAdjust =
{
    1.5f,
    -0.5f,
    0.0f,
    0.0f
};

static SMP s_smpSmoothCpdefi =
{
    1500.0f,
    0.0f,
    0.5f
};

static float R_JtKeyScale = 0.87599999;
static int SW_JtToolSpinRush = 50;
static int SW_JtToolSpinDive = 100;
static constexpr float DT_JtBalancePredict = 0.1f;       // 0x002637D8
static constexpr float SDV_JtBalanceMax = 1000.0f;       // 0x002637DC
static constexpr float XY_JtBalanceMax = 20.0f;          // 0x002637E0
static constexpr float Z_JtBalanceMax = -7.5f;           // 0x002637E4
static constexpr float U_JtBalanceHands = 0.5f;          // 0x002637E8

static constexpr CLQ s_clqUSdvToULean = { 0.0f, 2.0f, -1.0f, 0.0f }; // 0x002637F0

static glm::vec3 s_posJtRailGut = { -20.0f, 5.0f, -25.0f };             // 0x00263800
static glm::vec3 s_radJtRailGut = { 0.0f, -0.6f, 0.5f };                // 0x00263810
static glm::vec3 s_posJtSlipperyRailGut = { -20.0f, 5.0f, -25.0f };    // 0x00263820
static glm::vec3 s_radJtSlipperyRailGut = { 0.0f, -0.6f, 0.5f };       // 0x00263830
static glm::vec3 s_posJtRailGutPivot = { 0.0f, 0.0f, -100.0f };        // 0x00263840

static constexpr float U_JtRailGutRoll = -0.5f;       // 0x00263850
static constexpr float DZ_JtRailFootSpan = 100.0f;    // 0x00263854

static SMPA s_smpRailBalance = { 4.0f, 0.0f, 0.0f }; // 0x00263858
static CLQ s_clqGutHeelAdjust = { 0.0f, 0.5f, 0.0f, 0.0f }; // 0x00263870

static glm::vec3 s_dposJtJumpTargetDefault(0.0f, 0.0f, 75.0f);
static glm::vec3 s_dposJtJumpTargetClass118(0.0f, 0.0f, -225.0f);
static constexpr float S_JtPipeJumpTarget = -75.0f;

const float s_adtJtJumpCandidate[] =
{
    0.7f,
    0.8f,
    0.9f,
    1.0f,
    1.2f,
    1.4f,
    1.6f,
    1.8f,
    2.0f
};

static glm::vec3 s_adposJtLandingProbe[8] =
{
    glm::vec3(40.0f,   0.0f, 0.0f),
    glm::vec3(28.3f,  28.3f, 0.0f),
    glm::vec3(0.0f,  40.0f, 0.0f),
    glm::vec3(-28.3f,  28.3f, 0.0f),
    glm::vec3(-40.0f,   0.0f, 0.0f),
    glm::vec3(-28.3f, -28.3f, 0.0f),
    glm::vec3(0.0f, -40.0f, 0.0f),
    glm::vec3(28.3f, -28.3f, 0.0f)
};

static const glm::vec3 s_dposJtJumpSearchPadding = glm::vec3(200.0f, 200.0f, 200.0f);

static const CLQ s_clqJtJumpAvoidance =
{
    400.0f,
    -1.0f,
    0.0f
};

static const LM s_lmJtJumpAvoidance =
{
    0.0f,
    1000.0f
};

constexpr float DT_JtFootfallSampleOffset = -0.033f; // 0x00274B64

static SMPA s_smpaJtEdge =                             // 0x00263918
{
    1000.0f,
    0.0f,
    0.25f,
    0.9f
};

constexpr float UZ_JtBallRecoverMin = 0.9f;      // 0x00263924, overlaps s_smpaJtEdge.sdvMax
constexpr float SW_JtBallRecoverMax = 8.0f;      // 0x00263928

constexpr float DT_JtCharmBeforeEnabled = 0.15f;
constexpr float DT_JtMinBall = 0.25f;
constexpr float U_JtHideFailSound = 0.6f;

extern char g_worldWarpContext[];

constexpr float DT_JtBoostPredict = 0.1f;
constexpr float DT2_JtBoostPredict = 0.005f;
constexpr float SV_JtHideSearch = 60.0f;
constexpr float SV_JtBasketSearch = 200.0f;
constexpr float DT_StepFootfallOffset = -0.033f;

static SMP s_smpEdge =
{
    1000.0f,
    0.0f,
    0.25f
};

constexpr float Z_JtBallRecoverMin = 0.9f;
inline constexpr float G_MAX_FLOAT = (std::numeric_limits<float>::max)(); // 0x0024AB7C, 0x7F7FFFFF

inline constexpr float SVXY_JtAirSteer = 455.0f;             // 0x00262E14, 0x43E38000
inline constexpr float R_JtFromWaterJumpAirSteer = 0.7f;     // 0x00262E18, 0x3F333333
inline constexpr float R_JtEdgeJumpAirSteer = 0.6f;          // 0x00262E1C, 0x3F19999A
inline constexpr float U_JtSoundPrimary = 0.7f;              // 0x00262E20, 0x3F333333
inline constexpr float U_JtSoundSecondary = 0.6f;            // 0x00262E24, 0x3F19999A
inline constexpr float SV_JtBallImpulse = 850.0f;            // 0x00262E2C, 0x44548000
inline constexpr float S_JtBallRadius = 50.0f;               // 0x00262E30, 0x42480000

// 0x00263980-0x0026398F is one aligned PS2 VECTOR. Ghidra gives it
// the local-static aliases s_dvThermal.1002 and s_dvJump.1014.
inline const glm::vec3 DV_JtThermalAndJump(0.0f, 0.0f, 500.0f);

inline constexpr float DOT_JtRailReverse = -0.2f;            // 0x00263990, 0xBE4CCCCD
inline constexpr float U_JtImpactSound = 1.0f;               // 0x00263994, 0x3F800000
inline constexpr float U_JtImpactSoundSecondary = 0.5f;      // 0x00263998, 0x3F000000
inline constexpr float DT_JtSmashContact = 0.25f;            // 0x0026399C, 0x3E800000

// Polynomial used to blend the water-death facing direction:
// clamp(c0 + speed * (c1 + speed * c2), 0, 1).
inline constexpr float CLQ_JtWaterDeathTurn0 = -1.0f;        // 0x002639A0, 0xBF800000
inline constexpr float CLQ_JtWaterDeathTurn1 = 0.004f;       // 0x002639A4, 0x3B83126F
inline constexpr float CLQ_JtWaterDeathTurn2 = 0.0f;         // 0x002639A8, 0x00000000

inline constexpr float SV_JtBallUseHeading = 150.0f;         // 0x002639B0, 0x43160000
inline constexpr bool F_JtZapFreeze = false;                 // 0x002639B8, 0x00000000

inline constexpr float DS_JtPipeLimitHand = 35.0f;           // 0x00274F00, 0x420C0000
inline constexpr float DS_JtPipeLimitFoot = 100.0f;          // 0x00274F04, 0x42C80000
inline constexpr float DZ_JtPipeReverse = 0.5f;              // 0x00274F08, 0x3F000000
inline constexpr float DS_JtPipeProbe = 50.0f;               // 0x00274F14, 0x42480000

// Exact address-preserving aliases for matching the Ghidra listing.
inline constexpr float DAT_0024AB7C = G_MAX_FLOAT;
inline constexpr float DAT_00262E14 = SVXY_JtAirSteer;
inline constexpr float DAT_00262E18 = R_JtFromWaterJumpAirSteer;
inline constexpr float DAT_00262E1C = R_JtEdgeJumpAirSteer;
inline constexpr float DAT_00262E20 = U_JtSoundPrimary;
inline constexpr float FLOAT_00262E24 = U_JtSoundSecondary;
inline constexpr float DAT_00262E2C = SV_JtBallImpulse;
inline constexpr float DAT_00262E30 = S_JtBallRadius;
inline constexpr float DAT_00263984 = 0.0f;   // DV_JtThermalAndJump.y
inline constexpr float DAT_00263988 = 500.0f; // DV_JtThermalAndJump.z
inline constexpr float DAT_0026398C = 0.0f;   // PS2 VECTOR padding/w component
inline constexpr float DAT_00263990 = DOT_JtRailReverse;
inline constexpr float DAT_00263994 = U_JtImpactSound;
inline constexpr float DAT_00263998 = U_JtImpactSoundSecondary;
inline constexpr float DAT_0026399C = DT_JtSmashContact;
inline constexpr float DAT_002639A0 = CLQ_JtWaterDeathTurn0;
inline constexpr float DAT_002639A4 = CLQ_JtWaterDeathTurn1;
inline constexpr float DAT_002639A8 = CLQ_JtWaterDeathTurn2;
inline constexpr float DAT_002639B0 = SV_JtBallUseHeading;
inline constexpr std::int32_t DAT_002639B8 = 0;
inline constexpr float DAT_00274F00 = DS_JtPipeLimitHand;
inline constexpr float DAT_00274F04 = DS_JtPipeLimitFoot;
inline constexpr float DAT_00274F08 = DZ_JtPipeReverse;
inline constexpr float DAT_00274F14 = DS_JtPipeProbe;

static glm::vec3 s_dvTailInWater(0.0f, 0.0f, 500.0f);
static CLQ s_clqSvToAlpha = { -1.5, 0.001, 0.0 };
static CLQ s_clqDtToAlpha = { 1.0, -4.0, 0.0 };

inline SNIP s_asnipLoad[151] =
{
    { 0x00, (OID)0x145, offsetof(JT, psmIdle) },
    { 0x00, (OID)0x082, offsetof(JT, ablrunRun[0].paseg) },
    { 0x00, (OID)0x083, offsetof(JT, ablrunRun[1].paseg) },
    { 0x02, (OID)0x084, offsetof(JT, ablrunRun[2].paseg) },
    { 0x02, (OID)0x085, offsetof(JT, ablrunRun[3].paseg) },
    { 0x02, (OID)0x086, offsetof(JT, ablrunRun[4].paseg) },
    { 0x02, (OID)0x087, offsetof(JT, ablrunRun[5].paseg) },
    { 0x00, (OID)0x092, offsetof(JT, ablssSidestep[0].paseg) },
    { 0x00, (OID)0x093, offsetof(JT, ablssSidestep[1].paseg) },
    { 0x00, (OID)0x094, offsetof(JT, ablssSidestep[2].paseg) },
    { 0x00, (OID)0x095, offsetof(JT, ablssSidestep[3].paseg) },
    { 0x00, (OID)0x096, offsetof(JT, ablssSidestep[4].paseg) },
    { 0x00, (OID)0x097, offsetof(JT, ablssSidestep[5].paseg) },
    { 0x00, (OID)0x098, offsetof(JT, ablssSidestep[6].paseg) },
    { 0x00, (OID)0x099, offsetof(JT, ablssSidestep[7].paseg) },
    { 0x00, (OID)0x09F, offsetof(JT, ablssFlatten[0].paseg) },
    { 0x00, (OID)0x0A0, offsetof(JT, ablssFlatten[1].paseg) },
    { 0x00, (OID)0x0A1, offsetof(JT, ablssFlatten[2].paseg) },
    { 0x00, (OID)0x0A2, offsetof(JT, ablssFlatten[3].paseg) },
    { 0x00, (OID)0x0A3, offsetof(JT, ablssFlatten[4].paseg) },
    { 0x00, (OID)0x0A4, offsetof(JT, ablssFlatten[5].paseg) },
    { 0x00, (OID)0x0A5, offsetof(JT, ablssFlatten[6].paseg) },
    { 0x00, (OID)0x0A6, offsetof(JT, ablssFlatten[7].paseg) },
    { 0x00, (OID)0x0A7, offsetof(JT, pasegFlattenStand) },
    { 0x00, (OID)0x0A9, offsetof(JT, pasegFlattenPeekLeft) },
    { 0x00, (OID)0x0A8, offsetof(JT, pasegFlattenPeekRight) },
    { 0x00, (OID)0x0AA, offsetof(JT, ablssDuck[0].paseg) },
    { 0x00, (OID)0x0AB, offsetof(JT, ablssDuck[1].paseg) },
    { 0x00, (OID)0x0AC, offsetof(JT, ablssDuck[2].paseg) },
    { 0x00, (OID)0x0AD, offsetof(JT, ablssDuck[3].paseg) },
    { 0x00, (OID)0x0AE, offsetof(JT, ablssDuck[4].paseg) },
    { 0x00, (OID)0x0AF, offsetof(JT, ablssDuck[5].paseg) },
    { 0x00, (OID)0x0B0, offsetof(JT, ablssDuck[6].paseg) },
    { 0x00, (OID)0x0B1, offsetof(JT, ablssDuck[7].paseg) },
    { 0x00, (OID)0x0B2, offsetof(JT, pasegDuckStand) },
    { 0x00, (OID)0x0B4, offsetof(JT, pasegDuckPeekLeft) },
    { 0x00, (OID)0x0B3, offsetof(JT, pasegDuckPeekRight) },
    { 0x00, (OID)0x0B5, offsetof(JT, ablrunBasket[0].paseg) },
    { 0x00, (OID)0x0B6, offsetof(JT, ablrunBasket[1].paseg) },
    { 0x02, (OID)0x0B7, offsetof(JT, ablrunBasket[2].paseg) },
    { 0x02, (OID)0x0B8, offsetof(JT, ablrunBasket[3].paseg) },
    { 0x02, (OID)0x0B9, offsetof(JT, ablrunBasket[4].paseg) },
    { 0x02, (OID)0x0BA, offsetof(JT, ablrunBasket[5].paseg) },
    { 0x00, (OID)0x0BB, offsetof(JT, pasegBasketStand) },
    { 0x00, (OID)0x0BC, offsetof(JT, pasegBasketJumpIn) },
    { 0x00, (OID)0x0BD, offsetof(JT, pasegBasketJumpOut) },
    { 0x02, (OID)0x0BE, offsetof(JT, ablrunRail[0].paseg) },
    { 0x02, (OID)0x0BF, offsetof(JT, ablrunRail[1].paseg) },
    { 0x02, (OID)0x0C0, offsetof(JT, ablrunRail[2].paseg) },
    { 0x02, (OID)0x0C1, offsetof(JT, ablrunRail[3].paseg) },
    { 0x02, (OID)0x0C2, offsetof(JT, ablrunRail[4].paseg) },
    { 0x02, (OID)0x0C3, offsetof(JT, ablrunRail[5].paseg) },
    { 0x02, (OID)0x0C4, offsetof(JT, pasegReachStand) },
    { 0x02, (OID)0x0DD, offsetof(JT, pasegblPipeEnter) },
    { 0x02, (OID)0x0DE, offsetof(JT, pasegblPipeExit) },
    { 0x00, (OID)0x08C, offsetof(JT, pasegJumpStanding) },
    { 0x00, (OID)0x08D, offsetof(JT, pasegJumpRunning) },
    { 0x00, (OID)0x08E, offsetof(JT, pasegJumpSwinging) },
    { 0x00, (OID)0x090, offsetof(JT, pasegJumpTarget) },
    { 0x00, (OID)0x091, offsetof(JT, pasegJumpBoost) },
    { 0x00, (OID)0x157, offsetof(JT, ablSwing[0].paseg) },
    { 0x00, (OID)0x158, offsetof(JT, ablSwing[1].paseg) },
    { 0x00, (OID)0x159, offsetof(JT, ablSwing[2].paseg) },
    { 0x00, (OID)0x08B, offsetof(JT, pasegEdge) },
    { 0x02, (OID)0x09E, offsetof(JT, pasegPipeInit) },
    { 0x00, (OID)0x09A, offsetof(JT, pasegPipeStay) },
    { 0x00, (OID)0x09D, offsetof(JT, pasegPipeSpin) },
    { 0x00, (OID)0x09B, offsetof(JT, ablcl[0].paseg) },
    { 0x00, (OID)0x09C, offsetof(JT, ablcl[1].paseg) },
    { 0x00, (OID)0x07D, offsetof(JT, prchmReach) },
    { 0x00, (OID)0x0F3, offsetof(JT, ablSweepAttackFore[0].paseg) },
    { 0x00, (OID)0x0F7, offsetof(JT, ablSweepAttackFore[1].paseg) },
    { 0x00, (OID)0x0F5, offsetof(JT, ablSweepAttackBack[0].paseg) },
    { 0x00, (OID)0x0F9, offsetof(JT, ablSweepAttackBack[1].paseg) },
    { 0x02, (OID)0x0FB, offsetof(JT, ablStandingSweepAttackFore[0].paseg) },
    { 0x02, (OID)0x0FF, offsetof(JT, ablStandingSweepAttackFore[1].paseg) },
    { 0x02, (OID)0x0FD, offsetof(JT, ablStandingSweepAttackBack[0].paseg) },
    { 0x02, (OID)0x101, offsetof(JT, ablStandingSweepAttackBack[1].paseg) },
    { 0x02, (OID)0x103, offsetof(JT, pasegblStandingSweepForeReturn) },
    { 0x02, (OID)0x104, offsetof(JT, pasegblStandingSweepBackReturn) },
    { 0x00, (OID)0x105, offsetof(JT, pasegRushAttack) },
    { 0x00, (OID)0x107, offsetof(JT, pasegSmashAttack) },
    { 0x00, (OID)0x1B8, offsetof(JT, pasegDiveAttack) },
    { 0x02, (OID)0x106, offsetof(JT, pasegRushBounce) },
    { 0x02, (OID)0x108, offsetof(JT, pasegSmashBounce) },
    { 0x02, (OID)0x11A, offsetof(JT, pasegBall) },
    { 0x00, (OID)0x064, offsetof(JT, mpjtspso[0]) },
    { 0x02, (OID)0x065, offsetof(JT, mpjtspso[1]) },
    { 0x02, (OID)0x066, offsetof(JT, mpjtspso[2]) },
    { 0x02, (OID)0x067, offsetof(JT, mpjtspso[7]) },
    { 0x02, (OID)0x068, offsetof(JT, mpjtspso[8]) },
    { 0x02, (OID)0x069, offsetof(JT, mpjtspso[17]) },
    { 0x02, (OID)0x06A, offsetof(JT, mpjtspso[18]) },
    { 0x02, (OID)0x06B, offsetof(JT, mpjtspso[19]) },
    { 0x02, (OID)0x06C, offsetof(JT, mpjtspso[20]) },
    { 0x02, (OID)0x06D, offsetof(JT, mpjtspso[23]) },
    { 0x02, (OID)0x06E, offsetof(JT, mpjtbspso[40]) },
    { 0x02, (OID)0x06F, offsetof(JT, mpjtbspso[41]) },
    { 0x02, (OID)0x070, offsetof(JT, mpjtbspso[42]) },
    { 0x02, (OID)0x071, offsetof(JT, mpjtbspso[43]) },
    { 0x02, (OID)0x072, offsetof(JT, mpjtbspso[44]) },
    { 0x02, (OID)0x073, offsetof(JT, mpjtbspso[45]) },
    { 0x02, (OID)0x074, offsetof(JT, mpjtbspso[46]) },
    { 0x02, (OID)0x075, offsetof(JT, mpjtbspso[47]) },
    { 0x02, (OID)0x076, offsetof(JT, mpjtbspso[49]) },
    { 0x02, (OID)0x078, offsetof(JT, mpjtbspso[50]) },
    { 0x02, (OID)0x077, offsetof(JT, mpjtbspso[51]) },
    { 0x02, (OID)0x079, offsetof(JT, mpjtbspso[52]) },
    { 0x02, (OID)0x07A, offsetof(JT, mpjtbspso[53]) },
    { 0x02, (OID)0x07B, offsetof(JT, mpjtspso[55]) },
    { 0x02, (OID)0x07C, offsetof(JT, mpjtspso[13]) },
    { 0x00, (OID)0x0E2, offsetof(JT, mpjtbspaseg[36]) },
    { 0x00, (OID)0x0E3, offsetof(JT, mpjtbspaseg[37]) },
    { 0x00, (OID)0x0E4, offsetof(JT, mpjtbspaseg[38]) },
    { 0x00, (OID)0x0E5, offsetof(JT, mpjtbspaseg[39]) },
    { 0x00, (OID)0x0E6, offsetof(JT, mpjtbspaseg[40]) },
    { 0x00, (OID)0x0E7, offsetof(JT, mpjtbspaseg[41]) },
    { 0x00, (OID)0x0E8, offsetof(JT, mpjtbspaseg[42]) },
    { 0x00, (OID)0x0E9, offsetof(JT, mpjtbspaseg[43]) },
    { 0x00, (OID)0x0EA, offsetof(JT, mpjtbspaseg[45]) },
    { 0x00, (OID)0x0EC, offsetof(JT, mpjtbspaseg[46]) },
    { 0x00, (OID)0x0EB, offsetof(JT, mpjtbspaseg[47]) },
    { 0x00, (OID)0x0ED, offsetof(JT, mpjtbspaseg[48]) },
    { 0x00, (OID)0x0EE, offsetof(JT, mpjtbspaseg[49]) },
    { 0x02, (OID)0x0EF, offsetof(JT, mpjtbspaseg[28]) },
    { 0x02, (OID)0x0F1, offsetof(JT, mpjtbspaseg[29]) },
    { 0x02, (OID)0x0F2, offsetof(JT, mpjtbspaseg[30]) },
    { 0x02, (OID)0x109, offsetof(JT, pasegPeekEnter) },
    { 0x02, (OID)0x10A, offsetof(JT, pasegPeekExit) },
    { 0x02, (OID)0x172, offsetof(JT, paloShadow) },
    { 0x06, (OID)0x051, offsetof(JT, ptailTail) },
    { 0x06, (OID)0x05A, offsetof(JT, ptailMask) },
    { 0x02, (OID)0x17D, offsetof(JT, jtWaterContact[0].pexpl) },
    { 0x02, (OID)0x17E, offsetof(JT, jtWaterContact[1].pexpl) },
    { 0x02, (OID)0x17F, offsetof(JT, jtWaterContact[2].pexpl) },
    { 0x02, (OID)0x180, offsetof(JT, jtWaterContact[3].pexpl) },
    { 0x02, (OID)0x181, offsetof(JT, jtWaterContact[4].pexpl) },
    { 0x02, (OID)0x182, offsetof(JT, jtWaterContact[5].pexpl) },
    { 0x02, (OID)0x183, offsetof(JT, jtWaterContact[6].pexpl) },
    { 0x02, (OID)0x184, offsetof(JT, jtWaterContact[7].pexpl) },
    { 0x02, (OID)0x185, offsetof(JT, pexplJumpLand) },
    { 0x02, (OID)0x186, offsetof(JT, pexplHideJumpLand) },
    { 0x02, (OID)0x18B, offsetof(JT, pemitterShadow) },
    { 0x20, (OID)0x18D, offsetof(JT, paloUseCharm) },
    { 0x02, (OID)0x11B, offsetof(JT, ablRailBalance[0].paseg) },
    { 0x02, (OID)0x11C, offsetof(JT, ablRailBalance[1].paseg) },
    { 0x02, (OID)0x11D, offsetof(JT, ablRailBalance[2].paseg) },
    { 0x02, (OID)0x153, offsetof(JT, pasegDecoy) },
    { 0x02, (OID)0x154, offsetof(JT, pasegStun) },
    { 0x02, (OID)0x155, offsetof(JT, pasegMinePlace) },
    { 0x02, (OID)0x156, offsetof(JT, pasegMineActivate) },
};

inline SNIP s_asnipJtPostLoad[27] =
{
    { 0x04, (OID)0x060, offsetof(JT, paloGut) },
    { 0x02, (OID)0x161, offsetof(JT, pzi.mpccharmpaloCharm[1]) },
    { 0x02, (OID)0x162, offsetof(JT, pzi.mpccharmpaloCharm[2]) },
    { 0x02, (OID)0x163, offsetof(JT, pzi.mpccharmpaloCharm[3]) },
    { 0x04, (OID)0x05B, offsetof(JT, paloSkinStandard) },
    { 0x02, (OID)0x062, offsetof(JT, paloSkinElectric) },
    { 0x02, (OID)0x063, offsetof(JT, paloSkinFire) },
    { 0x04, (OID)0x16F, offsetof(JT, paloTool) },
    { 0x04, (OID)0x170, offsetof(JT, paloHook) },
    { 0x02, (OID)0x173, offsetof(JT, paloKey) },
    { 0x04, (OID)0x160, offsetof(JT, pzi.mpccharmpaloCharm[0]) },
    { 0x20, (OID)0x164, offsetof(JT, paloCollectTarget[0]) },
    { 0x20, (OID)0x165, offsetof(JT, paloCollectTarget[1]) },
    { 0x20, (OID)0x166, offsetof(JT, paloCollectTarget[2]) },
    { 0x22, (OID)0x169, offsetof(JT, ptnKeyCelebration) },
    { 0x22, (OID)0x07E, offsetof(JT, psoSweepEffect) },
    { 0x02, (OID)0x07F, offsetof(JT, psoSmashEffect) },
    { 0x02, (OID)0x080, offsetof(JT, psoRushEffect) },
    { 0x02, (OID)0x1B9, offsetof(JT, psoDiveEffect) },
    { 0x22, (OID)0x081, offsetof(JT, psoBallEffect) },
    { 0x06, (OID)0x050, offsetof(JT, plboneMineOrigin) },
    { 0x26, (OID)0x18E, offsetof(JT, apaloMineHat[0]) },
    { 0x26, (OID)0x18F, offsetof(JT, apaloMineHat[1]) },
    { 0x26, (OID)0x190, offsetof(JT, apaloMineHat[2]) },
    { 0x26, (OID)0x191, offsetof(JT, apaloMineHat[3]) },
    { 0x22, (OID)0x192, offsetof(JT, pmine) },
    { 0x26, (OID)0x151, offsetof(JT, pdecoy) },
};

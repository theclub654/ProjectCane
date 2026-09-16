#pragma once
#include "step.h"

enum HFDK
{
    HFDK_Nil = -1,
    HFDK_Match = 0,
    HFDK_Left = 1,
    HFDK_Right = 2,
    HFDK_Stand = 3,
    HFDK_Max = 4
};

struct MJH 
{
    class JT* pjt;
    class HSHAPE* phshape;
    glm::vec3 vJump;
};

struct MJHPNT
{
    JT* pjt;
    HPNT* phpnt;
    glm::vec3 vJump;
};

struct MJR
{
    class JT* pjt;
    class RAIL* prail;
    glm::vec3 vJump;
};

struct MJT
{
    JT* pjt;                    
    ALO* paloTargetParent;      
    glm::vec3 posTargetLocal; 
};

struct MJP
{
    JT* pjt;
    PIPE* ppipe;
};

int  JtbsChooseJtHide(JT* pjt, LO* ploForce, int* pjthk);
void MeasureJtJumpToTarget(JT* pjt, glm::vec3* pvJump, ALO* paloTarget, glm::vec3* pposTarget, glm::vec3* pdvTarget, float* pdtJump, float* pgInteg, glm::vec3* pposPredict, glm::vec3* pvPredict);
void GetJtRailLanding(JT* pjt, RAIL* prail, float uRail, glm::vec3* ppos, glm::vec3* pv);
float GMeasureJumpRail(MJR* pmjr, float u);
float GMeasureJumpHpnt(MJHPNT* pmjhpnt, float u);
float GMeasureJumpToHshape(MJH* pmjh, float s);
void  MeasureJtJumpToTargetAtTime(float dt, JT* pjt, ALO* paloTargetParent, const glm::vec3* pposTargetLocal, float* pgMeasure, glm::vec3* pposGoal, glm::vec3* pvGoal);
float GMeasureJumpTargetTime(MJT* pmjt, float dt);
float GMeasureJumpPipe(MJP* pmjp, float s);
bool  FFindJtObstruction(JT* pjt, float dt, PFNFILTER pfnFilter, void* pvFilter, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, glm::vec3* pposFinal);
float GAdjustJtLandingScore(float g, float gPenalty, JT* pjt, LO* ploCandidate);
int  JtbsChooseJtLanding(JT* pjt, LO* ploForce);
void UpdateJtActiveHide(JT* pjt, JOY* pjoy);
void MatchJtXmgRail(JT* pjt, XMG* pxmg, ACTADJ* pactadj);
void UpdateJtHide(JT* pjt);
void UpdateJtInternalXpsHide(JT* pjt);
void SetJtHfdk(JT* pjt, HFDK hfdk);
void ResetJtDynamicTunnel(JT* pjt);
void PresetJtAccelHide(JT* pjt);
bool FJtSafeFromStepguards(JT* pjt);

static float S_JtVaultTrigger = 250;
static float S_JtFlattenMax = 150;
constexpr float DT_JtRailLimit = 0.2f;
constexpr float S_JtObstructionSlack = 100.0f;
// FLOAT_00274e20 in the retail executable.
constexpr float G_JtAutoJumpMax = 2500.0f;
constexpr float DS_HshapeSearch = 100.0f;
static float DS_JtRailLimit = 10.0;
constexpr float DT_JtFlattenSmooth = 0.3f;  // 0x00274e48
constexpr float SV_JtFlattenSidestepMin = 25.0f; // 0x00274e4c
constexpr float R_JtHideSpline = 1.5f;  // 0x00274e50
constexpr float RAD_JtExitSpin = 0.25f; // 0x00274e54
constexpr float U_JtShadowSneakMax = 0.5f;  // 0x00274e58
static SMP s_smpSvRailMax =
{
    2500.0f, // svFast
    0.0f,    // svSlow
    1.0f     // dtFast
};

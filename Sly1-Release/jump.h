#pragma once
#include "pnt.h"
#include "jt.h"

class JMT : public PNT
{
	public:
	DLE dleJmt;
	float sRadius;
	short oidSetIgnoreJt;
	class SO* psoSetIgnoreJt;
};

void PresetJtAccelJump(JT* pjt);
void UpdateJtActiveJump(JT* pjt);
int  FCheckJtSingleEdge(JT* pjt, SO* psoOther, EDGE* pedge, glm::vec3* pnormal);
int  FCheckJtEdgeNormal(JT* pjt, glm::vec3* pposEdge, glm::vec3* pnormal);
int  FCheckJtEdgeGrab(JT* pjt, SO* psoOther, glm::vec3* pposEdge, XPK xpk, void* pv, glm::vec3* pnormal);
XP*  PxpFindJtBestEdge(JT* pjt, int* pixpdBest, glm::vec3* pnormalBest);
int  JtsResetJt(JT* pjt);
void HandleJtDiveEffect(JT* pjt);
bool FCheckJtStuck(JT* pjt);
void UpdateJtJump(JT* pjt);
int  FTimeJtJumpToTarget(JT* pjt, glm::vec3* pposJt, glm::vec3* pvJt, glm::vec3* pposTarget, glm::vec3* pvTarget, float* pdtProject);
void GetJtJumpVelocity(JT* pjt, glm::vec3* pv);
void GetJtJumpToTargetVelocity(JT* pjt, glm::vec3* pv);
void GetJtJumpBoostVelocity(JT* pjt, glm::vec3* pv);

void StartupJmt();
void ResetJmtList();
JMT* NewJmt();
int  GetJmtSize();
void*GetJmtSRadius(JMT* pjmt);
void SetJmtSRadius(JMT* pjmt, float sRadius);
void SetJmtOidSetIgnoreJt(JMT* pjmt, int oidSetIgnoreJt);
void OnJmtAdd(JMT* pjmt);
void OnJmtRemove(JMT* pjmt);
void CloneJmt(JMT* pjmt, JMT* pjmtBase);
void PostJmtLoad(JMT* pjmt);
void DeleteJmt(JMT* pjmt);

extern DL g_dlJmt;
static float RVY_JtInitAccel = 10.0;
static CLQ s_clqThrownSteer = { 0.0, 5.0, 0.0 };
static CLQ s_clqDiveDamping = { 0.0, 10.0, 0.0 };
static float RVX_JtInitAccel = 60;
static float RVX_JtInitDecel = 20;
static float RVY_JtAccel = 2.5;
static float RVX_JtDecel = 3.5;
static float RVX_JtAccel = 2.5;
static const glm::vec3 s_aposCheck[22] =
{
    {  50.00f,   0.00f,   75.0f },
    {  50.00f,   0.00f, -150.0f },

    {  50.00f,   0.00f,   75.0f },
    { -60.00f,   0.00f,   75.0f },

    { -60.00f,   0.00f,   75.0f },
    { -60.00f,   0.00f,  -50.0f },

    { -57.96f,  15.53f,   75.0f },
    { -57.96f,  15.53f,  -50.0f },

    { -51.96f,  30.00f,   75.0f },
    { -51.96f,  30.00f,  -50.0f },

    { -42.43f,  42.43f,   75.0f },
    { -42.43f,  42.43f,  -50.0f },

    { -30.00f,  51.96f,   75.0f },
    { -30.00f,  51.96f,  -50.0f },

    { -57.96f, -15.53f,   75.0f },
    { -57.96f, -15.53f,  -50.0f },

    { -51.96f, -30.00f,   75.0f },
    { -51.96f, -30.00f,  -50.0f },

    { -42.43f, -42.43f,   75.0f },
    { -42.43f, -42.43f,  -50.0f },

    { -30.00f, -51.96f,   75.0f },
    { -30.00f, -51.96f,  -50.0f }
};
static float XY_JtEdgeGrabCheck = 60;
static float Z_JtEdgeGrabCheck = 70;
static constexpr float S_DiveEffectRadius = 300.0f;
static constexpr float S_DiveCoinRadius = 500.0f;
static constexpr float SV_DiveEffect = 1000.0f;
static constexpr float SDV_JtShockEquiv = 3000.0f;
static constexpr LM s_lmDzDiveEffect = { -150.0f, 150.0f };
static constexpr float VZ_JtTerminal = -750.0f;
static constexpr float RVZ_JtTerminal = 5.0f;
static constexpr float SVZ_JtSwingingJump = 700.0f;
static constexpr float DZ_JtStandingJump = 245.0f;
static constexpr float SVZ_JtFromWaterJump = 900.0f;
static constexpr float SVZ_JtEdgeJump = 800.0f;
static CLQ s_clqVzToVz = { 682.0f, 0.5f, 0.0f };
static LM s_lmVz = { -FLT_MAX, 682.0f };

inline constexpr float DT_JtJumpInit = 0.15f;    // 0x00263E50
inline constexpr float SVXY_JtJumpMatchHeading = 300.0f;   // 0x00263E54
inline constexpr float DT_JtJtjjInit = 0.07f;    // 0x00263E70
inline constexpr float DT_JtJtjjFull = 0.30f;    // 0x00263E74
inline constexpr float DT_JtMinSmash = 0.35f;    // 0x00264040
inline constexpr float DT_JtMinDive = 0.35f;    // 0x00264044
inline constexpr float S_JtJumpTargetPathMax = 10000.0f; // 0x00264048

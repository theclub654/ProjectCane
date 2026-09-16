#pragma once
#include "step.h"
typedef int GRFAHX;

void PostJtLoadSwing(JT* pjt, BL* ablSwing, ASEGBL** ppasegbl);
void AnticipateJtForce(JT* pjt, SO* psoOther, glm::vec3* pposOther, glm::vec3* pdv, FX* afx);
void CalculateJtHangAccel(JT* pjt);
void PresetJtAccelHang(JT* pjt);
void AddJtExternalAccelerations(JT* pjt, XA* pxa, float dt);
void UpdateJtActiveHang(JT* pjt, JOY* pjoy);
void CheckJtUnhook(JT* pjt);
void UpdateJtHookOx(JT* pjt, LO* ploHook, int fHook);
void AddJtHookXps(JT* pjt, GRFAHX grfahx, LO* ploHook, glm::vec3* pposHook, glm::vec3* pvecHook, glm::vec3* pposHang);
void GetJtHangHeading(JT* pjt, float* pradForward);
void UpdateJtIkHang(JT* pjt, glm::vec3* pposHand, glm::mat3* pmatHand, glm::mat3* pmatTool);
void UpdateJtHangIkGoals(JT* pjt);
void PredictJtRotation(JT* pjt, float dtOffset, glm::mat3* pmat, glm::vec3* pw);

constexpr float R_JtHangAxisDamping = -0.25f;
constexpr float R_JtHangDampingActive = -2.5f;
constexpr float R_JtHangDampingNeutral = -1.0f;
constexpr float DT_JtHangPump = 0.25f;
constexpr float SDW_JtHangPump = 5.0f;
inline CLQ s_clqDzToU =
{
    { 0.5f },
    -0.01f,
    0.0f,
    0.0f
};

inline SMPA s_smpaHang =
{
    1000.0f, // svFast
    0.0f,    // svSlow
    0.25f,   // dtFast
    5000.0f  // sdvMax
};

constexpr float S_JtHangMin = 75.0f;
constexpr float S_JtHangMax = 230.0f;

static SMP s_smpMat =
{
    { 25.0f }, // svFast
    0.0f,      // svSlow
    0.1f       // dtFast
};

constexpr float DT_HangForwardRelax = 1.4f;
constexpr float DT_HangBackwardRelax = 1.4f;
constexpr float DT_HangLastNeutral = 0.25f;
static SMP s_smpSwingForward =
{
    { 4.0f }, // svFast
    0.0f,     // svSlow
    0.1f      // dtFast
};


static SMP s_smpSwingActive = { 1.0, 0.0, 0.1 };

constexpr float RAD_SwingMax = 1.7f;
constexpr float RAD_JtHangWrist = 3.5f;
constexpr float RAD_JtHangTool = -1.7f;

static LM s_lmSvIdle = { 80, 120 };
static LM s_lmRadIdle = { 0.1, 0.4 };
static SMP s_smpHangIkPos = { 3000, 1000, 0.15 };
static SMP s_smpHangIkRot = { 30, 10, 0.15 };

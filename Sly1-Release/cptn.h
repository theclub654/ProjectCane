#pragma once
#include "cplcy.h"

struct SBI;

struct CPTN : public CPLCY
{
    struct TN* ptn;
    float radManual;
    float xyManual;
    float zManual;
    int fHome;
    float radHome;
    FTND ftnd;
    int fActivate;
    int fPanOnEntryDone;
    float swOrbit;
    float tLastOrbit;
    float radCur;
    float radFwdPrev;
    float radRevPrev;
    float tMoveLast;
    float uFollowCur;
    float vxy;
    float vz;
    glm::vec3 posEyePrev;
    float tActivate;
};

void ActivateCptn(CPTN* pcptn, TN* ptn);
void DeativateCptn(CPTN* pcptn);
void UpdateCptn(CPTN* pcptn, CPDEFI* pcpdefi, JOY* pjoy, float dt);

struct VTCPTN
{
    void (*pfnActivateCptn)(CPTN*, TN*) = ActivateCptn;
    void (*pfnDeactivateCptn)(CPTN*) = DeativateCptn;
    void (*pfnSetCptn)() = nullptr;
    void (*pfnRevokeCptn)() = nullptr;
    void (*pfnUpdateCptn)(CPTN*, CPDEFI*, JOY*, float) = UpdateCptn;
};

void UpdateCptnClosestPoint(CPTN* pcptn, CPDEFI* pcpdefi);
void UpdateCptnSquishCut(CPTN* pcptn);
float FtndCptnSquishConstraint(void* pvContext, float sRadius, const glm::vec3* pposEye, const SBI* psbi);
void FindCptnClearEyePosition(CPTN* pcptn, const glm::vec3* pposCenter, glm::vec3* pposEye, glm::vec3* pposEyeClear);

inline VTCPTN g_vtcptn;
extern SMP s_smpUFollow;
extern int g_fDisableSquish;
extern float DAMP_CptnPosition;
extern SMPA s_smpaHome;
extern float DSW_CptnOrbit;
extern float SW_CptnOrbitMax;
extern float DT_CptnSquishCheckDelay;
extern float DT_CptnSquishCutDelay;
extern float SPRING_CptnPosition;
static const glm::vec3 s_avecCptnSquishProbe[5] =
{
    glm::vec3(0.0f,   0.0f,   0.0f),
    glm::vec3(0.0f, -50.0f,   0.0f),
    glm::vec3(0.0f,  50.0f,   0.0f),
    glm::vec3(0.0f,   0.0f,  75.0f),
    glm::vec3(0.0f,   0.0f, -50.0f)
};

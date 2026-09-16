#pragma once
#include "po.h"
#include "asegbl.h"
#include "stepzap.h"

struct SFF
{
	float au[2];
	float auVol[2];
};

struct XMG 
{
    class ASEG* paseg;
    int fValid;
    float sSphere;
    glm::vec3 posExtrem;
	glm::vec3 posSphere;
	glm::vec3 posUp;
	glm::vec3 posDown;
    int fRetractValid;
	glm::vec3 posRetractStart;
	glm::vec3 posRetractEnd;
    float sRetract;
    LSG lsg;
};

class STEP : public PO
{
	public:
	ALO *paloLeftFoot;
	ALO *paloRightFoot;
	ALO *paloLeftHand;
	ALO *paloRightHand;
	ALO *paloLeftHeel;
	ALO *paloRightHeel;
	ALO *paloLeftBall;
	ALO *paloRightBall;
	float radJoy;
	float uJoyDeflect;
	float radTarget;
	float uTarget;
	glm::vec3 vTarget;
	float zBaseTarget;
	glm::mat3 matTarget;
	int fBaseXp;
	float tBaseXp;
	glm::vec3 posBaseXp;
	glm::vec3 normalBaseXp;
	SO *psoBaseXp;
	float uFootfall;
	float tLastFootfall;
	float dzBase;
	int iposBase;
	glm::vec3 *pposBase;
	glm::vec3 *pposBumper;
	SO *psoPhys;
	SMP smpSpin;
	CLQ clqDradToURun;
	float rGravity;
	SMP smpCompress;
	SMP smpExpand;
};

STEP*NewStep();
void InitStep(STEP* pstep);
int  GetStepSize();
void*GetStepDzBase(STEP* pstep);
void SetStepDzBase(STEP* pstep, float dzBase);
void CloneStep(STEP* pstep, STEP* pstepBase);
void PostStepLoad(STEP* pstep);
void LimitStepHands(STEP* pstep, int fLimit);
void RetractStepExtremity(STEP* pstep, ALO* paloExtrem, float sRadius, ALO* paloAdjust, ALO* paloIkh, XMG* pxmg);
void ClipStepRetractionToObject(STEP* pstep, SO* pso, BSP* pbspPruned, XMG* pxmg);
void ApplyStepExtremityRetraction(STEP* pstep, ALO* paloExtrem, ALO* paloAdjust, XMG* pxmg);
void RenderStepSelf(STEP* pstep, CM* pcm, RO* pro);
void ReadStepJoystick(STEP* pstep, JOY* pjoy);
void UpdateStepFootfall(STEP* pstep, float u, SFF* psff);
void UpdateStepPhys(STEP* pstep);
void SetStepPhys(STEP* pstep, SO* pso, int fForceSnap);
void UpdateStep(STEP* pstep, float dt);
CT   CtTorqueStep(STEP* pstep);
void PropagateStepForce(STEP* pstep, GRFSG grfsg, XP* pxp, int ixpd, DZ* pdz, FX* afx);
void RotateStepToMat(STEP* pstep, glm::mat3& pmat);
void CloneStepPhys(STEP* pstep, SO* psoPhys, int cposExtra);
void PresetStepAccel(STEP* pstep, float dt);
void PresetStepAccelBase(STEP* pstep);
int  FCheckStepXpBase(STEP* pstep, XP* pxp, int ixpd);
void AdjustStepNewXp(STEP* pstep, XP* pxp, int ixpd);
void AdjustStepDz(STEP* pstep, GRFADJ grfadj, DZ* pdz, int ixpd, float dt);
void AdjustStepDzBase(STEP* pstep, GRFADJ grfadj, DZ* pdz, int ixpd);
void UpdateStepMatTarget(STEP* pstep);
void AdjustStepXpVelocity(STEP* pstep, XP* pxp, int ixpd);
void UpdateStepXfWorld(STEP* pstep);
void AdjustStepXpVelocityBase(STEP* pstep, XP* pxp, int ixpd);
void AdjustStepXps(STEP* pstep);
void AddStepCustomXps(STEP* pstep, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst);
void AddStepCustomXpsBase(STEP* pstep, SO* psoOther, BSP* pbspPruned, XP** ppxpFirst);
void FixStepAngularVelocity(STEP* pstep);
void PredictStepPosition(STEP* pstep, float dtOffset, glm::vec3* ppos, glm::vec3* pv);
void PredictStepRotation(STEP* pstep, float dtOffset, glm::mat3* pmat, glm::vec3* pw);
void SetStepSpinRatio(STEP* pstep, float r);
void SetStepGravityRatio(STEP* pstep, float rGravity);
void DeleteStep(STEP *pstep);

extern SMP s_smpSpinStepDefault;
extern float MU_StepInternal;
extern float g_mpixpdr[2];
extern float G_StepBuoyancyDefault;
extern CLQ s_clqDradToURun;
extern SMP s_smpExpand;
extern SMP s_smpCompress;
extern SNIP s_asnipPostLoad[6];
extern SMP s_smpPhys;
extern float DT_StepFootfallInterval;

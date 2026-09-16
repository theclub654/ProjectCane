#pragma once
#include "shape.h"
#include "alo.h"

enum FTND;

enum TNS
{
	TNS_Nil = -1,
	TNS_Out = 0,
	TNS_In = 1,
	TNS_Max = 2
};

struct TSURF
{
	float gDot;
	glm::vec3 normal;
};
struct TBSP
{
	struct TSURF* ptsurf;
	struct TBSP* ptbspNeg;
	struct TBSP* ptbspPos;
};

struct TNFN 
{
	float dsCam;
	CLQ aclqCam[3];
	LM almCam[3];
	float xScreenPref;
	float yScreenPref;
	float uAdjust;
	float uFollow;
	float uLockOn;
	glm::vec3 vecFocus;
	float springStrength;
	float springDamping;
};

class TN : public ALO
{
	public:
	struct PO *ppo;
	int ctsurf;
	std::vector <TSURF> atsurf;
	int ctbsp;
	std::vector <TBSP> atbsp;
	std::shared_ptr <CRV> pcrv;
	TNFN tnfn;
	int fCylinder;
	int fSwitchInAir;
	int priCamera;
	int fFakeCylinder;
	float dradSlack;
	TNS tns;
	float tTns;
	float sClosest;
	glm::mat3 matXfm;
	glm::vec3 posXfm;
	float sTotal;
	glm::vec3 posFake;
	float sRadFake;
	int grftnd;
	int fPanOnEntry;
	int fPanOnIdle;
	float dtPanOnEntry;
	float rswPanOnEntry;
	float dtPanOnIdle;
	float rswPanOnIdle;
	int fUseVolume;
	CFK cfk;
	int fCutOnEntry;
	float radFOV;
	int fNoSquish;
	TNS tnsOverride;
	int fCutOnActivate;
	float radFOVOverride;
	float radFOVPrev;
	ALO* paloTarget;
	CLQ clqFollowTarget;
	LM lmFollowTarget;
};

TN*  NewTn();
TNFN* PtnfnFromTn(TN* ptn);
void GetTnfnNose(TNFN* ptnfn, CPDEFI* pcpdefi, glm::vec3* pposNose, TN* ptnAdjust);

void InitTn(TN* ptn);
void*GetTnDsCam(TN* ptn);
void SetTnDsCam(TN* ptn, float dsCam);
void*GetTnAclqCam0(TN* ptn);
void SetTnAclqCam0(TN* ptn, CLQ clqCam);
void*GetTnAlmCam0(TN* ptn);
void SetTnAlmCam0(TN* ptn, LM lmCam);
void*GetTnAclqCam1(TN* ptn);
void SetTnAclqCam1(TN* ptn, CLQ clqCam);
void*GetTnAlmCam1(TN* ptn);
void SetTnAlmCam1(TN* ptn, LM lmCam);
void*GetTnAclqCam2(TN* ptn);
void SetTnAclqCam2(TN* ptn, CLQ clqCam);
void*GetTnAlmCam2(TN* ptn);
void SetTnAlmCam2(TN* ptn, LM lmCam);
void*GetTnDradSlack(TN* ptn);
void SetTnDradSlack(TN* ptn, float dradSlack);
void*GetTnXScreenPref(TN* ptn);
void SetTnXScreenPref(TN* ptn, float xScreenPref);
void*GetTnYScreenPref(TN* ptn);
void SetTnYScreenPref(TN* ptn, float yScreenPref);
void*GetTnUAdjust(TN* ptn);
void SetTnUAdjust(TN* ptn, float uAdjust);
void*GetTnUFollow(TN* ptn);
void SetTnUFollow(TN* ptn, float uFollow);
void*GetTnULockOn(TN* ptn);
void SetTnULockOn(TN* ptn, float uLockOn);
void*GetTnFocusRatio(TN* ptn);
void*GetTnVecFocus(TN* ptn);
void SetTnVecFocus(TN* ptn, glm::vec3 vecFocus);
void*GetTnFCylinder(TN* ptn);
void SetTnFCylinder(TN* ptn, int fCylinder);
void*GetTnGrftnd(TN* ptn);
void SetTnGrftnd(TN* ptn, int grftnd);
void*GetTnFPanOnEntry(TN* ptn);
void SetTnFPanOnEntry(TN* ptn, int fPanOnEntry);
void*GetTnFPanOnIdle(TN* ptn);
void SetTnFPanOnIdle(TN* ptn, int fPanOnIdle);
void*GetTnPriCamera(TN* ptn);
void SetTnPriCamera(TN* ptn, int priCamera);
void*GetTnFSwitchInAir(TN* ptn);
void SetTnFSwitchInAir(TN* ptn, int fSwitchInAir);
void*GetTnCfk(TN* ptn);
void SetTnCfk(TN* ptn, CFK cfk);
void*GetTnDtPanOnEntry(TN* ptn);
void SetTnDtPanOnEntry(TN* ptn, float dtPanOnEntry);
void*GetTnRswPanOnEntry(TN* ptn);
void SetTnRswPanOnEntry(TN* ptn, float rswPanOnEntry);
void*GetTnDtPanOnIdle(TN* ptn);
void SetTnDtPanOnIdle(TN* ptn, float dtPanOnIdle);
void*GetTnRswPanOnIdle(TN* ptn);
void SetTnRswPanOnIdle(TN* ptn, float rswPanOnIdle);
void*GetTnFCutOnActivate(TN* ptn);
void SetTnFCutOnActivate(TN* ptn, int fCutOnActivate);
void*GetTnRadFOVOverride(TN* ptn);
void SetTnRadFOVOverride(TN* ptn, float radFOVOverride);
void*GetTnFNoSquish(TN* ptn);
void SetTnFNoSquish(TN* ptn, int fNoSquish);
void*GetTnPaloTarget(TN* ptn);
void SetTnPaloTarget(TN* ptn, ALO* paloTarget);
void*GetTnClqFollowTarget(TN* ptn);
void SetTnClqFollowTarget(TN* ptn, CLQ clqFollowTarget);
void*GetTnLmFollowTarget(TN* ptn);
void SetTnLmFollowTarget(TN* ptn, LM lmFollowTarget);
void*GetTnTns(TN* ptn);
void*GetTnTnsOverride(TN* ptn);
void SetTnTnsOverrideUser(TN* ptn, TNS tnsOverride);
void*GetTnSpringStrength(TN* ptn);
void SetTnSpringStrength(TN* ptn, float springStrength);
void*GetTnSpringDamping(TN* ptn);
void SetTnSpringDamping(TN* ptn, float springDamping);
int  GetTnSize();
void OnTnRemove(TN* ptn);
void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis);
void CloneTn(TN* ptn, TN* ptnBase);
void PostTnLoad(TN* ptn);
void SetTnTns(TN* ptn, TNS tns);
void SetTnTnsOverride(TN* ptn, TNS tnsOverride);
void UpdateTnCallback(TN* ptn, MSGID msgid, void*pv);
void UpdateTn(TN* ptn, float dt);
void RenderTnSelf(TN* ptn, CM* pcm, RO* pro);
void FreezeTn(TN* ptn, int fFreeze);
void SetTnFocusRatio(TN* ptn, float u);
void CalculateTnCrv(TN* ptn, glm::vec3* ppos, glm::vec3* pdposCrv, glm::vec3* pvecClosest);
void CalculateTnPos(TN* ptn, glm::vec3* pdposCrv, float sBase, float dsOffset, CLQ* aclq, LM* alm, FTND ftnd, glm::vec3* pposOther);
void LoadTbspFromBrx(int* pctsurf, std::vector<TSURF>& atsurf, int* pctbsp, std::vector<TBSP>& atbsp, CBinaryInputStream* pbis);
int  FCheckTbspPoint(TBSP* atbsp, glm::vec3* pposLocal);
void DeleteTn(TN* ptn);

extern TNFN g_tnfnDefault;
const TNFN g_tnfnDynamic = []()
	{
		TNFN tnfn{};

		tnfn.dsCam = 0.0f;

		tnfn.aclqCam[0].g0 = 0.0f;
		tnfn.aclqCam[0].g1 = 1.0f;
		tnfn.aclqCam[0].g2 = 0.0f;

		tnfn.aclqCam[1].g0 = -225.0f;
		tnfn.aclqCam[1].g1 = 1.0f;
		tnfn.aclqCam[1].g2 = 0.0f;

		tnfn.aclqCam[2].g0 = 20.0f;
		tnfn.aclqCam[2].g1 = 1.0f;
		tnfn.aclqCam[2].g2 = 0.0f;

		tnfn.almCam[0].gMin = -FLT_MAX;
		tnfn.almCam[0].gMax = FLT_MAX;

		tnfn.almCam[1].gMin = -FLT_MAX;
		tnfn.almCam[1].gMax = FLT_MAX;

		tnfn.almCam[2].gMin = -FLT_MAX;
		tnfn.almCam[2].gMax = FLT_MAX;

		tnfn.xScreenPref = 0.0f;
		tnfn.yScreenPref = 0.0f;
		tnfn.uAdjust = 1.0f;
		tnfn.uFollow = 0.0f;
		tnfn.uLockOn = 1.0f;

		tnfn.vecFocus = glm::vec3(1.0f, 1.0f, 1.0f);

		tnfn.springStrength = 1.0f;
		tnfn.springDamping = 1.0f;

		return tnfn;
	}();

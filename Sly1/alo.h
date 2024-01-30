#pragma once
#include "lo.h"
#include "act.h"
#include "freeze.h"
#include "glob.h"
#include "sqtr.h"

typedef uint32_t GRFZON;
typedef uint32_t GRFALOX;

struct XF
{
	glm::mat3 mat;
	glm::vec3 pos;
	glm::mat3 matWorld;
	glm::vec3 posWorld;
	glm::vec3 v;
	glm::vec3 w;
	glm::vec3 dv;
	glm::vec3 dw;
};

struct RO
{
	glm::mat4 mat;
	float uAlpha;
	float uAlphaCelBorder;
};

struct POSEC
{
	OID oid;
	std::vector <float> agPoses;
};

struct RSMG
{
	OID oidRoot;
	OID oidSM;
	OID oidTriggerGoal;
	OID oidUntriggerGoal;
};

enum ACK
{
	ACK_Nil = -1,
	ACK_None = 0,
	ACK_Spring = 1,
	ACK_Velocity = 2,
	ACK_Smooth = 3,
	ACK_Spline = 4,
	ACK_Drive = 5,
	ACK_SmoothForce = 6,
	ACK_SmoothLock = 7,
	ACK_SpringLock = 8,
	ACK_SmoothNoLock = 9,
	ACK_Max = 10
};

struct ALOX
{
	GRFALOX grfalox;
};

class ALO : public LO
{
	public:
		DL dlChild;
		DLE dleBusy;
		DLE dleMRD;
		ALO *paloRoot;
		ALO *paloFreezeRoot;
		DLE dleFreeze;
		DL dlFreeze;
		int cpmrg;
		struct MRG *apmrg[4];
		float sMRD;
		float sCelBorderMRD;
		GRFZON grfzon;
		float dsMRDSnap;
		char frz[64];
		XF xf;
		glm::vec3 posOrig;
		glm::mat3 matOrig;
		glm::vec3 eulOrig;
		DL dlAct;
		struct ACT* pactPos;
		struct ACT* pactRot;
		struct ACT* pactScale;
		struct ACT** apactPose;
		struct ACT* pactRestore;
		struct ACTLA* pactla;
		struct ACTBANK* pactbank;
		struct IKH* pikh;
		struct CLQ* pclqPosSpring;
		struct CLQ* pclqPosDamping;
		struct CLQ* pclqRotSpring;
		struct CLQ* pclqRotDamping;
		struct SMPA* psmpaPos;
		struct SMPA* psmpaRot;
		struct ALOX* palox;
		int cframeStatic;
		GLOBSET globset;
		struct SHADOW *pshadow;
		struct THROB* pthrob;
		float sFastShadowRadius;
		float sFastShadowDepth;
		int fRealClock;
		struct FADER* pfader;
		float dtUpdatePause;
		struct ASEGD* pasegd;
		float sRadiusRenderSelf;
		float sRadiusRenderAll;
		struct SFX* psfx;
		char ficg[5];
		int cposec;
		std::vector <POSEC> aposec;
		struct ACTREF* pactrefCombo;
		struct DLR* pdlrFirst;
		int mtlk : 8;
		uint32_t zons : 2;
		uint32_t viss : 2;
		uint32_t mrds : 2;
		uint32_t dms : 2;
		uint32_t fHidden : 1;
		uint32_t fFixedPhys : 1;
		uint32_t fMtlkFromDls : 1;
		uint32_t fWater : 1;
		uint32_t fForceCameraFade : 1;
		uint32_t fBusy : 1;
		uint32_t fFrozen : 1;
		uint32_t fRemerge : 1;
		uint32_t fNoFreeze : 1;
		uint32_t cpaloFindSwObjects : 4;
		uint32_t fApplyAseg : 1;
		ACK ackRot;
};

// Create ALO
void*NewAlo();
// Initialize ALO object
void InitAlo(ALO* palo); // NOT FINISHED
// Adds ALO parent and all the alo childs into the world
void AddAloHierarchy(ALO *palo);
void RemoveAloHierarchy(ALO *palo);
// Adds ALO to Hierarchy
void OnAloAdd(ALO* palo); // NOT FINISHED
// Removes ALO from Hierarchy
void OnAloRemove(ALO* palo);
// Makes a copy of ALO and all of its children
void CloneAloHierarchy(ALO* palo, ALO* paloBase);
// Makes a copy of ALO
void CloneAlo(ALO* palo, ALO* paloBase);
void ResolveAlo(ALO *palo);
// Sets a Alo object to a parent
void SetAloParent(ALO* palo, ALO* paloParent);
void ApplyAloProxy(ALO* palo, PROXY* pproxyApply);
// Updates the ALO objects transformations
void UpdateAloXfWorld(ALO* palo);
// Updates the ALO objects world transformation hierarchy
void UpdateAloXfWorldHierarchy(ALO* palo);
void TranslateAloToPos(ALO* palo, glm::vec3& ppos);
void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3 &pposFrom, glm::vec3 &pposTo);
void RotateAloToMat(ALO* palo, glm::mat3& pmat);
void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3 &pmatFrom, glm::mat3 &pmatTo);
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
// Loads bone data from binary file
void LoadAloAloxFromBrx(ALO* palo, CBinaryInputStream* pbis);
void UpdateAlo(ALO *palo, float dt);
void RenderAloAll(ALO* palo, CM* pcm, RO* proDup);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha);
void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro);
void DrawAlo(ALO *palo);
// Deletes Model from VRAM
void DeleteModel(ALO *palo);
int  GetAloSize();
void DeleteAlo(LO *palo);
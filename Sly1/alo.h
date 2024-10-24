#pragma once
#include "lo.h"
#include "act.h"
#include "freeze.h"
#include "glob.h"
#include "sqtr.h"

typedef uint32_t GRFZON;
typedef uint32_t GRFALOX;

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
enum MTLK : int
{
	MTLK_Nil = -1,
	MTLK_Default = 0,
	MTLK_Sand = 1,
	MTLK_Metal = 2,
	MTLK_Wood = 3,
	MTLK_Rock = 4,
	MTLK_Rubber = 5,
	MTLK_Ice = 6,
	MTLK_Tire = 7,
	MTLK_Velcro = 8,
	MTLK_NoBounce = 9,
	MTLK_NoFriction = 10,
	MTLK_NoFrictionElastic = 11,
	MTLK_Max = 12
};
enum ZONS : unsigned int
{
	ZONS_Nil = -1,
	ZONS_None = 0,
	ZONS_Fixed = 1,
	ZONS_Dynamic = 2,
	ZONS_Max = 3
};
enum VISS : unsigned int
{
	VISS_Nil = -1,
	VISS_None = 0,
	VISS_Glob = 1,
	VISS_Object = 2,
	VISS_Max = 3
};
enum MRDS : unsigned int
{
	MRDS_Nil = -1,
	MRDS_None = 0,
	MRDS_Glob = 1,
	MRDS_Object = 2,
	MRDS_Max = 3
};
enum DMS : unsigned int
{
	DMS_Normal = 0,
	DMS_UseMat = 1
};

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

struct ALOX
{
	GRFALOX grfalox;
};

struct FICG 
{
	union
	{
		byte grficSweep;
		byte agrfic[1];
	};

	byte grficRush;
	byte grficSmash;
	byte grficBomb;
	byte grficShock;
};

struct BITFIELD
{
	// First Byte
	unsigned int mtlk : 8;
	// Second Byte
	unsigned int zons : 2;
	unsigned int viss : 2;
	unsigned int mrds : 2;
	unsigned int dms : 2;
	// Third Byte
	unsigned int fHidden : 1;
	unsigned int fFixedPhys : 1;
	unsigned int fMtlkFromDls : 1;
	unsigned int fWater : 1;
	unsigned int fForceCameraFade : 1;
	unsigned int fBusy : 1;
	unsigned int fFrozen : 1;
	unsigned int fRemerge : 1;
	// Fourth Byte
	unsigned int fNoFreeze : 1;
	unsigned int cpaloFindSwObjects : 4;
	unsigned int fApplyAseg : 1;
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
		FICG ficg;
		int cposec;
		std::vector <POSEC> aposec;
		struct ACTREF* pactrefCombo;
		struct DLR* pdlrFirst;
		BITFIELD bitfield;
		ACK ackRot;
};

// Create ALO
ALO*NewAlo();
// Initialize ALO object
void InitAlo(ALO* palo); // NOT FINISHED
// Adds ALO parent and all the alo childs into the world
void AddAloHierarchy(ALO *palo);
// Removes ALO object from ALO Hierarchy
void RemoveAloHierarchy(ALO *palo);
// Adds ALO to Hierarchy
void OnAloAdd(ALO* palo); // NOT FINISHED
// Removes ALO from Hierarchy
void OnAloRemove(ALO* palo);
// Makes a copy of ALO and all of its children
void CloneAloHierarchy(ALO* palo, ALO* paloBase);
// Makes a copy of ALO object
void CloneAlo(ALO* palo, ALO* paloBase);
void ResolveAlo(ALO *palo);
// Sets a Alo object to a parent
void SetAloParent(ALO* palo, ALO* paloParent);
// Apply transformation to proxy ALO
void ApplyAloProxy(ALO* palo, PROXY* pproxyApply);
// Updates the ALO objects transformations
void UpdateAloXfWorld(ALO* palo);
// Updates the ALO objects world transformation hierarchy
void UpdateAloXfWorldHierarchy(ALO* palo);
// Moves ALO object to a position, Similar to the glm::translate function
void TranslateAloToPos(ALO* palo, glm::vec3& ppos);
void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3 &pposFrom, glm::vec3 &pposTo);
void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3 *pvecFrom, glm::vec3 *pvecTo);
void RotateAloToMat(ALO* palo, glm::mat3& pmat);
// Rotate or scale object to a new transformation, similar to the glm::rotate or glm::scale function
void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3 &pmatFrom, glm::mat3 &pmatTo);
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
// Loads bone data from binary file
void LoadAloAloxFromBrx(ALO* palo, CBinaryInputStream* pbis);
// Updates ALO object
void DebugClearLightingToF32(ALO* palo);
void DebugConvertLightingF32ToU8(ALO* palo);
void UpdateAlo(ALO *palo, float dt);
void RenderAloAll(ALO* palo, CM* pcm, RO* proDup);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha);
void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro);
// Draw Glob object
void DrawGlob(ALO *palo, int index);
// Deletes Model from VRAM
void DeleteModel(ALO *palo);
int  GetAloSize();
// Delete ALO object from memory
void DeleteAlo(ALO *palo);
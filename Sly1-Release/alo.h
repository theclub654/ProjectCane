#pragma once
#include "lo.h"
#include "glob.h"
#include "act.h"

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
enum SCSK
{
	SCSK_Nil = -1,
	SCSK_Fixed = 0,
	SCSK_Stretch = 1,
	SCSK_Max = 2
};
enum THROBK
{
	THROBK_Nil = -1,
	THROBK_Pipe = 0,
	THROBK_Ninja = 1,
	THROBK_Rail = 2,
	THROBK_Hide = 3,
	THROBK_Foo = 4,
	THROBK_Bar = 5,
	THROBK_Max = 6
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

struct SNIP
{
	int grfsnip;
	OID oid;
	int ib;
};

enum IAK
{
	IAK_Nil = -1,
	IAK_Time = 0,
	IAK_Proportion = 1,
	IAK_Nearest = 2,
	IAK_Max = 3
};

struct ASEGD
{
	struct ASEG* paseg;
	OID oidAseg;
	IAK iak;
	float tLocal;
	float svtLocal;
};


struct ALOX
{
	glm::mat3 matPreRotation;
	glm::mat3 matPostRotation;

	union
	{
		ALO* paloParent;
	} foster;

	union
	{
		OID oidIkh;
		ALO* paloIkh;
		int fInvalid;
	}ikj;

	union
	{
		glm::vec3 posIkh;
		glm::vec3 posWrist;
		ALO* paloShoulder;

		union
		{
			OID oidElbow;
			ALO* paloElbow;

		};

		ALO* paloCommon;
		float radTwistOrig;
		float radTwist;
		float dradTwist;
		GRFIK grfik;
	}ikh;

	union
	{
		ALO* paloSchRot;
		int ipaloRot;
		ALO* paloSchPos;
		int ipaloPos;
		int fInvalidRot;
		int fInvalidPos;

	}scj;

	union
	{
		glm::vec3 posSch;
		glm::vec3 posEnd;
		float gStrength;
		union
		{
			OID oidScjStart;
			ALO* paloScjStart;
		};

		union
		{
			OID oidScjEnd;
			ALO* paloScjEnd;
		};

		union
		{
			OID oidSchPrev;
			ALO* paloSchPrev;
		};
		ALO* paloCommon;
		SCSK scsk;
		int cpalo;
		ALO** apalo;

	}sch;

	union
	{
		union
		{
			OID oidFocus;
			PNT* ppntFocus;
		};

		union
		{
			OID oidTarget;
			PNT* ppntTarget;

		};
	}looker;

	union
	{
		glm::mat4 matInfluence;
		int fSsc;
		int fMatInfluence;
	}joint;
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
	ALO* paloRoot;
	ALO* paloFreezeRoot;
	DLE dleFreeze;
	DL dlFreeze;
	int cpmrg;
	struct MRG* apmrg[4];
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
	std::shared_ptr <ALOX> palox;
	int cframeStatic;
	GLOBSET globset;
	struct SHADOW* pshadow;
	struct THROB* pthrob;
	float sFastShadowRadius;
	float sFastShadowDepth;
	int fRealClock;
	struct FADER* pfader;
	float dtUpdatePause;
	std::shared_ptr <ASEGD> pasegd;
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
ALO* NewAlo();
// Initialize ALO object
void InitAlo(ALO* palo); // NOT FINISHED
// Adds ALO parent and all the alo childs into the world
void AddAloHierarchy(ALO* palo);
// Removes ALO object from ALO Hierarchy
void RemoveAloHierarchy(ALO* palo);
// Adds ALO to Hierarchy
void OnAloAdd(ALO* palo); // NOT FINISHED
// Removes ALO from Hierarchy
void OnAloRemove(ALO* palo);
void UpdateAloOrig(ALO* palo);
// Makes ALO object follow camera rotation
void AdjustAloRtckMat(ALO* palo, CM* pcm, RTCK rtck, glm::vec3* pposCenter, glm::mat4& pmat);
// Makes a copy of ALO and all of its children
void CloneAloHierarchy(ALO* palo, ALO* paloBase);
// Makes a copy of ALO object
void CloneAlo(ALO* palo, ALO* paloBase);
void ResolveAlo(ALO* palo);
// Sets a Alo object to a parent
void SetAloParent(ALO* palo, ALO* paloParent);
// Apply transformation to proxy ALO
void ApplyAloProxy(ALO* palo, PROXY* pproxyApply);
void BindAlo(ALO* palo);
void BindGlobset(GLOBSET* pglobset, ALO* palo);
// Updates the ALO objects transformations
void UpdateAloXfWorld(ALO* palo);
// Updates the ALO objects world transformation hierarchy
void UpdateAloXfWorldHierarchy(ALO* palo);
// Updates Alo and children
void UpdateAloHierarchy(ALO* palo, float dt);
// Moves ALO object to a position
void TranslateAloToPos(ALO* palo, glm::vec3& ppos);
void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3& pposFrom, glm::vec3& pposTo);
void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3* pvecFrom, glm::vec3* pvecTo);
void RotateAloToMat(ALO* palo, glm::mat3& pmat);
// Rotate or scale object to a new transformation
void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3& pmatFrom, glm::mat3& pmatTo);
void SetAloInitialVelocity(ALO* palo, glm::vec3* pv);
void SetAloInitialAngularVelocity(ALO* palo, const glm::vec3* pw);
ASEGD* PasegdEnsureAlo(ALO* palo);
void SetAloAsegdOid(ALO* palo, short oid);
void SetAloAsegdtLocal(ALO* palo, float tLocal);
void SetAloAsegdSvtLocal(ALO* palo, float svtLocal);
void SetAloAsegdiak(ALO* palo, int iak);
void SetAloFrozen(ALO* palo, bool fFrozen);
void SetAloEuler(ALO* palo, glm::vec3* peul);
void SetAloVelocityLocal(ALO* palo, glm::vec3* pvec);
void SetAloFastShadowRadius(ALO* palo, float sRadius);
void SetAloFastShadowDepth(ALO* palo, float sDepth);
void SetAloCastShadow(ALO* palo, int fCastShadow);
void SetAloNoFreeze(ALO* palo, int fNoFreeze);
void SetAloRestorePosition(ALO* palo, int fRestore);
void SetAloRestorePositionAck(ALO* palo, ACK ack);
void SetAloPositionSpring(ALO* palo, float r);
void SetAloPositionSpringDetail(ALO* palo, CLQ* pclq);
void SetAloPositionDamping(ALO* palo, float r);
void SetAloPositionDampingDetail(ALO* palo, CLQ* pclq);
void SetAloRestoreRotation(ALO* palo, int fRestore);
void SetAloRestoreRotationAck(ALO* palo, ACK ack);
void SetAloRotationSpring(ALO* palo, float r);
void SetAloRotationSpringDetail(ALO* palo, CLQ* pclq);
void SetAloRotationDamping(ALO* palo, float r);
void SetAloRotationDampingDetail(ALO* palo, CLQ* pclq);
void SetAloPositionSmooth(ALO* palo, float r);
void SetAloPositionSmoothDetail(ALO* palo, SMPA* psmpa);
void SetAloRotationSmooth(ALO* palo, float r);
void SetAloRotationSmoothDetail(ALO* palo, SMPA* psmpa);
void SetAloPositionSmoothMaxAccel(ALO* palo, float r);
void SetAloRotationSmoothMaxAccel(ALO* palo, float r);
void SetAloDefaultAckPos(ALO* palo, ACK ack);
void SetAloDefaultAckRot(ALO* palo, ACK ack);
void SetAloLookAt(ALO* palo, ACK ack);
void SetAloLookAtIgnore(ALO* palo, float sIgnore);
void SetAloLookAtPanFunction(ALO* palo, CLQ* pclq);
void SetAloLookAtPanLimits(ALO* palo, LM* plm);
void SetAloLookAtTiltFunction(ALO* palo, CLQ* pclq);
void SetAloLookAtTiltLimits(ALO* palo, LM* plm);
void SetAloLookAtEnabledPriority(ALO* palo, int nPriority);
void SetAloLookAtDisabledPriority(ALO* palo, int nPriority);
void SetAloTargetAttacks(ALO* palo, int grftak);
void SetAloTargetRadius(ALO* palo, float sRadiusTarget);
void SetAloThrobKind(ALO* palo, THROBK throbk);
void SetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor);
void SetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor);
void SetAloThrobDtInOut(ALO* palo, float dtInOut);
void SetAloSfxid(ALO* palo, SFXID sfxid);
void SetAloSStart(ALO* palo, float sStart);
void SetAloSFull(ALO* palo, float sFull);
void SetAloUVolumeSpl(ALO* palo, float uVol);
void SetAloUVolume(ALO* palo, float uVol);
void SetAloUPitchSpl(ALO* palo, float uPitch);
void SetAloUPitch(ALO* palo, float uPitch);
void SetAloSndRepeat(ALO* palo, LM* plm);
void SetAloUDoppler(ALO* palo, float uDoppler);
void SetAloInteractCane(ALO* palo, int grfic);
void SetAloInteractCaneSweep(ALO* palo, int grfic);
void SetAloInteractCaneRush(ALO* palo, int grfic);
void SetAloInteractCaneSmash(ALO* palo, int grfic);
void SetAloInteractBomb(ALO* palo, int grfic);
void SetAloInteractShock(ALO* palo, int grfic);
void SetAloPoseCombo(ALO* palo, OID oidCombo);
void SetAloForceCameraFade(ALO* palo, int fFade);
//GOTTA COME BACK TO THIS
void*GetAloFrozen(ALO* palo);
void*GetAloXfPos(ALO* palo);
void*GetAloXfPosOrig(ALO* palo);
void*GetAloXfPosWorld(ALO* palo);
void*GetAloXfMat(ALO* palo);
void*GetAloMatOrig(ALO* palo);
void*GetAloXfMatWorld(ALO* palo);
void*GetAloEuler(ALO* palo);
void GetAloVelocityLocal(ALO* palo, glm::vec3* pvec);
void*GetAloXfw(ALO* palo);
void*GetAloXfdv(ALO* palo);
void*GetAloXfdw(ALO* palo);
void*GetAloRoot(ALO* palo);
void GetAloFastShadowRadius(ALO* palo, float* psRadius);
void GetAloFastShadowDepth(ALO* palo, float* psDepth);
void GetAloCastShadow(ALO* palo, int* pfCastShadow);
void GetAloLookAtIgnore(ALO* palo, float* psIgnore);
void GetAloLookAtPanFunction(ALO* palo, CLQ* pclq);
void GetAloLookAtPanLimits(ALO* palo, LM* plm);
void GetAloLookAtTiltFunction(ALO* palo, CLQ* pclq);
void GetAloLookAtTiltLimits(ALO* palo, LM* plm);
void GetAloLookAtEnabledPriority(ALO* palo, int* pnPriority);
void GetAloLookAtDisabledPriority(ALO* palo, int* pnPriority);
int  FGetAloChildrenList(ALO* palo, void* pvstate);
void GetAloThrobKind(ALO* palo, THROBK* pthrobk);
void GetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor);
void GetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor);
void GetAloThrobDtInOut(ALO* palo, float* pdtInOut);
void GetAloSfxid(ALO* palo, SFXID* psfxid);
void GetAloSStart(ALO* palo, float* psStart);
void GetAloSFull(ALO* palo, float* psFull);
void GetAloUVolume(ALO* palo, float* puVol);
void GetAloUPitch(ALO* palo, float* puPitch);
void GetAloSndRepeat(ALO* palo, LM* plmRepeat);
void GetAloUDoppler(ALO* palo, float* puDoppler);
void GetAloInteractCane(ALO* palo, int* pgrfic);
void GetAloInteractCaneSweep(ALO* palo, int* pgrfic);
void GetAloInteractCaneRush(ALO* palo, int* pgrfic);
void GetAloInteractCaneSmash(ALO* palo, int* pgrfic);
void GetAloInteractBomb(ALO* palo, int* pgrfic);
void GetAloInteractShock(ALO* palo, int* pgrfic);
void*GetAlofRealClock(ALO* palo);
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
// Loads bone data from binary file
void LoadAloAloxFromBrx(ALO* palo, CBinaryInputStream* pbis);
void BindAloAlox(ALO* palo);
void SnipAloObjects(ALO* palo, int csnip, SNIP* asnip);
void PostAloLoad(ALO *palo);
// Updates ALO object
void UpdateAlo(ALO* palo, float dt);
void RenderAloAll(ALO* palo, CM* pcm, RO* pro);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void DupAloRo(ALO* palo, RO* proOrig, RO* proDup);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha);
void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro);
// Draw a 3D model submesh
void DrawGlob(RPL* prpl);
// Deletes Model from VRAM
void DeleteModel(ALO* palo);
int  GetAloSize();
// Delete ALO object from memory
void DeleteAlo(ALO* palo);

extern std::vector <ALO*> allSWAloObjs;
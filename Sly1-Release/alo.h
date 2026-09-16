#pragma once
#define ALO_BUILDING
#include "lo.h"
#include "glob.h"
#undef ALO_BUILDING

struct SHADOW;
struct FADER;
struct SFX;
struct ACT;
struct ACTLA;
struct ACTBANK;
struct ACTREF;
struct ACTADJ;
struct ACTSEG;
struct RO;
struct BLOT;

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

enum FREEZEMODE : unsigned int
{
	FREEZEMODE_Normal = 0,
	FREEZEMODE_AlwaysBusy = 1,
	FREEZEMODE_AlwaysFrozen = 2,
	FREEZEMODE_Max = 3
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

enum CT
{
	CT_Free = 0,
	CT_Tangent = 1,
	CT_Project = 2,
	CT_Locked = 3
};

struct WKR 
{
	LO* ploSource;
	LO* ploTarget;
	float sftMax;
	GRFIC grfic;
	GRFTAK grftak;
	float gSort;
	glm::vec3 pos;
	glm::vec3 v;
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

struct MSGTRANS
{
	struct ALO *palo;
	glm::vec3 posWorldPrev;
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

struct THROB 
{
	THROBK throbk;
	glm::vec3 hsvIn;
	glm::vec3 hsvOut;
	float dtInOut;
};

struct MSGDMG 
{
	LO* ploDamaged;
	LO* ploSource;
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

struct FOSTER
{
	ALO* paloParent;
};

struct IKJ
{
	OID oidIkh;
	ALO* paloIkh;
	int fInvalid;
};

struct IKHALOX
{
	glm::vec3 posIkh;
	glm::vec3 posWrist;
	ALO* paloShoulder;
	OID oidElbow;
	ALO* paloElbow;
	ALO* paloCommon;
	float radTwistOrig;
	float radTwist;
	float dradTwist;
	GRFIK grfik;
};

struct SCJ
{
	ALO* paloSchRot;
	int ipaloRot;
	ALO* paloSchPos;
	int ipaloPos;
	int fInvalidRot;
	int fInvalidPos;

};

struct SCH
{
	glm::vec3 posSch;
	glm::vec3 posEnd;
	float gStrength;

	OID oidScjStart;
	ALO* paloScjStart;

	OID oidScjEnd;
	ALO* paloScjEnd;

	OID oidSchPrev;
	ALO* paloSchPrev;
	ALO* paloCommon;
	SCSK scsk;
	int cpalo;
	std::vector <ALO*> apalo;
};

struct LOOKERALOX
{
	OID oidFocus;
	PNT* ppntFocus;

	OID oidTarget;
	PNT* ppntTarget;
};

struct JOINT
{
	glm::mat4 matInfluence;
	int fSsc;
	int fMatInfluence;
	OID oidParent;
	OID oidChild;
	ALO* paloParent;
	ALO* paloChild;
};

struct ALOX
{
	glm::mat3 matPreRotation;
	glm::mat3 matPostRotation;
	FOSTER foster;
	IKJ ikj;
	IKHALOX ikh;
	SCJ scj;
	SCH sch;
	LOOKERALOX looker;
	JOINT joint;
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

struct FRZ
{
	CT ctForce;
	CT ctTorque;
	glm::vec3 v;
	glm::vec3 w;
	unsigned int fLockedSelf : 1;
	unsigned int fLockedAll : 1;
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
	struct MRG* apmrg[4];
	float sMRD;
	float sCelBorderMRD;
	GRFZON grfzon;
	float dsMRDSnap;
	FRZ frz;
	XF xf;
	glm::vec3 posOrig;
	glm::mat3 matOrig;
	glm::vec3 eulOrig;
	DL dlAct;
	struct ACT *pactPos;
	struct ACT *pactRot;
	struct ACT *pactScale;
	std::vector <ACT*> apactPose;
	std::shared_ptr <ACT> pactRestore;
	std::shared_ptr <ACTLA> pactla;
	std::shared_ptr <ACTBANK> pactbank;
	struct IKH *pikh;
	std::shared_ptr <CLQ> pclqPosSpring;
	std::shared_ptr <CLQ> pclqPosDamping;
	std::shared_ptr <CLQ> pclqRotSpring;
	std::shared_ptr <CLQ> pclqRotDamping;
	std::shared_ptr <SMPA> psmpaPos;
	std::shared_ptr <SMPA> psmpaRot;
	std::shared_ptr <ALOX> palox;
	int cframeStatic;
	GLOBSET globset;
	std::shared_ptr <SHADOW> pshadow;
	std::shared_ptr <THROB> pthrob;
	float sFastShadowRadius;
	float sFastShadowDepth;
	int fRealClock;
	std::shared_ptr <FADER> pfader;
	float dtUpdatePause;
	std::shared_ptr <ASEGD> pasegd;
	float sRadiusRenderSelf;
	float sRadiusRenderAll;
	glm::vec3 posCenter;
	std::shared_ptr <SFX> psfx;
	FICG ficg;
	int cposec;
	std::vector <POSEC> aposec;
	struct ACTREF *pactrefCombo;
	std::shared_ptr <DLR> pdlrFirst;
	int mtlk : 8;
	ACK ackPos;
	ACK ackRot;
	unsigned int zons : 2;
	unsigned int viss : 2;
	unsigned int mrds : 2;
	unsigned int dms : 2;
	unsigned int fHidden : 1;
	unsigned int fFixedPhys : 1;
	unsigned int fMtlkFromDls : 1;
	unsigned int fWater : 1;
	unsigned int fForceCameraFade : 1;
	unsigned int fBusy : 1;
	unsigned int fFrozen : 1;
	unsigned int fRemerge : 1;
	union
	{
		unsigned int freezeMode : 2;
		unsigned int fNoFreeze : 1;
	};
	unsigned int cpaloFindSwObjects : 4;
	unsigned int fApplyAseg : 1;

};

#ifdef ALO_IMPLEMENTATION
// Implementation-only dependencies. Keeping them out of ordinary users of
// alo.h prevents the ALO/SO/emitter header cycle.
#include "act.h"
#include "wr.h"
#include "shadow.h"
#include "freeze.h"
#include "sound.h"
#include "fader.h"
#endif

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
void HandleAloMessage(ALO* palo, MSGID msgid, void* pv);
bool FIsZeroV(const glm::vec3& v);
bool FIsZeroW(const glm::vec3& w);
bool FIsZeroDv(const glm::vec3& dv);
bool FIsZeroDw(const glm::vec3& dw);
int  FIsAloStatic(ALO* palo);
void ResolveAlo(ALO* palo);
void InvalidateAloLighting(ALO* palo);
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
void TranslateAloToPos(ALO* palo, glm::vec3 *ppos);
void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3 *pposFrom, glm::vec3 *pposTo);
void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3 *pvecFrom, glm::vec3 *pvecTo);
void RotateAloToMat(ALO* palo, glm::mat3 *pmat);
// Rotate or scale object to a new transformation
void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3 *pmatFrom, glm::mat3 *pmatTo);
void SetAloTransformBasis(ALO* palo, ALO* paloPosBasis, ALO* paloRotBasis);
int  FDrivenAlo(ALO* palo);
void ConvertAloMovement(ALO* paloFrom, ALO* paloTo, glm::vec3* ppos, glm::vec3* pvFrom, glm::vec3* pwFrom, glm::vec3* pdvFrom, glm::vec3* pdwFrom, glm::vec3* pvTo, glm::vec3* pwTo, glm::vec3* pdvTo, glm::vec3* pdwTo);
void SetAloVelocityVec(ALO* palo, glm::vec3 *velocity);
void SetAloVelocityXYZ(ALO* palo, float x, float y, float z);
void SetAloAngularVelocityVec(ALO* palo, glm::vec3 *angularVelocity);
void SetAloAngularVelocityXYZ(ALO* palo, float x, float y, float z);
void PresetAloAccel(ALO* palo, float dt);
void ProjectAloTransform(ALO* palo, float dt, int fParentDirty);
void PredictAloTransform(ALO* paloLeaf, ALO* paloBasis, float dtOffset, glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pv, glm::vec3* pw);
void PredictAloTransformAdjust(ALO* paloLeaf, ALO* paloBasis, float dtOffset, glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pv, glm::vec3* pw);
void UpdateAloInfluences(ALO* palo, RO* pro);
void AdjustAloRotation(ALO* palo, glm::mat3* pmat, glm::vec3* pw);
void UnadjustAloRotation(ALO* palo, glm::mat3* pmat);
void RecacheAloActList(ALO* palo);
void ResortAloActList(ALO* palo);
ASEGA* PasegaFindAlo(ALO* palo, OID oidAseg);
ASEGA* PasegaFindAloNearest(ALO* paloLeaf);
void RetractAloDrive(ALO* palo);
SMA* PsmaFindAlo(ALO* palo, OID oidSm);
void CreateAloActadj(ALO* palo, int nPriority, ACTADJ** ppactadj);
void UpdateAloConstraints(ALO* palo);
int  FAbsorbAloWkr(ALO* palo, WKR* pwkr);
void SetAloInitialVelocity(ALO* palo, glm::vec3* pv);
void SetAloInitialAngularVelocity(ALO* palo, const glm::vec3* pw);
void MatchAloOtherObject(ALO* palo, ALO* paloOther);
ASEGD* PasegdEnsureAlo(ALO* palo);
OID* PasegdEnsureAloOid(ALO* palo);
float* PasegdEnsureAlotLocal(ALO* palo);
float* PasegdEnsureAlosvtLocal(ALO* palo);
IAK* PasegdEnsureAloiak(ALO* palo);
SHADOW* PshadowAloEnsure(ALO* palo);
SHADOW* PshadowInferAlo(ALO* palo);
void EnsureAloActRestore(ALO* palo);
void EnsureAloActla(ALO* palo);
void InsertAloAct(ALO* palo, ACT* pact);
void SetAloAsegdOid(ALO* palo, short oid);
void SetAloAsegdtLocal(ALO* palo, float tLocal);
void SetAloAsegdSvtLocal(ALO* palo, float svtLocal);
void SetAloAsegdiak(ALO* palo, int iak);
void SetAloFrozen(ALO* palo, bool fFrozen);
void SetAloEuler(ALO* palo, glm::vec3* peul);
void SetAloVelocityLocal(ALO* palo, glm::vec3* pvec);
void SetAloFastShadowRadius(ALO* palo, float sRadius);
void SetAloFastShadowDepth(ALO* palo, float sDepth);
void SetAloCastShadow(ALO* palo, byte fCastShadow);
void SetAloShadowShader(ALO* palo, OID oidShdShadow);
void GetAloShadowShader(ALO* palo, OID* poidShdShadow);
void GetAloShadowNearRadius(ALO* palo, float* psNearRadius);
void SetAloShadowNearRadius(ALO* palo, float sNearRadius);
void SetAloShadowFarRadius(ALO* palo, float sFarRadius);
void GetAloShadowFarRadius(ALO* palo, float* psFarRadius);
void SetAloShadowNearCast(ALO* palo, float sNearCast);
void GetAloShadowNearCast(ALO* palo, float* psNearCast);
void SetAloShadowFarCast(ALO* palo, float sFarCast);
void GetAloShadowFarCast(ALO* palo, float* psFarCast);
void SetAloShadowConeAngle(ALO* palo, float degConeAngle);
void GetAloShadowConeAngle(ALO* palo, float* pdegConeAngle);
void SetAloShadowFrustrumUp(ALO* palo, glm::vec3* pvecUp);
void GetAloShadowFrustrumUp(ALO* palo, glm::vec3* pvecUp);
void SetAloDynamicShadowObject(ALO* palo, OID oidDysh);
void SetAloNoFreeze(ALO* palo, int fNoFreeze);
void SetAloFreezeMode(ALO* palo, FREEZEMODE freezeMode);
void GetAloFreezeMode(ALO* palo, FREEZEMODE* pfreezeMode);
void SetAloRestorePosition(ALO* palo, int fRestore);
void SetAloRestorePositionAck(ALO* palo, ACK ack);
void SetAloPositionSpring(ALO* palo, float r);
void SetAloPositionSpringDetail(ALO* palo, CLQ* pclq);
void SetAloPositionDamping(ALO* palo, float r);
void SetAloPositionDampingDetail(ALO* palo, CLQ* pclq);
void SetAloRestoreRotation(ALO* palo, int fRestore);
void SetAloRestoreRotationAck(ALO* palo, ACK ack);
void SetAloActPriority(ALO* palo, int nPriority);
void SetAloRotationMatchesVelocity(ALO* palo, float uBank, float dtPredict, ACK ackRot);
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
TARGET* PtargetEnsureAlo(ALO* palo);
void SetAloTargetAttacks(ALO* palo, int grftak);
void SetAloTargetRadius(ALO* palo, float sRadiusTarget);
void SetAloTargetHitTest(ALO* palo, int fHitTest);
void EnsureAloThrob(ALO* palo);
void SetAloThrobKind(ALO* palo, THROBK throbk);
void SetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor);
void SetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor);
void SetAloThrobDtInOut(ALO* palo, float dtInOut);
void NewSfx(std::shared_ptr<SFX> &psfx);
void EnsureAloSfx(ALO* palo);
void SetAloSfxid(ALO* palo, SFXID sfxid);
void SetAloSStart(ALO* palo, float sStart);
void SetAloSFull(ALO* palo, float sFull);
void SetAloUVolumeSpl(ALO* palo, float uVol);
void SetAloUVolume(ALO* palo, float uVol);
void SetAloUPitchSpl(ALO* palo, float uPitch);
void SetAloUPitch(ALO* palo, float uPitch);
void SetAloSndRepeat(ALO* palo, LM plm);
void SetAloUDoppler(ALO* palo, float uDoppler);
void SetAloInteractCane(ALO* palo, int grfic);
void SetAloInteractCaneSweep(ALO* palo, int grfic);
void SetAloInteractCaneRush(ALO* palo, int grfic);
void SetAloInteractCaneSmash(ALO* palo, int grfic);
void SetAloInteractBomb(ALO* palo, int grfic);
void SetAloInteractShock(ALO* palo, int grfic);
void SetAloPoseCombo(ALO* palo, OID oidCombo);
void SetAloForceCameraFade(ALO* palo, int fFade);
void SetAloCelRgba(ALO* palo, RGBA prgba);
void SetAloOverrideCel(ALO* palo, glm::vec4* rgba);
void UpdateAloThrob(ALO* palo, float dt);
void SetAloBlotContext(ALO* palo, BLOT* pblot);
//GOTTA COME BACK TO THIS
void GetAloFrozen(ALO* palo, int* pfFrozen);
void SetAloHidden(ALO* palo, int fHidden);
void GetAloHidden(ALO* palo, int* pfHidden);
void* GetAloXfPos(ALO* palo);
void SetAloXfPos(ALO* palo, glm::vec3 value);
void* GetAloXfPosOrig(ALO* palo);
void SetAloXfPosOrig(ALO* palo, glm::vec3 value);
void* GetAloXfPosWorld(ALO* palo);
void SetAloXfPosWorld(ALO* palo, glm::vec3 value);
void* GetAloXfMat(ALO* palo);
void SetAloXfMat(ALO* palo, glm::mat3 value);
void* GetAloMatOrig(ALO* palo);
void SetAloMatOrig(ALO* palo, glm::mat3 value);
void* GetAloXfMatWorld(ALO* palo);
void SetAloXfMatWorld(ALO* palo, glm::mat3 value);
void GetAloEuler(ALO* palo, glm::vec3* peul);
void GetAloVelocityLocal(ALO* palo, glm::vec3* pvec);
void* GetAloXfw(ALO* palo);
void SetAloXfw(ALO* palo, glm::vec3 value);
void* GetAloXfdv(ALO* palo);
void SetAloXfdv(ALO* palo, glm::vec3 value);
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
int  FGetAloChildrenList(ALO* palo, void* pvstate); // GOTTA COME BACK TO THIS
void GetAloThrobKind(ALO* palo, THROBK* pthrobk);
void GetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor);
void GetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor);
void GetAloThrobDtInOut(ALO* palo, float* pdtInOut);
void SetAloSfxidSpl(ALO* palo, SFXID sfxid);
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
void SetAloRealClock(ALO* palo, int fRealClock);
void SetAloScrollingMasterSpeeds(ALO* palo, float svu, float svv);
void SetAloEyesClosed(ALO* palo, float uClosed);
void GetAloActlaTarget(ALO* palo, ALO** ppaloTarget);
void SetAloActlaTarget(ALO* palo, ALO* paloTarget);
void ResumeAloActla(ALO* palo);
void PauseAloActla(ALO* palo);
void StartAloSound(ALO* palo, SFXID sfxid, float sStart, float sFull, float uVol, LM* plmRepeat);
void StopAloSound(ALO* palo);
void EnsureAloFader(ALO* palo);
void FadeAloIn(ALO* palo, float dtFade);
void FadeAloOut(ALO* palo, float dtFade);
void CalculateAloDrive(ALO* palo, CLQ* pclqHoming, LM* plmHoming, float dt, float radPanCur, float* pradPanTarget, float* pradTiltTarget, float* psvTarget);
void CalculateAloMovement(ALO* paloLeaf, ALO* paloBasis, glm::vec3& pos, glm::vec3* pv, glm::vec3* pw, glm::vec3* pdv, glm::vec3* pdw);
void CalculateAloTransformAdjust(ALO* paloLeaf, ALO* paloBasis, glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pv, glm::vec3* pw);
void CalculateAloTransform(ALO* paloLeaf, ALO* paloBasis, int cpaloPredict, glm::vec3* apos, glm::mat3* amat, glm::vec3* av, glm::vec3* aw);
ACTSEG* PactsegNewAlo(ALO* palo);
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
// Loads bone data from binary file
void LoadAloAloxFromBrx(ALO* palo, CBinaryInputStream* pbis);
void SetAloIkhTarget(ALO* palo, ALO* ploTarget);
void BindAloAlox(ALO* palo);
void SnipAloObjects(ALO* palo, int csnip, SNIP* asnip);
void PostAloLoad(ALO *palo);
void PostAloLoadCallback(ALO* palo, MSGID msgid, void* pvData);
// Updates ALO object
void UpdateAlo(ALO* palo, float dt);
void RenderFastShadow(ALO* palo, CM* pcm, RO* pro);
void RenderAloAll(ALO* palo, CM* pcm, RO* pro);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void FreezeAlo(ALO* palo, int fFreeze);
void DupAloRo(ALO* palo, RO* proOrig, RO* proDup);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha);
// Deletes Model from VRAM
void DeleteModel(ALO* palo);
int  GetAloSize();
// Delete ALO object from memory
void DeleteAlo(ALO* palo);

extern std::vector <ALO*> allSWAloObjs;
extern THROB s_mpthrobkthrob[6];
extern CLQ g_clqPosSpringDefault;
extern CLQ g_clqPosDampingDefault;
extern CLQ g_clqRotSpringDefault;
extern CLQ g_clqRotDampingDefault;
extern SMPA g_smpaPosDefault;
extern SMPA g_smpaRotDefault;
extern CLQ s_clqHomingSpeed;
extern LM s_lmHomingSpeed;

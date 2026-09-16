#pragma once
#include "alo.h"
#include "bsp.h"
#include "mark.h"
#include "clip.h"
#include "dzg.h"

enum ZPK
{
	ZPK_Nil = -1,
	ZPK_Blunt = 0,
	ZPK_Electric = 1,
	ZPK_Fire = 2,
	ZPK_Water = 3,
	ZPK_Crush = 4,
	ZPK_Pit = 5,
	ZPK_Max = 6
};
enum CMK 
{
	CMK_Nil = -1,
	CMK_Translucent = 0,
	CMK_Fade = 1,
	CMK_Opaque = 2,
	CMK_Max = 3
};
enum EGK 
{
	EGK_Nil = -1,
	EGK_Inherit = 0,
	EGK_Yes = 1,
	EGK_No = 2,
	EGK_Max = 3
};

enum ZOK 
{
	ZOK_Nil = -1,
	ZOK_Inherit = 0,
	ZOK_Zap = 1,
	ZOK_NoZap = 2,
	ZOK_Ignore = 3,
	ZOK_ZapAndIgnore = 4,
	ZOK_Max = 5
};

enum CNSTR
{
	CNSTR_Nil = -1,
	CNSTR_Free = 0,
	CNSTR_None = 1,
	CNSTR_X = 2,
	CNSTR_Y = 3,
	CNSTR_Z = 4,
	CNSTR_XY = 5,
	CNSTR_XZ = 6,
	CNSTR_YZ = 7,
	CNSTR_Max = 8
};

struct ECNSTR 
{
	glm::vec3 *pnormal;
	CT ct;
};

struct FX
{
	glm::vec3 dv;
	glm::vec3 dw;
};

struct XPD
{
	SO* psoRoot;
	SO* psoLeaf;
	MTLK mtlk;
	glm::vec3 pos;
	glm::vec3 posLeaf;
	glm::vec3 v;
};

enum XPK
{
	XPK_Nil = -1,
	XPK_Illegal = 0,
	XPK_PointToSurface = 1,
	XPK_EdgeToSurface = 2,
	XPK_EdgeToEdge = 3,
	XPK_SphereToPoint = 4,
	XPK_SphereToEdge = 5,
	XPK_SphereToSurface = 6,
	XPK_SphereToSphere = 7,
	XPK_CenterToSurface = 8,
	XPK_CenterToSphere = 9,
	XPK_Internal = 10,
	XPK_Equality = 11,
	XPK_Limited = 12,
	XPK_Max = 13
};

struct XP
{
	XPD axpd[2];
	glm::vec3 normal;
	float gDist;
	float mu;
	float elas;
	XPK xpk;
	union
	{
		void* pv0;
		glm::vec3* ppos0;
		struct EDGE* pedge0;
		void* pvInternal;
		float sfMax;
	};
	union
	{
		void* pv1;
		glm::vec3* ppos1;
		struct EDGE* pedge1;
		struct SURF* psurf1;
	};
	glm::vec3 vRelAdj;
	float sfPivot;
	float sftImpact;
	float svNormal;
	int ipxp;
	struct XP *pxpNext;
};

struct SURFX
{
	int aipos[3];
};

struct LVO
{
	std::vector <glm::vec3> mpiposvLocal;
	std::vector <glm::vec3> mpiposdvLocal;
	std::vector <SURFX>     mpisurfsurfx;
};

struct ZPD
{
	class SO* pso;
	ZPK zpk;
	float dzThrow;
	int fThrowCut;
	int coidThrow;
	OID aoidThrow[32];
	int cploThrow;
	struct LO* aploThrow[32];
};

struct NPG
{
	int cmk;
	int ipglob;
};

struct HSG
{
	int ipglob;
	int ipsubglob;
};

struct CONSTR 
{
	glm::vec3 normal;
	CT ct;
};

// Static Object
class SO : public ALO
{
	public:
	DLE dleRoot;
	DL dlPhys;
	DLE dlePhys;
	glm::mat3 momintLocal;
	glm::mat3 momintInvLocal;
	glm::vec3 dvGravity;
	float gBuoyancy;
	float gViscosity;
	float m;
	glm::vec3 posWorldPrev;
	GEOM geomLocal;
	GEOM geomWorld;
	std::shared_ptr <LVO> plvo;
	float sRadiusSelf;
	float sRadiusAll;
	float sRadiusPrune;
	glm::vec3 posPrune;
	BSPC bspc;
	int cnpg;
	std::vector <NPG> anpg;
	std::vector <uint32_t> mpibspinpg;
	int chsg;
	std::vector <HSG> ahsg;
	std::vector <int> mpisurfihsgMic;
	DLE dleBusySo;
	glm::vec3 posMin;
	glm::vec3 posMax;
	CONSTR constrForce;
	CONSTR constrTorque;
	struct OXA* poxa;
	glm::vec3 dpos;
	glm::vec3 drot;
	struct XA* pxa;
	struct XP* pxpInternal;
	int grfpvaXpValid;
	int ipsoRoot;
	int ipso;
	glm::vec3 posComLocal;
	ZOK zok;
	SO *psoPhysHook;
	GEOM geomCameraLocal;
	GEOM geomCameraWorld;
	BSPC bspcCamera;
	CMK cmk : 4;
	EGK egk : 4;
	unsigned int fSphere : 1;
	unsigned int fClone : 1;
	unsigned int fNoXpsAll : 1;
	unsigned int fNoXpsSelf : 1;
	unsigned int fNoXpsCenter : 1;
	unsigned int fActive : 1;
	unsigned int fVelcro : 1;
	unsigned int fIgnoreLocked : 1;
	unsigned int fIceable : 1;
	unsigned int fRoot : 1;
	unsigned int fPhys : 1;
	unsigned int fNoGravity : 1;
	unsigned int fCenterXp : 1;
	unsigned int fLockedSelf : 1;
	unsigned int fLockedAll : 1;
	unsigned int fLockedAbove : 1;
	unsigned int fCpsoBuildContactGroup : 1;
	unsigned int fCpxpBuildArray : 1;
	unsigned int fUpdateXaList1 : 1;
	unsigned int fUpdateXaList2 : 1;
	unsigned int fRecalcSwXpAll : 1;
	unsigned int fHandleDiveEffect : 1;
	unsigned int fGenSpliceTouchEvents : 1;
	std::shared_ptr <STSO> pstso;
};

SO*  NewSo();
void InitSwBusySoDl(SW* psw);
void InitSwRootDl(SW* psw);
void InitSo(SO *pso);
int  GetSoSize();
// Adds a SO to heiarchy
void OnSoAdd(SO *pso); // NOT FINISHED
// Removes a SO from heiarchy
void OnSoRemove(SO* pso);
void EnableSoPhys(SO* pso, int fPhys);
void RebuildSoPhysHook(SO* pso);
// Clones a SO
void CloneSo(SO* pso, SO* psoBase);
void CloneSoPhys(SO* pso, SO* psoPhys, int cposExtra);
// Sets a parent to a SO 
void SetSoParent(SO* pso, ALO* paloParent);
// Apply transformtion to a proxy object
void ApplySoProxy(SO* pso, PROXY* pproxyApply);
void SetSoConstraints(SO* pso, CT ctForce, glm::vec3* pnormalForce, CT ctTorque, glm::vec3* pnormalTorque);
void EnsureSoLvo(SO* pso);
void ProjectSoLvo(SO* pso, float dt);
void ProjectSoTransform(SO* pso, float dt, int fForce);
void ApplySoConstraintLocal(SO* pso, CONSTR* pconstr, glm::vec3* pvecLocal, glm::vec3* pvecConstr, glm::vec3* pvecRemain);
void ApplyConstr(CONSTR* constr, glm::vec3* vec, glm::vec3* vecConstr, glm::vec3* vecRemain);
void AddSoAcceleration(SO* pso, glm::vec3* dv);
void PresetSoAccel(SO* pso, float dt);
// Updates the SO hierarchy transformations
void UpdateSoXfWorldHierarchy(SO* pso);
void UpdateSoXfWorld(SO* pso);
// Loads SO from binary file
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis);
bool FCalculateMuzzleVelocity(const glm::vec3& posLaunch, const glm::vec3& posTarget, float radVert, glm::vec3& muzzleVelocity, const SO* soLaunch);
bool FCalculateMuzzleVelocitySpeed(const glm::vec3& posLaunch, const glm::vec3& posTarget, float svMuzzle, glm::vec3& vecMuzzle, SO* psoLaunch);
bool FCalculateMuzzleVelocityAngle(const glm::vec3& posLaunch, const glm::vec3& posTarget, float radTilt, glm::vec3& vecMuzzle, SO* psoLaunch);
void TranslateSoToPos(SO* pso, glm::vec3 *ppos);
void TranslateSoToPosSafe(SO* pso, glm::vec3* ppos, glm::vec3* pdpos);
void RotateSoToMat(SO* pso, glm::mat3& pmat);
void ApplySoImpulse(SO* pso, glm::vec3* ppos, glm::vec3* pv, float sftMax);
void UpdateSo(SO *pso, float dt);
void DisplaceSo(SO* pso, int fParentDirty);
void ImpactSo(SO* pso, int fParentDirty);
void PivotSo(SO* pso, int fParentDirty);
void ExtendSoBounds(SO* pso, glm::vec3* ppos, float sRadius);
void SetSoCenterXp(SO* pso, int fCenterXp);
void GetSoCenterXp(SO* pso, int* pfCenterXp);
void RenumberSo(SO* pso, int* pcpso, SO** apso);
int  FIgnoreSoIntersection(SO* pso, SO* psoOther);
void RecalcSoLocked(SO* pso);
void AdjustSoXpLocal(SO* pso, XP* pxp, int ixpd);
void AdjustSoZapXps(SO* psoRoot, XP** ppxpZap);
float UProjectVector(glm::vec3* ppos0, glm::vec3* ppos1, glm::vec3* ppos);
int  FInflictSoZap(SO* pso, XP* pxp, ZPR* pzpr);
void UpdateSoBounds(SO* pso);
void UpdateSoImpacts(SO* pso);
void UpdateSoPosWorldPrev(SO* pso);
SO*  PsoHitTestLineObjects(GRFHTL grfhtl, glm::vec3* ppos0, glm::vec3* ppos1, std::vector <SO*> apso, LSG* plsgBest);
void GetSoCpdefi(SO* pso, float dt, CPDEFI* pcpdefi);
void AddSoWaterAcceleration(SO* pso, WATER* pwater, float dt);
int  FIsSoTouching(SO* pso, SO* psoTarget);
SO*  PsoFirstSoTouching(SO* pso);
void ApplySoConstraintWorld(SO* pso, CONSTR* pconstr, glm::vec3* pvecWorld, glm::vec3* pvecConstr, glm::vec3* pvecRemain);
void AddSoXa(SO* pso, XA* pxaAdd);
void RemoveSoXa(SO* pso, XA* pxaRemove);
void SetSoSphere(SO* pso, float sRadius);
void SetSoNoInteract(SO* pso, int fNoInteract);
void ConstrFromCnstr(CNSTR cnstr, CT* pct, glm::vec3* pnormal);
void SetSoCnstrForce(SO* pso, CNSTR cnstr);
void SetSoCnstrTorque(SO* pso, CNSTR cnstr);
void*GetSoMass(SO* pso);
void*GetSoDvGravity(SO* pso);
void SetSoDvGravity(SO* pso, glm::vec3 *pdvgravity);
void*GetSoGbuoyancy(SO* pso);
void SetSoGbuoyancy(SO* pso, float *pgbuoyancy);
void*GetSoGviscosity(SO* pso);
void SetSoGviscosity(SO* pso, float* pgviscosity);
void*GetSoZok(SO* pso);
void SetSoZok(SO* pso, ZOK *pzok);
void*GetSoSradiusAll(SO* pso);
void SetSoCameraStyle(SO* pso, CMK cmk);
void SetSoIgnoreLocked(SO* pso, int fIgnoreLocked);
void SetSoIceable(SO* pso, int fIceable);
void SetSoMtlk(SO* pso, MTLK mtlk);
void SetSoEdgeGrab(SO* pso, EGK egk);
void*AccessSoMass(SO* pso);
void*AccessSoDvGravity(SO* pso);
void*AccessSoGbuoyancy(SO* pso);
void*AccessSoGviscosity(SO* pso);
void*AccessSoZok(SO* pso);
void*AccessSoSradiusAll(SO* pso);
void SendSoMessage(SO* pso, MSGID msgid, void* pv);
XP*  PxpFindSoGround(SO* psoRoot, SO* psoLeaf, int* pixpd);
void SetSoNoGravity(SO* pso, int fNoGravity);
void SetSoNoXpsAll(SO* pso, int fNoXpsAll);
void SetSoNoXpsCenter(SO* pso, int fNoXpsCenter);
SO*  PsoFindSoPhysHook(SO* psoLeaf, int ib);
int  FGetSoContactList(SO* pso, void* pvstate);
void GetSoContacts(SO* pso, int* pcpso, SO*** papso);
int  FSoInStsoList(STSO* pstsoFirst, SO* pso);
std::shared_ptr <STSO> PstsoAlloc();
void GenerateSoSpliceTouchingEvents(SO* pso);
void FreeStsoList(std::shared_ptr <STSO> pstspFirst);
void CalculateSoTrajectoryApex(SO* pso, glm::vec3* pposTarget, float dzMax, glm::vec3* pv);
void CalculateSoEffect(SO* pso, glm::vec3* ppos, FX* pfx, glm::vec3* pdv);
void PropagateSoForce(SO* psoRoot, GRFSG grfsg, XP* pxp, int ixpd, DZ* pdz, FX* afx);
void DistributeSoEffects(SO* psoRoot, GRFSG grfsg, int cpso, SO** apso, FX* afx);
int  FAbsorbSoWkr(SO* pso, WKR* pwkr);
void AdjustSoMomint(SO* pso, float r);
void SetSoMass(SO* pso, float m);
void SetSoVelocityVec(SO* pso, glm::vec3* pv);
void SetSoAngularVelocityVec(SO* pso, glm::vec3* pw);
void AddSoAngularAcceleration(SO* pso, glm::vec3* pdw);
void CalculateSoAngularEffectWorld(SO* pso, glm::vec3* ptau, glm::vec3* pdw);
void AccelSoTowardPosSpring(SO* pso, glm::vec3* pposTarget, CLQ* pclqPos, glm::vec3* pvTarget, CLQ* pclqV, float dt);
void AccelSoTowardMatSpring(SO* pso, glm::mat3* pmatTarget, CLQ* pclqRot, glm::vec3* pwTarget, CLQ* pclqW, float dt);
void AccelSoTowardMatSmooth(SO* pso, float dt, glm::mat3* pmat, SMP* psmp);
void DiscardSoXps(SO* pso, int discardMode);
void FreezeSo(SO* pso, int fFreeze);
void RenderSoSelf(SO* pso, CM* pcm, RO* pro);
// Delete SO from memory
void DeleteSo(SO *pso);
// Delete all collision data from VRAM
void DeleteSwCollision();
// Deletes a SO collision data from VRAM
void DeleteSoGeom(SO* pso);
void DeallocateSoVector();

extern std::vector <SO*> allSWSoObjs;
extern ECNSTR s_dncnstr[8];

#pragma once
#include "alo.h"
#include "bsp.h"
extern std::vector<SO*> allSWSoObjs;
inline bool g_fRenderCollision;

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

struct ZPD
{
	struct SO* pso;
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

enum CT 
{
	CT_Free = 0,
	CT_Tangent = 1,
	CT_Project = 2,
	CT_Locked = 3
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
	struct LVO* plvo;
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
	int* mpisurfihsgMic;
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
	struct STSO* pstso;
};

SO*  NewSo();
void InitSwBusySoDl(SW* psw);
void InitSwRootDl(SW* psw);
void InitSo(SO *pso); // NOT FINISHED
int  GetSoSize();
// Adds a SO to heiarchy
void OnSoAdd(SO *pso); // NOT FINISHED
// Removes a SO from heiarchy
void OnSoRemove(SO* pso);
// Clones a SO
void CloneSo(SO* pso, SO* psoBase);
// Sets a parent to a SO
void SetSoParent(SO* pso, ALO* paloParent);
// Apply transformtion to a proxy object
void ApplySoProxy(SO* pso, PROXY* pproxyApply);
// Updates the SO hierarchy transformations
void UpdateSoXfWorldHierarchy(SO* pso);
void UpdateSoXfWorld(SO* pso);
// Loads SO from binary file
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis); // NOT FINISHED
void TranslateSoToPos(SO* pso, glm::vec3& ppos);
void RotateSoToMat(SO* pso, glm::mat3& pmat);
void UpdateSo(SO *pso, float dt); // NOT FINISHED
void RenderSoSelf(SO* pso, CM* pcm, RO* pro);
// Draws static object collision
void DrawCollision(CM* pcm, SO* pso);
// Delete SO from memory
void DeleteSo(SO *pso);
// Delete all collision data from VRAM
void DeleteSwCollision();
// Deletes a SO collision data from VRAM
void DeleteSoGeom(SO* pso);
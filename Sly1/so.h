#pragma once
#include "alo.h"
extern std::vector<SO*> allSWSoObjs;

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
	char bspc[12];
	int cnpg;
	std::vector <NPG> anpg;
	std::vector <uint32_t> mpibspinpg;
	int chsg;
	std::vector <HSG> ahsg;
	void* mpisurfihsgMic;
	DLE dleBusySo;
	glm::vec3 posMin;
	glm::vec3 posMax;
	char constrForce[32];
	char constrTorque[32];
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
	char bspcCamera[12];
	struct STSO* pstso;
};

void*NewSo();
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
void SetSoParent(SO* pso, ALO* paloParent);
void ApplySoProxy(SO* pso, PROXY* pproxyApply);
void UpdateSoXfWorldHierarchy(SO* pso);
void UpdateSoXfWorld(SO* pso);
// Loads SO from binary file
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis); // NOT FINISHED
void TranslateSoToPos(SO* pso, glm::vec3& ppos);
void RotateSoToMat(SO* pso, glm::mat3& pmat);
// Stores collsion model in VRAM
void MakeCollisionGLBuffers(GEOM *pgeom);
void UpdateSo(SO *pso, float dt); // NOT FINISHED
void RenderSoSelf(SO* pso, CM* pcm, RO* pro);
// Draws static object collision
void DrawSoCollision(SO* pso);
// Delete SO from memory
void DeleteSo(LO* plo);
// Delete all collision data from VRAM
void DeleteSwCollision();
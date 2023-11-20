#pragma once
#include "alo.h"

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
	void* plvo;
	float sRadiusSelf;
	float sRadiusAll;
	float sRadiusPrune;
	glm::vec3 posPrune;
	char BSPC[12];
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
	void* poxa;
	glm::vec3 dpos;
	glm::vec3 drot;
	void* pxa;
	void* pxpInternal;
	int grfpvaXpValid;
	int ipsoRoot;
	int ipso;
	glm::vec3 posComLocal;
	SO *psoPhysHook;
	GEOM geomCameraLocal;
	GEOM geomCameraWorld;
	char bspcCamera[12];
	void* pstso;
};

void* CreateSo();
void InitSo(SO *pso); // NOT FINISHED
void OnSoAdd(SO *pso); // NOT FINISHED
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis); // NOT FINISHED
void UpdateSo(SO *pso, float dt); // NOT FINISHED
void RenderSoSelf(SO* pso, CM* pcm, RO* pro);
void DeleteSo(LO* plo);
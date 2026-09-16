#pragma once
enum XPK;
#include "solve.h"

struct MTLKWA
{
    float w;
    float a;
};

struct SBI
{
	glm::vec3 pos;
	glm::vec3 normal;
	float gDist;
	XPK xpk;
	union
	{
		void* pv;
		struct SURF* psurf;
		struct EDGE* pedge;
		glm::vec3* ppos;
	};
};

struct VSXP 
{
    int ipos;
    glm::vec3 pos;
    glm::vec3 normal;
    float s;
};

XP* AllocSwXp(SW* psw);
float MuFromAmtlk(MTLK* amtlk);
float ElasFromAmtlk(MTLK* amtlk);
XP* PxpAdd(SO* pso0, SO* pso1, glm::vec3* ppos, glm::vec3* pnormal, float gDist, XPK xpk, void* pv0, void* pv1, XP** ppxpFirst);
void RecalcSwXpPos(SW* psw, int cpxp, XP** apxp);
void RecalcSwXpMovement(SW* psw, int cpxp, XP** apxp);
void FreeSwXpList(SW* psw, XP* pxp);
void FindClosestPointOnLine(const glm::vec3* ppos, const glm::vec3* pposSeg0, const glm::vec3* pposSeg1, float* pu, float* ps);
void FindClosestPointOnLineSegment(const glm::vec3* ppos, const glm::vec3* pposSeg0, const glm::vec3* pposSeg1, float* pu, float* ps);
int  ClsgClipSoAllEdges(SO* pso0, SO* pso1, BSP* pbsp, int clsgMax, LSG* alsg);
int  ClsgClipSoBspEdges(SO* pso0, SO* pso1, int cbspPruned, BSP* abspPruned, int clsgMax, LSG* alsg);
void CategorizeAlsg(int clsg, LSG* alsg, int* pciposEmbed, int* aiposEmbed, LSG** aplsgEmbed, LSG** pplsgEdge);
int  CplsgFindEdgePath(SURF* asurf, int isurf, int isurfTarget, LSG* plsgEdge, LSG** aplsg);
void MarkSoContactsBspBsp(SO* pso0, SO* pso1, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst);
int  CsbiIntersectSphereBsp(glm::vec3* pposCenter, float sRadius, int cbsp, BSP* absp, BSP* pbspPrune, GEOM* pgeom, int csbiMax, SBI* asbi);
void MarkSoContactsSphereBsp(SO* pso0, glm::vec3* pposCenter, float sRadius, SO* pso1, int cbsp, BSP* absp, BSP* pbspPrune, XP** ppxpFirst);
void MarkSoContactsSphereSphere(SO* pso0, SO* pso1, XP** ppxpFirst);
void CalculateCenterContact(SO* pso, glm::vec3* pnormal, float gDot, glm::vec3* ppos, float* ps);
int  FMarkSoContactsCenterBsp(SO* pso0, SO* pso1, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst);
int  FClipEdgeToSphere(glm::vec3* pposSphere, float sRadius, glm::vec3* ppos0, glm::vec3* ppos1, float* pu, glm::vec3* ppos, glm::vec3* pnormal, float* pgDot);
int  FMarkSoContactsCenterSphere(SO* pso0, SO* pso1, XP** ppxpFirst);
void AddSoXps(SO* psoRoot, SO* psoOther, int fCenterXp, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst);
XP*  PxpMarkSwContactsRoot(SW* psw, SO** apso, int fCenterXp);
static int IsurfFromBspSurf(const GEOM& geom, const SURF* psurf);

extern MTLKWA s_mpmtlkwaMu[12];
extern MTLKWA s_mpmtlkwaElas[12];
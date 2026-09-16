#pragma once
class SO;
#include "geom.h"

enum PRUNE 
{
    PRUNE_Neg = 0,
    PRUNE_Pos = 1,
    PRUNE_Mixed = 2
};

enum LSGK
{
    LSGK_Nil = -1,
    LSGK_EdgeToBsp = 0,
    LSGK_EdgeToSphere = 1,
    LSGK_EdgeToCylinder = 2,
    LSGK_EdgeToCbsp = 3,
    LSGK_Merge = 4,
    LSGK_Max = 5
};

struct LSG_Bsp
{
    SURF* apsurf[2];      // 0x50, 0x54
};

struct LSG_Cyl
{
    float au[2];          // 0x50, 0x54
};

union LSG_Data
{
    LSG_Bsp bsp;
    LSG_Cyl cyl;
};

union LSG_Id
{
    int n;
    int inpg;
};

struct LSG
{
    glm::vec3 apos[2];       // 0x00
    glm::vec3 anormal[2];    // 0x20
    float au[2];          // 0x40
    LSGK lsgk;            // 0x48
    LSG_Id id;            // 0x4c
    LSG_Data data;        // 0x50
    EDGE* pedge;          // 0x58
    int iiposSwap;        // 0x5c
    LSG* plsgNext;        // 0x60
    uint8_t padding[0x0c];// 0x64, sizeof(LSG) == 0x70
};

struct LSG_ENDPOINT
{
    float u;
    int iEndpoint;
    glm::vec3 pos;
    glm::vec3 normal;
};

struct FEP
{
    FEP* pfepSource;
    LSG* plsg;
    int isurf;
};

struct BSP 
{
    SURF *psurf;
    BSP  *pbspNeg;
	BSP  *pbspPos;
};

struct CEB
{
    BSP*  pbsp;
    float g;
    BSP*  pbspNeg;
    SURF* psurf;
};

struct BSPC
{
	int cbsp;
	int cbspFull;
	std::vector <BSP> absp;
};

void ReadBspc(GEOM *pgeom, BSPC *pbspc, CBinaryInputStream *pbis);
void CloneBspc(GEOM* pgeomSrc, BSPC* pbspcSrc, GEOM* pgeomDst, BSPC* pbspcDst);
PRUNE PruneBsp(BSP* pbsp, glm::vec3* ppos, float sRadius, BSP** ppbspNext);
int ClsgClipEdgeToBsp(BSP* pbspRoot, glm::vec3* ppos1, glm::vec3* ppos2, int* mpibspn, int clsgMax, LSG* alsg);
int ClsgClipEdgeToSphere(glm::vec3* pposSphere, float sRadius, glm::vec3* ppos0, glm::vec3* ppos1, int clsgMax, LSG* alsg);
int ClsgClipCylinderToObject(SO* pso, glm::vec3* ppos0, glm::vec3* ppos1, float sRadius, int clsgMax, LSG* alsg);
int ClsgClipCylinderToSphere(glm::vec3* pposSphere, float sRadiusSphere, glm::vec3* ppos0, glm::vec3* ppos1, float sRadiusCylinder, int clsgMax, LSG* alsg);
int ClsgClipCylinderToBsp(BSPC* pbspc, GEOM* pgeom, glm::vec3* ppos0, glm::vec3* ppos1, float sRadiusCylinder, int clsgMax, LSG* alsg);
int ClsgClipEdgeToCylinder(glm::vec3* pposCylinder0, glm::vec3* pposCylinder1, float sRadiusCylinder, glm::vec3* pposEdge0, glm::vec3* pposEdge1, int clsgMax, LSG* alsg);
int ClsgMergeAlsg(int clsg, LSG* alsg);
int ClsgClipEdgeToObjects(glm::vec3* ppos0, glm::vec3* ppos1, std::vector<SO*>* papso, int clsgMax, LSG* alsg);
int ClsgClipEdgeToObject(SO* pso, glm::vec3* ppos0, glm::vec3* ppos1, int clsgMax, LSG* alsg);
int ClsgClipLineToSphere(const glm::vec3* pposCenter, float radius, const glm::vec3* ppos0, const glm::vec3* ppos1, int clsgMax, LSG* plsg);
BSP* PbspPointInBspQuick(glm::vec3* ppos, BSP* pbsp);

extern BSP  s_bspAlwaysPos;
extern SURF s_surfAlwaysPos;

extern BSP  s_bspAlwaysNeg;
extern SURF s_surfAlwaysNeg;

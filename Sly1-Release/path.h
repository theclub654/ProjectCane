#pragma once
#include "lo.h"

struct LSG;

enum CBSK 
{
    CBSK_Nil = -1,
    CBSK_Pos = 0,
    CBSK_Neg = 1,
    CBSK_On = 2,
    CBSK_Max = 3
};

struct CBSP 
{
    glm::vec3 normal;
    float gDot;

    union
    {
        struct CBSP* pcbspNeg;
        struct CGT* pcgtNeg;
    };

    union 
    {
        struct CBSP* pcbspPos;
        struct CGT* pcgtPos;
    };
};

struct CGT 
{
    struct CGV* apcgv[3];
};

struct CGE 
{
    struct CGV* apcgv[2];
    struct CGT* apcgt[2];
    float s;
};

struct CGV 
{
    glm::vec3 pos;
    int cpcge;
    std::vector <CGE*> apcge;
};

struct CG 
{
    int ccgv;
    std::vector <CGV> acgv;
    int ccge;
    int ccgeBoundary;
    std::vector <CGE> acge;
    int ccgt;
    std::vector <CGT> acgt;
    int ccbsp;
    std::vector <CBSP> acbsp;
};

class PATHZONE : public LO
{
	public:
	CG cg;
	DLE dlePathzone;
};


PATHZONE* NewPathzone();
CBSP* PcbspExtract(CBSP* pcbspMod);
CGT* PcgtExtract(CGT* pcgtMod);
CGT* PcgtPointInCbspQuick(CBSP* pcbsp, const glm::vec3* ppos);
CGT* PcgtPointInCbspSafe(CBSP* pcbsp, const glm::vec3* ppos);
CBSK CbskFromG(float g);
int  ClsgClipEdgeToCbsp(CBSP* pcbspRoot, const glm::vec3* ppos1, const glm::vec3* ppos2, int clsgMax, LSG* alsg);
bool FClipEdgeToCbsp(CBSP* pcbspRoot, const glm::vec3* ppos1, const glm::vec3* ppos2);
int  IcgvFromPcgv(CG* pcg, CGV* pcgv);
void FindPathAStar(CG* pcg, CGV* pcgvStart, CGV* pcgvEnd, int cpcgvMax, int* pcpcgv, CGV** apcgv);
void SimplifyApcgvNeighbor(CBSP* pcbsp, int* pcpcgv, CGV** apcgv);
int  CposFindPath(CG* pcg, glm::vec3* pposStart, glm::vec3* pposEnd, int cposMax, glm::vec3* apos);
void FindClosestPointInCg(CG* pcg, glm::vec3* ppos, glm::vec3* pposClosest);
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);
void ClonePathzone(PATHZONE* ppathzone, PATHZONE* ppathzoneBase);
void HookupCg(CG* pcg);
int  CposFindPathzonePath(PATHZONE* ppathzone, glm::vec3* pposStart, glm::vec3* pposEnd, int cposMax, glm::vec3* apos);
void FindPathzoneClosestPoint(PATHZONE* ppathzone, glm::vec3* ppos, glm::vec3* pposClosest);
int  ClipPathzoneToBsp(PATHZONE* ppathzone, const glm::vec3* pposStart, const glm::vec3* pposEnd, glm::vec3* pposClip, float* pu);
void ClipPathzoneDirection(float s, PATHZONE* ppathzone, const glm::vec3* pposStart, glm::vec3* pdir);
void ChoosePathzoneRandomPoint(PATHZONE* ppathzone, glm::vec3* ppos);
void OnPathzoneAdd(PATHZONE* ppathzone);
void OnPathzoneRemove(PATHZONE* ppathzone);
int  GetPathzoneSize();
void DeletePathzone(PATHZONE* ppathzone);

extern float s_arad[11];

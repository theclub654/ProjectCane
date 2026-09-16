#pragma once
struct XP;
#include "so.h"

enum DZK
{
	DZK_Normal = 0,
	DZK_Friction = 1,
	DZK_Equality = 2
};

struct DZ 
{
    struct XP* pxp;
    glm::vec3 f;
	std::unique_ptr<FX[]> aafxOwner[2];
	struct FX* aafx[2];
    DZK dzk;
    int fMax;
    float sfMax;
    float sf;
    float sdv;
    float dsdv;
	std::vector <glm::vec3> mpipxpdv;
	std::vector <float> mpidzg;
    DZ* pdzOther;
    DLE dle;
};


struct DZG 
{
	int cdzMax;
	int cdz;
	std::vector <DZ> adz;
	DL dlPos;
	DL dlZero;
	DL dlMax;
	DL dlUncat;
	int cdzPos;
	std::vector <float> aagPos;
	std::vector <float> aagPosCrout;
	std::vector <float> asdv;
	std::vector <float> adsfPos;
};

void InitDzg(DZG* pdzg, int cpxp);
void AppendDzgDz(DZG* pdzg, DZ* pdzOther);
void FillDzgDz(DZG* pdzg, GRFSG grfsg, DZ* pdzForce, int cpxp, XP** apxp, int* acpso, SO*** aapso);
void EnforceDzgDz(DZG* pdzg, DZ* pdzAdd);
void CroutDecomp(int c, float* aag, float* aagCrout);
void CroutSolve(int c, float* aagCrout, float* agConst, float* agSoln);
void ApplyDzg(DZG* pdzg, int cpsoRoot, int* acpso, SO*** aapso, float sdvMax, float sdwMax);
void SolveInequalities(int c, const float* aag, const float* ag, float* agSoln);
void ClearDzgSolution(DZG* pdzg);
void SolveDzgFric(DZG* pdzg, GRFSG grfsg, int cpxp, XP** apxp, int* acpso, SO*** aapso, float dt);
void SolveDzg(DZG* pdzg, GRFSG grfsg, int cpxp, XP** apxp, int* acpso, SO*** aapso);
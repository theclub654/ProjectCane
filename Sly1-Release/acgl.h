#pragma once
#include "acg.h"

struct ACGL : public ACG
{
    int ckgl;
    std::vector <KGL> akgl;
};

void LoadAcglFromBrx(ACGL* pacgl, CBinaryInputStream* pbis);
void EvaluateAcgl(ACGL* pacgl, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg);
void GetAcglTimes(ACGL* pacgl, int* pct, std::vector <float>& pat);

struct VTACGL 
{
    void(*pfnEvaluateAcgl)(ACGL*, ALO*, float, float, GRFEVAL, float*, float*) = EvaluateAcgl;
    void(*pfnLoadAcglFromBrx)(ACGL*, CBinaryInputStream*) = LoadAcglFromBrx;
    void(*pfnGetAcglTimes)(ACGL*, int*, std::vector <float>&) = GetAcglTimes;
};

extern VTACGL g_vtacgl;
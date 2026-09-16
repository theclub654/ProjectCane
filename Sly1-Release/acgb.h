#pragma once
#include "acg.h"

struct ACGB : public ACG
{
    int ckgb;
    std::vector <KGB> akgb;
};

void LoadAcgbFromBrx(ACGB* pacgb, CBinaryInputStream* pbis);
void EvaluateAcgb(ACGB* pacgb, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg);
void GetAcgbTimes(ACGB* pacgb, int* pct, std::vector <float>& pat);

struct VTACGB 
{
    void(*pfnEvaluateAcgb)(ACGB*, ALO*, float, float, GRFEVAL, float*, float*) = EvaluateAcgb;
    void(*pfnLoadAcgbFromBrx)(ACGB*, CBinaryInputStream*) = LoadAcgbFromBrx;
    void(*pfnGetAcgbTimes)(ACGB*, int*, std::vector <float>&) = GetAcgbTimes;
};

extern VTACGB g_vtacgb;
#pragma once
#include "acg.h"

struct ACGBW : public ACG
{
    int ckgbw;
    std::vector <KGBW> akgbw;
};

void LoadAcgbwFromBrx(ACGBW* pacgbw, CBinaryInputStream* pbis);
void EvaluateAcgbw(ACGBW* pacgbw, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg);
void GetAcgbwTimes(ACGBW* pacgbw, int* pct, std::vector <float> &pat);

struct VTACGBW 
{
    void(*pfnEvaluateAcgbw)(ACGBW*, ALO*, float, float, GRFEVAL, float*, float*) = EvaluateAcgbw;
    void(*pfnLoadAcgbwFromBrx)(ACGBW*, CBinaryInputStream*) = LoadAcgbwFromBrx;
    void(*pfnGetAcgbwTimes)(ACGBW*, int*, std::vector <float>&) = GetAcgbwTimes;
};

extern VTACGBW g_vtacgbw;
#pragma once
#include "ac.h"

struct ACG
{
    union
    {
        struct VTACG* pvtacg;
        struct VTACGB* pvtacgb;
        struct VTACGBW* pvtacgbw;
        struct VTACGL* pvtacgl;
        struct VTACGBLT* pvtacgblt;
        struct VTACGBLP* pvtacgblp;
    };
    ACGK acgk;
};

std::shared_ptr <ACG> PacgNew(ACGK acgk);
void EvaluateAcg(ACG* pacg, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg);
void GetAcgTimes(ACG* pacg, int* pct, std::vector <float> &pat);

struct VTACG 
{
    void(*pfnEvaluateAcg)(ACG*, ALO*, float, float, GRFEVAL, float*, float*) = EvaluateAcg;
    void(*pfnLoadAcgFromBrx)(ACG*, CBinaryInputStream*) = nullptr;
    void(*pfnGetAcgTimes)(ACG*, int*, std::vector <float>&) = GetAcgTimes;
};

extern VTACG g_vtacg;
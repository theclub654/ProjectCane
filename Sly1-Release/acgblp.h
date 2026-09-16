#pragma once
#include "acp.h"
#include "acg.h"

struct ACGBLP : public ACG
{
    struct ASEGBL* pasegbl;
    struct MRSGC* pmrsgc;
    std::vector <ACG*> mpiblpacg;
    int ipose;
};

void EvaluateAcgblp(ACGBLP* pacgblp, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg);

struct VTACGBLP 
{
    void(*pfnEvaluateAcgblp)(ACGBLP*, ALO*, float, float, GRFEVAL, float*, float*) = EvaluateAcgblp;
    void(*pfnLoadAcgFromBrx) = nullptr;
    void(*pfnGetAcgTimes)(ACG*, int*, std::vector <float>&) = GetAcgTimes;
};

extern VTACGBLP g_vtacgblp;
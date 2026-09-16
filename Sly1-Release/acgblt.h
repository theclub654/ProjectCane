#pragma once
#include "acp.h"
#include "acg.h"

struct ACGBLT : public ACG
{
    struct ASEGBL* pasegbl;
    struct MRSGC* pmrsgc;
    std::vector <ACG*> mpiblpacg;
};

void EvaluateAcgblt(ACGBLT* pacgblt, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg);

struct VTACGBLT 
{
    void(*pfnEvaluateAcgblt)(ACGBLT*, ALO*, float, float, GRFEVAL, float*, float*) = EvaluateAcgblt;
    void(*pfnLoadAcgFromBrx) = nullptr;
    void(*pfnGetAcgTimes)(ACG*, int*, std::vector <float>&) = GetAcgTimes;
};

extern VTACGBLT g_vtacgblt;
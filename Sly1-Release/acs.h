#pragma once
#include "ac.h"

struct ACS
{
    union
    {
        struct VTACS* pvtacs;
        struct VTACSC* pvtacsc;
        struct VTACSB* pvtacsb;
    };
    ACVK acvk;
};

std::shared_ptr <ACS> PacsNew(ACVK acvk);
void EvaluateAcs(ACS* pacs, ALO* palo, float t, GRFEVAL grfeval, glm::mat3* pmat);
void GetAcsTimes(ACS* pacs, int* pct, std::vector <float> &pat);

struct VTACS
{
    void(*pfnEvaluateAcs)(ACS*, ALO*, float, GRFEVAL, glm::mat3*) = EvaluateAcs;
    void(*pfnLoadAcsFromBrx)(ACS*, CBinaryInputStream*) = nullptr;
    void(*pfnGetAcsTimes)(ACS*, int*, std::vector <float>&) = GetAcsTimes;
};

extern VTACS g_vtacs;
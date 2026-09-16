#pragma once
#include "ac.h"

struct ACP
{
    union
    {
        struct VTACP* pvtacp;
        struct VTACPC* pvtacpc;
        struct VTACPB* pvtacpb;
        struct VTACPBL* pvtacpbl;
    };
    ACVK acvk;
    int fContiguous;
};

std::shared_ptr <ACP> PacpNew(ACVK acvk);
void EvaluateAcp(ACP* pacp, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* ppos, glm::vec3* pv);
void GetAcpTimes(ACP* pacp, int* pct, std::vector <float>& pat);

struct VTACP
{
    void(*pfnEvaluateAcp)(ACP*, ALO*, float, float, GRFEVAL, glm::vec3*, glm::vec3*) = EvaluateAcp;
    void(*pfnLoadAcpFromBrx)(ACP*, CBinaryInputStream*) = nullptr;
    void(*pfnGetAcpTimes)(ACP*, int*, std::vector <float>&) = GetAcpTimes;
};

extern VTACP g_vtacp;
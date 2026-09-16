#pragma once
#include "acp.h"

struct ACPBL : public ACP
{
    struct ASEGBL* pasegbl;
    struct MRSGC* pmrsgc;
    std::vector<ACP*> mpiblpacp;
};

// GOTTA COME BACK TO THIS 
void EvaluateAcpbl(ACPBL* pacpbl, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* ppos, glm::vec3* pv);

struct VTACPBL
{
    void(*pfnEvaluateAcpbl)(ACPBL*, ALO*, float, float, GRFEVAL, glm::vec3*, glm::vec3*) = EvaluateAcpbl;
    void(*pfnLoadAcpFromBrx) = nullptr;
    void(*pfnGetAcpTimes)(ACP*, int*, std::vector <float>&) = GetAcpTimes;
};

extern VTACPBL g_vtacpbl;
#pragma once
#include "acp.h"

struct ACPC : public ACP
{
    glm::vec3 posDefault;
    std::shared_ptr <ACG> apacg[3];
};

void EvaluateAcpc(ACPC* pacpc, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* ppos, glm::vec3* pv);
void LoadAcpcFromBrx(ACPC* pacpc, CBinaryInputStream* pbis);
void GetAcpcTimes(ACPC* pacpc, int* pct, std::vector <float> &pat);

struct VTACPC
{
    void(*pfnEvaluateAcpc)(ACPC*, ALO*, float, float, GRFEVAL, glm::vec3*, glm::vec3*) = EvaluateAcpc;
    void(*pfnLoadAcpcFromBrx)(ACPC*, CBinaryInputStream*) = LoadAcpcFromBrx;
    void(*pfnGetAcpcTimes)(ACPC*, int*, std::vector <float>&) = GetAcpcTimes;
};

extern VTACPC g_vtacpc;
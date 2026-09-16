#pragma once
#include "acp.h"

struct ACPB : public ACP
{
    int ckvb;
    std::vector <KVB> akvb;
};

void LoadAcpbFromBrx(ACPB* pacpb, CBinaryInputStream* pbis);
void EvaluateAcpb(ACPB* pacpb, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* ppos, glm::vec3* pv);
void GetAcpbTimes(ACPB* pacpb, int* pct, std::vector <float> &pat);

struct VTACPB
{
    void(*pfnEvaluateAcpb)(ACPB*, ALO*, float, float, GRFEVAL, glm::vec3*, glm::vec3*) = EvaluateAcpb;
    void(*pfnLoadAcpbFromBrx)(ACPB*, CBinaryInputStream*) = LoadAcpbFromBrx;
    void(*pfnGetAcpbTimes)(ACPB*, int*, std::vector <float>&) = GetAcpbTimes;
};

extern VTACPB g_vtacpb;
#pragma once
#include "acs.h"

struct ACSC : public ACS
{
    glm::vec3 vecDefault;
    std::shared_ptr <ACG> apacg[3];
};

void LoadAcscFromBrx(ACSC* pacsc, CBinaryInputStream* pbis);
void EvaluateAcsc(ACSC* pacsc, ALO* palo, float t, GRFEVAL grfeval, glm::mat3* pmat);
void GetAcscTimes(ACSC* pacsc, int* pct, std::vector <float> &pat);

struct VTACSC 
{
    void(*pfnEvaluateAcsc)(ACSC*, ALO*, float, GRFEVAL, glm::mat3*) = EvaluateAcsc;
    void(*pfnLoadAcscFromBrx)(ACSC*, CBinaryInputStream*) = LoadAcscFromBrx;
    void(*pfnGetAcscTimes)(ACSC*, int*, std::vector <float>&) = GetAcscTimes;
};

extern VTACSC g_vtacsc;
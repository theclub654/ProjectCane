#pragma once
#include "acr.h"

struct ACRC : public ACR
{
    glm::vec3 eulDefault;
    std::shared_ptr<ACG> apacg[3];
};

void LoadAcrcFromBrx(ACRC* pacrc, CBinaryInputStream* pbis);
void EvaluateAcrc(ACRC* pacrc, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::mat3* pmat, glm::vec3* pw);
void EvaluateAcrcEul(ACRC* pacrc, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* peul, glm::vec3* pdeul);
void GetAcrcTimes(ACRC* pacrc, int* pct, std::vector <float> &pat);

struct VTACRC 
{
    void(*pfnEvaluateAcrc)(ACRC*, ALO*, float, float, GRFEVAL, glm::mat3*, glm::vec3*) = EvaluateAcrc;
    void(*pfnEvaluateAcrcEul)(ACRC*, ALO*, float, float, GRFEVAL, glm::vec3*, glm::vec3*) = EvaluateAcrcEul;
    void(*pfnLoadAcrcFromBrx)(ACRC*, CBinaryInputStream*) = LoadAcrcFromBrx;
    void(*pfnGetAcrcTimes)(ACRC* pacrc, int* pct, std::vector <float>&) = GetAcrcTimes;
};

extern VTACRC g_vtacrc;
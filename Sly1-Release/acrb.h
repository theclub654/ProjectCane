#pragma once
#include "acr.h"

struct ACRB : public ACR
{
    int ckvb;
    std::vector <KVB> akvb;
};

void LoadAcrbFromBrx(ACRB* pacrb, CBinaryInputStream* pbis);
void EvaluateAcrb(ACRB* pacrb, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::mat3* pmat, glm::vec3* pw);
void EvaluateAcrbEul(ACRB* pacrb, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* peul, glm::vec3* pdeul);
void GetAcrbTimes(ACRB* pacrb, int* pct, std::vector <float> &pat);

struct VTACRB 
{
    void(*pfnEvaluateAcrb)(ACRB*, ALO*, float, float, GRFEVAL, glm::mat3*, glm::vec3*) = EvaluateAcrb;
    void(*pfnEvaluateAcrbEul)(ACRB*, ALO*, float, float, GRFEVAL, glm::vec3*, glm::vec3*) = EvaluateAcrbEul;
    void(*pfnLoadAcrbFromBrx)(ACRB*, CBinaryInputStream*) = LoadAcrbFromBrx;
    void(*pfnGetAcrbTimes)(ACRB*, int*, std::vector <float>&) = GetAcrbTimes;
};

extern VTACRB g_vtacrb;
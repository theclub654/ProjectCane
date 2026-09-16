#pragma once
#include "acs.h"

struct ACSB : public ACS
{
    int ckvb;
    std::vector <KVB> akvb;
};

void LoadAcsbFromBrx(ACSB* pacsb, CBinaryInputStream* pbis);
void EvaluateAcsb(ACSB* pacsb, ALO* palo, float t, GRFEVAL grfeval, glm::mat3* pmat);
void GetAcsbTimes(ACSB* pacsb, int* pct, std::vector <float> &pat);

struct VTACSB 
{
    void(*pfnEvaluateAcsb)(ACSB*, ALO*, float, GRFEVAL, glm::mat3*) = EvaluateAcsb;
    void(*pfnLoadAcsbFromBrx)(ACSB*, CBinaryInputStream*) = LoadAcsbFromBrx;
    void(*pfnGetAcsbTimes)(ACSB*, int*, std::vector <float>&) = GetAcsbTimes;
};

extern VTACSB g_vtacsb;
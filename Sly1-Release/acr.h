#pragma once
#include "ac.h"

struct ACR
{
    union
    {
        struct VTACR* pvtacr;
        struct VTACRC* pvtacrc;
        struct VTACRB* pvtacrb;
        struct VTACRBL* pvtacrbl;
    };
    ACVK acvk;
    int fContiguous;
};

std::shared_ptr <ACR> PacrNew(ACVK acvk);
void EvaluateAcr(ACR* pacr, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::mat3* pmat, glm::vec3* pw);
void EvaluateAcrEul(ACR* pacr, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* peul, glm::vec3* pdeul);
void GetAcrTimes(ACR* pacr, int* pct, std::vector <float>& pat);

struct VTACR
{
    void(*pfnEvaluateAcr)(ACR*, ALO*, float, float, GRFEVAL, glm::mat3*, glm::vec3*) = EvaluateAcr;
    void(*pfnEvaluateAcrEul)(ACR*, ALO*, float, float, GRFEVAL, glm::vec3*, glm::vec3*) = EvaluateAcrEul;
    void(*pfnLoadAcrFromBrx)(ACR* , CBinaryInputStream*) = nullptr;
    void(*pfnGetAcrTimes)(ACR*, int*, std::vector <float>&) = GetAcrTimes;
};

extern VTACR g_vtacr;
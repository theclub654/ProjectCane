#pragma once
#include "acr.h"

struct ACRBL : public ACR
{
    struct ASEGBL* pasegbl;
    struct MRSGC* pmrsgc;
    std::vector <ACR*> mpiblpacr;
    int fSlerp;
};

//GOTTA COME BACK TO THIS 
void EvaluateAcrbl(ACRBL* pacrbl, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::mat3* pmat, glm::vec3* pw);

struct VTACRBL 
{
    void(*pfnEvaluateAcrbl)(ACRBL*, ALO*, float, float, GRFEVAL, glm::mat3*, glm::vec3*) = EvaluateAcrbl;
    void(*pfnEvaluateAcrEul)(ACR*, ALO*, float, float, GRFEVAL, glm::vec3*, glm::vec3*) = EvaluateAcrEul;
    void(*pfnLoadAcrFromBrx) = nullptr;
    void(*pfnGetAcrTimes)(ACR* pacr, int* pct, std::vector <float>&) = GetAcrTimes;
};

extern VTACRBL g_vtacrbl;
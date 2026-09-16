#include "acsb.h"
#include "akvb.h"

void LoadAcsbFromBrx(ACSB* pacsb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacsb->ckvb, pacsb->akvb, pbis);
}

void EvaluateAcsb(ACSB* pacsb, ALO* palo, float t, GRFEVAL grfeval, glm::mat3* pmat)
{
    glm::vec3 scale;
    glm::vec3* pscale = pmat ? &scale : nullptr;

    EvaluateAkvb(pacsb->ckvb, pacsb->akvb.data(), t, 1.0f, grfeval, pscale, nullptr);

    if (pmat)
        *pmat = glm::mat3(scale.x, 0.0f, 0.0f, 0.0f, scale.y, 0.0f, 0.0f, 0.0f, scale.z);
}

void GetAcsbTimes(ACSB* pacsb, int* pct, std::vector <float> &pat)
{
    GetAkvbTimes(pacsb->ckvb, pacsb->akvb.data(), pct, pat);
}

VTACSB g_vtacsb;
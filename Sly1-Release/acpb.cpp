#include "acpb.h"
#include "akvb.h"

void LoadAcpbFromBrx(ACPB* pacpb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacpb->ckvb, pacpb->akvb, pbis);

    if (pacpb->ckvb < 2)
    {
        pacpb->fContiguous = true;
        return;
    }

    const glm::vec3& posFirst = pacpb->akvb[0].vec;
    const glm::vec3& posLast = pacpb->akvb[pacpb->ckvb - 1].vec;

    float distSq = glm::dot(posFirst - posLast, posFirst - posLast);

    pacpb->fContiguous = distSq < 1.0f;
}

void EvaluateAcpb(ACPB* pacpb, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* ppos, glm::vec3* pv)
{
    EvaluateAkvb(pacpb->ckvb, pacpb->akvb.data(), t, svt, grfeval, ppos, pv);

    if (palo != nullptr && palo->pvtalo != nullptr && palo->pvtalo->pfnAdjustAloPosition != nullptr)
        palo->pvtalo->pfnAdjustAloPosition(palo, ppos, pv);
}

void GetAcpbTimes(ACPB* pacpb, int* pct, std::vector <float> &pat)
{
    GetAkvbTimes(pacpb->ckvb, pacpb->akvb.data(), pct, pat);
}

VTACPB g_vtacpb;

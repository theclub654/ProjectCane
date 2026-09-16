#include "acpc.h"
#include "apacg.h"

void EvaluateAcpc(ACPC* pacpc, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* ppos, glm::vec3* pv)
{
    EvaluateApacg(pacpc->apacg, palo, t, svt, grfeval, &pacpc->posDefault, ppos, pv);

    if (palo && palo->pvtalo && palo->pvtalo->pfnAdjustAloPosition)
        palo->pvtalo->pfnAdjustAloPosition(palo, ppos, pv);
}

void LoadAcpcFromBrx(ACPC* pacpc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacpc->apacg, pacpc->posDefault, pbis);

    int ct = 0;
    std::vector <float> at;

    pacpc->pvtacp->pfnGetAcpTimes(pacpc, &ct, at);

    if (ct == 0)
    {
        pacpc->fContiguous = true;
        return;
    }

    glm::vec3 posFirst;
    glm::vec3 posLast;

    pacpc->pvtacp->pfnEvaluateAcp(pacpc, nullptr, at[0], 1.0f, 0, &posFirst, nullptr);
    pacpc->pvtacp->pfnEvaluateAcp(pacpc, nullptr, at[ct - 1], 1.0f, 0, &posLast, nullptr);

    pacpc->fContiguous = glm::length(posFirst - posLast) < 1.0f;
}

void GetAcpcTimes(ACPC* pacpc, int* pct, std::vector <float> &pat)
{
    GetApacgTimes(pacpc->apacg, pct, pat);
}

VTACPC g_vtacpc;
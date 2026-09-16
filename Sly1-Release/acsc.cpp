#include "acsc.h"
#include "apacg.h"

void LoadAcscFromBrx(ACSC* pacsc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacsc->apacg, pacsc->vecDefault, pbis);
}

void EvaluateAcsc(ACSC* pacsc, ALO* palo, float t, GRFEVAL grfeval, glm::mat3* pmat)
{
    glm::vec3 scale;
    glm::vec3* pscale = pmat ? &scale : nullptr;

    EvaluateApacg(pacsc->apacg, palo, t, 1.0f, grfeval, &g_vecOne, pscale, nullptr);

    if (pmat)
        *pmat = glm::mat3(scale.x, 0.0f, 0.0f, 0.0f, scale.y, 0.0f, 0.0f, 0.0f, scale.z);
}

void GetAcscTimes(ACSC* pacsc, int* pct, std::vector <float> &pat)
{
    GetApacgTimes(pacsc->apacg, pct, pat);
}

VTACSC g_vtacsc;
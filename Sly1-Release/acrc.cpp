#include "acrc.h"
#include "apacg.h"

void LoadAcrcFromBrx(ACRC* pacrc, CBinaryInputStream* pbis)
{
    LoadApacgFromBrx(pacrc->apacg, pacrc->eulDefault, pbis);

    int ct = 0;
    std::vector<float> at;

    pacrc->pvtacr->pfnGetAcrTimes(pacrc, &ct, at);

    if (ct < 1)
    {
        pacrc->fContiguous = true;
        return;
    }

    glm::mat3 matFirst;
    glm::mat3 matLast;
    glm::mat3 dmat;
    glm::vec3 normal;
    float rad = 0.0f;

    pacrc->pvtacr->pfnEvaluateAcr(pacrc, nullptr, at[0], 1.0f, 0, &matFirst, nullptr);
    pacrc->pvtacr->pfnEvaluateAcr(pacrc, nullptr, at[ct - 1], 1.0f, 0, &matLast, nullptr);

    CalculateDmat(&matFirst, &matLast, &dmat);
    DecomposeRotateMatrixRad(&dmat, &rad, &normal);

    pacrc->fContiguous = rad < 0.01f;
}

void EvaluateAcrc(ACRC* pacrc, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::mat3* pmat, glm::vec3* pw)
{
    glm::vec3 eul;
    glm::vec3 deul;

    glm::vec3* peul = pmat ? &eul : nullptr;
    glm::vec3* pdeul = pw ? &deul : nullptr;

    EvaluateApacg(pacrc->apacg, palo, t, svt, grfeval, &pacrc->eulDefault, peul, pdeul);

    if (pmat)
        LoadRotateMatrixEuler(&eul, pmat);

    if (pw)
        ConvertDeulToW(&eul, &deul, pw);

    if (palo && palo->pvtalo && palo->pvtalo->pfnAdjustAloRotation)
        palo->pvtalo->pfnAdjustAloRotation(palo, pmat, pw);
}

void EvaluateAcrcEul(ACRC* pacrc, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* peul, glm::vec3* pdeul)
{
    EvaluateApacg(pacrc->apacg, palo, t, svt, grfeval, &pacrc->eulDefault, peul, pdeul);
}

void GetAcrcTimes(ACRC* pacrc, int* pct, std::vector <float> &pat)
{
    GetApacgTimes(pacrc->apacg, pct, pat);
}

VTACRC g_vtacrc;
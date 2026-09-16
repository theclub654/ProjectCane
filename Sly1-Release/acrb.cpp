#include "acrb.h"
#include "akvb.h"

void LoadAcrbFromBrx(ACRB* pacrb, CBinaryInputStream* pbis)
{
    LoadAkvbFromBrx(&pacrb->ckvb, pacrb->akvb, pbis);

    if (pacrb->ckvb < 2)
    {
        pacrb->fContiguous = true;
        return;
    }

    glm::mat3 mat0;
    glm::mat3 mat1;
    glm::mat3 dmat;
    glm::vec3 normal;
    float rad;

    LoadRotateMatrixEuler(&pacrb->akvb[0].vec, &mat0);
    LoadRotateMatrixEuler(&pacrb->akvb[pacrb->ckvb - 1].vec, &mat1);

    CalculateDmat(&mat0, &mat1, &dmat);
    DecomposeRotateMatrixRad(&dmat, &rad, &normal);

    pacrb->fContiguous = (rad < 0.01f);
}

void EvaluateAcrb(ACRB* pacrb, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::mat3* pmat, glm::vec3* pw)
{
    glm::vec3 eul;
    glm::vec3 deul;

    glm::vec3* peul = pmat ? &eul : nullptr;
    glm::vec3* pdeul = pw ? &deul : nullptr;

    EvaluateAkvb(pacrb->ckvb, pacrb->akvb.data(), t, svt, grfeval, peul, pdeul);

    if (pmat)
        LoadRotateMatrixEuler(&eul, pmat);

    if (pw)
        ConvertDeulToW(&eul, &deul, pw);

    if (palo && palo->pvtalo && palo->pvtalo->pfnAdjustAloRotation)
        palo->pvtalo->pfnAdjustAloRotation(palo, pmat, pw);
}

void EvaluateAcrbEul(ACRB* pacrb, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* peul, glm::vec3* pdeul)
{
    EvaluateAkvb(pacrb->ckvb, pacrb->akvb.data(), t, svt, grfeval, peul, pdeul);
}

void GetAcrbTimes(ACRB* pacrb, int* pct, std::vector <float> &pat)
{
    GetAkvbTimes(pacrb->ckvb, pacrb->akvb.data(), pct, pat);
}

VTACRB g_vtacrb;
#include "acr.h"
#include "acrb.h"
#include "acrc.h"
#include "acrbl.h"

std::shared_ptr<ACR> PacrNew(ACVK acvk)
{
    std::shared_ptr<ACR> pacr;

    switch (acvk)
    {
        case ACVK_Bezier:
        {
            auto pacrb = std::make_shared<ACRB>();
            pacrb->pvtacrb = &g_vtacrb;
            pacr = pacrb;
            break;
        }

        case ACVK_Component:
        {
            auto pacrc = std::make_shared<ACRC>();
            pacrc->pvtacrc = &g_vtacrc;
            pacr = pacrc;
            break;
        }

        case ACVK_Blend:
        {
            auto pacrbl = std::make_shared<ACRBL>();
            pacrbl->pvtacrbl = &g_vtacrbl;
            pacr = pacrbl;
            break;
        }

        default:
        return nullptr;
    }

    pacr->acvk = acvk;
    return pacr;
}
void EvaluateAcr(ACR* pacr, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::mat3* pmat, glm::vec3* pw)
{

}

void EvaluateAcrEul(ACR* pacr, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* peul, glm::vec3* pdeul)
{

}

void GetAcrTimes(ACR* pacr, int* pct, std::vector <float>& pat)
{
    *pct = 0;
    pat.clear();
}

VTACR g_vtacr;
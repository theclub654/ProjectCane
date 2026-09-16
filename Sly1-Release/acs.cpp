#include "acs.h"
#include "acsc.h"
#include "acsb.h"

std::shared_ptr <ACS> PacsNew(ACVK acvk)
{
    std::shared_ptr <ACS> pacs{};

    if (acvk == ACVK_Component)
    {
        ACSC acsc{};
        acsc.pvtacsc = &g_vtacsc;
        acsc.acvk = acvk;
        pacs = std::make_shared <ACSC>(acsc);
    }
    else
    {
        if (acvk == ACVK_Bezier)
        {
            ACSB acsb{};
            acsb.pvtacsb = &g_vtacsb;
            acsb.acvk = acvk;
            pacs = std::make_shared <ACSB>(acsb);
        }
    }

    if (pacs != nullptr)
        pacs->acvk = acvk;

    return pacs;
}

void EvaluateAcs(ACS* pacs, ALO* palo, float t, GRFEVAL grfeval, glm::mat3* pmat)
{

}

void GetAcsTimes(ACS* pacs, int* pct, std::vector <float> &pat)
{
    *pct = 0;
    pat.clear();
}

VTACS g_vtacs;